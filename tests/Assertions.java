public class Assertions {
    static void println(int i) { System.out.println(i); }

    static {
        println(6);
    }

    public static void main(String[] args) {
        int i = 123;
        assert i == 123;
        println(i);
    }
}
