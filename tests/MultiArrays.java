public class MultiArrays {
    public static void println(int i) { System.out.println(i); }

    public static void main(String[] args) {
        int[][][][] arr = new int[5][10][15][20];

        for(int i = 0; i < arr.length; i++) {
            for(int j = 0; j < arr[0].length; j++) {
                for(int k = 0; k < arr[0][0].length; k++) {
                    for(int l = 0; l < arr[0][0][0].length; l++) {
                        arr[i][j][k][l] = i + 100 * (j + 100 * (k + 100 * l));
                    }
                }
            }
        }

        int sum = 0;
        for(int i = 0; i < arr.length; i++) {
            for(int j = 0; j < arr[0].length; j++) {
                for(int k = 0; k < arr[0][0].length; k++) {
                    for(int l = 0; l < arr[0][0][0].length; l++) {
                        sum += arr[i][j][k][l];
                    }
                }
            }
        }
        println(sum);
    }
}
