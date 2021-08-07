public class GarbageCollection {

	public static void main(String[] args) {
	    System.gc();
	    new Object();
	    Object o = new Object();
	    new Object();
	    System.gc();
	    System.gc();

        System.out.println(1);
        System.gc();
        System.out.println("foo");
        System.gc();
        System.out.println(2);
        System.gc();
        System.gc();
	}
}
