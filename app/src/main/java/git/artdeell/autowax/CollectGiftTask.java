package git.artdeell.autowax;

import git.artdeell.aw4c.CanvasMain;
import git.artdeell.aw4c.Locale;
import local.json.JSONObject;

public class CollectGiftTask implements Runnable {
    final AutoWax host;
    final long msgid;
    final String gtype;
    public CollectGiftTask(long msgid, String gtype, AutoWax host) {
        this.host = host;
        this.msgid = msgid;
        this.gtype = gtype;
    }
    @Override
    public void run() {
        boolean didFinish = false;
        byte retries = 0;
        while(!didFinish && retries < 6) {
            try {
                JSONObject req = host.genInitial();
                req.put("msg_id", msgid);
                JSONObject ret = host.doPost("/account/claim_message_gift", req);
                if(ret.has("result") && ret.getString("result").equals("ok")) {
                    didFinish = true;
                }else{
                    retries++;
                    CanvasMain.submitLogString(Locale.get(Locale.G_C_FAILED_RETRYING,msgid, "IncorrectResult"));
                }
            }catch (SkyProtocolException e) {
                retries++;
                CanvasMain.submitLogString(Locale.get(Locale.G_C_FAILED_RETRYING,msgid, e.toString()));
            }
            if(!didFinish) CanvasMain.submitLogString(Locale.get(Locale.G_C_FAILED, msgid));
            else CanvasMain.submitLogString(Locale.get(Locale.G_C_DONE, msgid) +" " + gtype);
        }
    }
}
