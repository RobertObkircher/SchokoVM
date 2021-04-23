public class InvokeStatic {
    public static void println(long i) { System.out.println(i); }
    public static void println(int i) { System.out.println(i); }

    public static void main(String[] args) {
        println(f());
        println(g());
        a();
    }

    static long f() {
        return 943043;
    }

    static int g() {
        long l = 3;
        int i = 5;
        return i + (int)l;
    }

    public static void a() {
        println(3);
        println(b());
        println(Inner.ili(-9332, -393939339393l, 8882822));
        println(4);
    }

    private static long b() {
        println(5);
        ili(1932001,909233210332L,1291293);
        println(6);
        ill(33291,19393293202L,329849324324L);
        println(Inner.ill(3932923,99393333L,92932));
        println(7);
        iil(19323201,10392932,1392392303L);
        println(8);
        return -1;
    }

    public static void ili(int i, long j, int k) {
        println(i);
        println(j);
        println(k);
        ill(i,j,j);
        ill(0,3,5);
        ill(i,j,j);
    }

    public static void ill(int i, long j, long k) {
        long l = k;
        println(i);
        println(j);
        println(l);
        iil(999,888,777);
        iil(i,i,k);
    }

    public static void iil(int i, int j, long k) {
        println(i);
        println(j);
        println(k);
        println(Inner.ili(-2, 393939339393l, 282822));
    }

    static class Inner {
        public static int ili(int i, long j, int k) {
            println(i);
            println(j);
            println(k);
            println(ill(i,j,j));
            println(ill(3939,-3,3935));
            println(ill(i,j,j));
            return 8188;
        }

        public static long ill(int i, long j, long k) {
            println(i);
            println(j);
            println(k);
            iil(-999,-888,-777);
            iil(i,i,k);
            return -8333330;
        }

        public static void iil(int i, int j, long k) {
            println(i);
            println(j);
            println(k);
        }
    }

}
