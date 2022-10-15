package git.artdeell.autowax.invitemanager;

import android.content.Context;

import java.util.ArrayList;

import git.artdeell.autowax.AutoWax;
import git.artdeell.aw4c.ContextOps;
import local.json.JSONArray;
import local.json.JSONObject;

public class InviteManager {
    private static InviteManager inviteManager;
    private final AutoWax host;
    private ArrayList<Invite> inviteList = new ArrayList<>();
    public static InviteManager get(AutoWax host) {
        if(inviteManager == null) inviteManager = new InviteManager(host);
        return inviteManager;
    }
    private InviteManager(AutoWax host) {
        this.host = host;
    }
    public void reload() {
        try {
            JSONObject rqst = host.doPost("/account/get_invites", host.genInitial());
            if (rqst.has("invites")) loadInvites(rqst.getJSONArray("invites"));
            putList();
        }catch (Exception e) {
            onInviteList(null);
        }
    }
    private void loadInvites(JSONArray array) {
        inviteList.clear();
        for (Object o : array) {
            JSONObject invite = (JSONObject) o;
            inviteList.add(new Invite(invite.getString("token_id"), invite.getString("nickname")));
        }
    }
    public void createInvite(String nickname) {
        try {
            JSONObject createRq = host.genInitial();
            createRq.put("invite_nickname", nickname);
            createRq = host.doPost("/account/create_invite", createRq);
            if(createRq.has("invites")) loadInvites(createRq.getJSONArray("invites"));
        }catch (Exception e) {
            e.printStackTrace();
        }
        putList();
    }
    public void deleteInvite(int idx) {
        try {
            JSONObject removeRq = host.genInitial();
            removeRq.put("token_id", inviteList.get(idx).token_id);
            removeRq = host.doPost("/account/delete_invite", removeRq);
            if(removeRq.has("invites")) loadInvites(removeRq.getJSONArray("invites"));
        }catch (Exception e) {
            e.printStackTrace();
        }
        putList();
    }
    public void copyInvite(int idx) {
        InviteUtils.copyInvite(inviteList.get(idx));
    }
    private void putList() {
        String[] inviteNames = new String[inviteList.size()];
        for(int i = 0; i < inviteNames.length; i++) {
            inviteNames[i] = inviteList.get(i).nickname;
        }
        onInviteList(inviteNames);
    }
    public static native void onInviteList(String[] invites);
}
