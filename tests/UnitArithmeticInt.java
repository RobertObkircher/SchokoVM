public class UnitArithmeticInt {
    private UnitArithmeticInt() {}
    public static void main(String[] args) {
        int i = 4;
        i += 2;
        i *= 4;
        i /= 12;
        i -= 20;
        long l = 5;
        l += 3;
        l *= 2;
        l /= 4;
        System.exit(i + (int) l);
    }
}
