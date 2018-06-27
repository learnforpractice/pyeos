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
   return *(float64_t*)&d;
}

static double from_softfloat64( float64_t d ) {
   return *(double*)&d;
}

class Double
{
public:
   union {
      float64_t f;
      uint64_t i;
      double d;
   } _v;

public:
    Double() {
       _v.i = 0;
    }

    Double(int32_t& n) {
       _v.f = i32_to_f64(n);
    }

    Double(uint32_t& n) {
       _v.f = ui32_to_f64(n);
    }

    Double(int64_t& n) {
       _v.f = i64_to_f64(n);
    }

    Double(uint64_t& n) {
       _v.f = ui64_to_f64(n);
    }

    Double(const float64_t& v) {
        _v.f = v;
     }

     Double(const double& v) {
        _v.d = v;
     }

     Double(const Double& v) {
        _v = v._v;
     }

     Double& operator= (const double& v) {
        _v.d = v;
        return *this;
     }

     Double& operator= (const Double& v) {
        if (this == &v) {
           return *this;
        }
        _v = v._v;
        return *this;
     }

     Double operator- () {
       float64_t __v = f64_sub(to_softfloat64(0.0), _v.f);
       return Double(__v);
    }

    Double operator-(const double& b) {
       float64_t __v = f64_sub(_v.f, to_softfloat64(b));
       return Double(__v);
    }

    Double operator-(const Double& b) {
       float64_t __v = f64_sub(_v.f, b._v.f);
       return Double(__v);
    }

    Double operator+(const double& b) {
       float64_t __v = f64_add(_v.f, to_softfloat64(b));
       return Double(__v);
    }

    Double operator+(const Double& b) {
       float64_t __v = f64_add(_v.f, b._v.f);
       return Double(__v);
    }

    Double operator* (const double& b) {
       float64_t __v = f64_mul(_v.f, to_softfloat64(b));
       return Double(__v);
    }

    Double operator* (const Double& b) {
       float64_t __v = f64_mul(_v.f, b._v.f);
       return Double(__v);
    }

    Double operator/ (const double& b) {
       float64_t __v = f64_div(_v.f, to_softfloat64(b));
       return Double(__v);
    }

    Double operator/ (const Double& b) {
       float64_t __v = f64_div(_v.f, b._v.f);
       return Double(__v);
    }

     Double& operator+=(const double& v) {
        _v.f = f64_add(_v.f, to_softfloat64(v) );
        return *this;
     }

     Double& operator+=(const Double& v)
     {
        _v.f = f64_add(_v.f, v._v.f );
         return *this;
     }

     Double& operator-=(const double& v) {
        _v.f = f64_sub(_v.f, to_softfloat64(v) );
        return *this;
     }

     Double& operator*=(const double& v) {
        _v.f = f64_mul(_v.f, to_softfloat64(v) );
        return *this;
     }

     Double& operator-=(const Double& v)
     {
        _v.f = f64_sub(_v.f, v._v.f );
         return *this;
     }

     Double& operator*=(const Double& v)
     {
        _v.f = f64_mul(_v.f, v._v.f);
         return *this;
     }

     Double& operator/=(const double& v) {
        _v.f = f64_div(_v.f, to_softfloat64(v) );
        return *this;
     }

     Double& operator/=(const Double& v)
     {
        _v.f = f64_div(_v.f, v._v.f);
         return *this;
     }


     bool operator>(const double& v) {
        return _v.d > v;
     }

     bool operator>(const Double& v)
     {
        return _v.d > v._v.d;
     }

     bool operator>=(const double& v) {
        return _v.d >= v;
     }

     bool operator>=(const Double& v)
     {
        return _v.d >= v._v.d;
     }


     bool operator< (const double& v) {
        return _v.d < v;
     }

     bool operator < (const Double& v)
     {
        return _v.d < v._v.d;
     }

     bool operator<= (const double& v) {
        return _v.d <= v;
     }

     bool operator <= (const Double& v)
     {
        return _v.d <= v._v.d;
     }

     operator int64_t()const {
        return int64_t(_v.d);
     }

     operator double()const {
        return _v.d;
     }

};

inline Double operator*(int val, Double const& x) {
   return Double(val)* x;
}

inline Double operator*(double val, Double const& x) {
   return Double(val)* x;
}

inline Double operator/(int val, Double const& x) {
   return Double(val) / x;
}

inline Double operator/(double val, Double const& x) {
   return Double(val) / x;
}

inline Double operator-(double val, Double const& x) {
   return Double(val) - x;
}

inline Double operator+(double val, Double const& x) {
   return Double(val) + x;
}


namespace std {
   Double pow(Double base,Double power);
}

#else
   typedef double Double;
#endif

#endif /* CONTRACTS_EOSIOLIB_NATIVE_Double_HPP_ */
