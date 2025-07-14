#include "utils.h"

void zero(int8 *str, int16 size) {
  int8 *p;
  int16 n;

  for (n=0, p=str; n<size; n++, p++)
    *p = 0;
  return;
}