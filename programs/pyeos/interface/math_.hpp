/*
 * math_.hpp
 *
 *  Created on: Oct 10, 2017
 *      Author: newworld
 */

#ifndef INTERFACE_MATH__HPP_
#define INTERFACE_MATH__HPP_

typedef unsigned long long uint64_t;
typedef unsigned __int128 uint128_t;

void mul64_(void* n1, void* n2, void* out);
void div64_(void* n1, void* n2, void* out);
void add64_(void* n1, void* n2, void* out);
void sub64_(void* n1, void* n2, void* out);
void mul128_(void* n1, void* n2, void* out);
void div128_(void* n1, void* n2, void* out);
void add128_(void* n1, void* n2, void* out);
void sub128_(void* n1, void* n2, void* out);

int gt64_(void* n1, void* n2);
int ge64_(void* n1, void* n2);
int eq64_(void* n1, void* n2);
int lt64_(void* n1, void* n2);
int le64_(void* n1, void* n2);

int gt128_(void* n1, void* n2);
int ge128_(void* n1, void* n2);
int eq128_(void* n1, void* n2);
int lt128_(void* n1, void* n2);
int le128_(void* n1, void* n2);

#endif /* INTERFACE_MATH__HPP_ */
