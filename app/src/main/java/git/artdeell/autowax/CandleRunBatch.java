package git.artdeell.autowax;

import local.json.JSONObject;

public class CandleRunBatch implements Runnable{
    final AutoWax host;
    final JSONObject runBatch;
    public CandleRunBatch(AutoWax host, JSONObject runBatch) {
        this.host = host;
        this.runBatch = runBatch;
    }

    @Override
    public void run() {
        byte attempts=0;
        while(attempts < 6) {
            try {
                host.doPost("/account/collect_pickup_batch", runBatch);
                attempts = 99;
            } catch (SkyProtocolException e) {
                e.printStackTrace();
                attempts++;
            }
        }
    }
}
