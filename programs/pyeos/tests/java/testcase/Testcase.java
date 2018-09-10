import java.io.*;

public class Testcase extends Contract {
	public Testcase() {
		
	}
	public int apply(long receiver, long account, long act) throws Exception{
		long code = receiver;
		long scope = receiver;
		long payer = code;
		long table_id = code;
		long key = code;
		if (act == VMJava.s2n("testexit")) {
			System.exit(0);
		} else if (act == VMJava.s2n("testfile")) {
			System.getProperty("user.dir");

			PrintWriter out = new PrintWriter(new FileWriter("OutFile.txt"));
			out.println("hello,world");
			out.close();
		}
		return 1;
	}
}
