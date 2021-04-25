public class Switch {
    static void println(int i) { System.out.println(i); }

    public static void main(String[] args) {
        tableswitch(-1);
        tableswitch(4);
        tableswitch(0);
        tableswitch(6);
        tableswitch(1);
        tableswitch(3);
        tableswitch(5);
        tableswitch(2);

        lookupswitch(404);
        lookupswitch(-1);
        lookupswitch(212);
        lookupswitch(405);
        lookupswitch(303);
        lookupswitch(403);
        lookupswitch(0);
        lookupswitch(101);
        lookupswitch(202);
    }

    static void tableswitch(int i) {
        switch (i) {
            case 1:
                println(111);
                break;
            case 2:
                println(222);
                break;
            case 3:
                println(333);
                break;
            case 4:
                println(444);
                break;
            default:
                println(42);
        }
    }

    static void lookupswitch(int i) {
        switch (i) {
            case 101:
                println(1111);
                break;
            case 202:
                println(2222);
                break;
            case 303:
                println(3333);
                break;
            case 404:
                println(4444);
                break;
            default:
                println(4242);
        }
    }
}
