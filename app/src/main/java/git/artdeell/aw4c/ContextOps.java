package git.artdeell.aw4c;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.util.Log;

public class ContextOps {
    static private ClipboardManager  clipboard;
    @SuppressLint({"PrivateApi", "DiscouragedPrivateApi"})
    public static boolean init() {
        Log.i("ContextOps", "Initiailizing...");
        try {
            Class<?> activityThread = Class.forName("android.app.ActivityThread");
            Context context = (Context) activityThread.getDeclaredMethod("getApplication").invoke(activityThread.getDeclaredMethod("currentActivityThread").invoke(null));
            if(context == null) return false;
            clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
            return true;
        }catch (Exception e) {
            Log.i("ContextOps", "Failed to initialize", e);
            e.printStackTrace();
        }
        return false;
    }
    public static String getClipboard() {
        if(clipboard.hasPrimaryClip()) {
            CharSequence text = clipboard.getPrimaryClip().getItemAt(0).getText();
            if(text != null) return text.toString();
        }
        return null;
    }
    public static void setClipboard(String clipboard_string) {
        clipboard.setPrimaryClip(ClipData.newPlainText("Canvas Copy", clipboard_string));
    }
}
