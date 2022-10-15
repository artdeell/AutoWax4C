package git.artdeell.autowax.invitemanager;

import android.util.Base64;

import java.util.UUID;

import git.artdeell.aw4c.ContextOps;

public class InviteUtils {
    public static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                    + Character.digit(s.charAt(i+1), 16));
        }
        return data;
    }
    public static void copyInvite(Invite link) {
        byte[] token = hexStringToByteArray(link.token_id.replace("-",""));
        System.out.println("Encoding invite: "+ UUID.nameUUIDFromBytes(token).toString());
        String token_b64 = Base64.encodeToString(token,Base64.URL_SAFE | Base64.NO_PADDING);
        ContextOps.setClipboard("https://sky.thatg.co/?i="+token_b64);
    }
}
