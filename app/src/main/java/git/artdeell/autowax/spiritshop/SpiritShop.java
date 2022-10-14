package git.artdeell.autowax.spiritshop;

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
    private final Stack<Long> stack = new Stack<>();
    private String[] elementStrings, nameStrings;
    private long[] gotos;
    public static SpiritShop get(AutoWax host) {
        if(spiritShop == null) spiritShop = new SpiritShop(host);
        return spiritShop;
    }
    private SpiritShop(AutoWax host) {
        this.host = host;
    }
    public void init() {
        try {
            JSONArray resp = host.doPost("/account/get_spirit_shops", host.genInitial()).optJSONArray("spirit_shops");
            if(resp == null) initDone(Locale.get(Locale.SS_NO_LIST));
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
            }
            stack.push(0L);
            processTopStack();
            initDone(null);
        }catch (Exception e) {
            initDone(e.toString());
        }
    }
    private void processTopStack() {
        long stackTop = stack.peek();
        if(tradeLists.containsKey(stackTop)) {
            ArrayList<Element> tradeList = tradeLists.get(stackTop);
            elementStrings = new String[tradeList.size()];
            nameStrings = new String[tradeList.size()];
            gotos = new long[tradeList.size()];
            for(int i = 0; i < tradeList.size(); i++) {
                Element element = tradeList.get(i);
                nameStrings[i] = element.nm;
                elementStrings[i] = Locale.get(Locale.SS_CURRENCY_STRING, element.cst, element.ctyp, element.ap?"yes":"no");
                gotos[i] = element.id;
            }
        }else{
            nameStrings = elementStrings = new String[0];
            gotos = new long[0];
        }
        newList(elementStrings, nameStrings, gotos);
    }
    public void pushLevel(long id) {
       stack.push(id);
       processTopStack();
    }
    public void popLevel() {
        if(stack.peek() != 0) stack.pop();
        processTopStack();
    }
    public void purchase(long id) {
        JSONObject rq = host.genInitial();
        rq.put("unlock_id", id);
        try {
            purchaseResult(host.doPost("/account/purchase_spirit_shop_item", rq).optString("result","Unknown"));
        }catch (Exception e) {
            purchaseResult(e.toString());
        }
    }
    public static native void purchaseResult(String result);
    public static native void newList(String[] elementStrings, String[] nameStrings, long[] gotos);
    public static native void initDone(String result);
}
