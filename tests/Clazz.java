public class Clazz {
    static void println(String i) { System.out.println(i); }
    static void println(boolean i) { System.out.println(i); }

	public static void main(String[] args) {
        println(Object.class.getName());
        println(Integer.class.getName());
        println(int.class.getName());
        println(int[][].class.getName());
        println(int[][].class.getComponentType().getName());

        String s = "xyz";
        Class<?> stringClass = s.getClass();
        println(stringClass.getName());
        println(stringClass == String.class);
        println(stringClass == Class.class);
        println(stringClass instanceof Class);

        int[][] i = new int[1][2];
        Class<?> iiArrayClass = i.getClass();
        println(iiArrayClass.getName());
        println(iiArrayClass == int[][].class);
        println(iiArrayClass instanceof Class);
	}
}
