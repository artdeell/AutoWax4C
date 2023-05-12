package git.artdeell.autowax.spiritshop;

import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Objects;
import java.util.Stack;

import git.artdeell.autowax.AutoWax;
import git.artdeell.aw4c.Locale;
import local.json.JSONArray;
import local.json.JSONObject;

public class SpiritShop {
    private static SpiritShop spiritShop;
    private final AutoWax host;
    private final HashMap<Long, ArrayList<Element>> tradeLists = new HashMap<>();
    private final Stack<LevelCouple> stack = new Stack<>();

    public static SpiritShop get(AutoWax host) {
        if(spiritShop == null) spiritShop = new SpiritShop(host);
        return spiritShop;
    }
    private SpiritShop(AutoWax host) {
        this.host = host;
    }
    public void init() {
        try {
            JSONArray resp = new JSONArray();
            long offset = 0;
            long pickedVersion = 0;
            while(true) {
                JSONObject spiritShopRq = host.genInitial();
                spiritShopRq.put("l", 1000);
                spiritShopRq.put("o", offset);
                spiritShopRq.put("v", pickedVersion);
                JSONObject fullRespone = host.doPost("/account/get_spirit_shops", spiritShopRq);
                JSONArray response = fullRespone.optJSONArray("spirit_shops");
                if(pickedVersion == 0 && fullRespone.has("spirit_shops_version")) {
                    pickedVersion = fullRespone.getInt("spirit_shops_version");
                }
                if(response != null) {
                    resp.putAll(response);
                    offset += response.length();
                    if(fullRespone.optLong("spirit_shops_total_count") <= offset) {
                        break;
                    }
                }else {
                    resp.clear();
                    break;
                }
            }

            if(resp.isEmpty()) {
                initDone(Locale.get(Locale.SS_NO_LIST));
                return;
            }
            for(Object o : resp) {
                Element element = new Element((JSONObject) o);
                if(tradeLists.containsKey(element.dep)) {
                    Objects.requireNonNull(tradeLists.get(element.dep)).add(element);
                }else{
                    ArrayList<Element> elements = new ArrayList<>();
                    elements.add(element);
                    tradeLists.put(element.dep, elements);
                }
            }
            if(!tradeLists.containsKey(0L)) {
                tradeLists.clear();
                initDone(Locale.get(Locale.SS_NO_ENTRYPOINT));
                return;
            }
            stack.push(new LevelCouple(0, 0.0f));
            processTopStack();
            initDone(null);
        }catch (Exception e) {
            initDone(e.toString());
        }
    }
    private void processTopStack() {
        LevelCouple stackTop = stack.peek();
        String[] elementStrings;
        String[] nameStrings;
        long[] gotos;
        if(tradeLists.containsKey(stackTop.levelId)) {
            ArrayList<Element> tradeList = tradeLists.get(stackTop.levelId);
            elementStrings = new String[tradeList.size()];
            nameStrings = new String[tradeList.size()];
            gotos = new long[tradeList.size()];
            final String yes = Locale.get(Locale.SS_YES);
            final String no = Locale.get(Locale.SS_NO);
            for(int i = 0; i < tradeList.size(); i++) {
                Element element = tradeList.get(i);
                nameStrings[i] = element.nm;
                elementStrings[i] = Locale.get(Locale.SS_CURRENCY_STRING, element.cst, element.ctyp, element.ap?yes:no);
                gotos[i] = element.id;
            }
        }else{
            nameStrings = elementStrings = new String[0];
            gotos = new long[0];
        }
        newList(elementStrings, nameStrings, gotos, stackTop.lastScrollPosition);
    }
    public void pushLevel(long id, float scrollPos) {
       stack.peek().lastScrollPosition = scrollPos;
       stack.push(new LevelCouple(id, 0));
       processTopStack();
    }
    public void popLevel() {
        if(stack.peek().levelId != 0) stack.pop();
        processTopStack();
    }
    public void purchase(long id) {
        JSONObject rq = host.genInitial();
        rq.put("unlock_id", id);
        try {
            purchaseResult(host.doPost("/account/purchase_spirit_shop_item", rq).optString("result",Locale.get(Locale.SS_UNKNOWN)));
        }catch (Exception e) {
            purchaseResult(e.toString());
        }
    }
    public static native void purchaseResult(String result);
    public static native void newList(String[] elementStrings, String[] nameStrings, long[] gotos, float scrollPos);
    public static native void initDone(String result);
}
