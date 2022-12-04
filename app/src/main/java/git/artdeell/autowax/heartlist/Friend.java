package git.artdeell.autowax.heartlist;

import java.util.Objects;

public class Friend {
    public final String id;
    public final String nickname;
    public Friend(String id, String nickname) {
        this.id = id;
        this.nickname = nickname;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Friend friend = (Friend) o;
        return id.equals(friend.id);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }
}
