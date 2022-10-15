package git.artdeell.autowax.invitemanager;

public class Invite {
    public final String token_id;
    public final String nickname;
    public Invite(String token_id, String nickname) {
        this.token_id = token_id;
        this.nickname = nickname;
    }
}
