//package javatest;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

public class VMMain extends ClassLoader{
	static {
		System.out.println(System.getProperty("user.dir")+"/../libs/libvm_javad.dylib");
		System.load(System.getProperty("user.dir")+"/../libs/libvm_javad.dylib");
	}
	
	private native void sayHello();
	private native void apply(long receiver, long account, long act);


	private native boolean is_account (long account);

	private native long s2n(String account);
	
	private native String n2s(long account);
	
	private native int action_data_size();
	
	private native byte[] read_action_data();
	
	private native void require_recipient(long account);
	
	private native void require_auth(long account);
	
	private native int db_store_i64(long scope, long table_id, long payer, long id, byte[] data);
	
	private native void db_update_i64(int itr, byte[] data);
	
	private native void db_remove_i64(int itr);
	
	private native byte[] db_get_i64(int itr);
	
	private native long db_next_i64(int itr);
	
	private native long db_previous_i64(int itr);
	
	private native int db_find_i64(long code, long scope, long table_id, long id);
	
	private native int db_lowerbound_i64(long code, long scope, long table_id, long id);
	
	private native int db_upperbound_i64(long code, long scope, long table_id, long id);
	
	private native int db_end_i64();

	public VMMain(ClassLoader parent) {
		super(parent);
	}

	public Class getClass(String name) throws ClassNotFoundException {
		byte[] b = loadClassFromFTP(name);
		return defineClass(name, b, 0, b.length);
	}
	
	@Override
	public Class loadClass(String name) throws ClassNotFoundException {
		if (name.startsWith("com.baeldung")) {
			System.out.println("Loading Class from Custom Class Loader");
			return getClass(name);
		}
		return super.loadClass(name);
	}
	
	private byte[] loadClassFromFTP(String fileName) {
		return new byte[1024];
	}
	
	public static void main(String[] argv) {
		for (String s: argv) {
			System.out.println("+++:"+s);
		}
		new VMMain(null).sayHello();
		new VMMain(null).apply(1, 2, 3);
	}
}

