import java.util.*;

public class sleep_exception extends Thread {
    public static Random r = new Random();

    public sleep_exception (String name) {
	super(name);
    }

    public void run() {
        for (int i = 0; i < 10; ++i) {
            System.out.println(getName());
            try {
                throw new Exception("Exception in thread");
            } catch (Exception e) {
              e.printStackTrace();
            }
            try {
		sleep((long) (r.nextFloat() * 1000));
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public static void main(String args[]) {
	sleep_exception t1 = new sleep_exception("a");
	sleep_exception t2 = new sleep_exception("b");

	t1.start();
	t2.start();
    }
}
