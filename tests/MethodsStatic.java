public class MethodsStatic {
    public static void println(int i) { System.out.println(i); }

    public static void main(String[] args) {
        println(InnerInterface.bar(15));

        println(Inner.foo(15));
        println(InnerSub.foo(15));
    }

    static class Inner {
        public static int foo(int i) {
            return 2 * i;
        }
    }

    static class InnerSub extends Inner {}

    static interface InnerInterface {
        public static int bar(int i) {
            return -2 * i;
        }
    }
}
