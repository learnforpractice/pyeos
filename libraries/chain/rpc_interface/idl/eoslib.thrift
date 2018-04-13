//thrift -out .. --gen py eoslib.thrift
namespace py idl

struct Result {
  1: i32 status = 0,
  2: binary value,
}

service eoslib_service {
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

service rpc_interface {
    i32 apply( 1:i64 account, 2:i64 action, 3:binary buffer );
}
