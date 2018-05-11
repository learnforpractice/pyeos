package main

import (
    "C"
	"fmt"
	"time"
	"rpc"
	"context"
	"crypto/tls"
	"git.apache.org/thrift.git/lib/go/thrift"
)

type RpcInterfaceImp struct {
}

func (p *RpcInterfaceImp) Apply(ctx context.Context, account int64, action int64) (r int32, err error) {
    handleApply()
    return 0, nil
}

var ctx = context.Background()

func currentTimeMillis() int64 {
	return time.Now().UnixNano() / 1000000
}

var _transportFactory thrift.TTransportFactory
var _protocolFactory thrift.TProtocolFactory
var rpcClient *rpc.RpcServiceClient
func handleClient(client *rpc.RpcServiceClient) (err error) {

    start := time.Now().UnixNano()
    count := 20000

    for i:=0;i<count;i++ {
        client.ReadAction(ctx)
    }

    duration := time.Now().UnixNano() - start
    fmt.Println("transactions per second", int64(count)*1e9/duration);

	return nil
}

func runClient(transportFactory thrift.TTransportFactory, protocolFactory thrift.TProtocolFactory, addr string, secure bool) error {
    runApplyServer(transportFactory, protocolFactory, addr, secure) 
    return nil
}

func handleApply() error {
	var transport thrift.TTransport
	var err error

    var transportFactory = _transportFactory
    var protocolFactory = _protocolFactory
    addr := "localhost:9192"
    secure := false

	if secure {
		cfg := new(tls.Config)
		cfg.InsecureSkipVerify = true
		transport, err = thrift.NewTSSLSocket(addr, cfg)
	} else {
		transport, err = thrift.NewTSocket(addr)
	}
	if err != nil {
		fmt.Println("Error opening socket:", err)
		return err
	}
	transport, err = transportFactory.GetTransport(transport)
	if err != nil {
		return err
	}
	defer transport.Close()
	if err := transport.Open(); err != nil {
		return err
	}

	iprot := protocolFactory.GetProtocol(transport)
	oprot := protocolFactory.GetProtocol(transport)
	client := rpc.NewRpcServiceClient(thrift.NewTStandardClient(iprot, oprot))
    rpcClient = client
	return handleClient(client)
}

func runApplyServer(transportFactory thrift.TTransportFactory, protocolFactory thrift.TProtocolFactory, addr string, secure bool) error {
	var transport thrift.TServerTransport
	var err error

    _transportFactory = transportFactory
    _protocolFactory = protocolFactory

	addr = "localhost:9192"
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

	handler := &RpcInterfaceImp{}
	processor := rpc.NewRpcInterfaceProcessor(handler)

//	handler := NewEoslibServiceHandler()
//	processor := idl.EoslibServiceProcessor(handler)
	server := thrift.NewTSimpleServer4(processor, transport, transportFactory, protocolFactory)

	fmt.Println("Starting the apply server... on ", addr)
	return server.Serve()
}

//export ReadAction
func ReadAction() (r []byte) {
    r, _ = rpcClient.ReadAction(ctx)
    return r
}

//export DbStoreI64
func DbStoreI64(scope int64, table int64, payer int64, id int64, buffer []byte) (r int32) {
    r, _ = rpcClient.DbStoreI64(ctx, scope, table, payer, id, buffer)
    return r
}

//export DbUpdateI64
func DbUpdateI64(itr int32, payer int64, buffer []byte) () {
    rpcClient.DbUpdateI64(ctx, itr, payer, buffer)
    return
}

//export DbRemoveI64
func DbRemoveI64(itr int32) () {
    rpcClient.DbRemoveI64(ctx, itr)
    return
}

//export DbGetI64
func DbGetI64(itr int32) (r []byte) {
    r, _ = rpcClient.DbGetI64(ctx, itr)
    return r
}

//export DbNextI64
func DbNextI64(itr int32) (itrRet int32, value []byte) {
    res, _ := rpcClient.DbNextI64(ctx, itr)
    return res.Status, res.Value
}

//export DbPreviousI64
func DbPreviousI64(itr int32) (itrRet int32, value []byte) {
    res, _ := rpcClient.DbPreviousI64(ctx, itr)
    return res.Status, res.Value
}

//export DbFindI64
func DbFindI64(code int64, scope int64, table int64, id int64) (itr int32) {
    itr, _ = rpcClient.DbFindI64(ctx, code, scope, table, id)
    return
}

//export DbLowerboundI64
func DbLowerboundI64(code int64, scope int64, table int64, id int64) (itr int32) {
    itr, _ = rpcClient.DbLowerboundI64(ctx, code, scope, table, id)
    return
}

//export DbUpperboundI64
func DbUpperboundI64(code int64, scope int64, table int64, id int64) (itr int32) {
    itr, _ = rpcClient.DbUpperboundI64(ctx, code, scope, table, id)
    return
}

//export DbEndI64
func DbEndI64(code int64, scope int64, table int64) (itr int32) {
    itr, _ = rpcClient.DbEndI64(ctx, code, scope, table)
    return
}
