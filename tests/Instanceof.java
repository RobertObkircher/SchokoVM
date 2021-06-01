public class Instanceof {
    static void println(int i) { System.out.println(i); }

    public static void main(String[] args) {
        check(new A());
        check(new B());
        check(new C());
        check(new D());
        //check(new Object());
        check(new X());
        check(new boolean[3]);
        check(new A[3]);
        check(new B[3]);
    }

    static void check(Object o) {
        if (o instanceof Object) { println(1); }
        if (o instanceof A) { println(1); }
        if (o instanceof B) { println(2); }
        if (o instanceof C) { println(3); }
        if (o instanceof D) { println(4); }
        if (o instanceof I) { println(5); }
        if (o instanceof J) { println(6); }
        if (o instanceof K) { println(7); }
        if (o instanceof L) { println(8); }
        if (o instanceof X) { println(9); }
        if (o instanceof boolean[]) { println(10); }
        if (o instanceof A[]) { println(11); }
        if (o instanceof B[]) { println(12); }
        if (o instanceof C[]) { println(13); }
        if (o instanceof D[]) { println(14); }
        if (o instanceof I[]) { println(15); }
        if (o instanceof J[]) { println(16); }
        if (o instanceof K[]) { println(17); }
        if (o instanceof L[]) { println(18); }
        if (o instanceof X[]) { println(19); }
        if (o instanceof Object[]) { println(20); }

        // checkcast fail
        try {
            String s = (String) o;
            println(10);
        } catch(ClassCastException e) {
            println(-10);
        }

        // checkcast success (sometimes)
        try {
            B b = (B) o;
            println(11);
        } catch(ClassCastException e) {
            println(-11);
        }
    }

    static class A {};
    static class B extends A{};

    static interface I {};
    static interface J {};

    static interface K extends I {};
    static interface L extends J, K {};

    static class C extends B implements L {};
    static class D extends A implements J, K {};

    static class X {};
}
