package git.artdeell.aw4c;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
public class KeyVerification {

    public static final String KEY_HOST = "https://beyer-ka.de/aw4c-api";
    public static byte[] doGet(String request) throws IOException, KeyException {
        HttpURLConnection connection = (HttpURLConnection) new URL(KEY_HOST+request).openConnection();
        connection.setRequestMethod("GET");
        connection.setDoOutput(true);
        connection.connect();
        if(connection.getResponseCode() != 200) {
            connection.disconnect();
            throw new KeyException(connection.getResponseCode());
        }
        int byteArrayOutputLength = connection.getContentLength() == -1 ? 32 : connection.getContentLength();
        byte[] buffer = new byte[128];
        int readCount;
        InputStream inputStream = connection.getInputStream();
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream(byteArrayOutputLength);
        while((readCount = inputStream.read(buffer)) != -1) {
            outputStream.write(buffer, 0, readCount);
        }
        connection.disconnect();
        return outputStream.toByteArray();
    }


    public static byte[] testUserId(String skyUserId) throws IOException, KeyException {
        return doGet("/test?user="+skyUserId);
    }
    public static void enrollUserId(String skyUserId, String key) throws IOException, KeyException {
        doGet("/enroll_key?user="+skyUserId+"&key="+key);
    }

    static class KeyException extends Exception {
        public static final int KEY_SERVER_ERROR = 0;
        public static final int KEY_NOT_FOUND = 1;
        public static final int KEY_INSUFFICENT = 2;
        public static final int KEY_UNKNOWN_FAILURE = 3;
        public static final int KEY_IOE = 4;
        public final int failReason;
        public KeyException(int httpErrorCode) {
            switch (httpErrorCode) {
                case 404:
                    failReason = KEY_NOT_FOUND;
                    break;
                case 500:
                    failReason = KEY_SERVER_ERROR;
                    break;
                case 402:
                    failReason = KEY_INSUFFICENT;
                    break;
                default:
                    failReason = KEY_UNKNOWN_FAILURE;
                    break;
            }
        }
    }
}
