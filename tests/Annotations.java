import java.lang.annotation.*;

public class Annotations {
    @Target(ElementType.FIELD)
    @Retention(RetentionPolicy.RUNTIME)
    static @interface Stable {}

    @Stable
    public static int x = 2;

    public static void main(String[] args) {

    }
}
