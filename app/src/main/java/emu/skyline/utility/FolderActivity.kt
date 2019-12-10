package emu.skyline.utility

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.PreferenceManager

class FolderActivity : AppCompatActivity() {
    override fun onCreate(state: Bundle?) {
        super.onCreate(state)
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT_TREE)
        this.startActivityForResult(intent, 1)
    }

    public override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode == Activity.RESULT_OK) {
            if (requestCode == 1) {
                val uri = data!!.data!!
                contentResolver.takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION)
                PreferenceManager.getDefaultSharedPreferences(this).edit()
                        .putString("search_location", uri.toString())
                        .putBoolean("refresh_required", true)
                        .apply()
                finish()
            }
        } else
            finish()
    }
}
