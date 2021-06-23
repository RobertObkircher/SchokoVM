public class ObjectArray {
    static class MyObject {
        int i;
        long l;
    }

    public static void main(String[] args) {
        MyObject[] arr = new MyObject[10];

        for(int i = 0; i < arr.length; i++) {
            System.out.println(arr == null);
        }

        for(int i = 0; i < arr.length; i++) {
            arr[i] = new MyObject();
            arr[i].i = 100 - i;
            arr[i].l = 100 * i;
        }
        for(int i = arr.length - 1; i >= 0; i--) {
            System.out.println(arr[i].i);
            System.out.println(arr[i].l);
        }

        MyObject[] copy = arr.clone();
        System.out.println(copy.length == copy.length);
        for(int i = 0; i < arr.length; i++) {
            System.out.println(arr[i] == copy[i]);
        }
    }
}
