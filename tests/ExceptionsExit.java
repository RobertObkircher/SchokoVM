public class ExceptionsExit {
    public static void println(int i) { System.out.println(i); }

    static class CustomException extends Exception {
        public int code;
        public CustomException() {
            this.code = code;
        }
    }

    public static void main(String[] args) throws CustomException {
        CustomException e = new CustomException();
        e.code = 123;
        throw e;
    }
}
