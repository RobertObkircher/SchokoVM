public class Bitwise {
    static void println(int i) { System.out.println(i); }
    static void println(long l) { System.out.println(l); }

    public static void main(String[] args) {
        checkInt(0, 9309232);
        checkInt(-1, 939402);
        checkInt(983232, 9812354);
        checkLong(983232, 9812354);
        checkLong(939393983232L, 39393939812354L);
        checkLong(0, 9309232);
        checkLong(0, 9392209309232L);
        checkLong(-1, 939402939022L);
    }

    static void checkInt(int a, int b) {
        println(a & b);
        println(a | b);
        println(a ^ b);
    }

    static void checkLong(long a, long b) {
        println(a & b);
        println(a | b);
        println(a ^ b);
    }
}
