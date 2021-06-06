public class Empty {
    static void println(String i) { System.out.println(i); }
    static void println(boolean i) { System.out.println(i); }

    static class I {
		public static String get(){
			return "abc";
		}
	}

	public static void main(String[] args) {
		String a = "abc";
		String b = I.get();
		println(a);
		println(b);
		println(a == b);
	}
}
