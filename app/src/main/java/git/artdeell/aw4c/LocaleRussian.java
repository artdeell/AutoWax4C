package git.artdeell.aw4c;

public class LocaleRussian implements LocaleInt{
    public void write(String[] strings) {
        strings[Locale.Q_STAT_SET] = "Установлен элемент статистики: %s";
        strings[Locale.Q_CURRENCY] = "%s (%d сезонных, %d обычных)";
        strings[Locale.Q_NO_CURRENCY] = "Нету данных о валюте";
        strings[Locale.Q_DENIED] = "Выполенение отменено";
        strings[Locale.Q_DATA_REFRESHED] = "Данные обновлены: %s";
        strings[Locale.Q_ACTIVATED] = "Квесты включены!";
        strings[Locale.G_EXCEPTION]= "Произошла ошибка\n%s";
        strings[Locale.C_RUNNING] = "Сбор свечей...";
        strings[Locale.C_CANDLE_PRINT_REGULAR] = "Свечи: %d";
        strings[Locale.C_CONVERSION_FAILED] = "Ошибка конвертации свечей: %s";
        strings[Locale.C_CANDLE_PRINT_SEASON] = "Сезонные свечи: %d";
        strings[Locale.F_FRIEND_QUERY_FAILED] = "Ошибка чтения списка друзей";
        strings[Locale.G_C_FAILED_RETRYING] = "Не удалось собрать %d из-за %s";
        strings[Locale.G_C_FAILED] = "Не удалось собрать %d";
        strings[Locale.G_C_DONE] = "Собран %d";
        strings[Locale.G_C_CANTREAD] ="Ошибка чтения входящих подарков";
        strings[Locale.SS_NO_LIST] = "Невозможно найти список";
        strings[Locale.SS_CURRENCY_STRING] = "%d %s (пропуск:%s)";
        strings[Locale.SS_NO_ENTRYPOINT] = "Невозможно найти начало списка";
        strings[Locale.W_FAILED] = "Ошибка сбора КС";
        strings[Locale.W_DONE] = "Успешно собрано %d КС из %d, не собрались: %s";
        strings[Locale.L_LOAD_FAILED] = "Ошибка загрузки списка свечей";
        strings[Locale.D_FAILED] = "Невозможно выбросить КС";
        strings[Locale.D_OK] = "Выброшено %d КС";
        strings[Locale.E_P1] = "Жертвуем ваш КС...";
        strings[Locale.E_P2] = "Материализуем ваши пожертвования...";
        strings[Locale.E_P3] = "Возвращаем свет...";
        strings[Locale.E_PRINT_C]  = "Свечи эдема: %d";
        strings[Locale.E_PRINT_W] = "Воск эдема: %d";
        strings[Locale.SS_YES] = "да";
        strings[Locale.SS_NO] = "нет";
        strings[Locale.SS_UNKNOWN] = "Результат неизвестен";
        strings[Locale.WQ_COLLECTED_QUEST] = "Выполнен мировой квест: %s";
        strings[Locale.WQ_COLLECTED_COLLECTIBLE] = "Собран дух: %s";
        strings[Locale.WQ_ALREADY_QUEST] ="Мировой квест уже собран";
        strings[Locale.WQ_UNKNOWN_RESPONSE] = "Неизвестный ответ: %s";
        strings[Locale.WQ_NO_RESPONSE] = "Нет ответа";
        strings[Locale.WQ_SKY_RES_MISSING] = "Ошибка чтения ресурсов Sky";
        strings[Locale.WQ_COLLECTIBLE_FAILED] = "Ошибка сбора духа %s";
        strings[Locale.C_RUN_QUEST_RESULT] = "Результат сбора экстра: %s";
    }
}
