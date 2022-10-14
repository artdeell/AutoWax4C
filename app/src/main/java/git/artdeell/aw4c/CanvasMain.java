package git.artdeell.aw4c;

import git.artdeell.autowax.AutoWax;
import git.artdeell.autowax.spiritshop.SpiritShop;

public class CanvasMain {
    private static final AutoWax aw = new AutoWax();
    public static void reauthorized() {
        String[] creds = getCredentials();
        aw.resetSession(creds[0], creds[1]);
    }
    public static void candleRun(boolean doQuests, boolean doCandles, boolean doSend, boolean doReceive) {
        new Thread(()->{
            reauthorized();
            if(doCandles) aw.doCandleRun();
            if(doQuests) aw.doQuests();
            if(doSend) aw.runGift();
            if(doReceive) aw.collectGifts();
            CanvasMain.unlockUI();
        }).start();
    }
    public static void spiritShop(byte op, long arg) {
        switch (op) {
            case 0:
                reauthorized();
                SpiritShop.get(aw).init();
                CanvasMain.unlockUI();
                break;
            case 1:
                SpiritShop.get(aw).pushLevel(arg);
                break;
            case 2:
                SpiritShop.get(aw).popLevel();
                break;
            case 3:
                SpiritShop.get(aw).purchase(arg);
        }
    }
    public static void collectLights(String lights, boolean world, boolean spirit) {
        new Thread(()->{
            reauthorized();
            aw.collectLights(lights, world, spirit);
            unlockWLCollector();
        }).start();
    }
    public static void dropLights(int count) {
        new Thread(()->{
            reauthorized();
            aw.runDrop(count);
            unlockWLCollector();
        }).start();
    }
    public static void init(int version, boolean isBeta) {
        AutoWax.initWithParameters(version, isBeta);
    }
    private static native String[] getCredentials();
    public static native void goReauthorize();
    public static native void submitLogString(String s);
    public static native void submitProgressBar(int cur, int max);
    public static native void unlockUI();
    public static native void unlockWLCollector();
}
