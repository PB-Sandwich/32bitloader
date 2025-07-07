#include <math.h>
#include <float.h>
#include <stdint.h>

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double scalbnl(long double x, int n)
{
    return scalbn(x, n);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double scalbnl(long double x, int n)
{
    union ldshape u;

    if (n > 16383)
    {
        x *= 0x1p16383L;
        n -= 16383;
        if (n > 16383)
        {
            x *= 0x1p16383L;
            n -= 16383;
            if (n > 16383)
                n = 16383;
        }
    }
    else if (n < -16382)
    {
        x *= 0x1p-16382L * 0x1p113L;
        n += 16382 - 113;
        if (n < -16382)
        {
            x *= 0x1p-16382L * 0x1p113L;
            n += 16382 - 113;
            if (n < -16382)
                n = -16382;
        }
    }
    u.f = 1.0;
    u.i.se = 0x3fff + n;
    return x * u.f;
}
#endif

double scalbn(double x, int n)
{
    union
    {
        double f;
        uint64_t i;
    } u;
    double y = x;

    if (n > 1023)
    {
        y *= 0x1p1023;
        n -= 1023;
        if (n > 1023)
        {
            y *= 0x1p1023;
            n -= 1023;
            if (n > 1023)
                n = 1023;
        }
    }
    else if (n < -1022)
    {
        /* make sure final n < -53 to avoid double
           rounding in the subnormal range */
        y *= 0x1p-1022 * 0x1p53;
        n += 1022 - 53;
        if (n < -1022)
        {
            y *= 0x1p-1022 * 0x1p53;
            n += 1022 - 53;
            if (n < -1022)
                n = -1022;
        }
    }
    u.i = (uint64_t)(0x3ff + n) << 52;
    x = y * u.f;
    return x;
}

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double fmodl(long double x, long double y)
{
    return fmod(x, y);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double fmodl(long double x, long double y)
{
    union ldshape ux = {x}, uy = {y};
    int ex = ux.i.se & 0x7fff;
    int ey = uy.i.se & 0x7fff;
    int sx = ux.i.se & 0x8000;

    if (y == 0 || isnan(y) || ex == 0x7fff)
        return (x * y) / (x * y);
    ux.i.se = ex;
    uy.i.se = ey;
    if (ux.f <= uy.f)
    {
        if (ux.f == uy.f)
            return 0 * x;
        return x;
    }

    /* normalize x and y */
    if (!ex)
    {
        ux.f *= 0x1p120f;
        ex = ux.i.se - 120;
    }
    if (!ey)
    {
        uy.f *= 0x1p120f;
        ey = uy.i.se - 120;
    }

    /* x mod y */
#if LDBL_MANT_DIG == 64
    uint64_t i, mx, my;
    mx = ux.i.m;
    my = uy.i.m;
    for (; ex > ey; ex--)
    {
        i = mx - my;
        if (mx >= my)
        {
            if (i == 0)
                return 0 * x;
            mx = 2 * i;
        }
        else if (2 * mx < mx)
        {
            mx = 2 * mx - my;
        }
        else
        {
            mx = 2 * mx;
        }
    }
    i = mx - my;
    if (mx >= my)
    {
        if (i == 0)
            return 0 * x;
        mx = i;
    }
    for (; mx >> 63 == 0; mx *= 2, ex--)
        ;
    ux.i.m = mx;
#elif LDBL_MANT_DIG == 113
    uint64_t hi, lo, xhi, xlo, yhi, ylo;
    xhi = (ux.i2.hi & -1ULL >> 16) | 1ULL << 48;
    yhi = (uy.i2.hi & -1ULL >> 16) | 1ULL << 48;
    xlo = ux.i2.lo;
    ylo = uy.i2.lo;
    for (; ex > ey; ex--)
    {
        hi = xhi - yhi;
        lo = xlo - ylo;
        if (xlo < ylo)
            hi -= 1;
        if (hi >> 63 == 0)
        {
            if ((hi | lo) == 0)
                return 0 * x;
            xhi = 2 * hi + (lo >> 63);
            xlo = 2 * lo;
        }
        else
        {
            xhi = 2 * xhi + (xlo >> 63);
            xlo = 2 * xlo;
        }
    }
    hi = xhi - yhi;
    lo = xlo - ylo;
    if (xlo < ylo)
        hi -= 1;
    if (hi >> 63 == 0)
    {
        if ((hi | lo) == 0)
            return 0 * x;
        xhi = hi;
        xlo = lo;
    }
    for (; xhi >> 48 == 0; xhi = 2 * xhi + (xlo >> 63), xlo = 2 * xlo, ex--)
        ;
    ux.i2.hi = xhi;
    ux.i2.lo = xlo;
#endif

    /* scale result */
    if (ex <= 0)
    {
        ux.i.se = (ex + 120) | sx;
        ux.f *= 0x1p-120f;
    }
    else
        ux.i.se = ex | sx;
    return ux.f;
}
#endif

double copysign(double x, double y)
{
    union
    {
        double f;
        uint64_t i;
    } ux = {x}, uy = {y};
    ux.i &= -1ULL / 2;
    ux.i |= uy.i & 1ULL << 63;
    return ux.f;
}

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double copysignl(long double x, long double y)
{
    return 0;
}
long double fabsl(long double x)
{
    return fabs(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double fabsl(long double x)
{
    union ldshape u = {x};

    u.i.se &= 0x7fff;
    return u.f;
}
#endif

double fabs(double x)
{
    union
    {
        double f;
        uint64_t i;
    } u = {x};
    u.i &= -1ULL / 2;
    return u.f;
}

double fmod(double x, double y)
{
    union
    {
        double f;
        uint64_t i;
    } ux = {x}, uy = {y};
    int ex = ux.i >> 52 & 0x7ff;
    int ey = uy.i >> 52 & 0x7ff;
    int sx = ux.i >> 63;
    uint64_t i;

    /* in the followings uxi should be ux.i, but then gcc wrongly adds */
    /* float load/store to inner loops ruining performance and code size */
    uint64_t uxi = ux.i;

    if (uy.i << 1 == 0 || isnan(y) || ex == 0x7ff)
        return (x * y) / (x * y);
    if (uxi << 1 <= uy.i << 1)
    {
        if (uxi << 1 == uy.i << 1)
            return 0 * x;
        return x;
    }

    /* normalize x and y */
    if (!ex)
    {
        for (i = uxi << 12; i >> 63 == 0; ex--, i <<= 1)
            ;
        uxi <<= -ex + 1;
    }
    else
    {
        uxi &= -1ULL >> 12;
        uxi |= 1ULL << 52;
    }
    if (!ey)
    {
        for (i = uy.i << 12; i >> 63 == 0; ey--, i <<= 1)
            ;
        uy.i <<= -ey + 1;
    }
    else
    {
        uy.i &= -1ULL >> 12;
        uy.i |= 1ULL << 52;
    }

    /* x mod y */
    for (; ex > ey; ex--)
    {
        i = uxi - uy.i;
        if (i >> 63 == 0)
        {
            if (i == 0)
                return 0 * x;
            uxi = i;
        }
        uxi <<= 1;
    }
    i = uxi - uy.i;
    if (i >> 63 == 0)
    {
        if (i == 0)
            return 0 * x;
        uxi = i;
    }
    for (; uxi >> 52 == 0; uxi <<= 1, ex--)
        ;

    /* scale result */
    if (ex > 0)
    {
        uxi -= 1ULL << 52;
        uxi |= (uint64_t)ex << 52;
    }
    else
    {
        uxi >>= -ex + 1;
    }
    uxi |= (uint64_t)sx << 63;
    ux.i = uxi;
    return ux.f;
}

double signbit(double x)
{
    union IEEEd2bits u;
    u.d = x;
    return (u.bits.sign);
}

#if LDBL_MAX_EXP != 0x4000
#error "Unsupported long double format"
#endif

long double
frexpl(long double x, int *ex)
{
    union IEEEl2bits u;

    u.e = x;
    switch (u.bits.exp)
    {
    case 0: /* 0 or subnormal */
        if ((u.bits.manl | u.bits.manh) == 0)
        {
            *ex = 0;
        }
        else
        {
            u.e *= 0x1.0p514;
            *ex = u.bits.exp - 0x4200;
            u.bits.exp = 0x3ffe;
        }
        break;
    case 0x7fff: /* infinity or NaN; value of *ex is unspecified */
        break;
    default: /* normal */
        *ex = u.bits.exp - 0x3ffe;
        u.bits.exp = 0x3ffe;
        break;
    }
    return (u.e);
}


long double
copysignl(long double x, long double y)
{
	union IEEEl2bits ux, uy;

	ux.e = x;
	uy.e = y;
	ux.bits.sign = uy.bits.sign;
	return (ux.e);
}