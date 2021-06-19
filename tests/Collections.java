import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Collections {
    static void println(int i) { System.out.println(i); }
    static void println(String i) { System.out.println(i); }

    public static void main(String[] args){
        Map<String, String> map = new ConcurrentHashMap<>();
        map.put("abc", "A");
        map.put("xyz", "X");

        println(map.size());
        println(map.get("abc"));
    }
}
