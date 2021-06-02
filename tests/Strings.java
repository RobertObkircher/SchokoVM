public class Strings {
    static void println(int i) { System.out.println(i); }
    static void println(boolean i) { System.out.println(i); }
    static void println(String i) { System.out.println(i); }

    public static void main(String[] args) {
        String s = "abcdef";
        println(s.length());
        println(s);

        // TODO because it compiles to
        // InvokeDynamic #0:makeConcatWithConstants:(Ljava/lang/String;)Ljava/lang/String;
        // s += "xyz";

        // println(s.length());
        // println(s);
    }
}
