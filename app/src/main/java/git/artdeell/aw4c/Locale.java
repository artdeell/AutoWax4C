package git.artdeell.aw4c;

public class Locale {
    private static final String[] STRINGS = new String[11];
    public static final int Q_STAT_SET = 0;
    public static final int Q_CURRENCY = 1;
    public static final int Q_NO_CURRENCY = 2;
    public static final int Q_DENIED = 3;
    public static final int Q_DATA_REFRESHED= 4;
    public static final int Q_ACTIVATED= 5;
    public static final int G_EXCEPTION = 6;
    public static final int C_RUNNING = 7;
    public static final int C_CONVERSION_DONE_C = 8;
    public static final int C_CONVERSION_FAILED = 9;
    public static final int C_CONVERSION_DONE_S = 10;
    static {
        STRINGS[Q_STAT_SET] = "Quest stat set: %s";
        STRINGS[Q_CURRENCY] = "%s (%d seasonals, %d candles)";
        STRINGS[Q_NO_CURRENCY] = "No currency response";
        STRINGS[Q_DENIED] = "Quest denied";
        STRINGS[Q_DATA_REFRESHED] = "Quest data refreshed: %s";
        STRINGS[Q_ACTIVATED] = "Quests activated!";
        STRINGS[G_EXCEPTION]= "An error has occured\n%s";
        STRINGS[C_RUNNING] = "Running for candles...";
        STRINGS[C_CONVERSION_DONE_C] = "Candles: %d";
        STRINGS[C_CONVERSION_FAILED] = "Failed to convert candles: %s";
        STRINGS[C_CONVERSION_DONE_S] = "Seasonal candles^ %d";
    }
    public static String get(int i) {
        return STRINGS[i];
    }
    public static String get(int i, Object... va) {
        return String.format(STRINGS[i], va);
    }
}
