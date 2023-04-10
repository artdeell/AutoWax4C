package git.artdeell.autowax.iap;

import android.content.res.Resources;

import git.artdeell.aw4c.ContextOps;
import git.artdeell.aw4c.Locale;
import local.json.JSONArray;
import local.json.JSONObject;

import git.artdeell.autowax.AutoWax;

public class IapPurchase {
    private static IapPurchase iapPurchase;
    private final AutoWax host;

    public IapPurchase(AutoWax host) {
        this.host = host;
    }

    public static IapPurchase get(AutoWax host) {
        if(iapPurchase == null) iapPurchase = new IapPurchase(host);
        return iapPurchase;
    }

    public void reload(String id) {
        try {
            JSONObject iap_rq = host.genInitial();
            iap_rq.put("platform", id);
            JSONObject iap_result = host.doPost("/account/iaplist", iap_rq);
            Resources skyResources = ContextOps.skyPName != null ? ContextOps.getSkyResources() : null;
            if(!iap_result.isNull("iap_list")) {
                JSONArray iap_list = iap_result.getJSONArray("iap_list");
                String[] iap_ids = new String[iap_list.length()];
                String[] iap_names = new String[iap_list.length()];
                String[] iap_icons = new String[iap_list.length()];
                for(int i = 0; i < iap_list.length(); i++) {
                    JSONObject iap_obj = iap_list.getJSONObject(i);
                    String name = iap_obj.optString("name", "");
                    String iap_id = iap_obj.getString("id");
                    if(!name.isEmpty() && skyResources != null) {
                        int locale_id = skyResources.getIdentifier(name, "string", ContextOps.skyPName);
                        if(locale_id != -1) name = skyResources.getString(locale_id);
                    }
                    if(name.isEmpty()) name = iap_id;
                    iap_ids[i] = iap_id;
                    iap_names[i] = name;
                    iap_icons[i] = iap_obj.optString("icon", "{{");
                }
                onIapList(iap_ids, iap_names, iap_icons);
                return;
            }
        }catch (Exception e) {
            e.printStackTrace();
        }
        onIapList(null, null, null);
    }

    public void buy(String platform, String id) {
        try {
            JSONObject purchase_rq = host.genInitial();
            purchase_rq.put("platform", "fake");
            purchase_rq.put("receipt", "com.tgc.sky.android.test.gold." + id);
            purchase_rq.put("target_pid", id);
            purchase_rq.put("target_uid", purchase_rq.get("user"));
            purchase_rq.put("restore", false);
            JSONObject result = host.doPost("/account/commerce/receipt", purchase_rq);
            if(!result.isNull("transactions")) {
                JSONArray tranactions = result.getJSONArray("transactions");
                if(tranactions.isEmpty()) {
                    onIapResult(Locale.get(Locale.I_MISSING_TRANSACTIONS));
                }
                StringBuilder transactionResultBuilder = new StringBuilder();
                for(int i = 0; i < tranactions.length(); i++) {
                    JSONObject transaction = tranactions.getJSONObject(i);
                    transactionResultBuilder.append(Locale.get(Locale.I_TRANSACTION_RESULT, transaction.optString("product_id", "?"), transaction.optString("status", "?")));
                    transactionResultBuilder.append('\n');
                }
                onIapResult(transactionResultBuilder.toString());
            }else{
                onIapResult(Locale.get(Locale.I_MISSING_TRANSACTIONS));
            }
        }catch (Exception e) {
            onIapResult(e.toString());
        }
    }


    public static native void onIapList(String[] iaps, String[] names, String[] icons);
    public static native void onIapResult(String result);
}
