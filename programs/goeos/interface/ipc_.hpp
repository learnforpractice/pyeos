#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

void* ipc_create(std::string& name, int max_num_msg, int max_msg_size);
void* ipc_open(std::string& name);
int ipc_receive(void* _mq, vector<char>& buffer);
int ipc_send(void* _mq, string& buffer);
void ipc_remove(string& name);
