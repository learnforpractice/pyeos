package main

import (
	"context"
	"crypto/tls"
	"encoding/binary"
	"fmt"
	"rpc"
	"time"
	"unsafe"

	"git.apache.org/thrift.git/lib/go/thrift"
)

/*
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void arg_size(int n);
void arg_add(char * arg);
void arg_show();

int goeos_main();

int read_action_(char* memory, size_t size);
int read_action(char* memory, size_t size);

int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size );
void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size );
void db_remove_i64( int itr );
int db_get_i64( int itr, char* buffer, size_t buffer_size );
int db_next_i64( int itr, uint64_t* primary );
int db_previous_i64( int itr, uint64_t* primary );
int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_end_i64( uint64_t code, uint64_t scope, uint64_t table );

void db_update_i64_ex( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id, const char* buffer, size_t buffer_size );
void db_remove_i64_ex( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id );

//mpeoslib.cpp
void set_client_mode(int client_mode);

*/
import "C"

var applyClient *rpc.RpcInterfaceClient

var applyStart = make(chan rpc.Apply, 1)
var applyFinish = make(chan rpc.ApplyResult_, 1)

//export onApply
func onApply(receiver uint64, account uint64, act uint64) (status C.int, l C.int, err *C.char) {
	//    initApplyClient()
	//    fmt.Println("+++++++onApply", receiver, account, act, applyClient)
	apply := rpc.Apply{int64(receiver), int64(account), int64(act)}
	applyStart <- apply
	select {
	case res := <-applyFinish:
		{
			//    fmt.Println(res.Status, res.Err);
			if res.Status == 0 {
				return C.int(0), C.int(0), nil
			}
			return C.int(res.Status), C.int(len(res.Err)), C.CString(res.Err) //CString freed in rpc_interface.cpp::apply
		}
	case <-time.After(20 * 1000 * time.Microsecond):
		{
			//apply timeout, something went wrong at the client side,
			//that indicates client side was possiblely brought down by an unknown bug triggered by a deliberate attack or not
			//to preventing the account from further damage of the network:
			//step 1:
			//  if node is a BP, freeze the account by sending a special transaction from BP and execute it immediately
			//  if node is a normal node that received this transaction,
			//  block out the account from executing smart contract
			//  and do not rebroadcast the transaction to the network
			//step 2:
			//  restart the client
			//step 3:
			//  filter out similar transaction?
			fmt.Println("waiting for apply timeout!")
			msg := "timeout"
			return C.int(911), C.int(len(msg)), C.CString(msg)
		}
	}
}

type RpcServiceImpl struct {
}

func (p *RpcServiceImpl) ApplyRequest(ctx context.Context) (r *rpc.Apply, err error) {
	apply := <-applyStart
	return &apply, nil
}

func (p *RpcServiceImpl) ApplyFinish(ctx context.Context, status int32, errMsg string) (err error) {
	res := rpc.ApplyResult_{status, errMsg}
	applyFinish <- res
	return nil
}

func (this *RpcServiceImpl) FunCall(ctx context.Context, callTime int64, funCode string, paramMap map[string]string) (r []string, err error) {
	//  fmt.Println("-->FunCall:", callTime, funCode, paramMap)
	for k, v := range paramMap {
		r = append(r, k+v)
	}
	return
}

func (p *RpcServiceImpl) ReadAction(ctx context.Context) (r []byte, err error) {
	var buffer [256]byte
	ret := 0 //:= C.read_action((*C.char)(unsafe.Pointer(&buffer[0])), C.size_t(len(buffer)))
	return buffer[:ret], nil
}

// Parameters:
//  - Scope
//  - Table
//  - Payer
//  - ID
//  - Buffer
func (p *RpcServiceImpl) DbStoreI64(ctx context.Context, scope int64, table int64, payer int64, id int64, buffer []byte) (r int32, err error) {
	if buffer == nil || len(buffer) == 0 {
		fmt.Println("+++++++++invalide buffer", buffer)
		return -1, nil
	}
	ret := C.db_store_i64(C.uint64_t(scope), C.uint64_t(table), C.uint64_t(payer), C.uint64_t(id), (*C.char)(unsafe.Pointer(&buffer[0])), C.size_t(len(buffer)))
	return int32(ret), nil
}

// Parameters:
//  - Itr
//  - Payer
//  - Buffer
func (p *RpcServiceImpl) DbUpdateI64(ctx context.Context, itr int32, payer int64, buffer []byte) (err error) {
	C.db_update_i64(C.int(itr), C.uint64_t(payer), (*C.char)(unsafe.Pointer(&buffer[0])), C.size_t(len(buffer)))
	return nil
}

// Parameters:
//  - Itr
func (p *RpcServiceImpl) DbRemoveI64(ctx context.Context, itr int32) (err error) {
	C.db_remove_i64(C.int(itr))
	return nil
}

// Parameters:
//  - Itr
func (p *RpcServiceImpl) DbGetI64(ctx context.Context, itr int32) ([]byte, error) {
	size := C.db_get_i64(C.int(itr), nil, C.size_t(0))
	if size <= 0 {
		return nil, nil
	}

	buffer := C.malloc(C.size_t(size))

	ret := C.db_get_i64(C.int(itr), (*C.char)(buffer), C.size_t(size))
	defer C.free(buffer)
	r := C.GoBytes(buffer, ret)
	return r, nil
}

func Int64ToBytes(i uint64) []byte {
	var buf = make([]byte, 8)
	binary.BigEndian.PutUint64(buf, uint64(i))
	return buf
}

func BytesToInt64(buf []byte) uint64 {
	return uint64(binary.BigEndian.Uint64(buf))
}

// Parameters:
//  - Itr
func (p *RpcServiceImpl) DbNextI64(ctx context.Context, itr int32) (r *rpc.Result_, err error) {
	var primary uint64
	var result rpc.Result_
	ret := C.db_next_i64(C.int(itr), (*C.uint64_t)(&primary))
	result.Status = int32(ret)
	result.Value = Int64ToBytes(primary)
	return &result, nil
}

// Parameters:
//  - Itr
func (p *RpcServiceImpl) DbPreviousI64(ctx context.Context, itr int32) (r *rpc.Result_, err error) {
	var primary uint64
	var result rpc.Result_

	ret := C.db_previous_i64(C.int(itr), (*C.uint64_t)(&primary))
	result.Status = int32(ret)
	result.Value = Int64ToBytes(primary)
	return &result, nil
}

// Parameters:
//  - Code
//  - Scope
//  - Table
//  - ID
func (p *RpcServiceImpl) DbFindI64(ctx context.Context, code int64, scope int64, table int64, id int64) (r int32, err error) {
	ret := C.db_find_i64(C.uint64_t(code), C.uint64_t(scope), C.uint64_t(table), C.uint64_t(id))
	return int32(ret), nil
}

// Parameters:
//  - Code
//  - Scope
//  - Table
//  - ID
func (p *RpcServiceImpl) DbLowerboundI64(ctx context.Context, code int64, scope int64, table int64, id int64) (r int32, err error) {
	ret := C.db_lowerbound_i64(C.uint64_t(code), C.uint64_t(scope), C.uint64_t(table), C.uint64_t(id))
	return int32(ret), nil
}

// Parameters:
//  - Code
//  - Scope
//  - Table
//  - ID
func (p *RpcServiceImpl) DbUpperboundI64(ctx context.Context, code int64, scope int64, table int64, id int64) (r int32, err error) {
	ret := C.db_upperbound_i64(C.uint64_t(code), C.uint64_t(scope), C.uint64_t(table), C.uint64_t(id))
	return int32(ret), nil
}

// Parameters:
//  - Code
//  - Scope
//  - Table
func (p *RpcServiceImpl) DbEndI64(ctx context.Context, code int64, scope int64, table int64) (int32, error) {
	ret := C.db_end_i64(C.uint64_t(code), C.uint64_t(scope), C.uint64_t(table))
	return int32(ret), nil
}

func (p *RpcServiceImpl) DbUpdateI64Ex(ctx context.Context, scope int64, payer int64, table int64, id int64, buffer []byte) (err error) {
	C.db_update_i64_ex(C.uint64_t(scope), C.uint64_t(payer), C.uint64_t(table), C.uint64_t(id), (*C.char)(unsafe.Pointer(&buffer[0])), C.size_t(len(buffer)))
	return nil
}

func (p *RpcServiceImpl) DbRemoveI64Ex(ctx context.Context, scope int64, payer int64, table int64, id int64) (err error) {
	C.db_remove_i64_ex(C.uint64_t(scope), C.uint64_t(payer), C.uint64_t(table), C.uint64_t(id))
	return nil
}

var client *rpc.RpcInterfaceClient
var __transportFactory thrift.TTransportFactory
var __protocolFactory thrift.TProtocolFactory

func runServer(transportFactory thrift.TTransportFactory, protocolFactory thrift.TProtocolFactory, addr string, secure bool) error {
	var transport thrift.TServerTransport
	var err error

	__transportFactory = transportFactory
	__protocolFactory = protocolFactory

	if secure {
		cfg := new(tls.Config)
		if cert, err := tls.LoadX509KeyPair("server.crt", "server.key"); err == nil {
			cfg.Certificates = append(cfg.Certificates, cert)
		} else {
			return err
		}
		transport, err = thrift.NewTSSLServerSocket(addr, cfg)
	} else {
		transport, err = thrift.NewTServerSocket(addr)
	}

	if err != nil {
		return err
	}
	fmt.Printf("%T\n", transport)

	handler := &RpcServiceImpl{}
	processor := rpc.NewRpcServiceProcessor(handler)

	//  handler := NewEoslibServiceHandler()
	//  processor := idl.EoslibServiceProcessor(handler)
	server := thrift.NewTSimpleServer4(processor, transport, transportFactory, protocolFactory)

	fmt.Println("Starting the simple server... on ", addr)
	return server.Serve()
}

func initApplyClient() error {
	var transport thrift.TTransport
	var err error

	addr := "localhost:9192"
	secure := false
	if applyClient != nil {
		return nil
	}

	if secure {
		cfg := new(tls.Config)
		cfg.InsecureSkipVerify = true
		transport, err = thrift.NewTSSLSocket(addr, cfg)
	} else {
		transport, err = thrift.NewTSocket(addr)
	}
	if err != nil {
		fmt.Println("Error opening socket:", err)
		return nil
	}
	transport, err = __transportFactory.GetTransport(transport)
	if err != nil {
		return nil
	}
	//    defer transport.Close()
	if err := transport.Open(); err != nil {
		return nil
	}

	iprot := __protocolFactory.GetProtocol(transport)
	oprot := __protocolFactory.GetProtocol(transport)

	applyClient = rpc.NewRpcInterfaceClient(thrift.NewTStandardClient(iprot, oprot))
	return nil
}
