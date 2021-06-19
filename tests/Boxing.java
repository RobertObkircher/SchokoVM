public class Boxing {
    static void println(int i) { System.out.println(i); }
    static void println(long i) { System.out.println(i); }

    public static void main(String[] args) {
        int i = 123;
        Integer ib = i;
        println(ib);

        long l = 123L;
        Long lb = l;
        println(lb);
    }
}
