package emu.skyline.loader

import android.content.Context
import android.graphics.BitmapFactory
import android.net.Uri
import emu.skyline.R
import emu.skyline.utility.RandomAccessDocument
import java.io.IOException

internal class NroLoader(context: Context) : BaseLoader(context, TitleFormat.NRO) {
    override fun getTitleEntry(file: RandomAccessDocument, uri: Uri): TitleEntry {
        return try {
            file.seek(0x18) // Skip to NroHeader.size
            val asetOffset = Integer.reverseBytes(file.readInt())
            file.seek(asetOffset.toLong()) // Skip to the offset specified by NroHeader.size
            val buffer = ByteArray(4)
            file.read(buffer)
            if (String(buffer) != "ASET") throw IOException()
            file.skipBytes(0x4)
            val iconOffset = java.lang.Long.reverseBytes(file.readLong())
            val iconSize = Integer.reverseBytes(file.readInt())
            if (iconOffset == 0L || iconSize == 0) throw IOException()
            file.seek(asetOffset + iconOffset)
            val iconData = ByteArray(iconSize)
            file.read(iconData)
            val icon = BitmapFactory.decodeByteArray(iconData, 0, iconSize)
            file.seek(asetOffset + 0x18.toLong())
            val nacpOffset = java.lang.Long.reverseBytes(file.readLong())
            val nacpSize = java.lang.Long.reverseBytes(file.readLong())
            if (nacpOffset == 0L || nacpSize == 0L) throw IOException()
            file.seek(asetOffset + nacpOffset)
            val name = ByteArray(0x200)
            file.read(name)
            val author = ByteArray(0x100)
            file.read(author)
            TitleEntry(String(name).substringBefore((0.toChar())), String(author).substringBefore((0.toChar())), romType, true, uri, icon)
        } catch (e: IOException) {
            TitleEntry(context, context.getString(R.string.aset_missing), romType, false, uri)
        }
    }

    override fun verifyFile(file: RandomAccessDocument): Boolean {
        try {
            file.seek(0x10) // Skip to NroHeader.magic
            val buffer = ByteArray(4)
            file.read(buffer)
            if (String(buffer) != "NRO0") return false
        } catch (e: IOException) {
            return false
        }
        return true
    }
}
