public class Exceptions {
    public static void println(int i) { System.out.println(i); }

    static class CustomException extends Exception {
        public int code;
        public CustomException() {
            this.code = code;
        }
    }

    public static void crash() throws CustomException {
        CustomException e = new CustomException();
        e.code = 123;
        throw e;
    }

    public static void crash_catch() throws CustomException {
        try {
            CustomException e = new CustomException();
            e.code = 123;
            throw e;
        } catch (CustomException e){
            println(e.code);
        }
    }

    public static void middle() throws CustomException {
        try {
            crash();
        } catch (CustomException e){
            println(e.code);
        }
    }

    public static void main(String[] args) throws CustomException {
        println(1);
        crash_catch();
        println(2);
        middle();
        println(3);
        crash();
        println(4);
    }
}
