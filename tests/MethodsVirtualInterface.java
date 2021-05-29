// Test case for "maximally-specific superinterface methods"
// from https://jvilk.com/blog/java-8-wtf-ambiguous-method-lookup/
public class MethodsVirtualInterface {
    public static void println(int i) { System.out.println(i); }

    public static void main(String[] args) {
        new EmptySpeakImpl().speak();
        new EmptySpeakImplChild().speak();
    }

    static interface ISpeak1 {
        default void speak() {
            println(1);
        }
    }

    static interface ISpeak2 extends ISpeak1 {
        default void speak() {
            println(2);
        }
    }

    static class EmptySpeakImpl implements ISpeak2 {}
    static class EmptySpeakImplChild extends EmptySpeakImpl implements ISpeak1 {}
}
