public class Empty {
    static void println(String i) { System.out.println(i); }
    static void println(boolean i) { System.out.println(i); }

	public static void main(String[] args) {
       // TODO native code doesn't do class loading yet
        String s = "a";
        Class<?> a = Object.class;
        println(a.getName());
	}
}
