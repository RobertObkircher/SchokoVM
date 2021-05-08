public class Native {
    public static void println(int i) { System.out.println(i); }
    public static void println(long i) { System.out.println(i); }
    public static void println(double v) { System.out.println(Double.doubleToLongBits(v)); }
    public static void println(float v) { System.out.println(Float.floatToIntBits(v)); }
    public static void println(char c) { System.out.println((int) c); }
    public static void println(short s) { System.out.println(s); }
    public static void println(byte b) { System.out.println(b); }
    public static void println(boolean z) { System.out.println(z); }

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

    static {
        // TODO this is a hack to detect if we are not running in SchokoVM
        if (System.out != null) {
            System.loadLibrary("NativeLib");
        }
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
    }
}
