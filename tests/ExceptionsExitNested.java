public class ExceptionsExitNested {
    public static void println(int i) { System.out.println(i); }

    static class CustomException extends Exception {
        public int code;
    }

    public static void c() throws CustomException {
        CustomException e = new CustomException();
        e.code = 123;
        throw e;
    }

    public static void b() throws CustomException {
        c();
    }

    public static void a() throws CustomException {
        b();
    }

    public static void main(String[] args) throws CustomException {
        a();
    }
}
