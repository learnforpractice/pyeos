package main

import (
	"fmt"
	"github.com/learnforpractice/panicwrap"
	_"os"
	"time"
)

func main() {
    fmt.Println("hello,worldd");
    for {
        wrap := panicwrap.WrapConfig{
        		Handler: panicHandler,
        		ExePath: "ls",
        		Args	: []string{"-l"},
        }
    	exitStatus, err := panicwrap.Wrap(&wrap)
    	fmt.Println("BasicWrap return: ", exitStatus, err)
    	if err != nil {
    		// Something went wrong setting up the panic wrapper. Unlikely,
    		// but possible.
    		panic(err)
    	}
    
    	if exitStatus < 0 {
            break
    	}
        time.Sleep(3*time.Second)
    }
	// Otherwise, exitStatus < 0 means we're the child. Continue executing as
	// normal...
    time.Sleep(5*time.Second)
	// Let's say we panic
	panic("oh shucks")

}

func panicHandler(output string) {
	// output contains the full output (including stack traces) of the
	// panic. Put it in a file or something.
	fmt.Printf("+++++++++++The child panicked:\n\n%s\n", output)
//	os.Exit(1)
}
