public class Clazz {
    static void println(String i) { System.out.println(i); }
    static void println(boolean i) { System.out.println(i); }

	public static void main(String[] args) {
//         TODO native code doesn't do class loading yet
        String temp = "a";

        println(Object.class.getName());
        println(Integer.class.getName());
        println(int.class.getName());
        println(int[][].class.getName());
	}
}
