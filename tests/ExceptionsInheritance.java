public class ExceptionsInheritance {
    public static void println(int i) { System.out.println(i); }

    static class CustomException extends Exception {
        static {
            println(10000);
        }
    }
    static class CustomExceptionA extends CustomException {
        static {
            println(10001);
        }
    }
    static class CustomExceptionB extends CustomException {
        static {
            println(10002);
        }
    }

    public static void throw_sub() throws CustomException {
        throw new CustomExceptionA();
    }

    public static void throw_super() throws CustomException {
        throw new CustomException();
    }

    public static void catch_sub() {
        try {
            throw_sub();
        } catch (CustomExceptionA e){
            println(100);
        } catch (CustomExceptionB e){
            println(101);
        } catch (CustomException e){
            println(102);
        } finally {
            println(103);
        }
    }

    public static void catch_sub_super() {
        try {
            throw_sub();
        } catch (CustomException e){
            println(202);
        } finally {
            println(203);
        }
    }

    public static void catch_super() {
        try {
            throw_super();
        } catch (CustomExceptionA e){
            println(300);
        } catch (CustomExceptionB e){
            println(301);
        } catch (CustomException e){
            println(302);
        } finally {
            println(303);
        }
    }

    public static void main(String[] args)  {
        catch_sub();
        catch_sub_super();
        catch_super();
    }
}
