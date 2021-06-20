public class ArrayCopy {
    public static void println(boolean i) { System.out.println(i); }
    public static void println(int i) { System.out.println(i); }

    static class A {}
    static class B extends A {
        public int value;
        public B(int value) {
            this.value = value;
        }
    }

    public static void main(String[] args) {
        int[] arr1Int = { 0, 1, 2, 3, 4, 5 };
        int[] arr2Int = { 10, 20, 30, 40, 50 };

        System.arraycopy(arr1Int, 2, arr2Int, 1, 2);
        for(int i = 0; i < 5; i++) {
            println(arr2Int[i]);
        }

        System.arraycopy(arr2Int, 2, arr2Int, 3, 2);
        for(int i = 0; i < 5; i++) {
            println(arr2Int[i]);
        }

        B[] arr1Object = { new B(0), new B(1), new B(2) };
        A[] arr2Object = { new A(), new A(), new A() };

        System.arraycopy(arr1Object, 0, arr2Object, 0, 2);
        println(((B) (arr2Object[0])).value);
        println(((B) (arr2Object[1])).value);
        println(arr2Object[2].getClass() == A.class);

        System.arraycopy(arr2Object, 0, arr2Object, 1, 2);
        println(((B) (arr2Object[0])).value);
        println(((B) (arr2Object[1])).value);
        println(((B) (arr2Object[2])).value);
    }
}
