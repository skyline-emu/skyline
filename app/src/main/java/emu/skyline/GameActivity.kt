package emu.skyline

import android.app.Activity
import android.net.Uri
import android.os.Bundle
import android.os.ParcelFileDescriptor
import android.util.Log
import android.view.*
import androidx.appcompat.app.AppCompatActivity
import emu.skyline.loader.getTitleFormat
import emu.skyline.utility.NpadButton
import emu.skyline.utility.ButtonState
import kotlinx.android.synthetic.main.game_activity.*
import java.io.File
import android.view.InputDevice.getDevice
import androidx.core.math.MathUtils
import emu.skyline.utility.NpadAxisId
import java.security.Key
import kotlin.math.abs


class GameActivity : AppCompatActivity(), SurfaceHolder.Callback {
    init {
        System.loadLibrary("skyline") // libskyline.so
    }

    private lateinit var rom: Uri
    private lateinit var romFd: ParcelFileDescriptor
    private lateinit var preferenceFd: ParcelFileDescriptor
    private lateinit var logFd: ParcelFileDescriptor
    private var surface: Surface? = null
    private var inputQueue: Long = 0L
    private var controllerHatY: Float = 0.0f
    private var controllerHatX: Float = 0.0f
    private lateinit var gameThread: Thread

    private external fun executeRom(romString: String, romType: Int, romFd: Int, preferenceFd: Int, logFd: Int)
    private external fun setHalt(halt: Boolean)
    private external fun setSurface(surface: Surface?)
    private external fun setButtonState(id: Long, state: Int)
    private external fun setAxisValue(id: Int, value: Int)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.game_activity)
        rom = intent.data!!
        val romType = getTitleFormat(rom, contentResolver).ordinal
        romFd = contentResolver.openFileDescriptor(rom, "r")!!
        val preference = File("${applicationInfo.dataDir}/shared_prefs/${applicationInfo.packageName}_preferences.xml")
        preferenceFd = ParcelFileDescriptor.open(preference, ParcelFileDescriptor.MODE_READ_WRITE)
        val log = File("${applicationInfo.dataDir}/skyline.log")
        logFd = ParcelFileDescriptor.open(log, ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE)
        window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN)
        game_view.holder.addCallback(this)
        //window.takeInputQueue(this)
        gameThread = Thread {
            while ((surface == null))
                Thread.yield()
            executeRom(Uri.decode(rom.toString()), romType, romFd.fd, preferenceFd.fd, logFd.fd)
            runOnUiThread { finish() }
        }
        gameThread.start()
    }

    override fun onDestroy() {
        super.onDestroy()
        setHalt(true)
        gameThread.join()
        romFd.close()
        preferenceFd.close()
        logFd.close()
    }

    override fun surfaceCreated(holder: SurfaceHolder?) {
        Log.d("surfaceCreated", "Holder: ${holder.toString()}")
        surface = holder!!.surface
        setSurface(surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
        Log.d("surfaceChanged", "Holder: ${holder.toString()}, Format: $format, Width: $width, Height: $height")
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
        Log.d("surfaceDestroyed", "Holder: ${holder.toString()}")
        surface = null
        setSurface(surface)
    }

    override fun dispatchKeyEvent(event: KeyEvent): Boolean {
        val action: ButtonState = when (event.action) {
            KeyEvent.ACTION_DOWN -> ButtonState.Pressed
            KeyEvent.ACTION_UP -> ButtonState.Released
            else -> return false
        }

        var buttonMap: Map<Int, NpadButton> = mapOf(
                KeyEvent.KEYCODE_BUTTON_A to NpadButton.A,
                KeyEvent.KEYCODE_BUTTON_B to NpadButton.B,
                KeyEvent.KEYCODE_BUTTON_X to NpadButton.X,
                KeyEvent.KEYCODE_BUTTON_Y to NpadButton.Y,
                KeyEvent.KEYCODE_BUTTON_THUMBL to NpadButton.L3,
                KeyEvent.KEYCODE_BUTTON_THUMBR to NpadButton.R3,
                KeyEvent.KEYCODE_BUTTON_L1 to NpadButton.L,
                KeyEvent.KEYCODE_BUTTON_R1 to NpadButton.R,
                KeyEvent.KEYCODE_BUTTON_L2 to NpadButton.ZL,
                KeyEvent.KEYCODE_BUTTON_R2 to NpadButton.ZR,
                KeyEvent.KEYCODE_BUTTON_START to NpadButton.Plus,
                KeyEvent.KEYCODE_BUTTON_SELECT to NpadButton.Minus,
                KeyEvent.KEYCODE_DPAD_DOWN to NpadButton.DpadDown,
                KeyEvent.KEYCODE_DPAD_UP to NpadButton.DpadUp,
                KeyEvent.KEYCODE_DPAD_LEFT to NpadButton.DpadLeft,
                KeyEvent.KEYCODE_DPAD_RIGHT to NpadButton.DpadRight)

        if (buttonMap.containsKey(event.keyCode)) {
            setButtonState(buttonMap.getValue(event.keyCode).id, action.ordinal);
            return true
        }

        val input: InputDevice = event.device
        return super.dispatchKeyEvent(event)
    }

    override fun dispatchGenericMotionEvent(event: MotionEvent): Boolean {
        if ((event.source and InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD ||
                (event.source and InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)  {
            var hatXMap: Map<Float, NpadButton> = mapOf(
                    -1.0f to NpadButton.DpadLeft,
                    +1.0f to NpadButton.DpadRight)

            var hatYMap: Map<Float, NpadButton> = mapOf(
                    -1.0f to NpadButton.DpadUp,
                    +1.0f to NpadButton.DpadDown)

            if (controllerHatX != event.getAxisValue(MotionEvent.AXIS_HAT_X)) {
                if (event.getAxisValue(MotionEvent.AXIS_HAT_X) == 0.0f)
                    setButtonState(hatXMap.getValue(controllerHatX).id, ButtonState.Released.ordinal)
                else
                    setButtonState(hatXMap.getValue(event.getAxisValue(MotionEvent.AXIS_HAT_X)).id, ButtonState.Pressed.ordinal)

                controllerHatX = event.getAxisValue(MotionEvent.AXIS_HAT_X)

                return true
            }

            if (controllerHatY != event.getAxisValue(MotionEvent.AXIS_HAT_Y)) {
                if (event.getAxisValue(MotionEvent.AXIS_HAT_Y) == 0.0f)
                    setButtonState(hatYMap.getValue(controllerHatY).id, ButtonState.Released.ordinal)
                else
                    setButtonState(hatYMap.getValue(event.getAxisValue(MotionEvent.AXIS_HAT_Y)).id, ButtonState.Pressed.ordinal)

                controllerHatY = event.getAxisValue(MotionEvent.AXIS_HAT_Y)
                
                return true
            }
        }

        if ((event.source and InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK && event.action == MotionEvent.ACTION_MOVE) {
            var axisMap: Map<Int, NpadAxisId> = mapOf(
                    MotionEvent.AXIS_X to NpadAxisId.LX,
                    MotionEvent.AXIS_Y to NpadAxisId.LY,
                    MotionEvent.AXIS_Z to NpadAxisId.RX,
                    MotionEvent.AXIS_RZ to NpadAxisId.RY)

            //TODO: Digital inputs based off of analog sticks
            event.device.motionRanges.forEach {
                if (axisMap.containsKey(it.axis)) {
                    var axisValue: Float = event.getAxisValue(it.axis)
                    if (abs(axisValue) <= it.flat)
                        axisValue = 0.0f

                    val ratio : Float = axisValue / (it.max - it.min)
                    val rangedAxisValue: Int = (ratio * (Short.MAX_VALUE - Short.MIN_VALUE)).toInt()

                    setAxisValue(axisMap.getValue(it.axis).ordinal, rangedAxisValue)
                }
            }
            return true
        }

        return super.dispatchGenericMotionEvent(event)
    }
}