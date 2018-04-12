/*
 * common.hpp
 *
 *  Created on: Nov 2, 2017
 *      Author: newworld
 */

#ifndef PROGRAMS_PYEOS_INTERFACE_COMMON_HPP_
#define PROGRAMS_PYEOS_INTERFACE_COMMON_HPP_
#include <eos/types/native.hpp>
#include <string>
#include <vector>

using namespace eosio::types;

static void emplace_scope(const name& scope, std::vector<name>& scopes) {
   auto i = std::upper_bound( scopes.begin(), scopes.end(), scope);
   if (i == scopes.begin() || *(i - 1) != scope ) {
     scopes.insert(i, scope);
   }
}

void vector_to_string(std::vector<char>& v, std::string& str) {
   str = std::string(v.begin(),v.end());
}

void string_to_vector(std::string& str, std::vector<char>& v) {
   v = std::vector<char>(str.begin(),str.end());
}

#endif /* PROGRAMS_PYEOS_INTERFACE_COMMON_HPP_ */
