package bridge

import "os"
import "unsafe"

// #cgo CFLAGS: -I/Users/newworld/dev/pyeos/programs/goeos/include
// #cgo LDFLAGS: -L/Users/newworld/dev/pyeos/build/programs/goeos /Users/newworld/dev/pyeos/build/programs/goeos/libgoeos.dylib
// #include <goeos.h>
// #include <stdlib.h>
import "C"

func GoeosMain() {
    C.arg_size(C.int(len(os.Args)))
    for _, v := range os.Args {
        arg := C.CString(v)
        C.arg_add(arg)
        C.free(unsafe.Pointer(arg))
    }

    C.goeos_main()
}

func test() {
}

