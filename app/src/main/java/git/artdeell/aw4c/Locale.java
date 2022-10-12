package git.artdeell.aw4c;

public class Locale {
    private static final String[] STRINGS = new String[10];
    public static final int Q_STAT_SET = 0;
    public static final int Q_CURRENCY = 1;
    public static final int Q_NO_CURRENCY = 2;
    public static final int Q_DENIED = 3;
    public static final int Q_DATA_REFRESHED= 4;
    public static final int Q_ACTIVATED= 5;
    public static final int G_EXCEPTION = 6;
    static {
        STRINGS[Q_STAT_SET] = "Quest stat set: %s";
        STRINGS[Q_CURRENCY] = "%s (%d seasonals, %d candles)";
        STRINGS[Q_NO_CURRENCY] = "No currency response";
        STRINGS[Q_DENIED] = "Quest denied";
        STRINGS[Q_DATA_REFRESHED] = "Quest data refreshed: %s";
        STRINGS[Q_ACTIVATED] = "Quests activated!";
        STRINGS[G_EXCEPTION]= "An error has occured\n%s";
    }
    public static String get(int i) {
        return STRINGS[i];
    }
    public static String get(int i, Object... va) {
        return String.format(STRINGS[i], va);
    }
}
