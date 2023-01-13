package git.artdeell.autowax.spiritshop;

public class LevelCouple {
    public final long levelId;
    public float lastScrollPosition;

    public LevelCouple(long levelId, float lastScrollPosition) {
        this.levelId = levelId;
        this.lastScrollPosition = lastScrollPosition;
    }
}
