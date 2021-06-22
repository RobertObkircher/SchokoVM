public class Clone {
    static class X implements Cloneable {
        int x;
        long y;
        String z;

        @Override
        public X clone() throws CloneNotSupportedException {
            return (X) super.clone();
        }
    }

    static class NotSupported {
        @Override
        public X clone() throws CloneNotSupportedException {
            return (X) super.clone();
        }
    }

    public static void main(String[] args) throws CloneNotSupportedException {
        X x = new X();
        x.x = 1;
        x.y = 12124L;
        x.z = "ABC";

        X y = x.clone();
        System.out.println(y.x);
        System.out.println(y.y);
        System.out.println(y.z);

        NotSupported ns = new NotSupported();
        try {
            ns.clone();
            System.out.println("no!");
        } catch(CloneNotSupportedException e) {
            System.out.println(e.getMessage());
        }

        int[] arr = new int[]{1,2,3,4,5,10000};
        int[] arr2 = arr.clone();
        System.out.println(arr.length);
        System.out.println(arr2.length);
        for(int i : arr2) {
            System.out.println(i);
        }
    }
}
