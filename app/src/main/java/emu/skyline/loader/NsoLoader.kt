package emu.skyline.loader

import android.content.Context
import android.net.Uri
import emu.skyline.R
import emu.skyline.utility.RandomAccessDocument
import java.io.IOException

internal class NsoLoader(context: Context) : BaseLoader(context, TitleFormat.NSO) {
    override fun getTitleEntry(file: RandomAccessDocument, uri: Uri): TitleEntry {
        return TitleEntry(context, "", romType, true, uri)
    }

    override fun verifyFile(file: RandomAccessDocument): Boolean {
        try {
            file.seek(0x0) // Skip to NsoHeader.magic
            val buffer = ByteArray(4)
            file.read(buffer)
            if (String(buffer) != "NSO0") return false
        } catch (e: IOException) {
            return false
        }
        return true
    }
}