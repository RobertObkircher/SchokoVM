import java.util.Properties;

public class PropertiesTest {
    public static void main(String[] args) {
        Properties props = new Properties();
        props.setProperty("User", "xyz");
        System.out.println(props.getProperty("User"));
    }
}
