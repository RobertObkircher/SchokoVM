public class Strings {
    static void println(int i) { System.out.println(i); }
    static void println(char i) { System.out.println((int) i); }
    static void println(String i) { System.out.println(i); }

    public static void main(String[] args) {
        // TODO the classfile parser doesn't handle this emoji correctly?
        // String s = "🐡abcdef";
        String s = "üabcdef";
        println(s.length());
        println(s);

        println(s.charAt(0));
        println(s.charAt(1));
        println(s.charAt(2));

        // TODO because it compiles to
        // InvokeDynamic #0:makeConcatWithConstants:(Ljava/lang/String;)Ljava/lang/String;
        // s += "xyz";

        // println(s.length());
        // println(s);
    }
}
