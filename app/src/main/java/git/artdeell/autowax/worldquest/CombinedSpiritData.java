package git.artdeell.autowax.worldquest;

public class CombinedSpiritData {
    final String quest_name;
    final String collectible_name;
    String icon;

    public CombinedSpiritData(String questId, String collectible_name, String icon) {
        this.quest_name = questId;
        this.collectible_name = collectible_name;
        this.icon = icon;
    }
}
