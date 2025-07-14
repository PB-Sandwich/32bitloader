/**
 * @file arith64.c
 * @author Sofia "MetalPizzaCat"
 * @brief Implementation of some long operation functions taken from https://github.com/glitchub/arith64/tree/master
 * @version 0.1
 * @date 2025-07-07
 *
 * @copyright Copyright (c) 2025
 *
 */

#define arith64_u64 unsigned long long int
#define arith64_s64 signed long long int
#define arith64_u32 unsigned int
#define arith64_s32 int

typedef union
{
    arith64_u64 u64;
    arith64_s64 s64;
    struct
    {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        arith64_u32 hi;
        arith64_u32 lo;
#else
        arith64_u32 lo;
        arith64_u32 hi;
#endif
    } u32;
    struct
    {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        arith64_s32 hi;
        arith64_s32 lo;
#else
        arith64_s32 lo;
        arith64_s32 hi;
#endif
    } s32;
} arith64_word;

// extract hi and lo 32-bit words from 64-bit value
#define arith64_hi(n) (arith64_word){.u64 = n}.u32.hi
#define arith64_lo(n) (arith64_word){.u64 = n}.u32.lo

// Negate a if b is negative, via invert and increment.
#define arith64_neg(a, b) (((a) ^ ((((arith64_s64)(b)) >= 0) - 1)) + (((arith64_s64)(b)) < 0))
#define arith64_abs(a) arith64_neg(a, a)

int __clzdi2(arith64_u64 a)
{
    int b, n = 0;
    b = !(a & 0xffffffff00000000ULL) << 5;
    n += b;
    a <<= b;
    b = !(a & 0xffff000000000000ULL) << 4;
    n += b;
    a <<= b;
    b = !(a & 0xff00000000000000ULL) << 3;
    n += b;
    a <<= b;
    b = !(a & 0xf000000000000000ULL) << 2;
    n += b;
    a <<= b;
    b = !(a & 0xc000000000000000ULL) << 1;
    n += b;
    a <<= b;
    return n + !(a & 0x8000000000000000ULL);
}

/// @brief Calculate both the quotient and remainder of the unsigned division of a by b.
/// @param a
/// @param b
/// @param c Remainder is placed in variable pointed to by c (if it's not NULL)
/// @return The return value is the quotient
arith64_u64 __divmoddi4(arith64_u64 a, arith64_u64 b, arith64_u64 *c)
{
    if (b > a) // divisor > numerator?
    {
        if (c)
            *c = a; // remainder = numerator
        return 0;   // quotient = 0
    }
    if (!arith64_hi(b)) // divisor is 32-bit
    {
        if (b == 0) // divide by 0
        {
            volatile char x = 0;
            x = 1 / x; // force an exception
        }
        if (b == 1) // divide by 1
        {
            if (c)
                *c = 0; // remainder = 0
            return a;   // quotient = numerator
        }
        if (!arith64_hi(a)) // numerator is also 32-bit
        {
            if (c) // use generic 32-bit operators
                *c = arith64_lo(a) % arith64_lo(b);
            return arith64_lo(a) / arith64_lo(b);
        }
    }

    // let's do long division
    char bits = __clzdi2(b) - __clzdi2(a) + 1; // number of bits to iterate (a and b are non-zero)
    arith64_u64 rem = a >> bits;               // init remainder
    a <<= 64 - bits;                           // shift numerator to the high bit
    arith64_u64 wrap = 0;                      // start with wrap = 0
    while (bits-- > 0)                         // for each bit
    {
        rem = (rem << 1) | (a >> 63);              // shift numerator MSB to remainder LSB
        a = (a << 1) | (wrap & 1);                 // shift out the numerator, shift in wrap
        wrap = ((arith64_s64)(b - rem - 1) >> 63); // wrap = (b > rem) ? 0 : 0xffffffffffffffff (via sign extension)
        rem -= b & wrap;                           // if (wrap) rem -= b
    }
    if (c)
        *c = rem;                 // maybe set remainder
    return (a << 1) | (wrap & 1); // return the quotient
}

/// @brief Return the quotient of the unsigned division of a by b.
arith64_u64 __udivdi3(arith64_u64 a, arith64_u64 b)
{
    return __divmoddi4(a, b, (void *)0);
}

/// @brief Return the remainder of the unsigned division of a by b.
arith64_u64 __umoddi3(arith64_u64 a, arith64_u64 b)
{
    arith64_u64 r;
    __divmoddi4(a, b, &r);
    return r;
}