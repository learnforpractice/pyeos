package main

import "bridge"
import "fmt"
import "os"

func main() {
    var s = "abc\nabc\r";
	fmt.Println(len(s));

	fmt.Printf("Hello ")
	bridge.PrintWorld()

    argsWithProg := os.Args
    argsWithoutProg := os.Args[1:]

   /*
   cs := C.CString("Go string")
   csRet := C.f2(cs)
   fmt.Printf("fmt: %s\n", C.GoString(csRet))
   C.free(unsafe.Pointer(cs))          // free memory right now ...
   */



    arg := os.Args[3]
    fmt.Println(argsWithProg)
    fmt.Println(argsWithoutProg)
    fmt.Println(arg)

}
