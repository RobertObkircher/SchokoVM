import java.util.Properties;

public class PropertiesTest {
    static void println(String i) { System.out.println(i); }

    public static void main(String[] args) {
        Properties props = new Properties();
        props.setProperty("User", "xyz");
        println(props.getProperty("User"));
    }
}
