import java.io.*;

public class Hello extends Contract {
	public Hello() {
		
	}
	public int apply(long receiver, long account, long act) {
		long code = receiver;
		long scope = receiver;
		long payer = code;
		long table_id = code;
		long key = code;
		if (act != VMJava.s2n("sayhello")) {
			return 0;
		}
//		System.out.println("+++++apply:"+receiver);
		
		VMJava java = new VMJava();

/*
		try {
			System.getProperty("user.dir");

			PrintWriter out = new PrintWriter(new FileWriter("OutFile.txt"));
			out.println("hello,world");
			out.close();
		} catch (java.io.UnsupportedEncodingException ex) {
			System.out.println(ex);
		} catch (IOException ex) {
			
		}
*/

			byte[] data;
			int itr = java.db_find_i64(code, scope, table_id, key);
			data = java.read_action_data();
//			System.out.println("+++++data.length:"+data.length);
//			System.out.println("++++"+new String(data, "UTF-8"));

			if (itr < 0) {
				VMJava.db_store_i64(scope, payer, table_id, key, data);
			} else {
				byte[] old_data = java.db_get_i64(itr);
//				System.out.println("+++++old_data.length:"+old_data.length);
				VMJava.db_update_i64(itr, payer, data);
			}

		System.out.println("hello, java world!");
		return 1;
	}
}
