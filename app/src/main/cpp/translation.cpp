//
// Created by maks on 19.10.2022.
//

#include <cstdio>
#include <cstring>
#include <jni.h>
#include <cstdlib>
#include "translation.h"
#include "main.h"
#include "translation_chinese.h"
static const char* locale_strings_ru[] = {
        "Похоже текущая сессия была прервана.\nНажмите кнопку ниже, чтобы продолжить.",
        "Перезагрузить",
        "Собрать свечи",
        "Выполнить квесты",
        "Собрать подарки",
        "Отправить подарки",
        "Запуск",
        "Магазины духов",
        "Собрать КС",
        "Менеджер приглашений",
        "Пробег по Эдему",
        "Скачивание...",
        "Загрузка...",
        "Скачиваниек не удалось: %s",
        "Повторить попытку",
        "Открыть",
        "Купить",
        "Копировать",
        "Вставить",
        "Очистить",
        "КС мира",
        "КС духов",
        "Сборщик КС работает! Смотрите в журнал для подробностей",
        "Сборщик работает",
        "Выкинуть",
        "Удалить",
        "Добавить",
        "Назад",
        "Перезагрузить",
        "Ошибка загрузки списка духов: %s",
        "Пройти",
        "Духи",
        "AW4C - бесплатное ПО для личного использования\nЕсли вы не пользуетесь им коммерчески, верните свои средства.",
        "Ой! Похоже нужно обновление...",
        "Собрать гонки",
        "Настройки торговли сердцами",
        "Ошибка загрузки списка друзей",
        "Добавить",
        "Убрать",
        "Зачем ты вообще играешь в эту игру?",
        "Перезагрузить",
        "Загрузить из файла",
        "Прыгнуть на крату",
        "Прыгнуть",
        "Ошибка загрузки магазина",
        "Платформа",
        "ID товара",
        "Купить предмет",
        "Магазин",
        "Введите ваш ключ AW4C.\nЕсли у вас его нет, вы можете получить его бесплатно с использованием Telegram-бота @aw4cbot, или используя Discord.",
        "https://discord.gg/VThJQz5atA",
        "Копировать Discord-ссылку",
        "Отправить ключ",
        "Вставить ключ",
        "Ошибка на сервере",
        "Этого ключа не существует",
        "Вы не можете зарегистрировать больше пользователей на этом ключе. Проконсультируйтесь с Telegram/Discord каналом для получения дополнительной информации",
        "Ошибка ввода-вывода. Проверьте своё соединение с сетью.",
        "Неизвестная ошибка"
};
static const char* locale_strings_default[] = {
        "It seems like the current session was terminated. \nPress the button below when you are ready to continue.",
        "Reload",
        "Run candles",
        "Run quests",
        "Collect gifts",
        "Send gifts",
        "Run",
        "Spirit Shops",
        "Collect WL",
        "Invite Manager",
        "Edem Run",
        "Downloading...",
        "Loading...",
        "Failed to download: %s",
        "Retry",
        "Open",
        "Buy",
        "Copy",
        "Paste",
        "Clear",
        "World lights",
        "Spirit lights",
        "The collector is running! Check log for more info",
        "WL collector running",
        "Drop",
        "Remove",
        "Add",
        "Back",
        "Reload",
        "Failed to load spirits: %s",
        "Run",
        "Spirits",
        "AW4C is free for personal use. \nIf you don't use it commercially, please return your money",
        "Whoops! Looks like we need an update...",
        "Collect races",
        "Heart trade config",
        "Failed to load friend list",
        "Add",
        "Remove",
        "Why are you even playing at this point?",
        "Refresh",
        "Load from file",
        "Jump to level",
        "Jump",
        "Failed to load shop",
        "Platform",
        "Item ID",
        "Buy IAP",
        "Purchase IAPs",
        "Please enter your AW4C key.\nIf you don't have one, you can obtain it for free using Telegram @aw4cbot, or using Discord.",
        "https://discord.gg/VThJQz5atA",
        "Copy Discord server link",
        "Submit key",
        "Paste key",
        "Server error",
        "This key does not exist",
        "You can't register more users on this key. Consult the telegram/discord channel for how to get more",
        "I/O error. Please check your network connection.",
        "Unknown error"



};
char** locale_strings = (char**)locale_strings_default;

typedef struct {char* key; char** val;} lookup_type;
static const lookup_type table[] = {{"ru", (char**)locale_strings_ru}, {"zh", (char**)locale_strings_chinese}};

extern "C"
jboolean
Java_git_artdeell_aw4c_Locale_setLocaleNative(JNIEnv *env, [[maybe_unused]] jclass clazz, jstring locale) {
    const char* localeName = env->GetStringUTFChars(locale, nullptr);
    for(auto member : table) {
        if(strcmp(member.key, localeName) == 0) {
            locale_strings = member.val;
            env->ReleaseStringUTFChars(locale, localeName);
            return true;
        }
    }
    locale_strings = (char**)locale_strings_default;
    env->ReleaseStringUTFChars(locale, localeName);
    return false;
}
const JNINativeMethod methods[] = {
        { "setLocaleNative",     "(Ljava/lang/String;)Z", (void*)&Java_git_artdeell_aw4c_Locale_setLocaleNative}
};
void translation_init(JNIEnv* env) {
    jclass class_Locale = LoadClass(env, "git.artdeell.aw4c.Locale");
    env->RegisterNatives(class_Locale, methods, 1);
    env->CallStaticVoidMethod(class_Locale, env->GetStaticMethodID(class_Locale, "init", "()V"));
}