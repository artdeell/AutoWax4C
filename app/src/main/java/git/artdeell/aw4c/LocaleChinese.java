package git.artdeell.aw4c;

public class LocaleChinese implements LocaleInt{
    public void write(String[] strings) {
        strings[Locale.Q_STAT_SET] = "任务统计中: %s";
        strings[Locale.Q_CURRENCY] = "%s (%d 季节蜡烛, %d 蜡烛)";
        strings[Locale.Q_NO_CURRENCY] = "无蜡烛响应";
        strings[Locale.Q_DENIED] = "任务拒绝";
        strings[Locale.Q_DATA_REFRESHED] = "刷新任务数据: %s";
        strings[Locale.Q_ACTIVATED] = "任务激活！";
        strings[Locale.G_EXCEPTION]= "发生错误\n%s";
        strings[Locale.C_RUNNING] = "蜡烛奔跑中...";
        strings[Locale.C_CANDLE_PRINT_REGULAR] = "蜡烛: %d";
        strings[Locale.C_CONVERSION_FAILED] = "无法转换蜡烛: %s";
        strings[Locale.C_CANDLE_PRINT_SEASON] = "季节蜡烛: %d";
        strings[Locale.F_FRIEND_QUERY_FAILED] = "读取好友列表失败";
        strings[Locale.G_C_FAILED_RETRYING] = "领取失败 %d 由于 %s";
        strings[Locale.G_C_FAILED] = "领取失败 %d";
        strings[Locale.G_C_DONE] = "已近领取了 %d";
        strings[Locale.G_C_CANTREAD] ="无法读取收到的礼物";
        strings[Locale.SS_NO_LIST] = "找不到列表";
        strings[Locale.SS_CURRENCY_STRING] = "%d %s (季票:%s)";
        strings[Locale.SS_NO_ENTRYPOINT] = "找不到列表的开头";
        strings[Locale.W_FAILED] = "领取翼失败";
        strings[Locale.W_DONE] = "领取成功 %d 领取的翼一共有 %d, 缺少的翼: %s";
        strings[Locale.L_LOAD_FAILED] = "无法加载翼列表";
        strings[Locale.D_FAILED] = "掉翼失败";
        strings[Locale.D_OK] = "掉了 %d 的翼";
        strings[Locale.E_P1] = "牺牲翼中...";
        strings[Locale.E_P2] = "实现您的牺牲...";
        strings[Locale.E_P3] = "归还您的翼...";
        strings[Locale.E_PRINT_C]  = "伊甸蜡烛: %d";
        strings[Locale.E_PRINT_W] = "伊甸蜡: %d";
        strings[Locale.SS_YES] = "是";
        strings[Locale.SS_NO] = "不是";
        strings[Locale.SS_UNKNOWN] = "未知";
        strings[Locale.WQ_FAILED_TO_FILTER] = "过滤世界任务失败";
        strings[Locale.WQ_FORMAT_STR] = "%s (%s)";
        strings[Locale.WQ_COLLECTED_QUEST] = "执行世界任务: %s";
        strings[Locale.WQ_COLLECTED_COLLECTIBLE] = "领取的先祖: %s";
        strings[Locale.WQ_ALREADY_QUEST] ="世界任务已近领取";
        strings[Locale.WQ_UNKNOWN_RESPONSE] = "未知响应: %s";
        strings[Locale.WQ_NO_RESPONSE] = "没有响应";
        strings[Locale.WQ_SKY_RES_MISSING] = "光遇读取资源失败";
        strings[Locale.WQ_COLLECTIBLE_FAILED] = "领取先祖失败 %s";
        strings[Locale.C_RUN_QUEST_RESULT]= "额外收集结果: %s";
        strings[Locale.C_RUN_QUEST_FAILED] = "额外收集失败: %s";
        strings[Locale.FL_UNNAMED] = "<无名>";
    }
}
