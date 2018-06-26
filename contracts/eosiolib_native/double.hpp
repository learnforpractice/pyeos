/*
 * double.hpp
 *
 *  Created on: Jun 26, 2018
 *      Author: newworld
 */

#ifndef CONTRACTS_EOSIOLIB_NATIVE_DOUBLE_HPP_
#define CONTRACTS_EOSIOLIB_NATIVE_DOUBLE_HPP_

#ifndef __WASM
#include <softfloat.hpp>

static float64_t to_softfloat64( double d ) {
   float64_t f;
   f.v = (uint64_t)d;
   return f;
}

class Double
{
public:
    float64_t _v;
public:
    Double() : _v(){

    }

     Double(const float64_t& v) : _v(to_softfloat64(0.0)){

     }

     Double(const double& v) : _v(to_softfloat64(v)){

     }

     Double(const Double& v) : _v(v._v){

     }

     Double& operator= (const double& __v) {
        _v = to_softfloat64(__v);
        return *this;
     }

     Double& operator= (const Double& __v) {
        _v = __v._v;
        return *this;
     }

     Double operator- () {
       float64_t __v = f64_sub(to_softfloat64(0.0), _v);
       return Double(__v);
    }

    Double operator-(const double& b) {
       float64_t __v = f64_sub(to_softfloat64(0.0), to_softfloat64(b));
       return Double(__v);
    }

    Double operator-(const Double& b) {
       float64_t __v = f64_sub(_v, b._v);
       return Double(__v);
    }

     Double operator+(const double& b) {
        float64_t __v = f64_add(_v, to_softfloat64(b));
        return Double(__v);
     }

    Double operator+(const Double& b) {
       float64_t __v = f64_add(_v, b._v);
       return Double(__v);
    }

    Double operator* (const double& b) {
       float64_t __v = f64_mul(_v, to_softfloat64(b));
       return Double(__v);
    }

    Double operator* (const Double& b) {
       float64_t __v = f64_mul(_v, b._v);
       return Double(__v);
    }

    Double operator/ (const double& b) {
       float64_t __v = f64_div(_v, to_softfloat64(b));
       return Double(__v);
    }

    Double operator/ (const Double& b) {
       float64_t __v = f64_div(_v, b._v);
       return Double(__v);
    }

     Double& operator+=(const double& __v) {
        _v = f64_add(_v, to_softfloat64(__v) );
        return *this;
     }

     Double& operator-=(const double& __v) {
        _v = f64_sub(_v, to_softfloat64(__v) );
        return *this;
     }

     Double& operator*=(const double& __v) {
        _v = f64_mul(_v, to_softfloat64(__v) );
        return *this;
     }

     Double& operator+=(const Double& __v)
     {
        _v = f64_add(_v, __v._v );
         return *this;
     }

     Double& operator-=(const Double& __v)
     {
        _v = f64_sub(_v, __v._v );
         return *this;
     }

     Double& operator*=(const Double& __v)
     {
        _v = f64_mul(_v, __v._v );
         return *this;
     }

     Double& operator/=(const double& __v) {
        _v = f64_div(_v, to_softfloat64(__v) );
        return *this;
     }

     Double& operator/=(const Double& __v)
     {
        _v = f64_div(_v, __v._v );
         return *this;
     }

     operator int64_t()const {
        return int64_t(_v.v);
     }

};

namespace std {
   Double pow(Double base,Double power);
}

#else
   typedef double Double;
#endif

#endif /* CONTRACTS_EOSIOLIB_NATIVE_Double_HPP_ */
