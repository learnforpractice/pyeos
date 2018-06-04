#pragma once
#include <string>
#include <micropython/mpeoslib.h>

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
