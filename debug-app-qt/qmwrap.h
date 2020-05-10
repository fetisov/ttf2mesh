#ifndef QMWRAP_H
#define QMWRAP_H

#include <stdint.h>
#include <math.h>
#include <float.h>
#include <QVector>

namespace qmwrap {

// Vec class definition

template <typename T> class Vec
{
public:
    Vec() : _data(NULL), _count(0), _cap(0) {}
    Vec(const Vec<T> &v);
    template <typename T2> Vec(const Vec<T2> &v);
    template <typename T2> Vec(const QVector<T2> &v);
    Vec(int count);
    Vec(int count, const T &fill);
    ~Vec();
    inline int count() const { return _count; }
    inline int &count() { return _count; }
    inline int capacity() const { return _cap; }
    inline const T *data() const { return _data; }
    inline T *data() { return _data; }
    inline void clear();
    inline void resize(int n);
    inline void reserve(int n);
    inline void push_back(const T &value);
    inline const T operator[](int i) const { return _data[i]; }
    inline T &operator[](int i) { return _data[i]; }
    Vec<T> operator[](const Vec<bool> &) const;
    Vec<T> operator[](const Vec<int> &) const;
    Vec<T> mid(int pos, int len = -1);
    template <typename T2> Vec<T> &operator <<(const T2 &v);
    template <typename T2> Vec<T> &operator <<(const Vec<T2> &v);
    Vec<T> &operator =(const Vec<T> &v);
    template <typename T2> Vec<T> &operator =(const Vec<T2> &v);
    static inline const Vec<T> cast(const T *ptr, int count);
    static inline Vec<T> cast(T *ptr, int count);
public:
    typedef T *iterator;
    typedef const T *const_iterator;
    inline iterator begin() { return _data; }
    inline const_iterator begin() const { return _data; }
    inline iterator end() { return _data + _count; }
    inline const_iterator end() const { return _data + _count; }
protected:
    T *_data;
    int _count;
    int _cap;
};

// Short names for custom Vec classes

typedef Vec<double> dvec;
typedef Vec<float> fvec;
typedef Vec<bool> bvec;
typedef Vec<int> ivec;
typedef Vec<uint8_t> uvec8;
typedef Vec<uint16_t> uvec16;
typedef Vec<uint32_t> uvec32;
typedef Vec<uint64_t> uvec64;
typedef Vec<int8_t> ivec8;
typedef Vec<int16_t> ivec16;
typedef Vec<int32_t> ivec32;
typedef Vec<int64_t> ivec64;

// Range class definition

template <typename T> class Range
{
public:
    inline Range(T from = 0, T to = 0) { value[0] = from; value[1] = to; }
    inline Range(const Vec<T> &v);
    inline T min() const { return value[0]; }
    inline T max() const { return value[1]; }
    inline T &operator [](int index) { return value[index]; }
    inline bool in(T v) const { return v >= value[0] && v <= value[1]; }
    inline T size() const { return value[1] - value[0]; }
    inline void swapMinMax() { T t = value[0]; value[0] = value[1]; value[1] = t; }
    inline void fix() { if (value[0] > value[1]) swapMinMax(); }
    inline double expandedMin(double part = 1e-3 /* 0.1% */) { return value[0] - size() * part; }
    inline double expandedMax(double part = 1e-3 /* 0.1% */) { return value[1] + size() * part; }
    T value[2];
};

// Templates, functions and operators of Vec class

template <typename T, typename T2> bvec operator >(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> bvec operator >=(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> bvec operator <(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> bvec operator <=(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> bvec operator ==(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> bvec operator !=(const Vec<T> &a, const T2 &b);
bvec operator &&(const bvec &a, const bvec &b);
bvec operator ||(const bvec &a, const bvec &b);
// vector and scalar operators
template <typename T, typename T2> Vec<T> operator +(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> Vec<T> operator -(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> Vec<T> operator *(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> Vec<T> operator /(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> Vec<T> operator &(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> Vec<T> operator |(const Vec<T> &a, const T2 &b);
template <typename T, typename T2> Vec<T> operator ^(const Vec<T> &a, const T2 &b);
// scalar and vector operators
template <typename T, typename T2> Vec<T> operator +(const T2 &b, const Vec<T> &a);
template <typename T, typename T2> Vec<T> operator -(const T2 &b, const Vec<T> &a);
template <typename T, typename T2> Vec<T> operator *(const T2 &b, const Vec<T> &a);
template <typename T, typename T2> Vec<T> operator /(const T2 &b, const Vec<T> &a);
template <typename T, typename T2> Vec<T> operator &(const T2 &b, const Vec<T> &a);
template <typename T, typename T2> Vec<T> operator |(const T2 &b, const Vec<T> &a);
template <typename T, typename T2> Vec<T> operator ^(const T2 &b, const Vec<T> &a);
// vector and vector operators
template <typename T, typename T2> Vec<T> operator +(const Vec<T> &a, const Vec<T2> &b);
template <typename T, typename T2> Vec<T> operator -(const Vec<T> &a, const Vec<T2> &b);
template <typename T, typename T2> Vec<T> operator *(const Vec<T> &a, const Vec<T2> &b);
template <typename T, typename T2> Vec<T> operator /(const Vec<T> &a, const Vec<T2> &b);
template <typename T, typename T2> Vec<T> operator &(const Vec<T> &a, const Vec<T2> &b);
template <typename T, typename T2> Vec<T> operator |(const Vec<T> &a, const Vec<T2> &b);
template <typename T, typename T2> Vec<T> operator ^(const Vec<T> &a, const Vec<T2> &b);

// some vector utility functions and templates

int vsum(const bvec &v);
template <typename T> T vsum(const Vec<T> &v);
template <typename T> ivec vfind(const Vec<T> &v);
template <typename T> Vec<T> vabs(const Vec<T> &v);
fvec vabs(const fvec &v);
dvec vabs(const dvec &v);
template <typename T> Vec<T> vfloor(const Vec<T> &v);
template <typename T> Vec<T> vceil(const Vec<T> &v);
template <typename T> Vec<T> vround(const Vec<T> &v);
template <typename T, typename T2> Vec<T> vpow(const Vec<T> &v, const T2 &p);
template <typename T> int min_finit_id(const Vec<T> &v);
template <typename T> int max_finit_id(const Vec<T> &v);
template <typename T> T min_finit(const Vec<T> &v);
template <typename T> T max_finit(const Vec<T> &v);
template <typename T> void minmax_finit(const Vec<T> &v, T &min, T &max);
template <typename T> float vmeanf(const Vec<T> &v);
template <typename T> double vmeand(const Vec<T> &v);
template <typename T> float vstdf(const Vec<T> &v);
template <typename T> double vstdd(const Vec<T> &v);
template <typename T, typename T2> Vec<T> vconv(const Vec<T> &a, const Vec<T2> &b);
template <typename T> Vec<T> downsample(const Vec<T> v, unsigned factor);
template <typename T> inline Range<T> vrange(const Vec<T> &v);
template <typename T> Vec<T> linspace(const Range<T> &range, int count);
template <typename T, typename T2> Vec<T> linspace(const T &from, const T2 &to, int count);
template <typename T> Vec<T> normpdf(const Vec<T> &x, const T &mu, const T &sigma);
fvec randn(int count, float mu, float sigma);
double mag2db(double mag);
double pow2db(double pow);

// ------------------------------- IMPLEMENTATION -------------------------------
// --------------------------- OF INLINE AND TEMPLATE ---------------------------
// ---------------------------------- FUNCTIONS ---------------------------------

// Vec class implementation

template <typename T>
Vec<T>::Vec(const Vec<T> &v) :
    _count(v.count()), _cap(v.count())
{
    _data = new T[v.capacity()];
    for (int i = 0; i < _count; i++)
        _data[i] = v[i];
}

template <typename T>
template <typename T2>
Vec<T>::Vec(const Vec<T2> &v) :
    _count(v.count()), _cap(v.count())
{
    _data = new T[_cap];
    for (int i = 0; i < _count; i++)
        _data[i] = v[i];
}

template <typename T>
template <typename T2>
Vec<T>::Vec(const QVector<T2> &v) :
    _count(v.count()), _cap(v.count())
{
    _data = new T[v.capacity()];
    for (int i = 0; i < _count; i++)
        _data[i] = v[i];
}

template <typename T> Vec<T>::Vec(int count)
{
    _count = count;
    _cap = count;
    _data = new T[count];
}

template <typename T> Vec<T>::Vec(int count, const T &fill)
{
    _count = count;
    _cap = count;
    _data = new T[count];
    for (int i = 0; i < count; i++)
        _data[i] = fill;
}

template <typename T> Vec<T>::~Vec()
{
    if (_cap != -1)
        delete [] _data;
}

template <typename T> inline void Vec<T>::clear()
{
    if (_cap != -1)
        delete [] _data;
    _count = 0;
    _cap = 0;
    _data = NULL;
}

template <typename T> inline void Vec<T>::resize(int n)
{
    if (n <= _cap)
    {
        _count = n;
        return;
    }
    T *newData = new T[n];
    for (int i = 0; i < _count; i++)
        newData[i] = _data[i];
    if (_cap != -1)
        delete [] _data;
    _data = newData;
    _count = n;
    _cap = n;
}

template <typename T> inline void Vec<T>::reserve(int n)
{
    if (n <= _cap) return;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    T *newData = new T[n];
    for (int i = 0; i < _count; i++)
        newData[i] = _data[i];
    if (_cap != -1)
        delete [] _data;
    _data = newData;
    _cap = n;
}

template <typename T> inline void Vec<T>::push_back(const T &value)
{
    reserve(_count + 1);
    _data[_count++] = value;
}

template <typename T> Vec<T> Vec<T>::operator[](const Vec<bool> &b) const
{
    int n = _count < b.count() ? _count : b.count();
    int m = 0;
    for (int i = 0; i < n; i++)
        m += static_cast<int>(b[i] & 1);
    Vec<T> res(m);
    m = 0;
    for (int i = 0; i < n; i++)
        if (b[i])
            res[m++] = _data[i];
    return res;
}

template <typename T> Vec<T> Vec<T>::operator[](const Vec<int> &indeces) const
{
    Vec<T> res(indeces.count());
    for (int i = 0; i < _count; i++)
        res[i] = _data[indeces[i]];
    return res;
}

template <typename T> Vec<T> Vec<T>::mid(int pos, int len)
{
    if (pos < 0)
    {
        len += pos;
        pos = 0;
    }
    if (pos >= _count)
        return Vec<T>();
    if (pos + len > _count)
        len = _count - pos;
    return Vec<T>(Vec<T>::cast(_data + pos, len));
}

template <typename T>
template <typename T2>
Vec<T> &Vec<T>::operator <<(const T2 &v)
{
    reserve(_count + 1);
    _data[_count++] = v;
    return *this;
}

template <typename T>
template <typename T2>
Vec<T> &Vec<T>::operator <<(const Vec<T2> &v)
{
    int n = count();
    int m = v.count();
    reserve(n + m);
    for (int i = 0; i < m; i++)
        _data[i + n] = v[i];
    _count += m;
    return *this;
}

template <typename T>
template <typename T2>
Vec<T> &Vec<T>::operator =(const Vec<T2> &v)
{
    if (static_cast<const void *>(&v) == static_cast<const void *>(this))
        return *this;
    T *newData = new T[v.count()];
    if (_cap != -1)
        delete [] _data;
    _count = v.count();
    _cap = v.count();
    _data = newData;
    for (int i = 0; i < _count; i++)
        _data[i] = v[i];
    return *this;
}

template <typename T>
Vec<T> &Vec<T>::operator =(const Vec<T> &v)
{
    if (static_cast<const void *>(&v) == static_cast<const void *>(this))
        return *this;
    T *newData = new T[v.count()];
    if (_cap != -1)
        delete [] _data;
    _count = v.count();
    _cap = v.count();
    _data = newData;
    for (int i = 0; i < _count; i++)
        _data[i] = v[i];
    return *this;
}

template <typename T>
inline const Vec<T> Vec<T>::cast(const T *ptr, int count)
{
    Vec<T> res;
    res._cap = -1;
    res._count = count;
    res._data = static_cast<T *>(ptr);
    return res;
}

template <typename T>
inline Vec<T> Vec<T>::cast(T *ptr, int count)
{
    Vec<T> res;
    res._cap = -1;
    res._count = count;
    res._data = ptr;
    return res;
}

// Range class implementation

template <typename T> inline Range<T>::Range(const Vec<T> &v)
{
    value[0] = min_finit(v);
    value[1] = max_finit(v);
}

// Templates, functions and operators of Vec class

#define VEC_EXPRESSION(res_type, res_count, expr) \
{ \
    Vec<res_type> res(res_count); \
    for (int i = 0; i < res.count(); i++) \
        res[i] = expr; \
    return res; \
}
template <typename T, typename T2> bvec operator >(const Vec<T> &a, const T2 &b)  VEC_EXPRESSION(bool, a.count(), a[i] > b)
template <typename T, typename T2> bvec operator >=(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(bool, a.count(), a[i] >= b)
template <typename T, typename T2> bvec operator <(const Vec<T> &a, const T2 &b)  VEC_EXPRESSION(bool, a.count(), a[i] < b)
template <typename T, typename T2> bvec operator <=(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(bool, a.count(), a[i] <= b)
template <typename T, typename T2> bvec operator ==(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(bool, a.count(), a[i] == b)
template <typename T, typename T2> bvec operator !=(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(bool, a.count(), a[i] != b)

template <typename T, typename T2> Vec<T> operator +(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(T, a.count(), a[i] + b)
template <typename T, typename T2> Vec<T> operator -(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(T, a.count(), a[i] - b)
template <typename T, typename T2> Vec<T> operator *(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(T, a.count(), a[i] * b)
template <typename T, typename T2> Vec<T> operator /(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(T, a.count(), a[i] / b)
template <typename T, typename T2> Vec<T> operator &(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(T, a.count(), a[i] & b)
template <typename T, typename T2> Vec<T> operator |(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(T, a.count(), a[i] | b)
template <typename T, typename T2> Vec<T> operator ^(const Vec<T> &a, const T2 &b) VEC_EXPRESSION(T, a.count(), a[i] ^ b)
template <typename T, typename T2> Vec<T> operator +(const T2 &b, const Vec<T> &a) VEC_EXPRESSION(T, a.count(), b + a[i])
template <typename T, typename T2> Vec<T> operator -(const T2 &b, const Vec<T> &a) VEC_EXPRESSION(T, a.count(), b - a[i])
template <typename T, typename T2> Vec<T> operator *(const T2 &b, const Vec<T> &a) VEC_EXPRESSION(T, a.count(), b * a[i])
template <typename T, typename T2> Vec<T> operator /(const T2 &b, const Vec<T> &a) VEC_EXPRESSION(T, a.count(), b / a[i])
template <typename T, typename T2> Vec<T> operator &(const T2 &b, const Vec<T> &a) VEC_EXPRESSION(T, a.count(), b & a[i])
template <typename T, typename T2> Vec<T> operator |(const T2 &b, const Vec<T> &a) VEC_EXPRESSION(T, a.count(), b | a[i])
template <typename T, typename T2> Vec<T> operator ^(const T2 &b, const Vec<T> &a) VEC_EXPRESSION(T, a.count(), b ^ a[i])

template <typename T, typename T2> Vec<T> operator +(const Vec<T> &a, const Vec<T2> &b) VEC_EXPRESSION(T, a.count() < b.count() ? a.count() : b.count(), a[i] + b[i])
template <typename T, typename T2> Vec<T> operator -(const Vec<T> &a, const Vec<T2> &b) VEC_EXPRESSION(T, a.count() < b.count() ? a.count() : b.count(), a[i] - b[i])
template <typename T, typename T2> Vec<T> operator *(const Vec<T> &a, const Vec<T2> &b) VEC_EXPRESSION(T, a.count() < b.count() ? a.count() : b.count(), a[i] * b[i])
template <typename T, typename T2> Vec<T> operator /(const Vec<T> &a, const Vec<T2> &b) VEC_EXPRESSION(T, a.count() < b.count() ? a.count() : b.count(), a[i] / b[i])
template <typename T, typename T2> Vec<T> operator &(const Vec<T> &a, const Vec<T2> &b) VEC_EXPRESSION(T, a.count() < b.count() ? a.count() : b.count(), a[i] & b[i])
template <typename T, typename T2> Vec<T> operator |(const Vec<T> &a, const Vec<T2> &b) VEC_EXPRESSION(T, a.count() < b.count() ? a.count() : b.count(), a[i] | b[i])
template <typename T, typename T2> Vec<T> operator ^(const Vec<T> &a, const Vec<T2> &b) VEC_EXPRESSION(T, a.count() < b.count() ? a.count() : b.count(), a[i] ^ b[i])

template <typename T> T vsum(const Vec<T> &v)
{
    T res = T();
    for (int i = 0; i < v.count(); i++)
        res += v[i];
    return res;
}

template <typename T> ivec vfind(const Vec<T> &v)
{
    ivec res(v.count());
    int n = 0;
    const T null = T();
    for (int i = 0; i < v.count(); i++)
        if (v[i] == null)
            v[n++] = i;
    res.count() = n;
    return res;
}

template <typename T> Vec<T> vabs(const Vec<T> &v) VEC_EXPRESSION(T, v.count(), abs(v[i]))
template <typename T> Vec<T> vfloor(const Vec<T> &v) VEC_EXPRESSION(T, v.count(), floor(v[i]))
template <typename T> Vec<T> vceil(const Vec<T> &v) VEC_EXPRESSION(T, v.count(), ceil(v[i]))
template <typename T> Vec<T> vround(const Vec<T> &v) VEC_EXPRESSION(T, v.count(), round(v[i]))
template <typename T, typename T2> Vec<T> vpow(const Vec<T> &v, const T2 &p) VEC_EXPRESSION(T, v.count(), pow(v[i], p))

template <typename T> int min_finit_id(const Vec<T> &v)
{
    int res = -1;
    int i = 0;
    for (; i < v.count(); i++)
        if (std::isfinite(v[i]))
        {
            res = i;
            break;
        }
    i++;
    for (; i < v.count(); i++)
        if (v[i] < v[res] && std::isfinite(v[i]))
            res = i;
    return res;
}

template <typename T> int max_finit_id(const Vec<T> &v)
{
    int res = -1;
    int i = 0;
    for (; i < v.count(); i++)
        if (std::isfinite(v[i]))
        {
            res = i;
            break;
        }
    i++;
    for (; i < v.count(); i++)
        if (v[i] > v[res] && std::isfinite(v[i]))
            res = i;
    return res;
}

template <typename T> T min_finit(const Vec<T> &v)
{
    int id = min_finit_id(v);
    return id == -1 ? 0 : v[id];
}

template <typename T> T max_finit(const Vec<T> &v)
{
    int id = max_finit_id(v);
    return id == -1 ? 0 : v[id];
}

template <typename T> void minmax_finit(const Vec<T> &v, T &min, T &max)
{
    min = min_finit(v);
    max = max_finit(v);
}

template <typename T> float vmeanf(const Vec<T> &v)
{
    float res = 0;
    if (v.count() == 0) return 0;
    for (int i = 0; i < v.count(); i++)
        res = res + v[i];
    return res / v.count();
}

template <typename T> double vmeand(const Vec<T> &v)
{
    double res = 0;
    if (v.count() == 0) return 0;
    for (int i = 0; i < v.count(); i++)
        res = res + v[i];
    return res / v.count();
}

template <typename T> float vstdf(const Vec<T> &v)
{
    if (v.count() == 0) return 0;
    float mean = vmeanf(v);
    float res = 0;
    for (int i = 0; i < v.count(); i++)
    {
        float a = mean - v[i];
        res += a * a;
    }
    return sqrt(res / v.count());
}

template <typename T> double vstdd(const Vec<T> &v)
{
    if (v.count() == 0) return 0;
    double mean = vmeand(v);
    double res = 0;
    for (int i = 0; i < v.count(); i++)
    {
        double a = mean - v[i];
        res += a * a;
    }
    return sqrt(res / v.count());
}

template <typename T, typename T2> Vec<T> vconv(const Vec<T> &a, const Vec<T2> &b)
{
    if (a.count() == 0 || b.count() == 0)
        return Vec<T>();
    if (b.count() > a.count())
        return vconv(b, a);

    Vec<T> res(a.count() + b.count() - 1);

    // Left part
    int k = 0;
    for (int i = 0; i < b.count() - 1; i++)
    {
        res[k] = 0;
        for (int j = 0; j <= i; j++)
            res[k] += a[j] * b[b.count() - i + j - 1];
        k++;
    }
    // Middle part
    for (int i = 0; i <= a.count() - b.count(); i++)
    {
        res[k] = 0;
        for (int j = 0; j < b.count(); j++)
            res[k] += a[i + j] * b[j];
        k++;
    }
    // Right part
    for (int i = 0; i < b.count() - 1; i++)
    {
        res[k] = 0;
        for (int j = 0; j < b.count() - i - 1; j++)
            res[k] += a[a.count() - b.count() + j + i + 1] * b[j];
        k++;
    }
    return res;
}

template <typename T> Vec<T> downsample(const Vec<T> v, unsigned factor)
{
    Vec<T> res;
    res.resize(v.count() / factor);
    int i = 0;
    int j = 0;
    while (i < res.count())
    {
        res[i++] = v[j];
        j += factor;
    }
    return res;
}

template <typename T> inline Range<T> vrange(const Vec<T> &v)
{
    return Range<T>(v);
}

template <typename T> Vec<T> linspace(const Range<T> &range, int count)
{
    return linspace(range.value[0], range.value[1], count);
}

template <typename T, typename T2> Vec<T> linspace(const T &from, const T2 &to, int count)
{
    Vec<T> res;
    if (count == 0) return res;
    res.resize(count);
    T *x = res.data();
    if (count == 1)
    {
        x[0] = (from + to) / 2;
        return res;
    }
    T step = (static_cast<T>(to) - from) / (count - 1);
    for (int i = 0; i < count; i++)
        x[i] = from + step * i;
    return res;
}

template <typename T> Vec<T> normpdf(const Vec<T> &x, const T &mu, const T &sigma)
{
    Vec<T> res;
    // 1 / (sqrt(2pi) sigma) * exp(- (x - mu)^2 / sigma^2)
    if (sigma <= 0)
        return Vec<T>(x.count(), T());
    res.resize(x.count());
    T mul = 1.0 / sqrt(M_PI * 2) / sigma;
    for (int i = 0; i < res.count(); i++)
    {
        T v;
        v = (x[i] - mu) / sigma;
        res[i] = mul * exp(-v * v * 0.5);
    }
    return res;
}

}

#endif // QMWRAP_H
