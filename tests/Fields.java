public class Fields {
    static void println(byte    x) { System.out.println(x); }
    static void println(short   x) { System.out.println(x); }
    static void println(char    x) { System.out.println(x); }
    static void println(int     x) { System.out.println(x); }
    static void println(float   x) { System.out.println(x); }
    static void println(long    x) { System.out.println(x); }
    static void println(double  x) { System.out.println(x); }

    static class MyObject {
        byte b;
        short s;
        char c;
        int i;
        float f;
        long l;
        double d;
        MyObject next;
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
