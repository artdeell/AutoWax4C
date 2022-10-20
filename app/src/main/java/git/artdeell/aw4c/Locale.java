package git.artdeell.aw4c;

import java.util.HashMap;
import java.util.Objects;

@Keep public class Locale {
    private static final HashMap<String, LocaleInt> locales = new HashMap<>();
    private static final String[] STRINGS = new String[41];
    public static final int Q_STAT_SET = 0;
    public static final int Q_CURRENCY = 1;
    public static final int Q_NO_CURRENCY = 2;
    public static final int Q_DENIED = 3;
    public static final int Q_DATA_REFRESHED= 4;
    public static final int Q_ACTIVATED= 5;
    public static final int G_EXCEPTION = 6;
    public static final int C_RUNNING = 7;
    public static final int C_CANDLE_PRINT_REGULAR = 8;
    public static final int C_CONVERSION_FAILED = 9;
    public static final int C_CANDLE_PRINT_SEASON = 10;
    public static final int F_FRIEND_QUERY_FAILED = 11;
    public static final int G_C_FAILED_RETRYING = 12;
    public static final int G_C_FAILED = 13;
    public static final int G_C_DONE = 14;
    public static final int G_C_CANTREAD = 15;
    public static final int SS_NO_LIST = 16;
    public static final int SS_CURRENCY_STRING= 17;
    public static final int SS_NO_ENTRYPOINT = 18;
    public static final int W_FAILED = 19;
    public static final int W_DONE = 20;
    public static final int L_LOAD_FAILED = 21;
    public static final int D_OK = 22;
    public static final int D_FAILED = 23;
    public static final int E_P1 = 24;
    public static final int E_P2 = 25;
    public static final int E_P3= 26;
    public static final int E_PRINT_C = 27;
    public static final int E_PRINT_W = 28;
    public static final int SS_YES = 29;
    public static final int SS_NO = 30;
    public static final int SS_UNKNOWN = 31;
    public static final int WQ_FAILED_TO_FILTER = 32;
    public static final int WQ_FORMAT_STR = 33;
    public static final int WQ_COLLECTED_QUEST = 34;
    public static final int WQ_COLLECTED_COLLECTIBLE = 35;
    public static final int WQ_ALREADY_QUEST = 36;
    public static final int WQ_UNKNOWN_RESPONSE = 37;
    public static final int WQ_NO_RESPONSE = 38;
    public static final int WQ_SKY_RES_MISSING = 39;
    public static final int WQ_COLLECTIBLE_FAILED = 40;
    static {
        locales.put(null,new LocaleDefault());
        locales.put("ru", new LocaleRussian());
        setLocale(null);
    }
    public static String get(int i) {
        return STRINGS[i];
    }
    public static String get(int i, Object... va) {
        return String.format(STRINGS[i], va);
    }
    @Keep public static void init() {
        java.util.Locale locale = java.util.Locale.getDefault();
        String localeName = locale.getLanguage()+" "+locale.getCountry();
        if(setLocale(localeName) && setLocaleNative(localeName)) return;
        localeName = locale.getLanguage();
        setLocale(localeName);
        setLocaleNative(localeName);
    }
    public static boolean setLocale(String locale) {
        String key = locales.containsKey(locale) ? locale : null;
        Objects.requireNonNull(locales.get(key)).write(STRINGS);
        return key != null;
    }
    @Keep public static native boolean setLocaleNative(String locale);
}
