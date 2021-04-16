public class UnitArithmeticOverflow {
    private UnitArithmeticOverflow() {}
    public static void main(String[] args) {
        int a = 2147483647;
        int b = -2147483648;
        System.exit(((a + 2) - b) + ((b - 2) - a));
    }
}
