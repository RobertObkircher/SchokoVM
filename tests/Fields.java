public class Fields {
    public static void println(int i) { System.out.println(i); }
    public static void println(long i) { System.out.println(i); }
    public static void println(double v) { System.out.println(Double.doubleToLongBits(v)); }
    public static void println(float v) { System.out.println(Float.floatToIntBits(v)); }
    public static void println(char c) { System.out.println((int) c); }
    public static void println(short s) { System.out.println(s); }
    public static void println(byte b) { System.out.println(b); }
    public static void println(boolean z) { System.out.println(z); }

    static class MyObject implements MyInterface {
        boolean bool;
        byte b;
        short s;
        char c;
        int i;
        float f;
        long l;
        double d;
        MyObject next;
    }

    public interface MyInterface {
        int erface = 0;
        double interfaceDouble = 0.0;
    }

    static class MyObjectChild extends MyObject {
        boolean child1;
        int child2;
    }

    static class MyStatic {
        static boolean bool;
        static byte b;
        static short s;
        static char c;
        static int i;
        static float f;
        static long l;
        static double d;
        static MyObject next;

        public static void print() {
            println(bool);
            println(b);
            println(s);
            println(c);
            println(i);
            println(f);
            println(l);
            println(d);

            if (next != null) {
                printMyObject(next);
            } else {
                println(3333333333333.333333);
            }
        }
    }

    public static void main(String[] args) {
        MyObject object1 = new MyObject();

        // initial values
        printMyObject(object1);

        object1.b = 2;
        object1.s = 3;
        object1.c = 4;
        object1.i = 5;
        object1.f = 6f;
        object1.l = 7L;
        object1.d = 8.0;

        // changed values
        printMyObject(object1);

        MyObject object2 = new MyObject();

        // unchanged / initial
        printMyObject(object1);
        printMyObject(object2);

        object1.next = object2;
        object2.i = 999;

        // with changes
        printMyObject(object1);
        printMyObject(object2);

        // fields in child class
        MyObjectChild object3 = new MyObjectChild();
        printMyObjectChild(object3);
        object3.child1 = true;
        object3.child2 = 42;
        printMyObjectChild(object3);

        // initial static values
        MyStatic.print();

        MyStatic.b = 2;
        MyStatic.s = 3;
        MyStatic.c = 4;
        MyStatic.i = 5;
        MyStatic.f = 6f;
        MyStatic.l = 7L;
        MyStatic.d = 8.0;
        MyStatic.next = object1;

        // changed static values
        MyStatic.print();

        // interface constants
        println(object1.erface);
        println(object1.interfaceDouble);
        println(MyObject.erface);
        println(MyObject.interfaceDouble);
        println(object2.erface);
        println(object2.interfaceDouble);
        println(MyObjectChild.erface);
        println(MyObjectChild.interfaceDouble);
    }

    public static void printMyObjectChild(MyObjectChild o) {
        println(o.child1);
        println(o.child2);
        printMyObject(o);
    }

    public static void printMyObject(MyObject o) {
        println(o.b);
        println(o.s);
        println(o.c);
        println(o.i);
        println(o.f);
        println(o.l);
        println(o.d);

        if (o.next != null) {
            printMyObject(o.next);
        } else {
            println(3333333333333.333333);
        }
    }

}
