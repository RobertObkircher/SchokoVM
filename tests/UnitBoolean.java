public class UnitBoolean {
    private UnitBoolean() {}
    public static void main(String[] args) {
        boolean b1 = false;
        int i = 1;
        b1 = b1 || i < 2;

        boolean b2 = false;
        if(i <= 2) {
            i++;
        } else {
            b2 = false;
        }

        boolean b3 = false;
        if(i == 2){
            b3 = true;
        }

        boolean b4 = false;
        long l = 100;
        if(l > 50){
            l += 10;
        }
        if(l == 60) {
            b4 = true;
        }
        System.exit((b1 && b2 && b3 && b4) ? 0 : 1);
    }
}
