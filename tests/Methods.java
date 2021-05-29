public class Methods {
    public static void println(int i) { System.out.println(i); }

    public static void main(String[] args) {
        Other obj = new SubOther();
        println(obj.add(3, 5));
        println(obj.value);
        println(obj.getValue());
        println(obj.invokePriv());

        SubOther subObj = new SubOther();
        println(subObj.runInterface1(19));
        println(subObj.runInterface2(19));

        InvokeInterface i = (InvokeInterface) new InvokeInterfaceSub();
        i.foo();
    }

    static class Other {
        public int value = 4;

        public static int add(int a, int b) {
            println(a);
            println(b);
            return a + b;
        }

        public int getValue() {
            return this.value;
        }

        public void setValue(int value) {
            this.value = value;
        }

        public int invokePriv() {
            return this.priv();
        }

        private int priv() {
            return 1;
        }
    }
    static interface OtherInterface {
        public default int runInterface1(int i) {
            return i + 1;
        }

        public default int runInterface2(int i) {
            return i + 2;
        }
    }
    static class SubOther extends Other implements OtherInterface {
        private int priv() {
            return 2;
        }

        public int getValue() {
            return this.value + 100;
        }

        public int runInterface2(int i) {
            return i + 20;
        }
    }

    static interface InvokeInterface {
        public void foo();
    }
    static class InvokeInterfaceSub implements InvokeInterface {
        public void foo(){
            println(1);
        }
    }
}
