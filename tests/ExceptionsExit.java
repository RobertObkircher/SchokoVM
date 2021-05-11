public class ExceptionsExit {
    public static void println(int i) { System.out.println(i); }

    static class CustomException extends Exception {
    }

    public static void main(String[] args) throws CustomException {
        throw new CustomException();
    }
}
