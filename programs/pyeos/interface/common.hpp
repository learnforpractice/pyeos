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

using namespace eos::types;
using namespace std;

static void emplace_scope(const Name& scope, std::vector<Name>& scopes) {
   auto i = std::upper_bound( scopes.begin(), scopes.end(), scope);
   if (i == scopes.begin() || *(i - 1) != scope ) {
     scopes.insert(i, scope);
   }
}

void vector_to_string(vector<char>& v, string& str) {
   str = string(v.begin(),v.end());
}

void string_to_vector(string& str, vector<char>& v) {
   v = vector<char>(str.begin(),str.end());
}

#endif /* PROGRAMS_PYEOS_INTERFACE_COMMON_HPP_ */
