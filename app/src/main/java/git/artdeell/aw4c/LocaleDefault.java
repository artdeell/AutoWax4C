package git.artdeell.aw4c;

public class LocaleDefault implements LocaleInt{
    public void write(String[] strings) {
        strings[Locale.Q_STAT_SET] = "Quest stat set: %s";
        strings[Locale.Q_CURRENCY] = "%s (%d seasonals, %d candles)";
        strings[Locale.Q_NO_CURRENCY] = "No currency response";
        strings[Locale.Q_DENIED] = "Quest denied";
        strings[Locale.Q_DATA_REFRESHED] = "Quest data refreshed: %s";
        strings[Locale.Q_ACTIVATED] = "Quests activated!";
        strings[Locale.G_EXCEPTION]= "An error has occured\n%s";
        strings[Locale.C_RUNNING] = "Running for candles...";
        strings[Locale.C_CANDLE_PRINT_REGULAR] = "Candles: %d";
        strings[Locale.C_CONVERSION_FAILED] = "Failed to convert candles: %s";
        strings[Locale.C_CANDLE_PRINT_SEASON] = "Seasonal candles: %d";
        strings[Locale.F_FRIEND_QUERY_FAILED] = "Failed to read friend list";
        strings[Locale.G_C_FAILED_RETRYING] = "Failed to collect %d due to %s";
        strings[Locale.G_C_FAILED] = "Failed to collect %d";
        strings[Locale.G_C_DONE] = "Collected %d";
        strings[Locale.G_C_CANTREAD] ="Failed to read incoming gifs";
        strings[Locale.SS_NO_LIST] = "Can't find the list";
        strings[Locale.SS_CURRENCY_STRING] = "%d %s (spass:%s)";
        strings[Locale.SS_NO_ENTRYPOINT] = "Can't find the start of the list";
        strings[Locale.W_FAILED] = "Failed to collect WL";
        strings[Locale.W_DONE] = "Successfully collected %d winged lights out of %d, missing lights: %s";
        strings[Locale.L_LOAD_FAILED] = "Failed to load WL list";
        strings[Locale.D_FAILED] = "Failed to drop lights";
        strings[Locale.D_OK] = "Dropped %d lights";
        strings[Locale.E_P1] = "Sacrificing your lights...";
        strings[Locale.E_P2] = "Materializing your sacrifices...";
        strings[Locale.E_P3] = "Returning your light...";
        strings[Locale.E_PRINT_C]  = "Edem candles: %d";
        strings[Locale.E_PRINT_W] = "Edem wax: %d";
        strings[Locale.SS_YES] = "yes";
        strings[Locale.SS_NO] = "no";
        strings[Locale.SS_UNKNOWN] = "Unknown";
        strings[Locale.WQ_FAILED_TO_FILTER] = "Failed to filter world quests";
        strings[Locale.WQ_FORMAT_STR] = "%s (%s)";
        strings[Locale.WQ_COLLECTED_QUEST] = "Executed world quest: %s";
        strings[Locale.WQ_COLLECTED_COLLECTIBLE] = "Grabbed spirit: %s";
        strings[Locale.WQ_ALREADY_QUEST] ="World quest already collected";
        strings[Locale.WQ_UNKNOWN_RESPONSE] = "Unknown response: %s";
        strings[Locale.WQ_NO_RESPONSE] = "No response";
        strings[Locale.WQ_SKY_RES_MISSING] = "Failed to read Sky resources";
        strings[Locale.WQ_COLLECTIBLE_FAILED] = "Failed to collect spirit %s";
        strings[Locale.C_RUN_QUEST_RESULT]= "Extra collect result: %s";
        strings[Locale.C_RUN_QUEST_FAILED] = "Failed to run extra: %s";
        strings[Locale.FL_UNNAMED] = "<unnamed>";
    }
}
