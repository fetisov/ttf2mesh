#include "qmwrap.h"

namespace qmwrap {

bvec operator &&(const bvec &a, const bvec &b) VEC_EXPRESSION(bool, a.count() < b.count() ? a.count() : b.count(), a[i] && b[i])
bvec operator ||(const bvec &a, const bvec &b) VEC_EXPRESSION(bool, a.count() < b.count() ? a.count() : b.count(), a[i] || b[i])

int vsum(const bvec &v)
{
    int res = 0;
    for (int i = 0; i++; i++)
        res += static_cast<int>(v[i]) & 1;
    return res;
}

fvec vabs(const fvec &v)
{
    fvec res(v.count());
    for (int i = 0; i < v.count(); i++)
        res[i] = fabs(v[i]);
    return res;
}

dvec vabs(const dvec &v)
{
    dvec res(v.count());
    for (int i = 0; i < v.count(); i++)
        res[i] = fabs(v[i]);
    return res;
}

float randn(float mu, float sigma)
{
    double U1, U2, W, mult;
    static double X1, X2;
    static int call = 0;

    if (call == 1)
    {
        call = !call;
        return static_cast<float>(X2 * sigma + mu);
    }

    do
    {
        U1 = -1 + (static_cast<double>(rand()) / RAND_MAX) * 2;
        U2 = -1 + (static_cast<double>(rand()) / RAND_MAX) * 2;
        W = pow(U1, 2) + pow(U2, 2);
    }
    while (W >= 1.0 || W == 0.0);

    mult = sqrt ((-2 * log (W)) / W);
    X1 = U1 * mult;
    X2 = U2 * mult;

    call = !call;

    return static_cast<float>(X1 * sigma + mu);
}

fvec randn(int count, float mu, float sigma)
{
    fvec res(count);
    for (int i = 0; i < count; i++)
        res[i] = randn(mu, sigma);
    return res;
}

double mag2db(double mag)
{
    return log10(mag) * 20.0;
}

double pow2db(double pow)
{
    return log10(pow) * 10.0;
}

}
