import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.util.List;
import java.util.Arrays;

public class InvokeMethodHandle {
    static void println(boolean i) { System.out.println(i); }
    static void println(int i) { System.out.println(i); }
    static void println(String i) { System.out.println(i); }

	public static void main(String[] args) throws Throwable {
	    // from https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/lang/invoke/MethodHandle.html
        String s; int i;
        MethodType mt; MethodHandle mh;
        MethodHandles.Lookup lookup = MethodHandles.lookup();

        // mt is (char,char)String
        mt = MethodType.methodType(String.class, char.class, char.class);
        mh = lookup.findVirtual(String.class, "replace", mt);
        s = (String) mh.invokeExact("daddy",'d','n');
        // invokeExact(Ljava/lang/String;CC)Ljava/lang/String;
        println(s); //assertEquals(s, "nanny");

        // weakly typed invocation (using MHs.invoke)
        s = (String) mh.invokeWithArguments("sappy", 'p', 'v');
        println(s); //assertEquals(s, "savvy");

        // mt is (Object[])List
        mt = MethodType.methodType(java.util.List.class, Object[].class);
        mh = lookup.findStatic(java.util.Arrays.class, "asList", mt);
        assert(mh.isVarargsCollector());
        List<String> x = (List<String>) mh.invoke("one", "two");
        // invoke(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;
        println(x.equals(Arrays.asList("one","two")));

        // mt is (Object,Object,Object)List
        mt = MethodType.genericMethodType(3).changeReturnType(java.util.List.class);
        System.out.println(mt);
        mh = mh.asType(mt);
        List<Object> y = (List<Object>) mh.invokeExact((Object)1, (Object)2, (Object)3);
        // invokeExact(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;
        println(y.equals(Arrays.asList(1, 2, 3)));
        // mt is ()int

        mt = MethodType.methodType(int.class);
        mh = lookup.findVirtual(java.util.List.class, "size", mt);
        i = (int) mh.invokeExact(java.util.Arrays.asList(1,2,3));
        // invokeExact(Ljava/util/List;)I
        println(i); // assert(i == 3);

        mt = MethodType.methodType(void.class, String.class);
        mh = lookup.findVirtual(java.io.PrintStream.class, "println", mt);
        mh.invokeExact(System.out, "Hello, world.");
        // invokeExact(Ljava/io/PrintStream;Ljava/lang/String;)V
    }
}
