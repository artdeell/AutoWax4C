package git.artdeell.autowax.heartlist;


import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import git.artdeell.autowax.AutoWax;
import git.artdeell.aw4c.ContextOps;
import git.artdeell.aw4c.Locale;
import local.json.JSONArray;
import local.json.JSONObject;

public class HeartList {
    private final AutoWax host;
    private final List<Friend> friendList = new ArrayList<>();
    private Set<String> friendsEnabled;

    private static HeartList heartList;
    public static HeartList get(AutoWax host) {
        if(heartList == null) heartList = new HeartList(host);
        return heartList;
    }

    public HeartList(AutoWax host) {
        this.host = host;
    }

    public void reload() {
        try {
            friendList.clear();
            JSONObject request = host.genInitial();
            request.put("max", 512);
            request.put("sort_ver", 1);
            JSONArray friendStatues = host.doPost("/account/get_friend_statues", request).optJSONArray("set_friend_statues");
            if(friendStatues == null) {
                pushList(null);
                return;
            }
            String[] rawNameList = new String[friendStatues.length()];
            for(int i = 0; i < friendStatues.length(); i++) {
                JSONObject friend = friendStatues.getJSONObject(i);
                String nickname = friend.optString("nickname");
                friendList.add(new Friend(friend.getString("friend_id"), nickname));
                rawNameList[i] = nickname != null ? nickname : Locale.get(Locale.FL_UNNAMED);
            }
            pushList(rawNameList);
            resendEnableList();
        }catch (Exception e) {
            Log.d("aw4c", "cond 1 fail", e);
            pushList(null);
        }
    }
    public void resendEnableList() {
        friendsEnabled = ContextOps.sharedPreferences.getStringSet("trade_buddies", new HashSet<>());
        boolean[] enabledList = new boolean[friendList.size()];
        for(int i = 0; i < enabledList.length; i++) {
            Friend f = friendList.get(i);
            enabledList[i] = friendsEnabled.contains(f.id);
        }
        pushEnableList(enabledList);
    }

    @SuppressLint("ApplySharedPref")
    public void add(int idx) {
        friendsEnabled.add(friendList.get(idx).id);
        ContextOps.sharedPreferences.edit().remove("trade_buddies").commit();
        ContextOps.sharedPreferences.edit().putStringSet("trade_buddies", friendsEnabled).commit();
        resendEnableList();
    }

    @SuppressLint("ApplySharedPref")
    public void rem(int idx) {
        friendsEnabled.remove(friendList.get(idx).id);
        ContextOps.sharedPreferences.edit().remove("trade_buddies").commit();
        ContextOps.sharedPreferences.edit().putStringSet("trade_buddies", friendsEnabled).commit();
        resendEnableList();
    }
    public static native void pushList(String[] names);
    public static native void pushEnableList(boolean[] booleans);
}
