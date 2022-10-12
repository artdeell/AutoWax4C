package git.artdeell.autowax;

public class SkyProtocolException extends Exception{
    public SkyProtocolException(String s) {
        super(s);
    }
}
class LostSessionException extends SkyProtocolException {
    public LostSessionException(String s) {
        super(s);
    }
}