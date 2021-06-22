import java.lang.reflect.Array;
import java.lang.reflect.Modifier;

public class ReflectionTest {
    public static void println(int i) { System.out.println(i); }
    public static void println(boolean i) { System.out.println(i); }
    public static void println(String i) { System.out.println(i); }

    public static void main(String[] args) {
        int[] arrInt = (int[]) Array.newInstance(int.class, 10);
        println(arrInt.getClass().getName());
        println(arrInt.getClass() == int[].class);
        for(int i = 0; i < 10; i++) {
            arrInt[i] = 2 * i;
        }
        for(int i = 0; i < 10; i++) {
           println(arrInt[i]);
        }

        String[] arrString = (String[]) Array.newInstance(String.class, 2);
        println(arrString.getClass().getName());
        println(arrString.getClass() == String[].class);
        arrString[0] = "a";
        arrString[1] = "b";
        for(int i = 0; i < 2; i++) {
           println(arrString[i]);
        }

        println(A.class.getModifiers());
        println(B.class.getModifiers());
        println(C.class.getModifiers());
        println(D.class.getModifiers());
        println(E.class.getModifiers());
        println(F.class.getModifiers());
        println(B[].class.getModifiers());
        println(int.class.getModifiers());
        println(int[].class.getModifiers());
        println(ReflectionTest.class.getModifiers());
    }

    private static class A {}
    protected static class B {}
    public abstract static class C {}
    static class D {}
    static enum E {}
    static interface F {}
}
