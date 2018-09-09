//package javatest;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

class NativeInterface extends ClassLoader {
	static {
		System.out.println(System.getProperty("user.dir")+"/../libs/libvm_javad.dylib");
		System.load(System.getProperty("user.dir")+"/../libs/libvm_javad.dylib");
	}
	
	public static native void sayHello();
//	public static native void apply(long receiver, long account, long act);

	public static native byte[] get_code(long account);
	
	public static native boolean is_account (long account);

	public static native long s2n(String account);
	
	public static native String n2s(long account);
	
	public static native int action_data_size();
	
	public static native byte[] read_action_data();
	
	public static native void require_recipient(long account);
	
	public static native void require_auth(long account);
	
	public static native int db_store_i64(long scope, long table_id, long payer, long id, byte[] data);
	
	public static native void db_update_i64(int itr, long payer, byte[] data);
	
	public static native void db_remove_i64(int itr);
	
	public static native byte[] db_get_i64(int itr);
	
	public static native long db_next_i64(int itr);
	
	public static native long db_previous_i64(int itr);
	
	public static native int db_find_i64(long code, long scope, long table_id, long id);
	
	public static native int db_lowerbound_i64(long code, long scope, long table_id, long id);
	
	public static native int db_upperbound_i64(long code, long scope, long table_id, long id);
	
	public static native int db_end_i64(long code, long scope, long table_id);

	public NativeInterface(ClassLoader parent) {
		super(parent);
	}
	
	@Override
	public Class loadClass(String name) throws ClassNotFoundException {
		System.out.println("+++++loadClass: " + name);
		if (name.toLowerCase() == name) {
			byte[] b = loadClassFromDB(name);
			name = name.substring(0, 1).toUpperCase() + name.substring(1);
			return defineClass(name, b, 0, b.length);
		}
		return super.loadClass(name);
	}
	
	private byte[] loadClassFromDB(String fileName) {
		System.out.println("loadClassFromDB:"+fileName);
		long n = s2n(fileName);
		System.out.println(n+":"+fileName);
		byte[] code = get_code(n);
		System.out.println(code);
		return code;
	}
	
	public static void main(String[] argv) {
		for (String s: argv) {
			System.out.println("+++:"+s);
		}
	}
}

public class VMJava {
    public static byte[] get_code(long account) {
        return NativeInterface.get_code(account);
    }
    
    public static boolean is_account (long account) {
        return NativeInterface.is_account(account);
    }

    public static long s2n(String account) {
        return NativeInterface.s2n(account);
    }
    
    public static String n2s(long account) {
        return NativeInterface.n2s(account);
    }
    
    public static int action_data_size() {
        return NativeInterface.action_data_size();
    }
    
    public static byte[] read_action_data() {
        return NativeInterface.read_action_data();
    }
    
    public static void require_recipient(long account) {
        NativeInterface.require_recipient(account);
    }
    
    public static void require_auth(long account) {
        NativeInterface.require_auth(account);
    }
    
    public static int db_store_i64(long scope, long table_id, long payer, long id, byte[] data) {
        return NativeInterface.db_store_i64(scope, table_id, payer, id, data);
    }
    
    public static void db_update_i64(int itr, long payer, byte[] data) {
        NativeInterface.db_update_i64(itr, payer, data);
    }
    
    public static void db_remove_i64(int itr) {
        NativeInterface.db_remove_i64(itr);
    }
    
    public static byte[] db_get_i64(int itr) {
        return NativeInterface.db_get_i64(itr);
    }
    
    public static long db_next_i64(int itr) {
        return NativeInterface.db_next_i64(itr);
    }
    
    public static long db_previous_i64(int itr) {
        return NativeInterface.db_previous_i64(itr);
    }
    
    public static int db_find_i64(long code, long scope, long table_id, long id) {
        return NativeInterface.db_find_i64(code, scope, table_id, id);
    }
    
    public static int db_lowerbound_i64(long code, long scope, long table_id, long id) {
        return NativeInterface.db_lowerbound_i64(code, scope, table_id, id);
    }
    
    public static int db_upperbound_i64(long code, long scope, long table_id, long id) {
        return NativeInterface.db_upperbound_i64(code, scope, table_id, id);
    }
    
    public static int db_end_i64(long code, long scope, long table_id) {
        return NativeInterface.db_end_i64(code, scope, table_id);
    }
    public void sayHello() {
    	System.out.println("hello, world!");
    }
    
	public static int apply(long receiver, long account, long act) {
		System.out.println("+++++:"+receiver+":"+account+":"+act);
		try {
			NativeInterface vmMain = new NativeInterface(VMJava.class.getClassLoader());
//			String a = vmMain.n2s(receiver);
//			System.out.println("++++apply:" + a);
			Class cls = vmMain.loadClass("hello");
			Class mainArgType[] = { long.class, long.class, long.class };
			Method main = cls.getMethod("apply", mainArgType);
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