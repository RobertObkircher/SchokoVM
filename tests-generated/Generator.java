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

        generate(path, "ArithmeticLongAdd", w -> generateArithmeticLong(w, "+"));
        generate(path, "ArithmeticLongSub", w -> generateArithmeticLong(w, "-"));
        generate(path, "ArithmeticLongMul", w -> generateArithmeticLong(w, "*"));
        generate(path, "ArithmeticLongDiv", w -> generateArithmeticLong(w, "/"));
    }

    public static void generate(Path directory, String name, Consumer<PrintWriter> generator) {
        try (BufferedWriter bw = Files.newBufferedWriter(directory.resolve(name + ".java"));
             PrintWriter w = new PrintWriter(bw)
        ) {
            w.println("public class " + name + " {");
            generator.accept(w);
            w.println("}");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void generateArithmeticInt(PrintWriter w, String op1) {
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

        w.println("public static void println(int i) { System.out.println(i); } ");

        w.println(BEGIN_MAIN);

        // we use variables to make sure that javac doesn't fold literals
        w.println("        int a, b, c;");

        for (String op : new String[] {"+", "-", "*", "/"}) {
        for (int i : numbers) {
            w.println("        a = " + i + "; //////////////////////////////");
            for (int j : numbers) {
                // TODO emit a try catch ArithmeticException instead
                if (op != "/" || j != 0) {
                    w.println("        b = " + j + ";");
                    w.println("        c = a " + op + " b;");
                    w.println("        println(c);");
                }
            }
        }
        }
        w.println(END_MAIN);
    }

    public static void generateArithmeticLong(PrintWriter w, String op1) {
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
            for (int i = 0; i < 10; ++i) {
                numbers.add(random.nextLong());
            }

            w.println("public static void println(long i) { System.out.println(i); } ");

            w.println(BEGIN_MAIN);

            // we use variables to make sure that javac doesn't fold literals
            w.println("        long a, b, c;");

            for (String op : new String[] {"+", "-", "*", "/"}) {
            for (long i : numbers) {
                w.println("        a = " + i + "L; //////////////////////////////");
                for (long j : numbers) {
                    // TODO emit a try catch ArithmeticException instead
                    if (op != "/" || j != 0) {
                        w.println("        b = " + j + "L;");
                        w.println("        c = a " + op + " b;");
                        w.println("        println(c);");
                    }
                }
            }
            }
            w.println(END_MAIN);
        }
}