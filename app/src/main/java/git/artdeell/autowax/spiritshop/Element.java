package git.artdeell.autowax.spiritshop;

import local.json.JSONObject;

public class Element {
    private static final byte dev_fmt = 0;
    public final long id;
    public final long dep;
    public final String ctyp;
    public final String nm;
    public final int cst;
    public final boolean ap;
    public Element(JSONObject obj) {
        if(dev_fmt == 0) {
            this.id = obj.getLong("id");
            this.dep = obj.getLong("dep");
            this.nm = obj.getString("nm");
            this.ctyp = obj.getString("ctyp");
            this.cst = obj.getInt("cst");
            this.ap = obj.getBoolean("ap");
        }else if(dev_fmt == 1){
            this.id = obj.getLong("item_id");
            this.dep = obj.getLong("dependency");
            this.nm = obj.getString("item_name");
            this.ctyp = obj.getString("currency_type");
            this.cst = obj.getInt("unlock_cost");
            this.ap = obj.getBoolean("adventure_pass");
        }else throw new RuntimeException();
    }
}
