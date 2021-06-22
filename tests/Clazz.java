public class Clazz {
	public static void main(String[] args) {
        System.out.println(Object.class.getName());
        System.out.println(Integer.class.getName());
        System.out.println(int.class.getName());
        System.out.println(int[][].class.getName());
        System.out.println(int[][].class.getComponentType().getName());

        String s = "xyz";
        Class<?> stringClass = s.getClass();
        System.out.println(stringClass.getName());
        System.out.println(stringClass == String.class);
        System.out.println(stringClass == Class.class);
        System.out.println(stringClass instanceof Class);

        int[][] i = new int[1][2];
        Class<?> iiArrayClass = i.getClass();
        System.out.println(iiArrayClass.getName());
        System.out.println(iiArrayClass == int[][].class);
        System.out.println(iiArrayClass instanceof Class);

        System.out.println(int.class.getModule().getName());
        System.out.println(String.class.getModule().getName());
        System.out.println(String[].class.getModule().getName());
	}
}
