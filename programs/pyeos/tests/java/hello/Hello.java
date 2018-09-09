public class Hello {
	public static int apply(long receiver, long account, long act) {
		long code = receiver;
		long scope = receiver;
		long payer = code;
		long table_id = code;
		long key = code;
		VMJava java = new VMJava();
		System.out.println("+++++from Hello.apply111");

		java.sayHello();
		System.out.println("+++++from Hello.apply222");

		System.out.println("+++++from Hello.apply333 "+java.n2s(receiver));
		System.out.println("+++++from Hello.apply444 "+new String(java.read_action_data()));

		
		int itr = java.db_find_i64(code, scope, table_id, key);
		System.out.println("+++++from Hello.apply");
		byte[] bs = new byte[] {'h','e', 'l', 'l','o', '!'};

		if (itr < 0) {
			VMJava.db_store_i64(scope, payer, table_id, key, bs);
		} else {
			System.out.println("++++"+new String(bs));
			VMJava.db_update_i64(itr, payer, bs);
		}

		System.out.println("hello, java world!");
		return 1;
	}
}
