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

        boolean b5 = false;
        float f = 1124.5f;
        if(f > 50){
            f += 40.0f;
        }
        if(f == 1164.5f) {
            b5 = true;
        }

        boolean b6 = false;
        double d = -1124.0;
        if(d < 50){
            d -= 20;
        }
        if(d != 60) {
            b6 = true;
        }
        System.exit((b1 && b2 && b3 && !b4 && b5 && b6) ? 0 : 1);
    }
}
