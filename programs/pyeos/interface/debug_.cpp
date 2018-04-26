#include "debug_.hpp"
#include <libdevcore/CommonData.h>
#include <fc/log/logger.hpp>
#include <fc/variant_object.hpp>
#include <fc/reflect/reflect.hpp>

#include <fc/io/raw.hpp>

#include <fc/reflect/variant.hpp>


using namespace dev;

using namespace fc;
using namespace std;

struct ABC {
   int a;
   int b;
//   int c;
   std::vector<char> c;
};

FC_REFLECT( ABC, (a)(b)(c) )

class visitor {
public:
   fc::mutable_variant_object& var;
   ABC& v;
   visitor(fc::mutable_variant_object& _var, ABC& _v) : var(_var), v(_v) {
   }
   template <typename Member, class Class, Member (Class::*member)>
   void operator() (const char* name) const {
      add(var, name, (v.*member)); // or an ugly but efficient solution: (*((fc::mutable_variant_object*)&var))(name, v.*member);
      wlog("${n}", ("n", v.*member));
   }
   template<typename M>
   void add(fc::mutable_variant_object& v, const char* name, M& m) const {
      v(name, m);
   }
};

//#include <fc/io/raw_fwd.hpp>
//#include <fc/io/raw_variant.hpp>

class pack_visitor {
public:
   ABC& v;
   fc::datastream<char*>& ds; //can not declare as fc::datastream<char*> ds, C++ is really tricky.
   pack_visitor(ABC& _v, fc::datastream<char*>& _ds) : v(_v), ds(_ds) {
   }
   template <typename Member, class Class, Member (Class::*member)>
   void operator() (const char*) const {
      pack(ds, v.*member); // can not call fc::raw::pack( ds, m); here since method is declared as "CONST"
//      ((fc::datastream<char*>*)&ds)->write((char*)&(v.*member), sizeof(Member)); //wrong implementation
      wlog("${n}", ("n", v.*member));
   }

   template<typename M>
   void pack(fc::datastream<char*>& _ds, M& m) const {
      fc::raw::pack( _ds, m);
   }
};


void debug_test() {
   std::vector<char> v;
   auto mvar = fc::mutable_variant_object()("a", v)("b", 123);
   mvar("d", 133.5);
   wlog("${a}, ${b}, ${d}", ("a", mvar["a"])("b", mvar["b"])("d", mvar["d"]));

   variant var;

   ABC a1 = {1, 2};
   ABC a2 = {3, 4};

   auto r = fc::reflector<ABC>();
   fc::reflector<ABC>::visit(visitor(mvar, a1));

   v.resize(sizeof(ABC));
   fc::datastream<char*> ds(v.data(), v.size());
   fc::reflector<ABC>::visit(pack_visitor(a1, ds));

   //defined in #include <fc/io/raw.hpp>
   fc::raw::pack_size(a1);
   fc::raw::pack(ds, a1);
   fc::raw::pack(a1);

   wlog("${n}", ("n", fc::variant(v).as_string()));

   fc::variant vv = fc::variant(a1); // or fc::to_variant(a1, vv); DONOT FORGET to include <fc/reflect/variant.hpp>
   auto vo = vv.get_object();
   wlog("${a}, ${b}", ("a", vo["a"])("b", vo["b"]));
   try {
      vo["d"];
   } catch (fc::key_not_found_exception& ex) {
      elog(ex.to_detail_string());
   }

   auto a3 = vv.as<ABC>();
   wlog("${a}, ${b}", ("a", a3.a)("b", a3.b));

//  the same
//   fc::to_variant(a1, vv);

   fc::to_variant(mvar, vv);
   fc::from_variant(vv, mvar);

   wlog("${n}", ("n", fc::variant(v).as_string()));

}

void debug_test2() {
   std::string s("abc.py");
   wlog("${n}", ("n", s.find(".py")));
   wlog("${n}", ("n", s.rfind(".py")));
}

void debug_test__() {
   u256 _key = u256(300);
   dev::bytes __key = dev::toBigEndian(_key);
   u256 ___key = dev::fromBigEndian<u256>(__key);
   wlog(___key.str());

}
