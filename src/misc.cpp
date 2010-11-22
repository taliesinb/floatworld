#include "misc.hpp"

unsigned int randomstate = 5;

float RandGauss()
{
  float x = 0; int n = 5;
  for (int i = 0; i < n * n; i++) x += RandFloat();
  return 3.4739 * (x - (n * n)/2.0) / n; // A good approximation
}  


