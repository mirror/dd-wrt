import java.io.*;

public class prop {
    public static void main(String [] argv) throws IOException {
        System.getProperties().store(System.out, "all properties:");
    }
}
