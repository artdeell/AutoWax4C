package git.artdeell.autowax.worldquest;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import git.artdeell.autowax.AutoWax;
import git.artdeell.autowax.SkyProtocolException;
import git.artdeell.aw4c.CanvasMain;
import git.artdeell.aw4c.ContextOps;
import git.artdeell.aw4c.Keep;
import git.artdeell.aw4c.Locale;
import local.json.JSONArray;
import local.json.JSONObject;

@Keep
public class Spirits {
    private static Spirits collectibles;
    private final AutoWax host;
    private final List<CombinedSpiritData> spirits = new ArrayList<>();
    public static Spirits get(AutoWax host) {
        if(collectibles == null) collectibles = new Spirits(host);
        return collectibles;
    }
    private Spirits(AutoWax host) {
        this.host = host;
    }

    private static String dump(String path) throws IOException {
        InputStream is = ContextOps.skyResources.getAssets().open(path);
        StringBuilder sb = new StringBuilder();
        byte[] buf = new byte[512];int c;
        while ((c = is.read(buf)) != -1) {
            sb.append(new String(buf,0,c));
        }
        is.close();
        return sb.toString();
    }

    public void load() {
        if(ContextOps.skyResources == null) {
            onLoadResult(Locale.get(Locale.WQ_SKY_RES_MISSING));
            return;
        }
        spirits.clear();
        try {
            JSONArray questDefs = new JSONArray(dump("Data/Resources/WorldQuestDefs.json"));
            JSONArray collectibleDefs = new JSONArray(dump("Data/Resources/CollectibleDefs.json"));
            final HashMap<String, CombinedSpiritData> dataHashMap = new HashMap<>();
            for(int i = 0; i < questDefs.length(); i++) {
                JSONObject quest = questDefs.getJSONObject(i);
                String collectibleName = quest.optString("collectible_name", "");
                CombinedSpiritData data = new CombinedSpiritData(quest.getString("id"), collectibleName, switchFrame(quest.optString("icon", "")));
                spirits.add(i, data);
                dataHashMap.put(collectibleName, data);
            }
            for(int i = 0; i < collectibleDefs.length(); i++) {
                JSONObject collectible = collectibleDefs.getJSONObject(i);
                String collectibleName = collectible.getString("name");
                if(dataHashMap.containsKey(collectibleName)) {
                    CombinedSpiritData data = dataHashMap.get(collectibleName);
                    if(data.icon.isEmpty()) {
                        data.icon = switchFrame(collectible.optString("icon", ""));
                    }
                }else{
                    if(!collectible.optString("type", "").equals("quest_giver")) {
                        CombinedSpiritData data = new CombinedSpiritData("", collectibleName, switchFrame(collectible.optString("icon", "")));
                        dataHashMap.put(collectibleName, data);
                        spirits.add(data);
                    }
                }
            }
            if(filterCollectibles(dataHashMap)) sendNativeList();
            dataHashMap.clear();
        }catch (Exception e) {
            onLoadResult(Locale.get(Locale.G_EXCEPTION, e.toString()));
        }
    }
    private boolean filterCollectibles(HashMap<String, CombinedSpiritData> dataByCollectibleName) throws SkyProtocolException {
        JSONArray quests = host.doPost("/account/get_account_world_quests",host.genInitial()).optJSONArray("set_world_quests");
        if(quests == null) {
            onLoadResult(Locale.get(Locale.WQ_FAILED_TO_FILTER));
            return false;
        }
        JSONArray collectibles = host.doPost("/account/get_collectibles",host.genInitial()).optJSONArray("collectibles");
        if(collectibles== null) {
            onLoadResult(Locale.get(Locale.WQ_FAILED_TO_FILTER));
            return false;
        }
        final HashMap<String, CombinedSpiritData> dataByQuestName = new HashMap<>();
        for(CombinedSpiritData spiritData : spirits) {
            dataByQuestName.put(spiritData.quest_name, spiritData);
        }
        for(Object o : quests) {
            JSONObject quest = (JSONObject) o;
            if(quest.getBoolean("ever_completed")) {
                spirits.remove(dataByQuestName.get( quest.getString("world_quest_def_id")));
            }
        }
        for(Object o : collectibles) {
            JSONObject collectible = (JSONObject) o;

            spirits.remove(dataByCollectibleName.get(collectible.getString("id")));
        }
        return true;
    }
    public void executeQuest(int idx) {
        try {
            CombinedSpiritData quest = spirits.get(idx);
            boolean runCollectible = true;
            if(!quest.quest_name.isEmpty()) {
                JSONObject questRq = host.genInitial();
                questRq.put("name", quest.quest_name);
                questRq.put("bonus_percent", 0);
                JSONObject rsp = host.doPost("/account/claim_quest_reward", questRq);
                switch(rsp.optString("result","unknown")) {
                    case "already":
                        CanvasMain.submitLogString(Locale.get(Locale.WQ_ALREADY_QUEST));
                        break;
                    case "ok":
                    case "unknown_quest":
                        CanvasMain.submitLogString(Locale.get(Locale.WQ_COLLECTED_QUEST, quest.quest_name));
                        break;
                    case "unknown":
                        CanvasMain.submitLogString(Locale.get(Locale.WQ_NO_RESPONSE));
                        runCollectible = false;
                        break;
                    default:
                        CanvasMain.submitLogString(Locale.get(Locale.WQ_UNKNOWN_RESPONSE, rsp.getString("result")));
                        runCollectible = false;
                        break;
                }
            }
            if(!quest.collectible_name.isEmpty() && runCollectible) {
                JSONObject questRq = host.genInitial();
                questRq.put("name", quest.collectible_name);
                questRq.put("carrying", "false");
                JSONObject rsp = host.doPost("/account/collect_collectible", questRq);
                if (rsp.has("collectibles"))
                    CanvasMain.submitLogString(Locale.get(Locale.WQ_COLLECTED_COLLECTIBLE, quest.collectible_name));
                else
                    CanvasMain.submitLogString(Locale.get(Locale.WQ_COLLECTIBLE_FAILED, quest.collectible_name));
            }
        }catch (Exception e) {
            CanvasMain.submitLogString(Locale.get(Locale.G_EXCEPTION, e.toString()));
        }
        load();
    }
    private void sendNativeList() {
        String[] strs = new String[spirits.size()];
        String[] icns = new String[spirits.size()];
        for(int i =0; i < strs.length; i++) {
            CombinedSpiritData quest = spirits.get(i);
            if(!quest.collectible_name.isEmpty() && !quest.quest_name.isEmpty()) {
                strs[i] = Locale.get(Locale.WQ_FORMAT_STR, quest.collectible_name, quest.quest_name);
            }else if(quest.collectible_name.isEmpty()) {
                strs[i] = quest.quest_name;
            }else{
                strs[i] = quest.collectible_name;
            }
            icns[i] = quest.icon;
        }
        onNewList(strs, icns);
    }
    private static String switchFrame(String string) {
        int lastIndex = string.lastIndexOf("Anim");
        if (lastIndex < 0)
            return string;
        String tail = string.substring(lastIndex).replaceFirst("Anim", "0");
        return string.substring(0, lastIndex) + tail;
    }
    @Keep public static native void onLoadResult(String str);
    @Keep public static native void onNewList(String[] ids, String[] icons);
}
