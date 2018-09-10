import java.io.*;

public class Testcase extends Contract {
	public Testcase() {
		
	}
	long N(String name) {
		return VMJava.s2n(name);
	}
	public int apply(long receiver, long account, long act) throws Exception{
		long code = receiver;
		long scope = receiver;
		long payer = code;
		long table_id = code;
		long key = code;
		System.out.println("++++apply: act " + VMJava.n2s(act));
		if (act == N("testexit")) {
			System.exit(0);
		} else if (act == N("testfile")) {
			System.getProperty("user.dir");

			PrintWriter out = new PrintWriter(new FileWriter("OutFile.txt"));
			out.println("hello,world");
			out.close();
		} else if (act == N("testmemory")) {
			int size = 1024*1024*1024;//
			long[] xs = new long[size];
		}
		return 1;
	}
}
