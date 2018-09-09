//package javatest;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

public class VMMain extends ClassLoader{
	static {
		System.out.println(System.getProperty("user.dir")+"/../libs/libvm_javad.dylib");
		System.load(System.getProperty("user.dir")+"/../libs/libvm_javad.dylib");
	}
	
	private native void sayHello();
//	private native void apply(long receiver, long account, long act);

	private native byte[] get_code(long account);
	
	private native boolean is_account (long account);

	private native long s2n(String account);
	
	private native String n2s(long account);
	
	private native int action_data_size();
	
	private native byte[] read_action_data();
	
	private native void require_recipient(long account);
	
	private native void require_auth(long account);
	
	private native int db_store_i64(long scope, long table_id, long payer, long id, byte[] data);
	
	private native void db_update_i64(int itr, long payer, byte[] data);
	
	private native void db_remove_i64(int itr);
	
	private native byte[] db_get_i64(int itr);
	
	private native long db_next_i64(int itr);
	
	private native long db_previous_i64(int itr);
	
	private native int db_find_i64(long code, long scope, long table_id, long id);
	
	private native int db_lowerbound_i64(long code, long scope, long table_id, long id);
	
	private native int db_upperbound_i64(long code, long scope, long table_id, long id);
	
	private native int db_end_i64(long code, long scope, long table_id);

	public VMMain(ClassLoader parent) {
		super(parent);
	}

	public Class getClass(String name) throws ClassNotFoundException {
		byte[] b = loadClassFromDB(name);
		return defineClass(name, b, 0, b.length);
	}
	
	@Override
	public Class loadClass(String name) throws ClassNotFoundException {
		return getClass(name);
	}
	
	private byte[] loadClassFromDB(String fileName) {
		long n = s2n(fileName);
		return get_code(n);
	}
	
	public static void main(String[] argv) {
		for (String s: argv) {
			System.out.println("+++:"+s);
		}
		try {
			VMMain vmMain = new VMMain(null);
			vmMain.sayHello();
			vmMain.apply(1, 2, 3);
			Class test = vmMain.loadClass("Test");
			Class mainArgType[] = { long.class, long.class, long.class };
			Method main = test.getMethod("apply", mainArgType);
			
			Object argsArray[] = { 11, 22, 33 };
			main.invoke(null, argsArray);
		} catch (ClassNotFoundException ex) {
			System.out.println(ex);
		} catch (NoSuchMethodException ex) {
			System.out.println(ex);
		} catch (IllegalAccessException ex) {
			System.out.println(ex);
		} catch (InvocationTargetException ex)  {
			System.out.println(ex);
		}
	}
	
	public static int apply(long receiver, long account, long act) {
		System.out.println(receiver+":"+account+":"+act);
		try {
			VMMain vmMain = new VMMain(null);
			Class test = vmMain.loadClass("Test");
			Class mainArgType[] = { long.class, long.class, long.class };
			Method main = test.getMethod("apply", mainArgType);
			Object argsArray[] = { receiver, account, act };
			main.invoke(null, argsArray);
		} catch (ClassNotFoundException ex) {
			System.out.println(ex);
		} catch (NoSuchMethodException ex) {
			System.out.println(ex);
		} catch (IllegalAccessException ex) {
			System.out.println(ex);
		} catch (InvocationTargetException ex)  {
			System.out.println(ex);
		}
		return 1;
	}
}

