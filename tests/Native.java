public class Native {
    public static void println(int i) { System.out.println(i); }
    public static void println(long i) { System.out.println(i); }
    public static void println(double v) { System.out.println(Double.doubleToLongBits(v)); }
    public static void println(float v) { System.out.println(Float.floatToIntBits(v)); }
    public static void println(char c) { System.out.println((int) c); }
    public static void println(short s) { System.out.println(s); }
    public static void println(byte b) { System.out.println(b); }
    public static void println(boolean z) { System.out.println(z); }
    public static void println(String z) { System.out.println(z); }

    public static native long return42();
    public static native boolean returnTrue();
    public static native long returnId(long id);
    public static native long xor(long first, long second);
    public static native boolean intsEqual(int x, int y);
    public static native double plus(double d, float f);
    public static native void vvvvvvvvv();
    public static native void setValue(int i);
    public static native int getValue();
    public static native double sum(int i, double d, float f, byte c, long  l);

    public native int first(int a, int b, int c);
    public native int second(int a, int b, int c);
    public native int third(int a, int b, int c);

    public static native int overloaded(int a);
    public static native int overloaded(long b);
    public static native String overloaded(String s);
    public static native int overloaded(int[] a);

    public static native byte[] strings(String str, int start, int len);
    public static native byte[] strings8(String str, int start, int len);

    public static native int summm(
                  int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10,
                  int a11, int a12, int a13, int a14, int a15, int a16, int a17, int a18, int a19
    );

    static {
        System.loadLibrary("NativeLib");
    }

    public static void main(String[] args) {
        println(return42());
        println(returnTrue());
        println(returnId(777L));
        println(xor(777L, 839L));
        println(intsEqual(1, 2));
        println(intsEqual(2, 2));
        println(plus(2.0, 29.0f));
        println(getValue());
        vvvvvvvvv();
        println(getValue());
        setValue(50);
        println(getValue());
        vvvvvvvvv();
        println(getValue());
        println(sum(1, 40.3, 99.8f, (byte)5, 8439854389543L));
        println(4);

        Native n = new Native();
        println(n.first(1,2,3));
        println(n.second(1,2,3));
        println(n.third(1,2,3));

        println(summm(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19));

        println(overloaded(123));
        println(overloaded(123L));
        println(overloaded("abcdef"));
        println(overloaded(new int[]{1, 2, 3, 4, 5}));
        String x = "Hell😀\u00a5ü🐡ao, world!";
        println(x);
        println(java.util.Arrays.toString(strings(x, 3, 10)));
        println(java.util.Arrays.toString(strings8(x, 3, 10)));
    }
}
