public class UnitArithmeticOverflow {
    private UnitArithmeticOverflow() {}
    public static void main(String[] args) {
        int a = 2147483647;
        int b = -2147483648;

        boolean res1 = ((a + 2) - b) == 1;
        boolean res2 = ((b - 2) - a) == -1;
        boolean res3 = (2 * a) == -2;
        System.exit((res1 && res2 && res3) ? 0 : 1);
    }
}
