public class ReferenceComparisons {
    static void println(int i) { System.out.println(i); }
    static void println(long l) { System.out.println(l); }
    static void println(boolean z) { System.out.println(z); }

    public static void main(String[] args) {
        Object x = null;
        Object y = null; // aconst_null + astore_2

        boolean a = x == y; // if_acmpne
        println(a);

        boolean b = x != y; // if_acmpeq
        println(b);

        boolean c = x == null; // ifnonnull
        println(c);

        boolean d = x != null; // ifnull
        println(d);

    }

}
