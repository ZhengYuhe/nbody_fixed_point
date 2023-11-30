#pragma once


#include <stdlib.h>
#include <iostream>
#include "ap_fixed.h"
#include "hls_math.h"

#define INPUT_LENGTH (100)
#define BATCH_SIZE (8)
#define ITERATIONS (10)

typedef ap_fixed<16, 12, AP_RND, AP_WRAP> fixed_t;



extern "C"
{
  // void nBodySimulation2D(float *particles);
  void krnl_nbody(fixed_t *particles, fixed_t *temp, int iterations);
  
}



