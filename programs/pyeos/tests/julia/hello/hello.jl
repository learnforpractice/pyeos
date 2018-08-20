
module hello

using Libdl

lib = dlopen("../libs/libeosio_native.dylib") # Open the library explicitly.
sym_is_account = dlsym(lib, :is_account)
sym_action_data_size = dlsym(lib, :action_data_size)
sym_read_action_data = dlsym(lib, :read_action_data)
sym_string_to_uint64 = dlsym(lib, :string_to_uint64)
sym_uint64_to_string = dlsym(lib, :uint64_to_string)

function is_account(account::UInt64)
  ret = ccall(sym_is_account, Cuchar, (Culonglong,), account)
  return ret != 0
end

function action_data_size()
  ret = ccall(sym_action_data_size, Cuint, ())
  return ret
end

function read_action_data()
  size = action_data_size()
  buf = Array{UInt8, 1}(undef, size)
  ret = ccall(sym_read_action_data, Cuint, (Ptr{UInt8}, Cuint), buf, size)
  return String(buf)
end

function N(account::String)
  ret = ccall(sym_string_to_uint64, Culonglong, (Cstring,), account)
  return ret
end

function n2s(n::UInt64)
  buf = Array{UInt8, 1}(undef, 13)
  ret = ccall(sym_uint64_to_string, Cuint, (Culonglong, Ptr{UInt8}, Cuint), n, buf, sizeof(buf))
  return String(buf[1:ret])
end


function apply(receiver::UInt64, code::UInt64, action::UInt64)
  if action == N("sayhello")
    println(n2s(receiver))
    println(n2s(code))
    println(n2s(action))

    println(is_account(receiver))
    println(is_account(action))
    println(N("hello"))

    r = read_action_data()
    println(r)
  end
  return 1
end

end

