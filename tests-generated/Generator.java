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

        generate(path, "InvokeStaticPermutations", Generator::generateInvokeStaticPermutations);
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
        w.println("        " + toPrimiteType(type) + " a, b, c;");

        for (Object i : numbers) {
            w.println("        a = " + i + getLiteralPostfix(type) + "; //////////////////////////////");
            for (Object j : numbers) {
                // TODO emit a try catch ArithmeticException instead
                if (!((op == "/" || op == "%") && ((Number)j).doubleValue() == 0)) {
                    w.println("        b = " + j + getLiteralPostfix(type) + ";");
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
        w.println("        " + toPrimiteType(type) + " a, b;");

        for (String op : new String[] {"==", ">=", "<=", "<", ">", "!="}) {
        for (Object i : numbers) {
            w.println("        a = " + i + getLiteralPostfix(type) + "; //////////////////////////////");
            for (Object j : numbers) {
                w.println("        b = " + j + getLiteralPostfix(type) + ";");
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
         w.println("        " + toPrimiteType(type) + " a;");
        for (Object i : numbers) {
            w.println("        a = " + i + getLiteralPostfix(type) + ";");
            w.println("        println(-a);");
        }
        w.println(END_MAIN);
    }

    public static String getLiteralPostfix(Class<?> type) {
        if(type == Float.class) {
            return "f";
        } else if(type == Double.class) {
            return "d";
        } else if(type == Long.class) {
            return "L";
        } else {
            return "";
        }
    }
    public static String toPrimiteType(Class<?> type) {
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
            for (int i = 0; i < 8; ++i) {
                numbers.add(random.nextFloat());
            }
        } else if(type == Double.class) {
            numbers.add(Double.MAX_VALUE);
            numbers.add(Double.MAX_VALUE - 1);
            numbers.add(Double.MIN_VALUE);
            numbers.add(Double.MIN_VALUE + 1);
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
}
