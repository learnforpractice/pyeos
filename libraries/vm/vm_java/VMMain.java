//package javatest;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

public class VMMain extends ClassLoader{
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
			System.out.println(s);
		}
	}
}

