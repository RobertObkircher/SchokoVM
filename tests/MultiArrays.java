public class MultiArrays {
    public static void println(int i) { System.out.println(i); }

    static class Element {
        int value;

        public Element(int value) {
            this.value = value;
        }
    }

    public static void main(String[] args) {
        int[][][][] arr = new int[5][10][15][20];
        Element[][][][] elems = new Element[5][10][15][20];

        for(int i = 0; i < arr.length; i++) {
            for(int j = 0; j < arr[0].length; j++) {
                for(int k = 0; k < arr[0][0].length; k++) {
                    for(int l = 0; l < arr[0][0][0].length; l++) {
                        arr[i][j][k][l] = i + 100 * (j + 100 * (k + 100 * l));
                        elems[i][j][k][l] = new Element(i + 100 * (j + 100 * (k + 100 * l)));
                    }
                }
            }
        }

        int sum = 0;
        int sum2 = 0;
        for(int i = 0; i < arr.length; i++) {
            for(int j = 0; j < arr[0].length; j++) {
                for(int k = 0; k < arr[0][0].length; k++) {
                    for(int l = 0; l < arr[0][0][0].length; l++) {
                        sum += arr[i][j][k][l];
                        sum2 += elems[i][j][k][l].value;
                    }
                }
            }
        }
        println(sum);
        println(sum2);
    }
}
