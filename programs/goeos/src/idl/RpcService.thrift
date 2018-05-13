
namespace go rpc
namespace java rpc

struct Result {
  1: i32 status = 0,
  2: binary value,
}

struct Apply {
    1:i64 receiver, 
    2:i64 account, 
    3:i64 action
}

service RpcService {
    Apply apply_request();
	void apply_finish()
	list<string> funCall(1:i64 callTime, 2:string funCode, 3:map<string, string> paramMap);
	binary read_action();
    
    i32 db_store_i64( 1:i64 scope, 2:i64 table, 3:i64 payer, 4:i64 id, 5:binary buffer );
    void db_update_i64( 1:i32 itr, 2:i64 payer, 3:binary buffer );
    void db_remove_i64( 1:i32 itr );
    binary db_get_i64( 1:i32 itr );
    Result db_next_i64( 1:i32 itr);
    Result db_previous_i64( 1:i32 itr );
    i32 db_find_i64( 1:i64 code, 2:i64 scope, 3:i64 table, 4:i64 id );
    i32 db_lowerbound_i64( 1:i64 code, 2:i64 scope, 3:i64 table, 4:i64 id );
    i32 db_upperbound_i64( 1:i64 code, 2:i64 scope, 3:i64 table, 4:i64 id );
    i32 db_end_i64( 1:i64 code, 2:i64 scope, 3:i64 table );

}

service RpcInterface {
    i32 apply(1:i64 receiver, 2:i64 account, 3:i64 action);
}