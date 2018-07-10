#[no_mangle]
pub extern fn hello_rust(i64 receiver, i64 account, i64 action) -> i32 {
    println!("{0} {1} {2}", receiver, account, action);
}
