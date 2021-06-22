public class GarbageCollection {

	public static void main(String[] args) {
	    System.gc();
	    new Object();
	    Object o = new Object();
	    new Object();
	    System.gc();
	    System.gc();
	}
}
