public class Methods {
    public static void println(long i) { System.out.println(i); }
    public static void println(int i) { System.out.println(i); }

    public static void main(String[] args) {
        Inner inner = new Inner();
        println(inner.add(1, 2, 3));
    }

    static class Inner {
        public static long add(long i, int j, long k) {
            println(i);
            println(j);
            println(k);
            return i + j + k;
        }
    }
}
