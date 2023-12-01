#pragma once


#include <stdlib.h>
#include <iostream>
#include "ap_fixed.h"
#include "hls_math.h"

#define INPUT_LENGTH (10000)
#define BATCH_SIZE (8)
#define ITERATIONS (10)

//typedef ap_fixed<16, 12, AP_RND, AP_WRAP> float;



extern "C"
{
  // void nBodySimulation2D(float *particles);
  void krnl_nbody(float *particles, int iterations);
  
}



