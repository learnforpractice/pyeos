#pragma once
#include <string>
#include <micropython/mpeoslib.h>

void debug_test();

void set_debug_mode(int mode);
int get_debug_mode();

void run_code_(std::string code);

//mpeoslib.cpp
struct mpapi& get_mpapi();
