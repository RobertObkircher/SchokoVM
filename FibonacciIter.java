public class FibonacciIter {
    static long fib(int n) {
        long f[] = new long[n + 1];

        f[0] = 0;
        f[1] = 1;

        for (int i = 2; i <= n; i++) {
            f[i] = f[i - 1] + f[i - 2];
        }

        return f[n];
    }

    public static void main(String args[]) {
        System.out.println(fib(Integer.valueOf(args[0])));
    }
}
