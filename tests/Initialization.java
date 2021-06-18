public class Initialization {
    static void println(String s) { System.out.println(s); }

    public static void main(String[] args) {
        J.bar();
        println("===");
        new C();
    }

    static interface I {
        int i = I.foo();
        static int foo(){
            println("I.foo");
            return 1;
        }
    }

    static interface ShouldAlsoBeInitializedWithC {
        int i = ShouldAlsoBeInitializedWithC.baz();
        static int baz(){
            println("ShouldAlsoBeInitializedWithC.baz");
            return 1;
        }

        default int nonAbstractNonStatic() {
            return 4;
        }
    }

    static interface ShouldBeInitializedWithC extends ShouldAlsoBeInitializedWithC {
        int i = baz();
        static int baz(){
            println("ShouldBeInitializedWithC.baz");
            return 1;
        }

        default int nonAbstractNonStatic() {
            return 4;
        }
    }

    static interface J extends I, ShouldBeInitializedWithC {
        static void bar(){
            println("J.bar");
        }
    }

    static class C implements J {
        static void bar(){
            println("C.bar");
        }
    }
}

