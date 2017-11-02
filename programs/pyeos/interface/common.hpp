/*
 * common.hpp
 *
 *  Created on: Nov 2, 2017
 *      Author: newworld
 */

#ifndef PROGRAMS_PYEOS_INTERFACE_COMMON_HPP_
#define PROGRAMS_PYEOS_INTERFACE_COMMON_HPP_
#include <eos/types/native.hpp>

using namespace eos::types;

static void emplace_scope(const Name& scope, std::vector<Name>& scopes) {
   auto i = std::upper_bound( scopes.begin(), scopes.end(), scope);
   if (i == scopes.begin() || *(i - 1) != scope ) {
     scopes.insert(i, scope);
   }
}


#endif /* PROGRAMS_PYEOS_INTERFACE_COMMON_HPP_ */
