#pragma once
#include <float.h>
#include <stdint.h>
#include <endian.h>
#define INFINITY HUGE_VALF

#define HUGE_VALF 1e10000f
#define HUGE_VALL 1e10000L

#ifndef _IEEE_WORD_ORDER
#define	_IEEE_WORD_ORDER	__BYTE_ORDER
#endif
#ifndef _BYTE_ORDER
#define _BYTE_ORDER __BYTE_ORDER
#endif

#define _LITTLE_ENDIAN __LITTLE_ENDIAN
#define _BIG_ENDIAN __BIG_ENDIAN

/* This will raise an "invalid" exception outside static initializers,
   but is the best that can be done in ISO C while remaining a
   constant expression.  */
#define NAN (0.0f / 0.0f)

union IEEEf2bits {
	float	f;
	struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
		unsigned int	man	:23;
		unsigned int	exp	:8;
		unsigned int	sign	:1;
#else /* _BIG_ENDIAN */
		unsigned int	sign	:1;
		unsigned int	exp	:8;
		unsigned int	man	:23;
#endif
	} bits;
};

union IEEEd2bits {
	double	d;
	struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
#if _IEEE_WORD_ORDER == _LITTLE_ENDIAN
		unsigned int	manl	:32;
#endif
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
#if _IEEE_WORD_ORDER == _BIG_ENDIAN
		unsigned int	manl	:32;
#endif
#else /* _BIG_ENDIAN */
		unsigned int	sign	:1;
		unsigned int	exp	:11;
		unsigned int	manh	:20;
		unsigned int	manl	:32;
#endif
	} bits;
};

union IEEEl2bits
{
	long double e;
	struct
	{
		unsigned int manl : 32;
		unsigned int manh : 32;
		unsigned int exp : 15;
		unsigned int sign : 1;
		unsigned int junk : 16;
	} bits;
	struct
	{
		unsigned long long man : 64;
		unsigned int expsign : 16;
		unsigned int junk : 16;
	} xbits;
};

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __LITTLE_ENDIAN
union ldshape
{
	long double f;
	struct
	{
		uint64_t m;
		uint16_t se;
	} i;
};
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __BIG_ENDIAN
/* This is the m68k variant of 80-bit long double, and this definition only works
 * on archs where the alignment requirement of uint64_t is <= 4. */
union ldshape
{
	long double f;
	struct
	{
		uint16_t se;
		uint16_t pad;
		uint64_t m;
	} i;
};
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __LITTLE_ENDIAN
union ldshape
{
	long double f;
	struct
	{
		uint64_t lo;
		uint32_t mid;
		uint16_t top;
		uint16_t se;
	} i;
	struct
	{
		uint64_t lo;
		uint64_t hi;
	} i2;
};
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __BIG_ENDIAN
union ldshape
{
	long double f;
	struct
	{
		uint16_t se;
		uint16_t top;
		uint32_t mid;
		uint64_t lo;
	} i;
	struct
	{
		uint64_t hi;
		uint64_t lo;
	} i2;
};
#else
#error Unsupported long double representation
#endif

long double fmodl(long double x, long double y);

double copysign(double x, double y);

double signbit(double x);

double fabs(double x);

long double copysignl(long double x, long double y);

long double fabsl(long double x);

long double scalbnl(long double x, int n);

double scalbn(double x, int n);

#ifdef BUILT_IN_FLOAT_NAN_AND_FINITE
#define isnan(x) __builtin_isnan(x)

#define isfinite(x) __builtin_isfinite(x)

#else
int isnan(double d);
int isnanf(float f);

int isfinite(double d);

int isfinitef(float f);

int isfinitel(long double e);

#endif

long double frexpl(long double x, int *ex);
