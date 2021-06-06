public class Strings {
    static void println(int i) { System.out.println(i); }
    static void println(char i) { System.out.println((int) i); }
    static void println(boolean i) { System.out.println(i); }
    static void println(String i) { System.out.println(i); }

    static class Inner {
		public static String get(){
			return "abc";
		}
	}

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

        String a = "abc";
        String b = Inner.get();
		println(a);
		println(b);
		println(a == b);
    }
}
