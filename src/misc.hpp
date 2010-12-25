#ifndef MISC_HPP
#define MISC_HPP

#include <QtGlobal>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

static inline int Mod(int x, int n) {
   return ((1000 * n) + x) % n;
}

static inline float FMod(float x, int n) {
   return fmod((1000 * n) + x, n);
}

#endif



