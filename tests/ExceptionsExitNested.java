public class ExceptionsExitNested {
    public static void println(int i) { System.out.println(i); }

    static class CustomException extends Exception {
    }

    public static void c() throws CustomException {
        throw new CustomException();
    }

    public static void b() throws CustomException {
        c();
    }

    public static void a() throws CustomException {
        println(2);
        b();
        println(3);
    }

    public static void main(String[] args) throws CustomException {
        println(4);
        a();
        println(5);
    }
}
