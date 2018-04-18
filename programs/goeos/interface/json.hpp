#pragma once
#include <fc/filesystem.hpp>
#include <fc/variant.hpp>
#include "../../goeos/interface/pyobject.hpp"

namespace python {
using namespace fc;

using std::ostream;

/**
 *  Provides interface for json serialization.
 *
 *  json strings are always UTF8
 */
class json {
  public:
   enum parse_type {
      legacy_parser = 0,
      strict_parser = 1,
      relaxed_parser = 2,
      legacy_parser_with_string_doubles = 3
   };
   enum output_formatting {
      stringify_large_ints_and_doubles = 0,
      legacy_generator = 1
   };

   static ostream& to_stream(ostream& out, const fc::string&);
   static ostream& to_stream(
       ostream& out, const variant& v,
       output_formatting format = stringify_large_ints_and_doubles);
   static ostream& to_stream(
       ostream& out, const variants& v,
       output_formatting format = stringify_large_ints_and_doubles);
   static ostream& to_stream(
       ostream& out, const variant_object& v,
       output_formatting format = stringify_large_ints_and_doubles);
   static PyDict& to_stream(
       PyDict& dict, const variants& v,
       output_formatting format = stringify_large_ints_and_doubles);

   static variant from_string(const string& utf8_str,
                              parse_type ptype = legacy_parser);
   static variants variants_from_string(const string& utf8_str,
                                        parse_type ptype = legacy_parser);
   static PyObject* to_string(
       const variant& v,
       output_formatting format = stringify_large_ints_and_doubles);

   static bool is_valid(const std::string& json_str,
                        parse_type ptype = legacy_parser);

   static variant from_file(const fc::path& p,
                            parse_type ptype = legacy_parser);

   template <typename T>
   static T from_file(const fc::path& p, parse_type ptype = legacy_parser) {
      return json::from_file(p, ptype).as<T>();
   }

   template <typename T>
   static PyObject* to_string(
       const T& v,
       output_formatting format = stringify_large_ints_and_doubles) {
      return to_string(variant(v), format);
   }
};

}  // namespace python
