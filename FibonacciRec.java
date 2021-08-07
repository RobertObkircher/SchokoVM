public class FibonacciRec {
    public static void main(String[] args) {
        System.out.println(fibonacci(Integer.valueOf(args[0])));
    }

    private static int fibonacci(int a) {
        if (a == 1 || a == 2) return 1;
        else return fibonacci(a - 1) + fibonacci(a - 2);
    }
}
