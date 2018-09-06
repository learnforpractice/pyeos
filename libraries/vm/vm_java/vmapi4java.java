//package myjni;
 
public class vmapi4java {
   static {
      System.loadLibrary("/Users/newworld/dev/pyeos/build-debug/libs/libvm_javad.dylib");
   }
   // A native method that receives nothing and returns void
   private native void sayHello();

	public static int square(int input){
		int output = input * input;
		return output;
	}
	
	public static int power(int input, int exponent){
		int output,i;
		output=1;
		for(i=0;i<exponent;i++){
			output *= input;
		}
		return output;
	}

   public static void main(String[] args) {
      new vmapi4java().sayHello();  // invoke the native method
   }
}

