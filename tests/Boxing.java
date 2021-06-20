public class Boxing {
    public static void println(double v) { System.out.println(Double.doubleToLongBits(v)); }
    public static void println(float v) { System.out.println(Float.floatToIntBits(v)); }

    public static void main(String[] args) {
        {
            boolean value = true;
            Boolean boxed = value;
            System.out.println(boxed);
            System.out.println((boolean) boxed);
        }
        {
            byte value = 14;
            Byte boxed = value;
            System.out.println(boxed);
            System.out.println((byte) boxed);
        }
        {
            char value = 'x';
            Character boxed = value;
            System.out.println(boxed);
            System.out.println((char) boxed);
        }
        {
            short value = 14124;
            Short boxed = value;
            System.out.println(boxed);
            System.out.println((short) boxed);
        }
        {
            int value = 123124142;
            Integer boxed = value;
            System.out.println(boxed);
            System.out.println((int) boxed);
        }
        {
            long value = 1241442241L;
            Long boxed = value;
            System.out.println(boxed);
            System.out.println((long) boxed);
        }
        {
            float value = 123.4f;
            Float boxed = value;
//             System.out.println(boxed);
            println((float) boxed);
        }
        {
            double value = 123678.0d;
            Double boxed = value;
//             System.out.println(boxed);
            println((double) boxed);
        }
    }
}
