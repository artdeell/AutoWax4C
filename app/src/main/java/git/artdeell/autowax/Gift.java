package git.artdeell.autowax;

import java.util.Objects;

public class Gift {
    final String targetId;
    String giftType;
    final String username;
    public Gift(String targetId, String giftType, String username) {
        this.targetId = targetId;
        this.giftType = giftType;
        this.username = username;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Gift that = (Gift) o;
        return targetId.equals(that.targetId) && giftType.equals(that.giftType);
    }
    @Override
    public int hashCode() {
        return Objects.hash(targetId, giftType);
    }
}
