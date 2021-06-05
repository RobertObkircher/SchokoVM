public class Strings {
    static void println(int i) { System.out.println(i); }
    static void println(char i) { System.out.println((int) i); }
    static void println(String i) { System.out.println(i); }

    public static void main(String[] args) {
        String s = "ü🐡abcdef";
        println(s.length());
        println(s);

        println(s.charAt(0));
        println(s.charAt(1));
        println(s.charAt(2));
        println(s.charAt(3));
        println(s.charAt(4));

        // TODO because it compiles to
        // InvokeDynamic #0:makeConcatWithConstants:(Ljava/lang/String;)Ljava/lang/String;
        // s += "xyz";

        // println(s.length());
        // println(s);
    }
}
