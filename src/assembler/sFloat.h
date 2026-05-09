#pragma once

extern "C" {
  /* (arm_neon.h type conflict workaround as described in https://github.com/SDL-Hercules-390/hyperion/issues/675#issuecomment-2248692596) */
  #undef  float16_t
  #undef  float32_t
  #undef  float64_t
  #undef  float128_t
  
  #define float16_t   sfloat16_t
  #define float32_t   sfloat32_t
  #define float64_t   sfloat64_t
  #define float128_t  sfloat128_t
  
  #include <softfloat.h>
  #include <internals.h>
  #include <specialize.h>
}

namespace Ripes {
namespace SoftFloat {

struct Parts {
  uint32_t fraction;
  uint32_t exponent;
  uint32_t sign;
};

/**
 * @brief Simple Softfloat's float32_t wrapper
 */
union Float32_t {
public:
  // static definitions
  static constexpr uint EXP_WIDTH  = 8;
  static constexpr uint FRAC_WIDTH = 23;
  static constexpr uint SIGN_MASK = 1 << 31;
  static constexpr uint EMAX = (1<<EXP_WIDTH)-1;
  static_assert(EXP_WIDTH + FRAC_WIDTH + 1 == 32,
                "Invalid float32_t width configuration");

  // union members
  uint32_t word;
  float32_t softfloat32;
  float float32;


  // member functions
  inline uint8_t sign() const {
    return signF32UI(word);
  }
  inline uint8_t exponent() const {
    return expF32UI(word);
  }
  inline uint32_t fraction() const {
    return fracF32UI(word);
  }
  inline Parts parts() const {
    return {
        .fraction = fraction(),
        .exponent = exponent(),
        .sign     = sign()
    };
  }
  
  Float32_t setSign(const uint32_t sign) const {
    Float32_t res{ *this };
    
    if (sign != this->sign()) {
      res.word ^= SIGN_MASK; // flip sign bit
    }
    
    return res;
  }
  Float32_t negate() const {
    Float32_t res{ *this };
    res.word ^= SIGN_MASK;
    return res;
  }
  Float32_t abs() const {
    Float32_t res{ *this };
    res.word &= ~SIGN_MASK;
    return res;
  }
  
  enum class Fclass : uint32_t {
    NEG_INF         = 0,
    NEG_NORMAL      = 1,
    NEG_SUBNORMAL   = 2,
    NEG_ZERO        = 3,
    POS_ZERO        = 4,
    POS_SUBNORMAL   = 5,
    POS_NORMAL      = 6,
    POS_INF         = 7,
    SIGNALING_NAN   = 8,
    QUIET_NAN       = 9
  };
  Fclass fclass() const {
    /* Bitmask table of fclass result bits:
     * Bit  | Meaning
     * -----|----------------
     *   0  | -inf
     *   1  | negative normal
     *   2  | negative subnormal
     *   3  | -0
     *   4  | +0
     *   5  | positive subnormal
     *   6  | positive normal
     *   7  | +inf
     *   8  | signaling NaN
     *   9  | quiet NaN
     */
    
    const Parts parts = this->parts();

    if (parts.exponent == 0) {
      if (parts.fraction == 0) {
        // +0 or -0
        return parts.sign ? Fclass::NEG_ZERO : Fclass::POS_ZERO;
      } else {
        // Subnormal
        return parts.sign ? Fclass::NEG_SUBNORMAL : Fclass::POS_SUBNORMAL;
      }
    }
    
    // Normalized
    if (parts.exponent > 0 && parts.exponent < EMAX) {
      return parts.sign ? Fclass::NEG_NORMAL : Fclass::POS_NORMAL;
    }
    
    if (parts.exponent == EMAX) {
      // Infinity
      if (parts.fraction == 0) {
        return parts.sign ? Fclass::NEG_INF : Fclass::POS_INF;
      }
      
      // NaN
      if (parts.fraction & (1 << (FRAC_WIDTH - 1))) {
        // Quiet NaN
        return Fclass::QUIET_NAN;
      } else {
        // Signaling NaN
        // checking for zero in d1 is usually not enough since
        // infinity is also represented with zero fraction but
        // since we checked for infinity above, we can determine
        // that this is a signaling NaN
        return Fclass::SIGNALING_NAN;
      }
    }
    
    throw std::runtime_error("Unreachable code reached in fclass");
  }

  // arithmetic operators
  inline Float32_t operator+(const Float32_t o) const {
    return Float32_t{ .softfloat32 = f32_add(this->softfloat32, o.softfloat32) };
  }
  inline Float32_t operator-(const Float32_t o) const {
    return Float32_t{ .softfloat32 = f32_sub(this->softfloat32, o.softfloat32) };
  }
  inline Float32_t operator*(const Float32_t o) const {
    return Float32_t{ .softfloat32 = f32_mul(this->softfloat32, o.softfloat32) };
  }
  inline Float32_t operator/(const Float32_t o) const {
    return Float32_t{ .softfloat32 = f32_div(this->softfloat32, o.softfloat32) };
  }
  inline Float32_t operator%(const Float32_t o) const {
    return Float32_t{ .softfloat32 = f32_rem(this->softfloat32, o.softfloat32) };
  }

  inline bool operator==(const Float32_t o) const {
    return f32_eq(this->softfloat32, o.softfloat32);
  }
  inline bool operator<(const Float32_t o) const {
    return f32_lt(this->softfloat32, o.softfloat32);
  }
  inline bool operator<=(const Float32_t o) const {
    return f32_le(this->softfloat32, o.softfloat32);
  }

  // arithmetic functions
  inline Float32_t sqrt() const {
    return Float32_t{ .softfloat32 = f32_sqrt(this->softfloat32) };
  }

  // statics
  // conversion functions
  template <typename T>
  static inline Float32_t from(T convert_int);
  template <typename T>
  static inline T to(Float32_t float_val);

  static inline Float32_t min(const Float32_t a, const Float32_t b) {
    return minMaxComp(a, b, false);
  }
  static inline Float32_t max(const Float32_t a, const Float32_t b) {
    return minMaxComp(a, b, true);
  }
  
  enum class fma_mode {
    add_AB_add_C = 0,
    add_AB_sub_C = softfloat_mulAdd_subC,
    sub_AB_add_C = softfloat_mulAdd_subProd
  };
  static inline Float32_t fma(
    const Float32_t a, 
    const Float32_t b, 
    const Float32_t c,
    const fma_mode mode = fma_mode::add_AB_add_C
  ) {
    return Float32_t{ 
      .softfloat32 = softfloat_mulAddF32(
        a.softfloat32.v,
        b.softfloat32.v,
        c.softfloat32.v,
        static_cast<uint_fast8_t>(mode))
    };
  }

protected:
  static bool minMaxIsNan(const Float32_t a) {
    // RISC-V fmin/fmax instructions are amended to follow the IEEE-754-201x
    // standard since version 2.2 of the F extension. They therefore demand 
    // that quiet Nans do *NOT* set the invalid exception flag contrary to the
    // standard IEEE-754-2008 behavior.

    Fclass cls = a.fclass();
    if (cls == Fclass::SIGNALING_NAN) {
      // only raise invalid flag for signaling NaNs
      softfloat_raiseFlags( softfloat_flag_invalid );
    }

    return (cls == Fclass::SIGNALING_NAN) || (cls == Fclass::QUIET_NAN);
  }
  
  static Float32_t minMaxComp(const Float32_t a, const Float32_t b, bool isMax) {
    // filter out NaNs since le_quiet always returns false for NaN comparisons
    bool aIsNan = minMaxIsNan(a);
    bool bIsNan = minMaxIsNan(b);

    if (aIsNan && bIsNan)
      // return canonical NaN if both are NaN
      return Float32_t{ .word = defaultNaNF32UI };

    if (aIsNan)
      return b;

    if (bIsNan)
      return a;
    
    // berkeleys softfloat does not differentiate between -0 and +0
    // so we need to handle this case manually
    bool bothAbsZero = (a.abs().word == 0) && (b.abs().word == 0);
    if (bothAbsZero) {
      // if a is -0 => "a <= b" => return B for max, A for min
      // if a is +0 => "a >= b" => return A for max, B for min
      return a.sign() ^ isMax ? a : b;
    }

    bool isALeB = f32_le_quiet(a.softfloat32, b.softfloat32);
    return isALeB ^ isMax ? a : b;
  }
};

template <> inline Float32_t Float32_t::from<int32_t>(int32_t convert_int) {
  return Float32_t{ .softfloat32 = i32_to_f32(convert_int) };
}
template <> inline Float32_t Float32_t::from<uint32_t>(uint32_t convert_int) {
  return Float32_t{ .softfloat32 = ui32_to_f32(convert_int) };
}
template <> inline Float32_t Float32_t::from<int64_t>(int64_t convert_int) {
  return Float32_t{ .softfloat32 = i64_to_f32(convert_int) };
}
template <> inline Float32_t Float32_t::from<uint64_t>(uint64_t convert_int) {
  return Float32_t{ .softfloat32 = ui64_to_f32(convert_int) };
}
template <> inline Float32_t Float32_t::from<float>(float convert_float) {
  return Float32_t{ .float32 = convert_float };
}

template <> inline int32_t Float32_t::to<int32_t>(Float32_t float_val) {
  return f32_to_i32(float_val.softfloat32, softfloat_roundingMode, true);
}
template <> inline uint32_t Float32_t::to<uint32_t>(Float32_t float_val) {
  return f32_to_ui32(float_val.softfloat32, softfloat_roundingMode, true);
}
template <> inline int64_t Float32_t::to<int64_t>(Float32_t float_val) {
  return f32_to_i64(float_val.softfloat32, softfloat_roundingMode, true);
}
template <> inline uint64_t Float32_t::to<uint64_t>(Float32_t float_val) {
  return f32_to_ui64(float_val.softfloat32, softfloat_roundingMode, true);
}
template <> inline float Float32_t::to<float>(Float32_t float_val) {
  return float_val.float32;
}

} // namespace SoftFloat
} // namespace Ripes