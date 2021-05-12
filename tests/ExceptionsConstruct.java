public class ExceptionsConstruct {
    public static void println(int i) { System.out.println(i); }

    static class CustomException extends Exception {
        public int code;
    }

    public static CustomException create() {
        CustomException e = new CustomException();
        e.code = 123;
        return e;
    }

    public static void crash(CustomException e) throws CustomException {
        throw e;
    }

    public static void main(String[] args) throws CustomException {
        CustomException e = create();
        // e.fillInStackTrace();
        crash(e);
    }
}
