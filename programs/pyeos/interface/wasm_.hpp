#ifndef __WASM_HPP_
#define __WASM_HPP_
#include <string>
#include <vector>
using namespace std;

int wasm_test_(string& code, string& func, vector<uint64_t>& args, uint64_t _account, uint64_t _action, vector<char>& data);


#endif
