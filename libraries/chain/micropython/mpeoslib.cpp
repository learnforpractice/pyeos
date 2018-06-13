#include "mpeoslib.h"

#include <eosio/chain/controller.hpp>
#include <eosio/chain/micropython_interface.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>

//#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/producer_schedule.hpp>
#include <eosio/chain/asset.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/types.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/transaction_context.hpp>
#include <eosio/chain/transaction.hpp>

#include <boost/filesystem.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

//#include <eosio/chain/wasm_interface_private.hpp>
//#include <eosio/chain/wasm_eosio_constraints.hpp>
#include <fc/exception/exception.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha1.hpp>
#include <fc/io/raw.hpp>
#include <fc/utf8.hpp>

#include <Runtime/Runtime.h>
#include "IR/Module.h"
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "IR/Operators.h"
#include "IR/Validate.h"
#include "IR/Types.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <mutex>
#include <thread>
#include <condition_variable>
#include <dlfcn.h>

using namespace eosio;
using namespace eosio::chain;

static struct mpapi s_mpapi;
static struct eosapi s_eosapi;
extern "C" void init_eosapi();

std::map<std::thread::id, struct mpapi*> api_map;

static std::mutex g_load_mutex;
static vector<char> print_buffer;
void print(const char * str, size_t len) {
   for (int i=0;i<len;i++) {
      if (str[i] == '\n') {
         string s(print_buffer.data(), print_buffer.size());
         print_buffer.clear();
         dlog(s);
         continue;
      }
      print_buffer.push_back(str[i]);
   }
}

//mpeoslib.c
typedef struct eosapi* (*fn_get_eosapi)();
static fn_get_eosapi get_eosapi = 0;

typedef void (*fn_set_max_execution_time)(int);

extern "C" void set_get_eosapi_func(fn_get_eosapi fn) {
   get_eosapi = fn;
}

mpapi& get_mpapi() {
   static int counter = 0;

   std::lock_guard<std::mutex> guard(g_load_mutex);

   std::thread::id this_id = std::this_thread::get_id();
   auto itr = api_map.find(this_id);
   if ( itr != api_map.end()) {
      return *itr->second;
   }

   char buffer[128];
   counter += 1;

#ifdef __APPLE__
   #ifdef DEBUG
      sprintf(buffer, "../libraries/vm_py/libvm_py-%dd.dylib", counter);
   #else
      sprintf(buffer, "../libraries/vm_py/libvm_py-%d.dylib", counter);
   #endif
#else
   #ifdef DEBUG
      sprintf(buffer, "../libraries/vm_py/libvm_py-%dd.so", counter);
   #else
      sprintf(buffer, "../libraries/vm_py/libvm_py-%d.so", counter);
   #endif
#endif


   void *handle = dlopen(buffer, RTLD_LAZY | RTLD_LOCAL);

   wlog("${n1}, ${n2}", ("n1", buffer)("n2", (uint64_t)handle));

   assert(handle != NULL);

   fn_mp_obtain_mpapi mp_obtain_mpapi = (fn_mp_obtain_mpapi)dlsym(handle, "mp_obtain_mpapi");

   fn_main_micropython main_micropython = (fn_main_micropython)dlsym(handle, "main_micropython");

   fn_set_max_execution_time set_time = (fn_set_max_execution_time)dlsym(handle, "set_max_execution_time");
   set_time(5000);

   struct mpapi* api = new mpapi();
   api->handle = handle;

   main_micropython(0, NULL);

   mp_obtain_mpapi(api);
   api_map[this_id] = api;
   api->set_printer(print);
   api->init = 0;

   struct eosapi* _api;
   if (get_eosapi) {
      _api = get_eosapi();
   } else {
      init_eosapi();
      _api = &s_eosapi;
   }
   api->_eosapi = _api;
   fn_mp_register_eosapi mp_register_eosapi = (fn_mp_register_eosapi)dlsym(handle, "mp_register_eosapi");
   mp_register_eosapi(_api);

   return *api;
}

extern "C" {
//typedef long long int64_t;
//typedef unsigned long long uint64_t;

struct mpapi* c_get_mpapi() {
   return &get_mpapi();
}

typedef uint64_t TableName;

uint64_t string_to_uint64_(const char* str) {
   try {
      return name(str).value;
   } catch (...) {
   }
   return 0;
}

mp_obj_t uint64_to_string_(uint64_t n) {
   string s = name(n).to_string();
//   ilog("n: ${n}",("n", s.size()));
   return get_mpapi().mp_obj_new_str(s.c_str(), s.size());
}

mp_obj_t pack_(const char* str, int nsize) {
   string raw(str,nsize);
   std::vector<char> o = fc::raw::pack<string>(raw);
   string out = string(o.begin(), o.end());
   return get_mpapi().mp_obj_new_bytes((const byte*)out.c_str(), out.size());
}

mp_obj_t unpack_(const char* str, int nsize) {
   string raw(str,nsize);
   std::vector<char> v(raw.begin(), raw.end());
   string out = fc::raw::unpack<string>(v);
   return get_mpapi().mp_obj_new_str(out.c_str(), out.size());
}

}

using namespace eosio::chain;

namespace eosio { namespace micropython {


static inline apply_context& ctx() {
   return apply_context::ctx();
}

class softfloat_api {
   public:
      // TODO add traps on truncations for special cases (NaN or outside the range which rounds to an integer)
//      using context_aware_api::context_aware_api;
      // float binops
      float _eosio_f32_add( float a, float b ) {
         float32_t ret = f32_add( to_softfloat32(a), to_softfloat32(b) );
         return *reinterpret_cast<float*>(&ret);
      }
      float _eosio_f32_sub( float a, float b ) {
         float32_t ret = f32_sub( to_softfloat32(a), to_softfloat32(b) );
         return *reinterpret_cast<float*>(&ret);
      }
      float _eosio_f32_div( float a, float b ) {
         float32_t ret = f32_div( to_softfloat32(a), to_softfloat32(b) );
         return *reinterpret_cast<float*>(&ret);
      }
      float _eosio_f32_mul( float a, float b ) {
         float32_t ret = f32_mul( to_softfloat32(a), to_softfloat32(b) );
         return *reinterpret_cast<float*>(&ret);
      }
      float _eosio_f32_min( float af, float bf ) {
         float32_t a = to_softfloat32(af);
         float32_t b = to_softfloat32(bf);
         if (is_nan(a)) {
            return af;
         }
         if (is_nan(b)) {
            return bf;
         }
         if ( sign_bit(a) != sign_bit(b) ) {
            return sign_bit(a) ? af : bf;
         }
         return f32_lt(a,b) ? af : bf;
      }
      float _eosio_f32_max( float af, float bf ) {
         float32_t a = to_softfloat32(af);
         float32_t b = to_softfloat32(bf);
         if (is_nan(a)) {
            return af;
         }
         if (is_nan(b)) {
            return bf;
         }
         if ( sign_bit(a) != sign_bit(b) ) {
            return sign_bit(a) ? bf : af;
         }
         return f32_lt( a, b ) ? bf : af;
      }
      float _eosio_f32_copysign( float af, float bf ) {
         float32_t a = to_softfloat32(af);
         float32_t b = to_softfloat32(bf);
         uint32_t sign_of_a = a.v >> 31;
         uint32_t sign_of_b = b.v >> 31;
         a.v &= ~(1 << 31);             // clear the sign bit
         a.v = a.v | (sign_of_b << 31); // add the sign of b
         return from_softfloat32(a);
      }
      // float unops
      float _eosio_f32_abs( float af ) {
         float32_t a = to_softfloat32(af);
         a.v &= ~(1 << 31);
         return from_softfloat32(a);
      }
      float _eosio_f32_neg( float af ) {
         float32_t a = to_softfloat32(af);
         uint32_t sign = a.v >> 31;
         a.v &= ~(1 << 31);
         a.v |= (!sign << 31);
         return from_softfloat32(a);
      }
      float _eosio_f32_sqrt( float a ) {
         float32_t ret = f32_sqrt( to_softfloat32(a) );
         return from_softfloat32(ret);
      }
      // ceil, floor, trunc and nearest are lifted from libc
      float _eosio_f32_ceil( float af ) {
         float32_t a = to_softfloat32(af);
         int e = (int)(a.v >> 23 & 0xFF) - 0X7F;
         uint32_t m;
         if (e >= 23)
            return af;
         if (e >= 0) {
            m = 0x007FFFFF >> e;
            if ((a.v & m) == 0)
               return af;
            if (a.v >> 31 == 0)
               a.v += m;
            a.v &= ~m;
         } else {
            if (a.v >> 31)
               a.v = 0x80000000; // return -0.0f
            else if (a.v << 1)
               a.v = 0x3F800000; // return 1.0f
         }

         return from_softfloat32(a);
      }
      float _eosio_f32_floor( float af ) {
         float32_t a = to_softfloat32(af);
         int e = (int)(a.v >> 23 & 0xFF) - 0X7F;
         uint32_t m;
         if (e >= 23)
            return af;
         if (e >= 0) {
            m = 0x007FFFFF >> e;
            if ((a.v & m) == 0)
               return af;
            if (a.v >> 31)
               a.v += m;
            a.v &= ~m;
         } else {
            if (a.v >> 31 == 0)
               a.v = 0;
            else if (a.v << 1)
               a.v = 0xBF800000; // return -1.0f
         }
         return from_softfloat32(a);
      }
      float _eosio_f32_trunc( float af ) {
         float32_t a = to_softfloat32(af);
         int e = (int)(a.v >> 23 & 0xff) - 0x7f + 9;
         uint32_t m;
         if (e >= 23 + 9)
            return af;
         if (e < 9)
            e = 1;
         m = -1U >> e;
         if ((a.v & m) == 0)
            return af;
         a.v &= ~m;
         return from_softfloat32(a);
      }
      float _eosio_f32_nearest( float af ) {
         float32_t a = to_softfloat32(af);
         int e = a.v>>23 & 0xff;
         int s = a.v>>31;
         float32_t y;
         if (e >= 0x7f+23)
            return af;
         if (s)
            y = f32_add( f32_sub( a, float32_t{inv_float_eps} ), float32_t{inv_float_eps} );
         else
            y = f32_sub( f32_add( a, float32_t{inv_float_eps} ), float32_t{inv_float_eps} );
         if (f32_eq( y, {0} ) )
            return s ? -0.0f : 0.0f;
         return from_softfloat32(y);
      }

      // float relops
      bool _eosio_f32_eq( float a, float b ) {  return f32_eq( to_softfloat32(a), to_softfloat32(b) ); }
      bool _eosio_f32_ne( float a, float b ) { return !f32_eq( to_softfloat32(a), to_softfloat32(b) ); }
      bool _eosio_f32_lt( float a, float b ) { return f32_lt( to_softfloat32(a), to_softfloat32(b) ); }
      bool _eosio_f32_le( float a, float b ) { return f32_le( to_softfloat32(a), to_softfloat32(b) ); }
      bool _eosio_f32_gt( float af, float bf ) {
         float32_t a = to_softfloat32(af);
         float32_t b = to_softfloat32(bf);
         if (is_nan(a))
            return false;
         if (is_nan(b))
            return false;
         return !f32_le( a, b );
      }
      bool _eosio_f32_ge( float af, float bf ) {
         float32_t a = to_softfloat32(af);
         float32_t b = to_softfloat32(bf);
         if (is_nan(a))
            return false;
         if (is_nan(b))
            return false;
         return !f32_lt( a, b );
      }

      // double binops
      double _eosio_f64_add( double a, double b ) {
         float64_t ret = f64_add( to_softfloat64(a), to_softfloat64(b) );
         return from_softfloat64(ret);
      }
      double _eosio_f64_sub( double a, double b ) {
         float64_t ret = f64_sub( to_softfloat64(a), to_softfloat64(b) );
         return from_softfloat64(ret);
      }
      double _eosio_f64_div( double a, double b ) {
         float64_t ret = f64_div( to_softfloat64(a), to_softfloat64(b) );
         return from_softfloat64(ret);
      }
      double _eosio_f64_mul( double a, double b ) {
         float64_t ret = f64_mul( to_softfloat64(a), to_softfloat64(b) );
         return from_softfloat64(ret);
      }
      double _eosio_f64_min( double af, double bf ) {
         float64_t a = to_softfloat64(af);
         float64_t b = to_softfloat64(bf);
         if (is_nan(a))
            return af;
         if (is_nan(b))
            return bf;
         if (sign_bit(a) != sign_bit(b))
            return sign_bit(a) ? af : bf;
         return f64_lt( a, b ) ? af : bf;
      }
      double _eosio_f64_max( double af, double bf ) {
         float64_t a = to_softfloat64(af);
         float64_t b = to_softfloat64(bf);
         if (is_nan(a))
            return af;
         if (is_nan(b))
            return bf;
         if (sign_bit(a) != sign_bit(b))
            return sign_bit(a) ? bf : af;
         return f64_lt( a, b ) ? bf : af;
      }
      double _eosio_f64_copysign( double af, double bf ) {
         float64_t a = to_softfloat64(af);
         float64_t b = to_softfloat64(bf);
         uint64_t sign_of_a = a.v >> 63;
         uint64_t sign_of_b = b.v >> 63;
         a.v &= ~(uint64_t(1) << 63);             // clear the sign bit
         a.v = a.v | (sign_of_b << 63); // add the sign of b
         return from_softfloat64(a);
      }

      // double unops
      double _eosio_f64_abs( double af ) {
         float64_t a = to_softfloat64(af);
         a.v &= ~(uint64_t(1) << 63);
         return from_softfloat64(a);
      }
      double _eosio_f64_neg( double af ) {
         float64_t a = to_softfloat64(af);
         uint64_t sign = a.v >> 63;
         a.v &= ~(uint64_t(1) << 63);
         a.v |= (uint64_t(!sign) << 63);
         return from_softfloat64(a);
      }
      double _eosio_f64_sqrt( double a ) {
         float64_t ret = f64_sqrt( to_softfloat64(a) );
         return from_softfloat64(ret);
      }
      // ceil, floor, trunc and nearest are lifted from libc
      double _eosio_f64_ceil( double af ) {
         float64_t a = to_softfloat64( af );
         float64_t ret;
         int e = a.v >> 52 & 0x7ff;
         float64_t y;
         if (e >= 0x3ff+52 || f64_eq( a, { 0 } ))
            return af;
         /* y = int(x) - x, where int(x) is an integer neighbor of x */
         if (a.v >> 63)
            y = f64_sub( f64_add( f64_sub( a, float64_t{inv_double_eps} ), float64_t{inv_double_eps} ), a );
         else
            y = f64_sub( f64_sub( f64_add( a, float64_t{inv_double_eps} ), float64_t{inv_double_eps} ), a );
         /* special case because of non-nearest rounding modes */
         if (e <= 0x3ff-1) {
            return a.v >> 63 ? -0.0 : 1.0; //float64_t{0x8000000000000000} : float64_t{0xBE99999A3F800000}; //either -0.0 or 1
         }
         if (f64_lt( y, to_softfloat64(0) )) {
            ret = f64_add( f64_add( a, y ), to_softfloat64(1) ); // 0xBE99999A3F800000 } ); // plus 1
            return from_softfloat64(ret);
         }
         ret = f64_add( a, y );
         return from_softfloat64(ret);
      }
      double _eosio_f64_floor( double af ) {
         float64_t a = to_softfloat64( af );
         float64_t ret;
         int e = a.v >> 52 & 0x7FF;
         float64_t y;
         double de = 1/DBL_EPSILON;
         if ( a.v == 0x8000000000000000) {
            return af;
         }
         if (e >= 0x3FF+52 || a.v == 0) {
            return af;
         }
         if (a.v >> 63)
            y = f64_sub( f64_add( f64_sub( a, float64_t{inv_double_eps} ), float64_t{inv_double_eps} ), a );
         else
            y = f64_sub( f64_sub( f64_add( a, float64_t{inv_double_eps} ), float64_t{inv_double_eps} ), a );
         if (e <= 0x3FF-1) {
            return a.v>>63 ? -1.0 : 0.0; //float64_t{0xBFF0000000000000} : float64_t{0}; // -1 or 0
         }
         if ( !f64_le( y, float64_t{0} ) ) {
            ret = f64_sub( f64_add(a,y), to_softfloat64(1.0));
            return from_softfloat64(ret);
         }
         ret = f64_add( a, y );
         return from_softfloat64(ret);
      }
      double _eosio_f64_trunc( double af ) {
         float64_t a = to_softfloat64( af );
         int e = (int)(a.v >> 52 & 0x7ff) - 0x3ff + 12;
         uint64_t m;
         if (e >= 52 + 12)
            return af;
         if (e < 12)
            e = 1;
         m = -1ULL >> e;
         if ((a.v & m) == 0)
            return af;
         a.v &= ~m;
         return from_softfloat64(a);
      }

      double _eosio_f64_nearest( double af ) {
         float64_t a = to_softfloat64( af );
         int e = (a.v >> 52 & 0x7FF);
         int s = a.v >> 63;
         float64_t y;
         if ( e >= 0x3FF+52 )
            return af;
         if ( s )
            y = f64_add( f64_sub( a, float64_t{inv_double_eps} ), float64_t{inv_double_eps} );
         else
            y = f64_sub( f64_add( a, float64_t{inv_double_eps} ), float64_t{inv_double_eps} );
         if ( f64_eq( y, float64_t{0} ) )
            return s ? -0.0 : 0.0;
         return from_softfloat64(y);
      }

      // double relops
      bool _eosio_f64_eq( double a, double b ) { return f64_eq( to_softfloat64(a), to_softfloat64(b) ); }
      bool _eosio_f64_ne( double a, double b ) { return !f64_eq( to_softfloat64(a), to_softfloat64(b) ); }
      bool _eosio_f64_lt( double a, double b ) { return f64_lt( to_softfloat64(a), to_softfloat64(b) ); }
      bool _eosio_f64_le( double a, double b ) { return f64_le( to_softfloat64(a), to_softfloat64(b) ); }
      bool _eosio_f64_gt( double af, double bf ) {
         float64_t a = to_softfloat64(af);
         float64_t b = to_softfloat64(bf);
         if (is_nan(a))
            return false;
         if (is_nan(b))
            return false;
         return !f64_le( a, b );
      }
      bool _eosio_f64_ge( double af, double bf ) {
         float64_t a = to_softfloat64(af);
         float64_t b = to_softfloat64(bf);
         if (is_nan(a))
            return false;
         if (is_nan(b))
            return false;
         return !f64_lt( a, b );
      }

      // float and double conversions
      double _eosio_f32_promote( float a ) {
         return from_softfloat64(f32_to_f64( to_softfloat32(a)) );
      }
      float _eosio_f64_demote( double a ) {
         return from_softfloat32(f64_to_f32( to_softfloat64(a)) );
      }
      int32_t _eosio_f32_trunc_i32s( float af ) {
         float32_t a = to_softfloat32(af);
         if (_eosio_f32_ge(af, 2147483648.0f) || _eosio_f32_lt(af, -2147483648.0f))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f32.convert_s/i32 overflow" );

         if (is_nan(a))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f32.convert_s/i32 unrepresentable");
         return f32_to_i32( to_softfloat32(_eosio_f32_trunc( af )), 0, false );
      }
      int32_t _eosio_f64_trunc_i32s( double af ) {
         float64_t a = to_softfloat64(af);
         if (_eosio_f64_ge(af, 2147483648.0) || _eosio_f64_lt(af, -2147483648.0))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f64.convert_s/i32 overflow");
         if (is_nan(a))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f64.convert_s/i32 unrepresentable");
         return f64_to_i32( to_softfloat64(_eosio_f64_trunc( af )), 0, false );
      }
      uint32_t _eosio_f32_trunc_i32u( float af ) {
         float32_t a = to_softfloat32(af);
         if (_eosio_f32_ge(af, 4294967296.0f) || _eosio_f32_le(af, -1.0f))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f32.convert_u/i32 overflow");
         if (is_nan(a))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f32.convert_u/i32 unrepresentable");
         return f32_to_ui32( to_softfloat32(_eosio_f32_trunc( af )), 0, false );
      }
      uint32_t _eosio_f64_trunc_i32u( double af ) {
         float64_t a = to_softfloat64(af);
         if (_eosio_f64_ge(af, 4294967296.0) || _eosio_f64_le(af, -1.0))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f64.convert_u/i32 overflow");
         if (is_nan(a))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f64.convert_u/i32 unrepresentable");
         return f64_to_ui32( to_softfloat64(_eosio_f64_trunc( af )), 0, false );
      }
      int64_t _eosio_f32_trunc_i64s( float af ) {
         float32_t a = to_softfloat32(af);
         if (_eosio_f32_ge(af, 9223372036854775808.0f) || _eosio_f32_lt(af, -9223372036854775808.0f))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f32.convert_s/i64 overflow");
         if (is_nan(a))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f32.convert_s/i64 unrepresentable");
         return f32_to_i64( to_softfloat32(_eosio_f32_trunc( af )), 0, false );
      }
      int64_t _eosio_f64_trunc_i64s( double af ) {
         float64_t a = to_softfloat64(af);
         if (_eosio_f64_ge(af, 9223372036854775808.0) || _eosio_f64_lt(af, -9223372036854775808.0))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f64.convert_s/i64 overflow");
         if (is_nan(a))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f64.convert_s/i64 unrepresentable");

         return f64_to_i64( to_softfloat64(_eosio_f64_trunc( af )), 0, false );
      }
      uint64_t _eosio_f32_trunc_i64u( float af ) {
         float32_t a = to_softfloat32(af);
         if (_eosio_f32_ge(af, 18446744073709551616.0f) || _eosio_f32_le(af, -1.0f))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f32.convert_u/i64 overflow");
         if (is_nan(a))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f32.convert_u/i64 unrepresentable");
         return f32_to_ui64( to_softfloat32(_eosio_f32_trunc( af )), 0, false );
      }
      uint64_t _eosio_f64_trunc_i64u( double af ) {
         float64_t a = to_softfloat64(af);
         if (_eosio_f64_ge(af, 18446744073709551616.0) || _eosio_f64_le(af, -1.0))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f64.convert_u/i64 overflow");
         if (is_nan(a))
            FC_THROW_EXCEPTION( eosio::chain::wasm_execution_error, "Error, f64.convert_u/i64 unrepresentable");
         return f64_to_ui64( to_softfloat64(_eosio_f64_trunc( af )), 0, false );
      }
      float _eosio_i32_to_f32( int32_t a )  {
         return from_softfloat32(i32_to_f32( a ));
      }
      float _eosio_i64_to_f32( int64_t a ) {
         return from_softfloat32(i64_to_f32( a ));
      }
      float _eosio_ui32_to_f32( uint32_t a ) {
         return from_softfloat32(ui32_to_f32( a ));
      }
      float _eosio_ui64_to_f32( uint64_t a ) {
         return from_softfloat32(ui64_to_f32( a ));
      }
      double _eosio_i32_to_f64( int32_t a ) {
         return from_softfloat64(i32_to_f64( a ));
      }
      double _eosio_i64_to_f64( int64_t a ) {
         return from_softfloat64(i64_to_f64( a ));
      }
      double _eosio_ui32_to_f64( uint32_t a ) {
         return from_softfloat64(ui32_to_f64( a ));
      }
      double _eosio_ui64_to_f64( uint64_t a ) {
         return from_softfloat64(ui64_to_f64( a ));
      }

      static bool is_nan( const float32_t f ) {
         return ((f.v & 0x7FFFFFFF) > 0x7F800000);
      }
      static bool is_nan( const float64_t f ) {
         return ((f.v & 0x7FFFFFFFFFFFFFFF) > 0x7FF0000000000000);
      }
      static bool is_nan( const float128_t& f ) {
         return (((~(f.v[1]) & uint64_t( 0x7FFF000000000000 )) == 0) && (f.v[0] || ((f.v[1]) & uint64_t( 0x0000FFFFFFFFFFFF ))));
      }
      static float32_t to_softfloat32( float f ) {
         return *reinterpret_cast<float32_t*>(&f);
      }
      static float64_t to_softfloat64( double d ) {
         return *reinterpret_cast<float64_t*>(&d);
      }
      static float from_softfloat32( float32_t f ) {
         return *reinterpret_cast<float*>(&f);
      }
      static double from_softfloat64( float64_t d ) {
         return *reinterpret_cast<double*>(&d);
      }
      static constexpr uint32_t inv_float_eps = 0x4B000000;
      static constexpr uint64_t inv_double_eps = 0x4330000000000000;

      static bool sign_bit( float32_t f ) { return f.v >> 31; }
      static bool sign_bit( float64_t f ) { return f.v >> 63; }

};

}
}

using namespace eosio::micropython;

extern "C" {


int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   /*
   int itr = ctx().db_find_i64(receiver, scope, table, id);
   if (itr >= 0) {
      ctx().db_update_i64( itr, payer, buffer, buffer_size );
      return itr;
   }
   */
   return ctx().db_store_i64( scope, table, payer, id, buffer, buffer_size );
}

void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   ctx().db_update_i64( itr, payer, buffer, buffer_size );
}

void db_update_i64_ex( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id, const char* buffer, size_t buffer_size ) {
   int itr = ctx().db_find_i64(ctx().get_receiver(), scope, table, id);
   if (itr >= 0) {
      ctx().db_update_i64( itr, payer, buffer, buffer_size );
   }
}

void db_remove_i64 ( int itr ) {
   ctx().db_remove_i64( itr );
}

void db_remove_i64_ex( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id ) {
   int itr = ctx().db_find_i64(ctx().get_receiver(), scope, table, id);
   if (itr >= 0) {
      ctx().db_remove_i64( itr );
   }
}

int db_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return ctx().db_get_i64( itr, buffer, buffer_size );
}

int db_get_i64_ex( int itr, uint64_t* primary, char* buffer, size_t buffer_size ) {
   return ctx().db_get_i64_ex( itr, *primary, buffer, buffer_size );
}

const char* db_get_i64_exex( int itr, size_t* buffer_size ) {
   return ctx().db_get_i64_exex( itr,  buffer_size);
}

int db_next_i64( int itr, uint64_t* primary ) {
   return ctx().db_next_i64(itr, *primary);
}
int db_previous_i64( int itr, uint64_t* primary ) {
   return ctx().db_previous_i64(itr, *primary);
}
int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return ctx().db_find_i64( code, scope, table, id );
}
int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return ctx().db_lowerbound_i64( code, scope, table, id );
}
int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return ctx().db_upperbound_i64( code, scope, table, id );
}
int db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return ctx().db_end_i64( code, scope, table );
}

#define DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(IDX, TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary , size_t len) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         try { \
            return ctx().IDX.store( scope, table, payer, id, *((TYPE*)secondary) );\
         } catch (...) { \
            return -1; \
         } \
      }\
      void db_##IDX##_update( int iterator, uint64_t payer, const char* secondary , size_t len ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.update( iterator, payer, *((TYPE*)secondary) );\
      }\
      void db_##IDX##_remove( int iterator ) {\
         return ctx().IDX.remove( iterator );\
      }\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* secondary , size_t len, uint64_t* primary ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.find_secondary(code, scope, table, *((TYPE*)secondary), *primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t primary ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.find_primary(code, scope, table, *((TYPE*)secondary), primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary , size_t len, uint64_t* primary ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.lowerbound_secondary(code, scope, table, *((TYPE*)secondary), *primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary , size_t len, uint64_t* primary ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.upperbound_secondary(code, scope, table, *((TYPE*)secondary), *primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return ctx().IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return ctx().IDX.next_secondary(iterator, *primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return ctx().IDX.previous_secondary(iterator, *primary);\
      }

#define DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(IDX, ARR_SIZE, ARR_ELEMENT_TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* data, size_t data_len) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.store(scope, table, payer, id, (ARR_ELEMENT_TYPE*)data);\
      }\
      void db_##IDX##_update( int iterator, uint64_t payer, const char* data, size_t data_len ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.update(iterator, payer, (ARR_ELEMENT_TYPE*)data);\
      }\
      void db_##IDX##_remove( int iterator ) {\
         return ctx().IDX.remove(iterator);\
      }\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* data, size_t data_len, uint64_t* primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.find_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.find_primary(code, scope, table, (ARR_ELEMENT_TYPE*)data, primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t* primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.lowerbound_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t* primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.upperbound_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return ctx().IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return ctx().IDX.next_secondary(iterator, *primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return ctx().IDX.previous_secondary(iterator, *primary);\
      }

#define DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY(IDX, TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary, size_t data_len ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return ctx().IDX.store( scope, table, payer, id, *((float64_t*)secondary) );\
      }\
      void db_##IDX##_update( int iterator, uint64_t payer, const char* secondary, size_t data_len ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return ctx().IDX.update( iterator, payer, *((float64_t*)secondary) );\
      }\
      void db_##IDX##_remove( int iterator ) {\
         return ctx().IDX.remove( iterator );\
      }\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return ctx().IDX.find_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* secondary, size_t data_len, uint64_t primary ) {\
         return ctx().IDX.find_primary(code, scope, table, *((float64_t*)secondary), primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return ctx().IDX.lowerbound_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return ctx().IDX.upperbound_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return ctx().IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return ctx().IDX.next_secondary(iterator, *primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return ctx().IDX.previous_secondary(iterator, *primary);\
      }

DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx64,  uint64_t)
DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx128, eosio::chain::uint128_t)
DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(idx256, 2, eosio::chain::uint128_t)
DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY(idx_double, uint64_t)


///////////////////////////////////////////////////////////////////////////////////////////////////
//context_free_transaction_api
int read_transaction( char* data, size_t data_len ) {
   bytes trx = ctx().get_packed_transaction();
   if (data_len >= trx.size()) {
      memcpy(data, trx.data(), trx.size());
   }
   return trx.size();
}

int transaction_size() {
   return ctx().get_packed_transaction().size();
}

int expiration() {
  return ctx().trx_context.trx.expiration.sec_since_epoch();
}

int tapos_block_num() {
   return ctx().trx_context.trx.ref_block_num;
}
int tapos_block_prefix() {
  return ctx().trx_context.trx.ref_block_prefix;
}

int get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size ) {
   return ctx().get_action( type, index, buffer, buffer_size );
}

///////////////////////////////////////////////////////////////////////////
//action_api

int read_action(char* memory, size_t size) {
   FC_ASSERT(size > 0);
   int minlen = std::min<size_t>(ctx().act.data.size(), size);
   memcpy((void *)memory, ctx().act.data.data(), minlen);
   return minlen;
}

int action_size() {
   return ctx().act.data.size();
}

uint64_t current_receiver() {
   return ctx().receiver.value;
}

uint64_t publication_time() {
   return ctx().trx_context.published.time_since_epoch().count();
}

uint64_t current_sender() {
   assert(0);
}

//apply_context
void require_auth(uint64_t account) {
   ctx().require_authorization(account_name(account));
}

void require_auth_ex(uint64_t account, uint64_t permission) {
   ctx().require_authorization(account_name(account), name(permission));
}

int is_account(uint64_t account) {
   return ctx().is_account(name(account));
}

void require_recipient(uint64_t account) {
   ctx().require_recipient(name(account));
}

//producer_api
int get_active_producers(uint64_t* producers, size_t datalen) {
   auto active_producers = ctx().get_active_producers();
   size_t len = active_producers.size();
   size_t cpy_len = std::min(datalen, len);
   memcpy(producers, active_producers.data(), cpy_len * sizeof(chain::account_name) );
   return len;
}

bool unpack_action(struct mp_action* mp_act , action& act) {
   act.account = name(mp_act->account);
   act.name = name(mp_act->name);
   act.data.resize(0);
   act.data.resize(mp_act->data_len);
   memcpy(act.data.data(), mp_act->data, mp_act->data_len);

   for (int i=0;i<mp_act->auth_len;i+=2) {
      account_name    actor = name(mp_act->auth[i]);
      permission_name permission = name(mp_act->auth[i+1]);
      act.authorization.push_back({actor, permission});
   }
   return true;
}

bool unpack_transaction(struct mp_transaction* mp_trx, transaction& trx) {
   if (mp_trx == NULL) {
      return false;
   }

   trx.expiration             = time_point_sec(mp_trx->expiration);
   trx.ref_block_num          = mp_trx->ref_block_num;
   trx.ref_block_prefix       = mp_trx->ref_block_prefix;
   trx.max_net_usage_words    = mp_trx->max_net_usage_words;
   trx.delay_sec              = mp_trx->delay_sec;

   for (int i=0;i<mp_trx->free_actions_len;i++) {
      action act;
      unpack_action(mp_trx->context_free_actions[i], act);
      trx.context_free_actions.emplace_back(std::move(act));
   }

   for (int i=0;i<mp_trx->actions_len;i++) {
      action act;
      unpack_action(mp_trx->actions[i], act);
      trx.actions.emplace_back(std::move(act));
   }

   for (int i=0;i<mp_trx->extensions_len;i++) {
      char* ext = mp_trx->extensions[i];
      int type = ((short*)ext)[0];
      int data_len = ((short*)ext)[1];
      trx.transaction_extensions.emplace_back(std::make_pair(type, vector<char>(ext+4, ext+4+data_len)));
   }

   return true;

}

void send_inline( struct mp_action* mp_act ) {
   // TODO: use global properties object for dynamic configuration of this default_max_gen_trx_size
   FC_ASSERT( mp_act && mp_act->data_len < config::default_max_inline_action_size, "inline action too big" );

   action act;
   unpack_action(mp_act, act);

   ctx().execute_inline(std::move(act));
}

void send_context_free_inline(struct mp_action* mp_act) {
   // TODO: use global properties object for dynamic configuration of this default_max_gen_trx_size
   FC_ASSERT( mp_act && mp_act->data_len < config::default_max_inline_action_size, "inline action too big" );

   action act;
   unpack_action(mp_act, act);

   ctx().execute_context_free_inline(std::move(act));
}

void send_deferred( eosio::chain::uint128_t sender_id, uint64_t payer, struct mp_transaction* mp_trx ) {

   eosio::chain::transaction trx;
   try {
      unpack_transaction(mp_trx, trx);
      ctx().schedule_deferred_transaction(sender_id, payer, std::move(trx), false);
   } FC_CAPTURE_AND_RETHROW((fc::to_hex((char*)&trx, sizeof(trx))));

}

void cancel_deferred( eosio::chain::uint128_t  val ) {
   fc::uint128_t sender_id(val>>64, uint64_t(val) );
//   ctx().cancel_deferred( (eosio::chain::uint128_t)sender_id );
}

uint32_t now() {
   auto& ctrl = ctx().control;
   return ctrl.head_block_time().sec_since_epoch();
}

void eosio_abort() {
   edump(("abort() called"));
   FC_ASSERT( false, "abort() called");
}

void eosio_assert(int condition, const char* str) {
   std::string message( str );
   if( !condition ) edump((message));
   FC_ASSERT( condition, "assertion failed: ${s}", ("s",message));
}

//class crypto_api
void assert_recover_key( const char* data, size_t data_len, const char* sig, size_t siglen, const char* pub, size_t publen ) {
   fc::sha256 digest( data, data_len);
   fc::crypto::signature s;
   fc::crypto::public_key p;
   fc::datastream<const char*> ds( sig, siglen );
   fc::datastream<const char*> pubds( pub, publen );

   fc::raw::unpack(ds, s);
   fc::raw::unpack(pubds, p);

   auto check = fc::crypto::public_key( s, digest, false );
   FC_ASSERT( check == p, "Error expected key different than recovered key" );
}

mp_obj_t recover_key(const char* data, size_t size, const char* sig, size_t siglen ) {
   char pub[256];
   fc::sha256 digest(data, size);
   fc::crypto::signature s;
   fc::datastream<const char*> ds( sig, siglen );
   fc::datastream<char*> pubds( pub, sizeof(pub) );

   fc::raw::unpack(ds, s);
   fc::raw::pack( pubds, fc::crypto::public_key( s, digest, false ) );

   return get_mpapi().mp_obj_new_str(pub, pubds.tellp());
}

void assert_sha256(const char* data, size_t datalen, const char* hash, size_t hash_len) {
   auto result = fc::sha256::hash( data, datalen );
   fc::sha256 hash_val( hash, hash_len );
   FC_ASSERT( result == hash_val, "hash miss match" );
}

void assert_sha1(const char* data, size_t datalen, const char* hash, size_t hash_len) {
   auto result = fc::sha1::hash( data, datalen );
   fc::sha1 hash_val( string(hash, hash_len) );
   FC_ASSERT( result == hash_val, "hash miss match" );
}

void assert_sha512(const char* data, size_t datalen, const char* hash, size_t hash_len) {
   auto result = fc::sha512::hash( data, datalen );
   fc::sha512 hash_val( string(hash, hash_len) );
   FC_ASSERT( result == hash_val, "hash miss match" );
}

void assert_ripemd160(const char* data, size_t datalen, const char* hash, size_t hash_len) {
   auto result = fc::ripemd160::hash( data, datalen );
   fc::ripemd160 hash_val( string(hash, hash_len) );
   FC_ASSERT( result == hash_val, "hash miss match" );
}

mp_obj_t sha1_(const char* data, size_t datalen) {
   string str_hash = fc::sha1::hash( data, datalen ).str();
   return get_mpapi().mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t sha256_(const char* data, size_t datalen) {
   string str_hash = fc::sha256::hash( data, datalen ).str();
   return get_mpapi().mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t sha512_(const char* data, size_t datalen) {
   string str_hash = fc::sha512::hash( data, datalen ).str();
   return get_mpapi().mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t ripemd160_(const char* data, size_t datalen) {
   string str_hash = fc::ripemd160::hash( data, datalen ).str();
   return get_mpapi().mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

uint64_t get_action_account() {
   return ctx().act.account.value;
}


#if 0
uint64_t string_to_symbol( uint8_t precision, const char* str ) {
   uint32_t len = 0;
   while( str[len] ) ++len;

   uint64_t result = 0;
   for( uint32_t i = 0; i < len; ++i ) {
      if( str[i] < 'A' || str[i] > 'Z' ) {
         /// ERRORS?
      } else {
         result |= (uint64_t(str[i]) << (8*(1+i)));
      }
   }

   result |= uint64_t(precision);
   return result;
}
#endif

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
  #include <windows.h>
  extern "C" void eosio_delay(int ms) {
     Sleep( ms );
  }
#else  /* presume POSIX */
  extern "C" void eosio_delay(int ms) {
     usleep(ms*1000);
  }
#endif

int split_path(const char* str_path, char *path1, size_t path1_size, char *path2, size_t path2_size) {
   std::vector<std::string> v;
   const boost::filesystem::path _path(str_path);
   for (const auto &p : _path) {
       v.push_back(p.filename().string());
   }

   if (v.size() >= 1) {
      if (path1_size > v[0].size()) {
         strncpy(path1, v[0].c_str(), v[0].size());
      } else {
         strncpy(path1, v[0].c_str(), path1_size);
      }
   }

   if (v.size() >= 2) {
      if (path1_size > v[1].size()) {
         strncpy(path2, v[1].c_str(), v[1].size());
      } else {
         strncpy(path2, v[1].c_str(), path1_size);
      }
   }

   return v.size();
}

int wasm_call_(uint64_t _code, const char* _func, size_t _func_size, uint64_t* _args, size_t _args_size) {
   if (_func == NULL || _func_size <= 0) {
      return 0;
   }

   string func(_func, _func_size);
   vector<uint64_t> args;
   if (_args == NULL || _args_size == 0) {
      //
   } else {
      args = vector<uint64_t>(_args, _args + _args_size);
   }

   apply_context& ctx = apply_context::ctx();

   const auto &a = ctx.control.db().get<account_object, by_name>(name(_code));
   auto& interface = ctx.control.get_wasm_interface();
   interface.call(a.code_version, a.code, func, args, ctx);

   return 1;

}

void init_eosapi() {
   static bool _init = false;
   if (_init) {
      return;
   }
   _init = true;
   s_eosapi.wasm_call = wasm_call_;
   s_eosapi.string_to_symbol = string_to_symbol;
   s_eosapi.eosio_delay = eosio_delay;
   s_eosapi.now = now;
   s_eosapi.abort_ = abort;
   s_eosapi.eosio_assert = eosio_assert;
   s_eosapi.assert_recover_key = assert_recover_key;

   s_eosapi.recover_key = recover_key;
   s_eosapi.assert_sha256 = assert_sha256;
   s_eosapi.assert_sha1 = assert_sha1;
   s_eosapi.assert_sha512 = assert_sha512;
   s_eosapi.assert_ripemd160 = assert_ripemd160;
   s_eosapi.sha1 = sha1_;
   s_eosapi.sha256 = sha256_;
   s_eosapi.sha512 = sha512_;
   s_eosapi.ripemd160 = ripemd160_;

   s_eosapi.string_to_uint64_ = string_to_uint64_;
   s_eosapi.uint64_to_string_ = uint64_to_string_;

   s_eosapi.pack_ = pack_;
   s_eosapi.unpack_ = unpack_;

//   s_eosapi.get_account_balance_
//   s_eosapi.get_active_producers_

   s_eosapi.read_transaction = read_transaction;
   s_eosapi.transaction_size = transaction_size;
   s_eosapi.expiration = expiration;
   s_eosapi.tapos_block_num = tapos_block_num;
   s_eosapi.tapos_block_prefix = tapos_block_prefix;
   s_eosapi.get_action = get_action;


   s_eosapi.require_auth = require_auth;
   s_eosapi.require_auth_ex = require_auth_ex;
   s_eosapi.is_account = is_account;
   s_eosapi.require_recipient = require_recipient;


   s_eosapi.read_action = read_action;
   s_eosapi.action_size = action_size;
   s_eosapi.current_receiver = current_receiver;
   s_eosapi.publication_time = publication_time;
   s_eosapi.current_sender = current_sender;

   s_eosapi.db_store_i64 = db_store_i64;
   s_eosapi.db_update_i64 = db_update_i64;
   s_eosapi.db_remove_i64 = db_remove_i64;

   s_eosapi.db_get_i64 = db_get_i64;
   s_eosapi.db_get_i64_ex = db_get_i64_ex;
   s_eosapi.db_get_i64_exex = db_get_i64_exex;

   s_eosapi.db_next_i64 = db_next_i64;
   s_eosapi.db_previous_i64 = db_previous_i64;
   s_eosapi.db_find_i64 = db_find_i64;
   s_eosapi.db_lowerbound_i64 = db_lowerbound_i64;
   s_eosapi.db_upperbound_i64 = db_upperbound_i64;
   s_eosapi.db_end_i64 = db_end_i64;

   s_eosapi.db_idx64_store = db_idx64_store;
   s_eosapi.db_idx64_update = db_idx64_update;
   s_eosapi.db_idx64_remove = db_idx64_remove;
   s_eosapi.db_idx64_find_secondary = db_idx64_find_secondary;
   s_eosapi.db_idx64_find_primary = db_idx64_find_primary;
   s_eosapi.db_idx64_lowerbound = db_idx64_lowerbound;
   s_eosapi.db_idx64_upperbound = db_idx64_upperbound;
   s_eosapi.db_idx64_end = db_idx64_end;
   s_eosapi.db_idx64_next = db_idx64_next;
   s_eosapi.db_idx64_previous = db_idx64_previous;

   s_eosapi.db_idx128_store = db_idx128_store;
   s_eosapi.db_idx128_update = db_idx128_update;
   s_eosapi.db_idx128_remove = db_idx128_remove;
   s_eosapi.db_idx128_find_secondary = db_idx128_find_secondary;
   s_eosapi.db_idx128_find_primary = db_idx128_find_primary;
   s_eosapi.db_idx128_lowerbound = db_idx128_lowerbound;
   s_eosapi.db_idx128_upperbound = db_idx128_upperbound;
   s_eosapi.db_idx128_end = db_idx128_end;
   s_eosapi.db_idx128_next = db_idx128_next;
   s_eosapi.db_idx128_previous = db_idx128_previous;

   s_eosapi.db_idx_double_store = db_idx_double_store;
   s_eosapi.db_idx_double_update = db_idx_double_update;
   s_eosapi.db_idx_double_remove = db_idx_double_remove;
   s_eosapi.db_idx_double_find_secondary = db_idx_double_find_secondary;
   s_eosapi.db_idx_double_find_primary = db_idx_double_find_primary;
   s_eosapi.db_idx_double_lowerbound = db_idx_double_lowerbound;
   s_eosapi.db_idx_double_upperbound = db_idx_double_upperbound;
   s_eosapi.db_idx_double_end = db_idx_double_end;
   s_eosapi.db_idx_double_next = db_idx_double_next;
   s_eosapi.db_idx_double_previous = db_idx_double_previous;

   s_eosapi.db_idx256_store = db_idx256_store;
   s_eosapi.db_idx256_update = db_idx256_update;
   s_eosapi.db_idx256_remove = db_idx256_remove;
   s_eosapi.db_idx256_find_secondary = db_idx256_find_secondary;
   s_eosapi.db_idx256_find_primary = db_idx256_find_primary;
   s_eosapi.db_idx256_lowerbound = db_idx256_lowerbound;
   s_eosapi.db_idx256_upperbound = db_idx256_upperbound;
   s_eosapi.db_idx256_end = db_idx256_end;
   s_eosapi.db_idx256_next = db_idx256_next;
   s_eosapi.db_idx256_previous = db_idx256_previous;

   s_eosapi.send_inline = send_inline;
   s_eosapi.send_context_free_inline = send_context_free_inline;
   s_eosapi.send_deferred = send_deferred;
   s_eosapi.cancel_deferred = cancel_deferred;

   s_eosapi.split_path = split_path;
   s_eosapi.get_action_account = get_action_account;
}

void init_api() {
   init_eosapi();
//   get_mpapi();

}



}


