#ifndef _VEC2_H_
#define _VEC2_H_

#include "TVecHelpers.h"
#include "half.h"

#include <assert.h>

namespace details {

template <typename T>
class TVec2 :   public TVecProductOperators<TVec2, T>,
                public TVecAddOperators<TVec2, T>,
                public TVecUnaryOperators<TVec2, T>,
                public TVecComparisonOperators<TVec2, T>,
                public TVecFunctions<TVec2, T>,
                public TVecDebug<TVec2, T> {
public:
    enum no_init { NO_INIT };
    typedef T value_type;
    typedef T& reference;
    typedef T const& const_reference;
    typedef size_t size_type;

    union {
        struct { T x, y; };
        struct { T s, t; };
        struct { T r, g; };
    };

    static constexpr size_t SIZE = 2;
    inline constexpr size_type size() const { return SIZE; }

    // array access
    inline constexpr T const& operator[](size_t i) const {
#if __cplusplus >= 201402L
        // only possible in C++0x14 with constexpr
        assert(i < SIZE);
#endif
        return (&x)[i];
    }

    inline T& operator[](size_t i) {
        assert(i < SIZE);
        return (&x)[i];
    }

    // -----------------------------------------------------------------------
    // we want the compiler generated versions for these...
    TVec2(const TVec2&) = default;
    ~TVec2() = default;
    TVec2& operator = (const TVec2&) = default;

    // constructors

    // leaves object uninitialized. use with caution.
    explicit
    constexpr TVec2(no_init) { }

    // default constructor
    constexpr TVec2() : x(0), y(0) { }

    // handles implicit conversion to a tvec4. must not be explicit.
    template<typename A, typename = typename std::enable_if<std::is_arithmetic<A>::value >::type>
    constexpr TVec2(A v) : x(v), y(v) { }

    template<typename A, typename B>
    constexpr TVec2(A x, B y) : x(static_cast<T>(x)), y(static_cast<T>(y)) { }

    template<typename A>
    explicit
    constexpr TVec2(const TVec2<A>& v) : x(v.x), y(v.y) { }

    // cross product works only on vectors of size 2 or 3
    template<typename RT>
    friend inline
    constexpr value_type cross(const TVec2& u, const TVec2<RT>& v) {
        return value_type(u.x*v.y - u.y*v.x);
    }
};

} // end namespace details

typedef details::TVec2<double> double2;
typedef details::TVec2<float> float2;
typedef details::TVec2<float> vec2;
typedef details::TVec2<half> half2;
typedef details::TVec2<int32_t> int2;
typedef details::TVec2<uint32_t> uint2;
typedef details::TVec2<int16_t> short2;
typedef details::TVec2<uint16_t> ushort2;
typedef details::TVec2<int8_t> byte2;
typedef details::TVec2<uint8_t> ubyte2;
typedef details::TVec2<bool> bool2;

TVECHELPERS_STD_HASH(::details::TVec2);

#endif // end _VEC2_H_