#ifndef MISC_HPP
#define MISC_HPP

#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define for_iterate(IT, CONT) typedef typeof(CONT) TYPEOF_##IT; for (TYPEOF_##IT::iterator IT = CONT.begin(); IT != CONT.end(); ++IT)

#define PRINT(X) std::cout << #X << " = " << (X) << std::endl;
#define PRINTDO(X) std::cout << ">> " << #X << std::endl << std::endl; X;

extern unsigned int randomstate;

template<class T>
inline void Swap(T& a, T& b) { T temp = a; a = b; b = temp; }

static inline int Mod(int x, int n) {
   return ((1000 * n) + x) % n;
}

static inline float FMod(float x, int n) {
   return fmod((1000 * n) + x, n);
}

static inline int ModDist(int x, int y, int n) {
   if (x > y) {int t = x; x = y; y = t;  }
   float d1 = y - x;
   float d2 = x + (n -  y);
   return (d1 < d2) ? d1 : d2;
}

static inline int Min(int x, int y) {
  return x < y ? x : y;
}

static inline int Max(int x, int y) {
  return x > y ? x : y;
}

static inline float Min(float x, float y) {
  return x < y ? x : y;
}

static inline float Max(float x, float y) {
  return x > y ? x : y;
}

static inline int ClipInt(int x, int min = 0, int max = 1) {
  return x > max ? max : (x < min ? min : x);
}

static inline float ClipFloat(float x, float min = 0.0, float max = 1.0) {
  return x > max ? max : (x < min ? min : x);
}

int RandInt(int n);

inline int RandInt(int a, int b) {
  return a + RandInt(b - a);
}

inline float RandFloat(float a=1.0) {
  return (a * RandInt(1e5)) / 1e5;
}

inline float RandFloat(float a, float b) {
  return a + RandFloat(b - a);
}

inline bool RandBool(float p)
{
  return RandFloat() < p;
}

float RandGauss();

inline float RandGauss(float mean, float sd) {
  return mean + sd * RandGauss();
}

inline int RandBit() {
  return RandInt(1);
}

inline int RandSign() {
  return 1 - 2 * RandBit();
}

void SetRandomSeed(int seed);

inline float SymF(float x) { return -1.0 + 2.0 * x; }

inline float SymI(int x)    { return -1.0 + 2.0 * x; }

#endif



