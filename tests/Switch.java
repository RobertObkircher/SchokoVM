public class Switch {
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

        alignment();
    }

    static void tableswitch(int i) {
        switch (i) {
            case 1:
                System.out.println(111);
                break;
            case 2:
                System.out.println(222);
                break;
            case 3:
                System.out.println(333);
                break;
            case 4:
                System.out.println(444);
                break;
            default:
                System.out.println(42);
        }
    }

    static void lookupswitch(int i) {
        switch (i) {
            case 101:
                System.out.println(1111);
                break;
            case 202:
                System.out.println(2222);
                break;
            case 303:
                System.out.println(3333);
                break;
            case 404:
                System.out.println(4444);
                break;
            default:
                System.out.println(4242);
        }
    }

    static void alignment(){
        String name = "abcabcabcabcabcabcabcabcabc";
        switch (name) {
            case "abcabcabcabcabcabcabcabcabc":
                System.out.println(2);
                break;
            default:
                System.out.println(3);
                break;
        }
    }
}
