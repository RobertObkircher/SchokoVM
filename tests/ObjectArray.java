public class ObjectArray {
    public static void println(int i) { System.out.println(i); } 
    public static void println(long i) { System.out.println(i); }

    static class MyObject {
        int i;
        long l;
    }

    public static void main(String[] args) {
        MyObject[] arr = new MyObject[10];

        for(int i = 0; i < arr.length; i++) {
            arr[i] = new MyObject();
            arr[i].i = 100 - i;
            arr[i].l = 100 * i;
        }
        for(int i = arr.length - 1; i >= 0; i--) {
            println(arr[i].i);
            println(arr[i].l);
        }
    }
}
