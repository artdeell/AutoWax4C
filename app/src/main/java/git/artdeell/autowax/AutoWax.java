package git.artdeell.autowax;

import android.util.Log;

import git.artdeell.autowax.worldquest.Spirits;
import git.artdeell.aw4c.CanvasMain;
import git.artdeell.aw4c.ContextOps;
import git.artdeell.aw4c.Locale;
import local.json.JSONArray;
import local.json.JSONException;
import local.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.HttpsURLConnection;

public class AutoWax {
    private static String API_HOST = "live.radiance.thatgamecompany.com";
    private static String userAgent;

    public final Object sessionLock = new Object();
    String userid = null;
    String session = null;

    public static void initWithParameters(int version, boolean isBeta) {
        API_HOST = isBeta ? "beta.radiance.thatgamecompany.com" : "live.radiance.thatgamecompany.com";
        userAgent = isBeta ? "Sky-Test-com.tgc.sky.android.test./0.15.1." + version + " (unknown; android 30.0.0; en)" : "Sky-Live-com.tgc.sky.android/0.15.1." + version + " (unknown; android 30.0.0; en)";
    }

    public void resetSession(String userid, String session) {
        this.userid = userid;
        this.session = session;
        synchronized (sessionLock) {
            sessionLock.notifyAll();
        }
    }

    public JSONObject genInitial() throws JSONException {
        JSONObject ret = new JSONObject();
        ret.put("user", userid);
        ret.put("session", session);
        return ret;
    }

    private static JSONObject _doPost(String sid, String uid, String path, String postData) throws SkyProtocolException {
        try {
            HttpsURLConnection conn = (HttpsURLConnection) new URL("https://" + API_HOST + path).openConnection();
            conn.setRequestMethod("POST");
            conn.setRequestProperty("User-Agent", userAgent);
            conn.setRequestProperty("Content-Type", "application/json; charset=utf-8");
            conn.setRequestProperty("Content-Length", postData.getBytes(StandardCharsets.UTF_8).length + "");
            conn.setRequestProperty("Host", API_HOST);
            if (sid != null) conn.setRequestProperty("session", sid);
            if (uid != null) conn.setRequestProperty("user-id", uid);
            conn.setUseCaches(false);
            conn.setDoInput(true);
            conn.setDoOutput(true);
            conn.connect();
            try (OutputStream wr = conn.getOutputStream()) {
                wr.write(postData.getBytes(StandardCharsets.UTF_8));
            }
            if (conn.getResponseCode() == 401) {
                throw new LostSessionException("");
            }
            Log.i("ErrorCode", conn.getResponseCode() + "");
            try (InputStream rd = conn.getResponseCode() >= 400 ? conn.getErrorStream() : conn.getInputStream()) {
                if (rd == null) return new JSONObject();
                StringBuilder ret = new StringBuilder();
                int cpt;
                byte[] buf = new byte[1024];
                while ((cpt = rd.read(buf)) != -1) {
                    ret.append(new String(buf, 0, cpt));
                }
                if (ret.toString().isEmpty()) return new JSONObject();
                return new JSONObject(ret.toString());
                //return ret;
            }
        } catch (IOException ex) {
            ex.printStackTrace();
            SkyProtocolException twr = new SkyProtocolException("An IOException was raised during request");
            twr.initCause(ex);
            throw twr;
        }
    }

    public JSONObject doPost(String path, JSONObject postData) throws SkyProtocolException {
        boolean doesRequireRerequest = true;
        JSONObject resp = null;
        while (doesRequireRerequest) {
            try {
                doesRequireRerequest = false;
                resp = _doPost(session, userid, path, postData.toString());
                if (resp.has("result") && resp.get("result").equals("timeout")) {
                    System.out.println("TGCDB timed out");
                    doesRequireRerequest = true;
                }
            } catch (LostSessionException e) {
                doesRequireRerequest = true;
                System.out.println("We lost session!");
                try {
                    CanvasMain.goReauthorize();
                    System.out.println("Reauthorization fired!");
                    synchronized (sessionLock) {
                        sessionLock.wait();
                    }
                    if (postData.has("user")) postData.put("user", userid);
                    if (postData.has("session")) postData.put("session", session);
                } catch (InterruptedException _e) {
                    throw new SkyProtocolException("Interrupted!");
                }
            }
        }
        System.out.println(resp);
        return resp;
    }

    private void forgeWithOutput(String source, String destination, JSONObject currency, int rate, int localeString) throws SkyProtocolException {
        int waxCount = currency.optInt(source, 0);
        int candleCount = currency.optInt(destination, 0);
        if (waxCount >= rate) {
            JSONObject forgeRq = genInitial();
            forgeRq.put("currency", destination);
            forgeRq.put("forge_currency", source);
            forgeRq.put("count", waxCount / rate);
            forgeRq.put("cost", rate);
            JSONObject resp = doPost("/account/buy_candle_wax", forgeRq);
            if (resp.optString("result", "").equals("ok")) {
                if (resp.has("currency")) {
                    candleCount = resp.getJSONObject("currency").getInt(destination);
                }
                CanvasMain.submitLogString(Locale.get(localeString, candleCount));
            } else {
                CanvasMain.submitLogString(Locale.get(Locale.C_CONVERSION_FAILED, resp.optString("result", "Unknown error")));
            }
        } else {
            CanvasMain.submitLogString(Locale.get(localeString, candleCount));
        }
    }
    private void execQuest(String qName, float bonus_percent) throws SkyProtocolException{
        JSONObject questRq = genInitial();
        questRq.put("name", qName);
        questRq.put("bonus_percent", bonus_percent);
        JSONObject rsp = doPost("/account/claim_quest_reward", questRq);
        CanvasMain.submitLogString(Locale.get(Locale.C_RUN_QUEST_RESULT, rsp.optString("result", "Unknown error")));
    }
    private void execQuests() throws Exception{
        if(!ContextOps.hasAssets()) {
            CanvasMain.submitLogString(Locale.get(Locale.WQ_SKY_RES_MISSING));
            return;
        }
        JSONArray quests = doPost("/account/get_account_world_quests",genInitial()).optJSONArray("set_world_quests");
        JSONArray gameQuests = new JSONArray(Spirits.dump("Data/Resources/WorldQuestDefs.json"));
        HashMap<String, JSONObject> questsByName = new HashMap<>();
        for(Object o : quests) {
            JSONObject quest = (JSONObject) o;
            questsByName.put(quest.optString("world_quest_def_id", null), quest);
        }
        for(Object o : gameQuests) {
            JSONObject quest = (JSONObject) o;
            String questId;
            if((questId = checkQuest(questsByName, quest)) != null) {
                Log.i("aw4c", "quest: "+questId );
                execQuest(questId, quest.optInt("bonus_amount", 0) != 0 ? 1 : 0);
            }
        }
    }
    private String checkQuest(HashMap<String, JSONObject> questsByName, JSONObject quest) {
        String qid = quest.optString("id");
        if(qid == null) return null;
        JSONObject playerQuest = questsByName.get(qid);
        if(playerQuest != null && playerQuest.optLong("cooldown_over_time", 0) > (System.currentTimeMillis() / 1000))
            return null;
        if((quest.optInt("reward_amount_1") != 0 || quest.optInt("reward_amount_2") != 0)
                && !(quest.getBoolean("once_lifetime") || quest.getBoolean("on_until_first_done")))
            return qid;
        else return null;
    }
    private void execQuestList(String... questsNames) throws SkyProtocolException {
        JSONArray quests = doPost("/account/get_account_world_quests",genInitial()).optJSONArray("set_world_quests");
        HashMap<String, JSONObject> questsByName = new HashMap<>();
        for(Object o : quests) {
            JSONObject quest = (JSONObject) o;
            questsByName.put(quest.optString("world_quest_def_id", null), quest);
        }
        for(int i = 0; i < questsNames.length; i++) {
            JSONObject playerQuest = questsByName.get(questsNames[i]);
            if(playerQuest != null && playerQuest.optLong("cooldown_over_time", 0) > (System.currentTimeMillis() / 1000)) continue;
            execQuest(questsNames[i], 1);
            CanvasMain.submitProgressBar(i, questsNames.length-1);
        }
    }
    public void doRaces(boolean extraRaces) {
        try {
            CanvasMain.submitProgressBar(0, 1);
            execQuestList("sunset_race", "sunset_flyrace", "sunset_yeti_race");
            CanvasMain.submitProgressBar(0, -1);
            //if(extraRaces) execQuest("test_multilevel_race1", 1);
        }catch (Exception e) {
            CanvasMain.submitLogString(Locale.get(Locale.C_CONVERSION_FAILED, e.toString()));
        }
    }
    public void doCandleRun() {
        CRArray.init();
        ThreadPoolExecutor tpe = new ThreadPoolExecutor(4, 4, 200, TimeUnit.MILLISECONDS, new LinkedBlockingQueue<>());
        int maxProgress = 0;
        CanvasMain.submitLogString(Locale.get(Locale.C_RUNNING));
        for (Object o : CRArray.LEVELS) {
            JSONObject level = (JSONObject) o;
            long levelId = level.getLong("level_id");
            JSONArray candles = level.getJSONArray("pickup_ids");
            JSONObject intermediaryBatch = null;
            for (int i = 0; i < candles.length(); i++) {
                if (i % 16 == 0) {
                    if (intermediaryBatch != null) {
                        intermediaryBatch.put("level_id", levelId);
                        tpe.execute(new CandleRunBatch(this, intermediaryBatch));
                        maxProgress++;
                    }
                    intermediaryBatch = genInitial();
                }
                intermediaryBatch.append("pickup_ids", candles.get(i));
            }
        }
        tpe.shutdown();
        try {
            while (!tpe.awaitTermination(50, TimeUnit.MILLISECONDS)) {
                //Log.log("MachineDispatch-Runner","Running for candles...");
                CanvasMain.submitProgressBar(((tpe.getActiveCount() + tpe.getQueue().size() * -1) + maxProgress), maxProgress);
            }
        } catch (InterruptedException ignored) {
        }
        try {
            JSONObject currency = doPost("/account/get_currency", genInitial()).optJSONObject("currency");
            if(currency != null) {
                forgeWithOutput("wax", "candles", currency, 150, Locale.C_CANDLE_PRINT_REGULAR);
                forgeWithOutput("season_wax", "season_candle", currency, 12, Locale.C_CANDLE_PRINT_SEASON);
            }
        } catch (Exception e) {
            CanvasMain.submitLogString(Locale.get(Locale.C_CONVERSION_FAILED, e.toString()));
        }
        CanvasMain.submitProgressBar(0, -1);
    }

    public void doQuests() {
        try {
            JSONArray quests = doPost("/account/get_season_quests", genInitial()).getJSONArray("season_quests");
            int questsLength = quests.length();

            for (int i = 0; i < questsLength; i++) {
                JSONObject quest = quests.getJSONObject(i);
                if (!quest.getBoolean("activated")) {
                    JSONObject questrq = genInitial();
                    questrq.put("quest_id", quest.getString("daily_quest_def_id"));
                    JSONObject questrsp = doPost("/account/activate_season_quest", questrq);
                    if (questrsp == null) continue;
                    JSONArray questUpdates = questrsp.getJSONArray("season_quest_activated");
                    for (int j = 0; j < questUpdates.length(); j++) {
                        JSONObject _quest = questUpdates.getJSONObject(j);
                        if (quest.getString("daily_quest_def_id").equals(_quest.getString("quest_id"))) {
                            quest.put("activated", true);
                            quest.put("start_value", _quest.getDouble("start_value"));
                            CanvasMain.submitLogString(Locale.get(Locale.Q_DATA_REFRESHED, _quest.getString("quest_id")));
                            //appendlnToLog("Quest data refreshed: " + _quest.getString("quest_id"));
                        }
                    }
                }
                CanvasMain.submitProgressBar(i + 1, questsLength * 2);
            }
            CanvasMain.submitLogString(Locale.get(Locale.Q_ACTIVATED));
            for (int i = 0; i < questsLength; i++) {
                JSONObject quest = quests.getJSONObject(i);
                double result = upstatAndClaim(quest, 0);
                if (result > 0)
                    upstatAndClaim(quest, result);
                CanvasMain.submitProgressBar(i + questsLength + 1, questsLength * 2);
            }
        } catch (Exception e) {
            CanvasMain.submitLogString(Locale.get(Locale.G_EXCEPTION, e.toString()));
        }
        CanvasMain.submitProgressBar(0, -1);
    }

    private double upstatAndClaim(JSONObject quest, double source) throws SkyProtocolException {
        if (quest.has("stat_type")) {
            double initialStat = source == 0 ? quest.getDouble("start_value") : source;
            double statMod = quest.has("stat_delta") ? quest.getDouble("stat_delta") : 30;
            JSONObject statChangeRq = genInitial();
            JSONObject stat = new JSONObject();
            stat.put("type", quest.getString("stat_type"));
            stat.put("value", (int) (initialStat + statMod));
            JSONArray statArray = new JSONArray();
            statArray.put(stat);
            statChangeRq.put("achievement_stats", statArray);
            if (doPost("/account/set_achievement_stats", statChangeRq) != null)
                CanvasMain.submitLogString(Locale.get(Locale.Q_STAT_SET, quest.get("stat_type")));
            JSONObject questClaimRq = genInitial();
            questClaimRq.put("quest_id", quest.getString("daily_quest_def_id"));
            JSONObject obj = doPost("/account/claim_season_quest_reward", questClaimRq);
            String claimResult = obj.has("season_quest_claim_result") ? obj.getString("season_quest_claim_result") : null;
            if ("ok".equals(claimResult) || "already".equals(claimResult)) {
                if (obj.has("currency")) {
                    JSONObject currency = obj.getJSONObject("currency");
                    CanvasMain.submitLogString(Locale.get(Locale.Q_CURRENCY, claimResult, currency.has("season_candle") ? currency.getInt("season_candle") : 0, currency.has("candles") ? currency.getInt("candles") : 0));
                } else {
                    CanvasMain.submitLogString(Locale.get(Locale.Q_NO_CURRENCY));
                }
                return 0;
            } else {
                CanvasMain.submitLogString(Locale.get(Locale.Q_DENIED));
                return initialStat + statMod;
            }
        } else {
            return -1;
        }
    }

    public void runGift() {
        try {
            Set<String> tradeBuddies = ContextOps.sharedPreferences != null ?
                    ContextOps.sharedPreferences.getStringSet("trade_buddies", null) :
                    null;
            JSONObject friendRq = genInitial();
            friendRq.put("max", 1024);
            friendRq.put("sort_ver", 1);
            JSONArray friendList = doPost("/account/get_friend_statues", friendRq).optJSONArray("set_friend_statues");
            if (friendList == null) {
                CanvasMain.submitLogString(Locale.get(Locale.F_FRIEND_QUERY_FAILED));
                return;
            }
            JSONObject gifts = doPost("/account/get_pending_messages", genInitial());
            ArrayList<Gift> alreadySent = new ArrayList<>();
            if (gifts.has("set_sent_messages")) {
                for (Object o : gifts.getJSONArray("set_sent_messages")) {
                    JSONObject sent = (JSONObject) o;
                    alreadySent.add(new Gift(sent.getString("to_id"), sent.getString("type"), null));
                }
            }

            //appendlnToLog(friendList.toString());
            ThreadPoolExecutor tpe = new ThreadPoolExecutor(4, 4, 200, TimeUnit.MILLISECONDS, new LinkedBlockingQueue<>());
            for (Object o : friendList) {
                JSONObject friend = (JSONObject) o;
                String friendId = friend.getString("friend_id");
                String friendName = friend.has("nickname") ? friend.getString("nickname") :  Locale.get(Locale.FL_UNNAMED);
                Gift cobj = new Gift(friendId, "gift_heart_wax", friendName);
                if (!alreadySent.contains(cobj))
                    tpe.execute(new GiftTask(cobj, this, false));
                if(tradeBuddies != null && tradeBuddies.contains(friendId)) {
                    Gift paid = new Gift(friendId, "gift", friendName);
                    if(!alreadySent.contains(paid))
                        tpe.execute(new GiftTask(paid, this, true));
                }
            }
            tpe.shutdown();
            tpe.awaitTermination(Long.MAX_VALUE, TimeUnit.DAYS);
        } catch (Exception e) {
            CanvasMain.submitLogString(Locale.get(Locale.G_EXCEPTION, e.toString()));
        }
    }

    public void collectGifts() {
        try {
            JSONObject gifts = doPost("/account/get_pending_messages", genInitial());
            if (gifts.has("set_recvd_messages")) {
                JSONArray giftsArray = gifts.getJSONArray("set_recvd_messages");
                System.out.println(giftsArray);
                ThreadPoolExecutor tpe = new ThreadPoolExecutor(4, 4, 200, TimeUnit.MILLISECONDS, new LinkedBlockingQueue<>());
                for (Object o : giftsArray) {
                    JSONObject gift = (JSONObject) o;
                    tpe.execute(new CollectGiftTask(gift.getLong("msg_id"), gift.getString("type"), this));
                }
                tpe.shutdown();
                boolean shutup_android_studio = tpe.awaitTermination(Long.MAX_VALUE, TimeUnit.DAYS);
                //tpe.execute(new CollectGiftTask());тщ
            } else CanvasMain.submitLogString(Locale.get(Locale.G_C_CANTREAD));
        } catch (SkyProtocolException | InterruptedException e) {
            CanvasMain.submitLogString(Locale.get(Locale.G_EXCEPTION, e.toString()));
        }
    }

    public void collectLights(String lights, boolean world, boolean spirit) {
        try {
            JSONObject collectRq = genInitial();
            List<String> lightsSplit = new ArrayList<>(Arrays.asList(lights.trim().split(",")));

            Iterator<String> iter = lightsSplit.iterator();
            while (iter.hasNext()) {
                String str = iter.next();
                if (!world && str.startsWith("l_"))
                    iter.remove();
                if (!spirit && str.startsWith("s_"))
                    iter.remove();
            }
            for (String s : lightsSplit) {
                collectRq.append("names", s);
            }
            JSONObject resp = doPost("/account/wing_buffs/collect", collectRq);

            List<String> missingBuffs = new ArrayList<>(lightsSplit);
            int collectedBuffs = 0;
            if (resp.has("update_wing_buffs")) {
                JSONArray wingBuffs = resp.getJSONArray("update_wing_buffs");
                for (Object o : wingBuffs) {
                    JSONObject wb = (JSONObject) o;
                    if (wb.has("collected") && wb.getBoolean("collected")) {
                        collectedBuffs++;
                        missingBuffs.remove(wb.getString("name"));
                    }
                }
                CanvasMain.submitLogString(Locale.get(Locale.W_DONE, collectedBuffs, lightsSplit.size(), Arrays.toString(missingBuffs.toArray(new String[0]))));
            } else {
                CanvasMain.submitLogString(Locale.get(Locale.W_FAILED));
            }
        } catch (Exception e) {
            CanvasMain.submitLogString(Locale.get(Locale.G_EXCEPTION, e.toString()));
        }
    }
    public void runDrop(int count) {
        try {
            if(count <= 0) return;
            JSONObject resp = doPost("/account/wing_buffs/get", genInitial());
            System.out.println(resp.keySet());
            if(!resp.has("wing_buffs")) {
                CanvasMain.submitLogString(Locale.get(Locale.L_LOAD_FAILED));
                return;
            }
            JSONArray wing_buffs = resp.getJSONArray("wing_buffs");
            JSONObject dropRq = genInitial();
            for(Object o : wing_buffs) {
                if (count == 0) {
                    Log.i("AutoWax","Finished generating lights list");
                    break;
                }
                JSONObject light = (JSONObject) o;
                if(light.has("collected") && light.getBoolean("collected")) {
                    dropRq.append("names",light.getString("name"));
                    count--;
                }
            }
            JSONObject dropResult = doPost("/account/wing_buffs/drop",dropRq);
            if(dropResult.has("result") && "ok".equals(dropResult.get("result"))) {
                CanvasMain.submitLogString(Locale.get(Locale.D_OK, dropRq.getJSONArray("names").length()));
            }else{
                CanvasMain.submitLogString(Locale.get(Locale.D_FAILED));
            }
        }catch(Exception e) {
            CanvasMain.submitLogString(Locale.get(Locale.G_EXCEPTION, e.toString()));
        }
    }
    public void edemRun() {
        try {
            JSONObject wingBuffs = doPost("/account/wing_buffs/get", genInitial());
            if(!wingBuffs.has("wing_buffs")) {
                CanvasMain.submitLogString(Locale.get(Locale.L_LOAD_FAILED));
                return;
            }
            JSONArray buffsList = wingBuffs.getJSONArray("wing_buffs");
            CanvasMain.submitLogString(Locale.get(Locale.E_P1));
            JSONObject fin = genInitial();
            for(Object o : buffsList) {
                JSONObject jobj = (JSONObject) o;
                if(jobj.getBoolean("collected") && !jobj.isNull("deposit_id")) {
                    JSONArray arr = new JSONArray();
                    arr.put(jobj.getString("name"));
                    arr.put(jobj.getLong("deposit_id"));
                    fin.append("name_deposit_id_pairs", arr);
                }
            }
            doPost("/account/wing_buffs/deposit",fin);
            CanvasMain.submitLogString(Locale.get(Locale.E_P2));
            JSONObject conversionResponse = doPost("/account/wing_buffs/convert", genInitial());
            if(conversionResponse.has("currency")) {
                JSONObject currency = conversionResponse.getJSONObject("currency");
                if(currency.has("prestige")) CanvasMain.submitLogString(Locale.get(Locale.E_PRINT_C, currency.getInt("prestige")));
                if(currency.has("prestige_wax")) CanvasMain.submitLogString(Locale.get(Locale.E_PRINT_W, currency.getInt("prestige_wax")));
            }
            if(conversionResponse.has("wing_buffs")) {
                CanvasMain.submitLogString(Locale.get(Locale.E_P3));
                JSONArray buffs = conversionResponse.getJSONArray("wing_buffs");
                int blen = buffs.length();
                StringBuilder buffList = new StringBuilder();
                for(int i = 0; i < blen; i++) {
                    buffList.append(buffs.getJSONObject(i).getString("name"));
                    if(i != blen-1) buffList.append(',');
                }
                collectLights(buffList.toString(), true, true);
            }
        }catch(Exception e) {
            CanvasMain.submitLogString(Locale.get(Locale.G_EXCEPTION, e.toString()));
        }
    }
}
