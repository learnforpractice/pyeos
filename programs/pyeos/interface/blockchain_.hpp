/*
 * chain_controller_.cpp
 *
 *  Created on: Oct 26, 2017
 *      Author: newworld
 */

namespace python {

class pychain_controller {
public:
   void new_apply_context();
};

void* new_apply_context_(char* code, void* trx, void* message);
int release_apply_context_(void* apply_ctx);
void set_current_context_(void* context_ptr);
void apply_message_(void* context_ptr);

}


