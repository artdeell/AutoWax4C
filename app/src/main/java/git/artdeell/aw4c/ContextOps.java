package git.artdeell.aw4c;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.util.Log;

import java.lang.reflect.Field;
import java.util.Arrays;

@Keep
public class ContextOps {
    static private ClipboardManager clipboard = null;
    private static Resources skyResources = null;
    private static AssetManager assetManager = null;
    public static SharedPreferences sharedPreferences;
    @SuppressLint({"PrivateApi", "DiscouragedPrivateApi"})
    @Keep
    public static boolean init() {
        Log.i("ContextOps", "Initiailizing...");
        try {
            Class<?> activityThread = Class.forName("android.app.ActivityThread");
            Context context = (Context) activityThread.getDeclaredMethod("getApplication").invoke(activityThread.getDeclaredMethod("currentActivityThread").invoke(null));
            try {
                skyResources = (Resources) context.getClass().getDeclaredField("skyRes").get(context);
            }catch (Exception e) {
                e.printStackTrace();
            }
            if(skyResources != null) assetManager = skyResources.getAssets();
            if(context == null) return false;
            clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
            sharedPreferences = context.getSharedPreferences("aw4c_prefs", Context.MODE_PRIVATE);
            return true;
        }catch (Exception e) {
            Log.i("ContextOps", "Failed to initialize", e);
            e.printStackTrace();
        }
        return false;
    }
    @Keep
    public static String getClipboard() {
        if(clipboard.hasPrimaryClip()) {
            CharSequence text = clipboard.getPrimaryClip().getItemAt(0).getText();
            if(text != null) return text.toString();
        }
        return null;
    }
    @Keep
    public static void setClipboard(String clipboard_string) {
        clipboard.setPrimaryClip(ClipData.newPlainText("Canvas Copy", clipboard_string));
    }

    @Keep
    public static boolean hasAssets() {
        return skyResources != null && assetManager != null;
    }
    
    @Keep
    public static AssetManager getAssetManager() {

        return assetManager;
    }
}
