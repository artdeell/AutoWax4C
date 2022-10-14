package git.artdeell.aw4c;

import git.artdeell.autowax.AutoWax;

public class CanvasMain {
    private static AutoWax aw = new AutoWax();
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
    public static void init(int version, boolean isBeta) {
        AutoWax.initWithParameters(version, isBeta);
    }
    private static native String[] getCredentials();
    public static native void goReauthorize();
    public static native void submitLogString(String s);
    public static native void submitProgressBar(int cur, int max);
    public static native void unlockUI();
}
