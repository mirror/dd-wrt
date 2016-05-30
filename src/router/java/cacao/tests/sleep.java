import java.util.*;

public class sleep extends Thread {
    public static Random r = new Random();

    public sleep (String name) {
	super(name);
    }

    public void run() {
        for (int i = 0; i < 10; ++i) {
            System.out.println(getName());
            try {
		sleep((long) (r.nextFloat() * 1000));
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public static void main(String args[]) {
	sleep t1 = new sleep("a");
	sleep t2 = new sleep("b");

	t1.start();
	t2.start();
    }
}
