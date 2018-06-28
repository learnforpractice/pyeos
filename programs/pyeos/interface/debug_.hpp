#pragma once
#include <string>
#include <micropython/mpeoslib.h>

using namespace std;

void debug_test();

void set_debug_mode(int mode);
int get_debug_mode();

void run_code_(std::string code);

//mpeoslib.cpp
struct mpapi& get_mpapi();

void py_debug_enable_(int enable);
bool py_debug_enabled_();

void wasm_debug_enable_(int enable);
bool wasm_debug_enabled_();

void set_debug_contract_(string& _account, string& path);

int mp_is_account2(string& account);

void wasm_enable_native_contract_(bool b);
bool wasm_is_native_contract_enabled_();

//mpeoslib.cpp
void mp_set_max_execution_time_(int _max);

//application.cpp
void app_set_debug_mode_(bool d);

uint64_t wasm_test_action_(const char* cls, const char* method);

void block_log_test_(string& path, int start_block, int end_block);
