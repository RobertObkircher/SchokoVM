import java.math.BigInteger;

public class FibonacciIterBig {
    static BigInteger fib(int n) {
        BigInteger f[] = new BigInteger[n + 1];

        f[0] = BigInteger.valueOf(0);
        f[1] = BigInteger.valueOf(1);

        for (int i = 2; i <= n; i++) {
            f[i] = f[i - 1].add(f[i - 2]);
        }

        return f[n];
    }

    public static void main(String args[]) {
        System.out.println(fib(Integer.valueOf(args[0])));
    }
}
