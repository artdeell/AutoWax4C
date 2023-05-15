package git.artdeell.aw4c;

import android.util.Log;

import java.io.IOException;

import git.artdeell.autowax.AutoWax;
import git.artdeell.autowax.heartlist.HeartList;
import git.artdeell.autowax.iap.IapPurchase;
import git.artdeell.autowax.invitemanager.InviteManager;
import git.artdeell.autowax.spiritshop.SpiritShop;
import git.artdeell.autowax.worldquest.Spirits;

@Keep
public class CanvasMain {
    private static final AutoWax aw = new AutoWax();
    @Keep
    public static void reauthorized() {
        String[] creds = getCredentials();
        if(creds == null) {

        }
        aw.resetSession(creds[0], creds[1]);
    }
    @Keep
    public static void candleRun(boolean doQuests, boolean doCandles, boolean doSend, boolean doReceive, boolean fragments) {
        new Thread(()->{
            reauthorized();
            if(fragments) aw.doRaces(false);
            if(doCandles) aw.doCandleRun();
            if(doQuests) aw.doQuests();
            if(doSend) aw.runGift();
            if(doReceive) aw.collectGifts();
            CanvasMain.unlockUI();
        }).start();
    }
    @Keep
    public static void spiritShop(byte op, long arg, float arg2) {
        switch (op) {
            case 0:
                reauthorized();
                SpiritShop.get(aw).init();
                CanvasMain.unlockUI();
                break;
            case 1:
                SpiritShop.get(aw).pushLevel(arg, arg2);
                break;
            case 2:
                SpiritShop.get(aw).popLevel();
                break;
            case 3:
                SpiritShop.get(aw).purchase(arg);
        }
    }
    @Keep
    public static void collectLights(String lights, boolean world, boolean spirit) {
        new Thread(()->{
            reauthorized();
            aw.collectLights(lights, world, spirit);
            unlockWLCollector();
        }).start();
    }
    @Keep
    public static void dropLights(int count) {
        new Thread(()->{
            reauthorized();
            aw.runDrop(count);
            unlockWLCollector();
        }).start();
    }
    @Keep
    public static void edemRun() {
        new Thread(()->{
            reauthorized();
            aw.edemRun();
            unlockEdem();
        }).start();
    }
    @Keep
    public static void inviteManager(byte op, int idx) {
        reauthorized();
        InviteManager mgr = InviteManager.get(aw);
        switch(op) {
            case 0:
                mgr.copyInvite(idx);
                break;
            case 1:
                mgr.deleteInvite(idx);
                break;
            case 2:
                mgr.reload();
                break;
        }
    }
    @Keep
    public static void createInvite(String nick) {
        reauthorized();
        InviteManager.get(aw).createInvite(nick);
    }
    @Keep
    public static void init(int version, boolean isBeta) {
        AutoWax.initWithParameters(version, isBeta);
    }
    @Keep
    public static void worldQuests(byte op, int idx) {
        reauthorized();
        switch(op) {
            case 0:
                Spirits.get(aw).load();
                break;
            case 1:
                Spirits.get(aw).executeQuest(idx);
                break;
        }
    }
    @Keep
    public static void heartSelector(byte op, int idx) {
        reauthorized();
        switch(op) {
            case 0:
                HeartList.get(aw).reload();
                break;
            case 1:
                HeartList.get(aw).add(idx);
                break;
            case 2:
                HeartList.get(aw).rem(idx);
                break;
        }
    }

    @Keep
    public static String[] getLevelNames() {
        try {
            if (ContextOps.hasAssets())
                return ContextOps.getAssetManager().list("Data/Levels");
        }catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    @Keep
    public static void iapReloadList(String platform) {
        reauthorized();
        IapPurchase.get(aw).reload(platform);
    }

    @Keep
    public static void iapBuy(String platform, String id) {
        IapPurchase.get(aw).buy(platform, id);
    }

    @Keep
    public static void authorizeKey(String key) {
        Log.i("CanvasMain", "Key entered: "+key);
        sendKeyData(new byte[0]);
    }

    @Keep private static native String[] getCredentials();
    @Keep public static native void goReauthorize();
    @Keep public static native void submitLogString(String s);
    @Keep public static native void submitProgressBar(int cur, int max);
    @Keep public static native void unlockUI();
    @Keep public static native void unlockWLCollector();
    @Keep public static native void unlockEdem();
    @Keep public static native String getUserId();
    @Keep public static native void sendKeyData(byte[] data);
}
