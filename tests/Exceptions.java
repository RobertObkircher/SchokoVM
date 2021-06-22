public class Exceptions {
    public static void println(int i) { System.out.println(i); }
    public static void println(String s) { System.out.println(s); }

    static class CustomException extends Exception {
        public int code;
    }

    public static void crash() throws CustomException {
        CustomException e = new CustomException();
        e.code = 123;
        e.printStackTrace();
        throw e;
    }

    public static void crash_catch() throws CustomException {
        try {
            CustomException e = new CustomException();
            e.code = 123;
            throw e;
        } catch (CustomException e){
            println(e.code);
        } finally {
            println(100);
        }
    }

    public static void middle() throws CustomException {
        try {
            crash();
        } catch (CustomException e){
            println(e.code);
            e.printStackTrace();
        }
    }

    static class IncludeInitializerOfNonException {
    	IncludeInitializerOfNonException() {
    		throw new RuntimeException();
    	}
    }

    static class IncludeInitializerOfException extends Exception {
    	IncludeInitializerOfException() throws Exception {
    		throw new Exception();
    	}
    }

    public static CustomException createWithooutThrowing() {
        CustomException e = new CustomException();
        e.code = 9843;
        return e;
    }

    public static void throwWithStacktraceFromCreation(CustomException e) throws CustomException {
        throw e;
    }

    public static void main(String[] args) throws CustomException {
        try {
            println(1);
        } finally {
            println(101);
        }
        crash_catch();
        try {
            println(1);
        } finally {
            println(102);
        }
        println(2);
        middle();
        println(3);

        Object x = null;
        try {
            println(x.toString());
        } catch (NullPointerException npe) {
            println("Got a npe: ");
            println(npe.toString());
            npe.printStackTrace();
        }

        try {
            new IncludeInitializerOfNonException();
        } catch (RuntimeException ex) {
            ex.printStackTrace();
        }

        try {
            new IncludeInitializerOfException();
        } catch (IncludeInitializerOfException ex) {
            ex.printStackTrace();
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        try {
            throw new IncludeInitializerOfException();
        } catch (IncludeInitializerOfException ex) {
            ex.printStackTrace();
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        CustomException custom = createWithooutThrowing();
        try {
            throwWithStacktraceFromCreation(custom);
        } catch(CustomException e) {
            e.printStackTrace();
        }
    }
}
