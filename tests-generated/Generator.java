import java.io.BufferedWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.function.Consumer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;
import java.util.List;

public class Generator {
    public static final String BEGIN_MAIN =  "    public static void main(String[] args) {";
    public static final String END_MAIN =  "    }";

    public static void main(String[] args) {
        Path path = Paths.get(args[0]);
        path.toFile().mkdirs();

        // Split into 4 files so that it is easier to see if only one of them fails
        generate(path, "ArithmeticIntAdd", w -> generateArithmeticInt(w, "+"));
        generate(path, "ArithmeticIntSub", w -> generateArithmeticInt(w, "-"));
        generate(path, "ArithmeticIntMul", w -> generateArithmeticInt(w, "*"));
        generate(path, "ArithmeticIntDiv", w -> generateArithmeticInt(w, "/"));
        generate(path, "ArithmeticIntRem", w -> generateArithmeticInt(w, "%"));
        generate(path, "ArithmeticIntShiftLeft", w -> generateArithmeticInt(w, "<<"));
        generate(path, "ArithmeticIntShiftRight", w -> generateArithmeticInt(w, ">>"));
        generate(path, "ArithmeticIntShiftRightU", w -> generateArithmeticInt(w, ">>>"));
        generate(path, "ArithmeticIntNeg", w -> generateArithmeticUnary(w, "-", Integer.class));

        generate(path, "ArithmeticLongAdd", w -> generateArithmeticLong(w, "+"));
        generate(path, "ArithmeticLongSub", w -> generateArithmeticLong(w, "-"));
        generate(path, "ArithmeticLongMul", w -> generateArithmeticLong(w, "*"));
        generate(path, "ArithmeticLongDiv", w -> generateArithmeticLong(w, "/"));
        generate(path, "ArithmeticLongRem", w -> generateArithmeticLong(w, "%"));
        generate(path, "ArithmeticLongShiftLeft", w -> generateArithmeticLong(w, "<<"));
        generate(path, "ArithmeticLongShiftRight", w -> generateArithmeticLong(w, ">>"));
        generate(path, "ArithmeticLongShiftRightU", w -> generateArithmeticLong(w, ">>>"));
        generate(path, "ArithmeticLongNeg", w -> generateArithmeticUnary(w, "-", Long.class));

        generate(path, "ArithmeticFloatAdd", w -> generateArithmeticFloating(w, "+", Float.class));
        generate(path, "ArithmeticFloatSub", w -> generateArithmeticFloating(w, "-", Float.class));
        generate(path, "ArithmeticFloatMul", w -> generateArithmeticFloating(w, "*", Float.class));
        generate(path, "ArithmeticFloatDiv", w -> generateArithmeticFloating(w, "/", Float.class));
        generate(path, "ArithmeticFloatRem", w -> generateArithmeticFloating(w, "%", Float.class));
        generate(path, "ArithmeticFloatNeg", w -> generateArithmeticUnary(w, "-", Float.class));

        generate(path, "ArithmeticDoubleAdd", w -> generateArithmeticFloating(w, "+", Double.class));
        generate(path, "ArithmeticDoubleSub", w -> generateArithmeticFloating(w, "-", Double.class));
        generate(path, "ArithmeticDoubleMul", w -> generateArithmeticFloating(w, "*", Double.class));
        generate(path, "ArithmeticDoubleDiv", w -> generateArithmeticFloating(w, "/", Double.class));
        generate(path, "ArithmeticDoubleRem", w -> generateArithmeticFloating(w, "%", Double.class));
        generate(path, "ArithmeticDoubleNeg", w -> generateArithmeticUnary(w, "-", Double.class));

        generate(path, "ComparisonsInt", w -> generateComparisons(w, Integer.class));
        generate(path, "ComparisonsLong", w -> generateComparisons(w, Long.class));

        generate(path, "Conversions", Generator::generateConversions);

        generate(path, "Goto", Generator::generateGoto);

        generate(path, "InvokeStaticPermutations", Generator::generateInvokeStaticPermutations);

        generate(path, "Wide", Generator::generateWide);

        generate(path, "Arrays", Generator::generateArrays);
    }

    public static void generate(Path directory, String name, Consumer<PrintWriter> generator) {
        try (BufferedWriter bw = Files.newBufferedWriter(directory.resolve(name + ".java"));
             PrintWriter w = new PrintWriter(bw)
        ) {
            w.println("public class " + name + " {");
            w.println("    public static void println(int i) { System.out.println(i); } ");
            w.println("    public static void println(long i) { System.out.println(i); } ");
            w.println("    public static void println(double v) { System.out.println(Double.doubleToLongBits(v)); } ");
            w.println("    public static void println(float v) { System.out.println(Float.floatToIntBits(v)); } ");
            w.println("    public static void println(char c) { System.out.println((int) c); } ");
            w.println("    public static void println(short s) { System.out.println(s); } ");
            w.println("    public static void println(byte b) { System.out.println(b); } ");
            w.println("    public static void println(boolean z) { System.out.println(z); } ");
            generator.accept(w);
            w.println("}");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void generateArithmeticInt(PrintWriter w, String op) {
        List<Integer> numbers = new ArrayList<>(Arrays.asList(new Integer[]{
            0, -1, -2, 1, 2,
            2147483647, 2147483646,
            -2147483648, -2147483647,
            127, -128,
            322, -322,
            2314908, -2314908,
            231490893, -194322323
        }));

        Random random = new Random(32498); // fixed seed
        for (int i = 0; i < 20; ++i) {
            numbers.add(random.nextInt());
        }

        w.println(BEGIN_MAIN);

        // we use variables to make sure that javac doesn't fold literals
        w.println("        int a, b, c;");

        for (int i : numbers) {
            w.println("        a = " + i + "; //////////////////////////////");
            for (int j : numbers) {
                // TODO emit a try catch ArithmeticException instead
                if (!((op == "/" || op == "%") && j == 0)) {
                    w.println("        b = " + j + ";");
                    w.println("        c = a " + op + " b;");
                    w.println("        println(c);");
                }
            }
        }
        w.println(END_MAIN);
    }

    public static void generateArithmeticLong(PrintWriter w, String op) {
        List<Long> numbers = new ArrayList<>(Arrays.asList(new Long[]{
            0L, -1L, -2L, 1L, 2L,
            9223372036854775807L, 9223372036854775806L,
            -9223372036854775808L, -9223372036854775807L,
            127L, -128L,
            322L, -322L,
            2314908L, -2314908L,
            231490893L, -194322323L
        }));

        Random random = new Random(23148); // fixed seed
        for (int i = 0; i < 20; ++i) {
            numbers.add(random.nextLong());
        }

        w.println(BEGIN_MAIN);

        // we use variables to make sure that javac doesn't fold literals
        w.println("        long a, b, c;");

        for (long i : numbers) {
            w.println("        a = " + i + "L; //////////////////////////////");
            for (long j : numbers) {
                // TODO emit a try catch ArithmeticException instead
                if (!((op == "/" || op == "%") && j == 0)) {
                    w.println("        b = " + j + "L;");
                    w.println("        c = a " + op + " b;");
                    w.println("        println(c);");
                }
            }
        }
        w.println(END_MAIN);
    }

    public static void generateArithmeticFloating(PrintWriter w, String op, Class<?> type) {
        List<Object> numbers = new ArrayList<>(Arrays.asList(new Object[]{
            0.0, -1.0, -2.0, 1.0, 2.0,
            127.0, -128.0,
            322.0, -322.0,
            2314908.0, -2314908.0,
            231490893.0, -194322323.0
        }));
        addBoundaryValues(type, numbers);

        w.println(BEGIN_MAIN);

        // we use variables to make sure that javac doesn't fold literals
        w.println("        " + toPrimitiveType(type) + " a, b, c;");

        for (Object i : numbers) {
            w.println("        a = " + toLiteral(type, i) + "; //////////////////////////////");
            for (Object j : numbers) {
                // TODO emit a try catch ArithmeticException instead
                if (!((op == "/" || op == "%") && ((Number)j).doubleValue() == 0)) {
                    w.println("        b = " + toLiteral(type, j) + ";");
                    w.println("        c = a " + op + " b;");
                    w.println("        println(c);");
                }
            }
        }
        w.println(END_MAIN);
    }

    public static void generateComparisons(PrintWriter w, Class<?> type) {
        List<Object> numbers = new ArrayList<>(Arrays.asList(new Object[]{
            0L, -1L, -2L, 1L, 2L,
            127L, -128L,
            231490893L, -194322323L
        }));
        addBoundaryValues(type, numbers);

        w.println(BEGIN_MAIN);

        // we use variables to make sure that javac doesn't fold literals
        w.println("        " + toPrimitiveType(type) + " a, b;");

        for (String op : new String[] {"==", ">=", "<=", "<", ">", "!="}) {
        for (Object i : numbers) {
            w.println("        a = " + toLiteral(type, i) + "; //////////////////////////////");
            for (Object j : numbers) {
                w.println("        b = " + toLiteral(type, j) + ";");
                w.println("        if(a " + op + " b)");
                w.println("          println(0);");
                w.println("        else");
                w.println("          println(1);");
            }
        }
        }
        w.println(END_MAIN);
    }

    static class Function {
        String name;
        boolean[] parameters;
    }

    public static void generateInvokeStaticPermutations(PrintWriter w) {
        Random random = new Random(2943148); // fixed seed

        ArrayList<Function> functions = new ArrayList<>();

        int id = 0;

        for (int count = 0; count < 5; ++count) {
           for (int i = 0; i < (1 << count); ++i) {
                Function f = new Function();
                f.name = "f" + (id++);
                f.parameters = new boolean[count];
                for (int j = 0; j < count; ++j) {
                    f.parameters[j] = (i & (1 << j)) != 0;
                }
                functions.add(f);
           }
        }

        w.println(BEGIN_MAIN);
        for (Function f : functions) {
            w.print("        " + f.name + "(");
            for (int i = 0; i < f.parameters.length; ++i) {
                if (i > 0) w.print(", ");
                if (f.parameters[i]) {
                    w.print(random.nextLong());
                    w.print("L");
                } else {
                    w.print(random.nextInt());
                }
            }
            w.println(");");
        }
        w.println(END_MAIN);

        for (Function f : functions) {
            w.print("    static void " + f.name + "(");
            for (int i = 0; i < f.parameters.length; ++i) {
                if (i > 0) w.print(", ");
                w.print(f.parameters[i] ? "long p" : "int p");
                w.print(i);
            }
            w.println(") {");
            for (int i = 0; i < f.parameters.length; ++i) {
                w.println("        println(p" + i +");");
            }
            w.println("    }");
        }
    }

    public static void generateArithmeticUnary(PrintWriter w, String operator, Class<?> type) {
        List<Object> numbers = new ArrayList<>(Arrays.asList(new Object[]{
            0L, -1L, -2L, 1L, 2L,
            127L, -128L,
            322L, -322L,
            2314908L, -2314908L,
            231490893L, -194322323L
        }));
        addBoundaryValues(type, numbers);

        w.println(BEGIN_MAIN);
         w.println("        " + toPrimitiveType(type) + " a;");
        for (Object i : numbers) {
            w.println("        a = " + toLiteral(type, i) + ";");
            w.println("        println(-a);");
        }
        w.println(END_MAIN);
    }

    public static String toLiteral(Class<?> type, Object i) {
        if(type == Float.class) {
            float f = ((Number) i).floatValue();
            if(Float.isNaN(f)) {
                return "(1.0f / 0.0f)";
            } else if(Float.isInfinite(f)) {
                if(f > 0) {
                    return "(1.0f / 0.0f)";
                } else {
                    return "(-1.0f / 0.0f)";
                }
            } else {
                return Float.toHexString(f) + "f";
            }
        } else if(type == Double.class) {
            double d = ((Number) i).doubleValue();
            if(Double.isNaN(d)) {
                return "(1.0d / 0.0d)";
            } else if(Double.isInfinite(d)) {
                if(d > 0) {
                    return "(1.0d / 0.0d)";
                } else {
                    return "(-1.0d / 0.0d)";
                }
            } else {
                return Double.toHexString(d) + "d";
            }
        } else if(type == Long.class) {
            long l = (long) i;
            return l + "L";
        } else if(type == Integer.class){
            return i.toString();
        } else {
            throw new RuntimeException("Unreachable");
        }
    }
    public static String toPrimitiveType(Class<?> type) {
        if(type == Float.class) {
            return "float";
        } else if(type == Double.class) {
            return "double";
        } else if(type == Long.class) {
            return "long";
        } else {
            return "int";
        }
    }

    public static void addBoundaryValues(Class<?> type, List<Object> numbers){
        Random random = new Random(23148); // fixed seed
        if(type == Float.class) {
            numbers.add(Float.MAX_VALUE);
            numbers.add(Float.MAX_VALUE - 1);
            numbers.add(Float.MIN_VALUE);
            numbers.add(Float.MIN_VALUE + 1);
            numbers.add(Float.MIN_NORMAL);
            numbers.add(Float.NEGATIVE_INFINITY);
            numbers.add(Float.POSITIVE_INFINITY);
            numbers.add(Float.NaN);
            for (int i = 0; i < 8; ++i) {
                numbers.add(random.nextFloat());
            }
        } else if(type == Double.class) {
            numbers.add(Double.MAX_VALUE);
            numbers.add(Double.MAX_VALUE - 1);
            numbers.add(Double.MIN_VALUE);
            numbers.add(Double.MIN_VALUE + 1);
            numbers.add(Double.MIN_NORMAL);
            numbers.add(Double.NEGATIVE_INFINITY);
            numbers.add(Double.POSITIVE_INFINITY);
            numbers.add(Double.NaN);
            for (int i = 0; i < 8; ++i) {
                numbers.add(random.nextDouble());
            }
        } else if(type == Long.class) {
            numbers.add(Long.MAX_VALUE);
            numbers.add(Long.MAX_VALUE - 1);
            numbers.add(Long.MIN_VALUE);
            numbers.add(Long.MIN_VALUE + 1);
            for (int i = 0; i < 8; ++i) {
                numbers.add(random.nextLong());
            }
        } else {
            numbers.add(Integer.MAX_VALUE);
            numbers.add(Integer.MAX_VALUE - 1);
            numbers.add(Integer.MIN_VALUE);
            numbers.add(Integer.MIN_VALUE + 1);
            for (int i = 0; i < 8; ++i) {
                numbers.add(random.nextInt());
            }
        }
    }

    public static void generateGoto(PrintWriter w) {
        w.println(BEGIN_MAIN);
        w.println("        for (int i = 0; i < 10; ++i) println(i);");
        w.println("        gotoW();");
        w.println(END_MAIN);

        w.println("    static void gotoW() {");
        w.println("        int x;");
        w.println("        for (int i = 0; i < 20; ++i) {"); // goto_w
        for (int i = 0; i < 15000; ++i)
            w.println("        x = 42;");
        w.println("        }"); // goto_w
        w.println("    }");
    }

    public static void generateConversions(PrintWriter w) {
        Random random = new Random(2943148); // fixed seed

        List<Object> ints = new ArrayList<>(Arrays.asList(new Integer[]{
            0, -1, -2, 1, 2,
            2147483647, 2147483646,
            -2147483648, -2147483647,
            127, -128,
            128, -129,
            322, -322,
            2314908, -2314908,
            231490893, -194322323
        }));
        addBoundaryValues(Integer.class, ints);
        for (int i = 0; i < 10; ++i)
            ints.add(random.nextInt());

        List<Object> longs = new ArrayList<>(Arrays.asList(new Long[]{
            0L, -1L, -2L, 1L, 2L,
            9223372036854775807L, 9223372036854775806L,
            -9223372036854775808L, -9223372036854775807L,
            127L, -128L,
            322L, -322L,
            2314908L, -2314908L,
            231490893L, -194322323L
        }));
        addBoundaryValues(Long.class, longs);
        for (int i = 0; i < 10; ++i)
            longs.add(random.nextLong());

        List<Object> floats = new ArrayList<>(Arrays.asList(new Float[]{
            0.0f, -1.0f, -2.0f, 1.0f, 2.0f,
            0.1f, 0.9f, 1.1f,
            -0.1f, -0.9f, -1.1f,
            127.0f, -128.0f,
            322.0f, -322.0f,
            2314908.0f, -2314908.0f,
            231490893.0f, -194322323.0f
        }));
        addBoundaryValues(Float.class, floats);
        for (int i = 0; i < 10; ++i)
            floats.add(random.nextFloat());

        List<Object> doubles = new ArrayList<>(Arrays.asList(new Double[]{
            0.0, -1.0, -2.0, 1.0, 2.0,
            0.1, 0.9, 1.1,
            -0.1, -0.9, -1.1,
            127.0, -128.0,
            322.0, -322.0,
            2314908.0, -2314908.0,
            231490893.0, -194322323.0
        }));
        addBoundaryValues(Double.class, doubles);
        for (int i = 0; i < 10; ++i)
            floats.add(random.nextDouble());

        w.println(BEGIN_MAIN);
        for (Object i : ints)
            w.println("        checkint(" + toLiteral(Integer.class, i) + ");");
        for (Object l : longs)
            w.println("        checklong(" + toLiteral(Long.class, l) + ");");
        for (Object f : floats)
            w.println("        checkfloat(" + toLiteral(Float.class, f) + ");");
        for (Object d : doubles)
            w.println("        checkdouble(" + toLiteral(Double.class, d) + ");");
        w.println(END_MAIN);

        for (String type : new String[] {"int", "long", "float", "double" }) {
            w.println("    public static void check" + type + "(" + type + " value) {");
            w.println("        println((int) value);");
            w.println("        println((long) value);");
            w.println("        println((float) value);");
            w.println("        println((double) value);");
            w.println("        println((char) value);");
            w.println("        println((short) value);");
            w.println("        println((byte) value);");
//             w.println("        println((boolean) value);"); makes no sense
            w.println("    }");
        }

    }

    public static void generateWide(PrintWriter w) {
        w.println(BEGIN_MAIN);

        // wide iinc (because of constant)
        w.println("        int inc1 = 4;");
        w.println("        inc1 -= 200;");
        w.println("        println(inc1);");

        for (int i = 0; i < 256; ++i)
            w.println("        int dummy" + i + " = " + i + ";");

        // wide stores
        w.println("        int i = 111;");
        w.println("        long l = 838292;");
        w.println("        float f = 12.0f;");
        w.println("        Object a = null;");
        w.println("        double d = 94.0;");

        // wide loads
        w.println("        println(i);");
        w.println("        println(l);");
        w.println("        println(f);");
        w.println("        println(a == null ? 0 : 1);");
        w.println("        println(d);");

        // wide iinc (because of variable)
        w.println("        i += 10;");
        w.println("        println(i);");

        w.println(END_MAIN);
    }

    public static void generateArrays(PrintWriter w) {
        w.println(BEGIN_MAIN);
            
        for (String type : new String[] {"boolean", "byte", "char", "int", "long", "float", "double" }) {
            w.println("        test_" + type + "();");
        }

        w.println(END_MAIN);

        for (String type : new String[] {"byte", "char", "int", "long", "float", "double" }) {
            w.println("    public static void test_" + type + "(){");
            w.println("        "+type+"[] arr = new "+type+"[20];");
            w.println("        for(int i = 0; i < arr.length; i++) {");
            w.println("            arr[i] = ("+type+") (100 - i);");
            w.println("        }");
            w.println("        int sum = 0;");
            w.println("        for(int i = arr.length - 1; i >= 0; i--) {");
            w.println("            sum += arr[i];");
            w.println("        }");
            w.println("        println(sum);");
            w.println("    }");
        }
        w.println("    public static void test_boolean(){");
        w.println("        boolean[] arr = new boolean[20];");
        w.println("        for(int i = 0; i < arr.length; i++) {");
        w.println("            arr[i] = (i % 2) == 0;");
        w.println("        }");
        w.println("        int sum = 0;");
        w.println("        for(int i = arr.length - 1; i >= 0; i--) {");
        w.println("            sum += arr[i] ? 1 : 0;");
        w.println("        }");
        w.println("        println(sum);");
        w.println("    }");
    }
}
