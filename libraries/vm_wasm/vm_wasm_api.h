/*
 * vm_wasm_api.h
 *
 *  Created on: Jun 16, 2018
 *      Author: newworld
 */

#ifndef VM_WASM_VM_WASM_API_H_
#define VM_WASM_VM_WASM_API_H_


struct vm_wasm_api {
   int  (*wasm_to_wast)( const uint8_t* data, size_t size, uint8_t* wast, size_t wast_size );
   int  (*wast_to_wasm)( const uint8_t* data, size_t size, uint8_t* wasm, size_t wasm_size );

};


#endif /* VM_WASM_VM_WASM_API_H_ */
