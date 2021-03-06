/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// vector.h

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

typedef float real;
//typedef double real;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef D2R
#define D2R (M_PI/180.0)
#endif

#ifndef R2D
#define R2D (180.0/M_PI)
#endif

#ifndef ASSERT
#define ASSERT(expr)
#endif

//// Vector ////

class Vector {
public:

  real x, y, z;

  Vector();
  Vector(const Vector &v);
  Vector(real x, real y, real z);
  Vector(const real *r);

  // specialize for desired types
  template <class T> Vector(const T &t);

  Vector& operator=(real r);
  Vector& zero();

  operator real*() { return &x; }
  operator const real*() const { return &x; }

  real operator[](int n) const { ASSERT((unsigned int)n < 3); return (&x)[n]; }
  real& operator[](int n) { ASSERT((unsigned int)n < 3); return (&x)[n]; }

  bool operator==(const Vector &v) const;

  bool operator!() const;

  Vector& operator+=(const Vector &v);
  Vector& operator-=(const Vector &v);
  Vector& operator*=(const Vector &v);
  Vector& operator*=(real s);
  Vector& operator/=(real s);

  template <class R> R length_sq() const;

  real length_sq() const;
  real length() const;

  Vector& normalize(real v=1);

  Vector& rotate_x(real a);
  Vector& rotate_y(real a);
  Vector& rotate_z(real a);
};

//// standalone operators ////

Vector operator-(const Vector &v);
Vector operator+(const Vector &a, const Vector &b);
Vector operator-(const Vector &a, const Vector &b);
Vector operator*(const Vector &v, real s);
Vector operator*(real s, const Vector &v);

// dot product
real operator*(const Vector &a, const Vector &b);

real dot(const Vector &a, const Vector &b);
Vector cross(const Vector &a, const Vector &b);

// normalize a
Vector norm(const Vector &a, real v=1);

// project b onto a
Vector proj(const Vector &a, const Vector &b);

// compute normal
Vector normal(const Vector &a, const Vector &b, const Vector &c);

////

//// Vector ////

inline Vector::Vector() : x(0), y(0), z(0) { }
inline Vector::Vector(const Vector &v) : x(v.x), y(v.y), z(v.z) { }
inline Vector::Vector(real x, real y, real z) : x(x), y(y), z(z) { }
inline Vector::Vector(const real *r) : x(r[0]), y(r[1]), z(r[2]) { }

inline Vector& Vector::operator=(real r)
{ x = y = z = r; return *this; }

inline Vector& Vector::zero()
{ x = y = z = 0; return *this; }

inline bool Vector::operator==(const Vector &v) const
{ return x == v.x && y == v.y && z == v.z; }

inline bool Vector::operator!() const
{ return x == 0 && y == 0 && z == 0; }

inline Vector& Vector::operator+=(const Vector &v)
{ x += v.x; y += v.y; z += v.z; return *this; }

inline Vector& Vector::operator-=(const Vector &v)
{ x -= v.x; y -= v.y; z -= v.z; return *this; }
  
inline Vector& Vector::operator*=(const Vector &v)
{ x *= v.x; y *= v.y; z *= v.z; return *this; }
  
inline Vector& Vector::operator*=(real s)
{ x *= s; y *= s; z *= s; return *this; }

inline Vector& Vector::operator/=(real s)
{ return *this *= 1.0 / s; }

template <class R>
inline R Vector::length_sq() const
{ return (R)x * x + (R)y * y + (R)z * z; }

inline real Vector::length_sq() const
{ return (*this) * (*this); }

inline real Vector::length() const
{ return sqrt(length_sq()); }

inline Vector& Vector::normalize(real v)
{ real l = length(); if(l > 0) (*this) *= v/l; return *this; }

// rotate about x-axis
// y' = y*cos - z*sin
// z' = y*sin + z*cos
inline Vector& Vector::rotate_x(real a)
{ real c = cos(a*D2R), s = sin(a*D2R); return *this = Vector(x, y*c - z*s, y*s + z*c); }

// rotate about y-axis
// x' =  x*cos + z*sin
// z' = -x*sin + z*cos
inline Vector& Vector::rotate_y(real a)
{ real c = cos(a*D2R), s = sin(a*D2R); return *this = Vector(x*c + z*s, y, -x*s + z*c); }

// rotate about z-axis
// x' = x*cos - y*sin
// y' = x*sin + y*cos
inline Vector& Vector::rotate_z(real a)
{ real c = cos(a*D2R), s = sin(a*D2R); return *this = Vector(x*c - y*s, x*s + y*c, z); }

//// standalone operators ////

inline Vector operator-(const Vector &v)
{ return Vector(-v.x, -v.y, -v.z); }

inline Vector operator+(const Vector &a, const Vector &b)
{ return Vector(a.x+b.x, a.y+b.y, a.z+b.z); }

inline Vector operator-(const Vector &a, const Vector &b)
{ return Vector(a.x-b.x, a.y-b.y, a.z-b.z); }

inline Vector operator*(const Vector &v, real s)
{ return Vector(v.x*s, v.y*s, v.z*s); }

inline Vector operator/(const Vector &v, real s)
{ return v * (1.0 / s); }

inline Vector operator*(real s, const Vector &v)
{ return Vector(v.x*s, v.y*s, v.z*s); }

// dot product
inline real operator*(const Vector &a, const Vector &b)
{ return dot(a, b); }

#ifdef OSTREAM
inline std::ostream &operator<<(std::ostream &out, const Vector &v)
{ return out << v.x << "," << v.y << "," << v.z; }
#endif

//// standalone functions ////

inline real dot(const Vector &a, const Vector &b)
{ return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vector cross(const Vector &a, const Vector &b)
{ return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }

inline Vector norm(const Vector &a, real v)
{ real l = a.length(); if(l > 0) return a * (v/l); return a; }

// project a onto b
// v = a * b / |b|^2 * b
inline Vector proj(const Vector &a, const Vector &b)
{ real l = b.length_sq(); return l > 0 ? (a * b / l) * b : Vector(); }

// compute normal
inline Vector normal(const Vector &a, const Vector &b, const Vector &c)
{ return cross(b - a, c - a).normalize(); }

////

#endif // VECTOR_H
