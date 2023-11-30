/*******************************************************************************
Vendor: Xilinx
Associated Filename: krnl_vadd.cpp
Purpose: Vitis vector addition example
*******************************************************************************
Copyright (C) 2019 XILINX, Inc.

This file contains confidential and proprietary information of Xilinx, Inc. and
is protected under U.S. and international copyright and other intellectual
property laws.

DISCLAIMER
This disclaimer is not a license and does not grant any rights to the materials
distributed herewith. Except as otherwise provided in a valid license issued to
you by Xilinx, and to the maximum extent permitted by applicable law:
(1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX
HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
in contract or tort, including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature related to, arising under
or in connection with these materials, including for any direct, or any indirect,
special, incidental, or consequential loss or damage (including loss of data,
profits, goodwill, or any type of loss or damage suffered as a result of any
action brought by a third party) even if such damage or loss was reasonably
foreseeable or Xilinx had been advised of the possibility of the same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any
application requiring fail-safe performance, such as life-support or safety
devices or systems, Class III medical devices, nuclear facilities, applications
related to the deployment of airbags, or any other applications that could lead
to death, personal injury, or severe property or environmental damage
(individually and collectively, "Critical Applications"). Customer assumes the
sole risk and liability of any use of Xilinx products in Critical Applications,
subject only to applicable laws and regulations governing limitations on product
liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
ALL TIMES.

*******************************************************************************/

//------------------------------------------------------------------------------
//
// kernel:  vadd
//
// Purpose: Demonstrate Vector Add Kernel
//

#include "nbody.h"
#include "hls_math.h"
/*
    Vector Addition Kernel Implementation 
    Arguments:
        in1   (input)     --> Input Vector1
        in2   (input)     --> Input Vector2
        out_r   (output)    --> Output Vector
        size  (input)     --> Size of Vector in Integer
*/


extern "C" {
void krnl_nbody(fixed_t *particles, 
                fixed_t *temp,
                int iterations){

    fixed_t* particles_tmp;
    fixed_t* temp_tmp;
    particles_tmp = particles;
    temp_tmp = temp;


    const fixed_t time_step = 0.1;
    const fixed_t G = 6.674; // G = 6.6743e-11f;
    const fixed_t G_scale = 1000;
    const fixed_t min_cul_radius = 0.01;

    fixed_t BufP[BATCH_SIZE][5]; // batch several particles to calculate in parallel
    fixed_t BufF[BATCH_SIZE][2]; // batch the forces on the particles

    #pragma HLS array_reshape dim=2 type=cyclic factor=2 variable=BufP
    #pragma HLS array_partition dim=1 type=complete variable=BufP

    #pragma HLS array_reshape dim=2 type=complete variable=BufF
    #pragma HLS array_partition dim=1 type=complete variable=BufF
     
    
    TIME_STEP: for (int t = 0; t < ITERATIONS; t++){
        #pragma HLS pipeline off
        
        Pi: for (int i = 0; i < (INPUT_LENGTH*5); i += (BATCH_SIZE*5)){
            #pragma HLS pipeline off
            int curr_index;
            Load_Batch:for (int p = 0; p < BATCH_SIZE; p++){
                #pragma HLS unroll
                curr_index = i + p*5;
                BufP[p][0] = particles_tmp[curr_index];      //x
                BufP[p][1] = particles_tmp[curr_index+1];  //y
                BufP[p][2] = particles_tmp[curr_index+2];  //vx
                BufP[p][3] = particles_tmp[curr_index+3];  //vy
                BufP[p][4] = particles_tmp[curr_index+4];  //mass
                BufF[p][0] = 0;                     //force_x
                BufF[p][1] = 0;                     //force_y
            }
            
            
            Pj: for (int j = 0; j < (INPUT_LENGTH *5); j += 5)
            {
                //#pragma HLS pipeline

                // read particle j
                fixed_t xj = particles_tmp[j];
                fixed_t yj = particles_tmp[j+1];
                fixed_t massj = particles_tmp[j+4];
                
                
                BATCH_FORCE: for (int b = 0; b < BATCH_SIZE; b++){
                    #pragma HLS unroll
                    // Calculate the distance between the two particles in 2D
                    if (i + b == j){continue;}

                    fixed_t dx = xj - BufP[b][0];
                    fixed_t dy = yj - BufP[b][1];

                     
                    fixed_t distance = hls::sqrt(dx * dx + dy * dy);

                    
                    //  Define gravitational constant
                    //  Calculate the gravitational force in 2D

                    if (distance > min_cul_radius){
                        fixed_t force_magnitude_1 = (G * BufP[b][4] * massj) / (distance * distance);
                        fixed_t force_magnitude = force_magnitude_1 / G_scale;
                        // fixed_t force_magnitude_2 = force_magnitude_3 / G_scale;
                        // fixed_t force_magnitude_1 = force_magnitude_2 / G_scale;
                        // fixed_t force_magnitude_0 = force_magnitude_1 / G_scale;
                        // fixed_t force_magnitude = force_magnitude_1 / 100;



                        // Calculate force components in 2D
                        fixed_t rx = dx / distance;
                        fixed_t ry = dy / distance;

                        fixed_t force_x = force_magnitude * rx;
                        fixed_t force_y = force_magnitude * ry;

                        BufF[b][0] += force_x; //dependency in accumulation
                        BufF[b][1] += force_y; //dependency in accumulation
                    }
                    
                    
                }
                    
            }
            
            
            Update_Batch: for (int p = 0; p < BATCH_SIZE; p++){
                #pragma HLS unroll
                curr_index = i + p*5;
                fixed_t ax = BufF[p][0] / BufP[p][4];
                fixed_t ay = BufF[p][1] / BufP[p][4];
                // Update velocity in 2D using the calculated acceleration and time step
                temp_tmp[curr_index+2] = BufP[p][2] + ax * time_step;
                temp_tmp[curr_index+3] = BufP[p][3] + ay * time_step;
                temp_tmp[curr_index] = BufP[p][0] + temp_tmp[curr_index+2] * time_step;
                temp_tmp[curr_index+1] = BufP[p][1] + temp_tmp[curr_index+3] * time_step;
                temp_tmp[curr_index+4] = BufP[p][4];
            }
        }
        
        fixed_t *placeholder = temp_tmp;
        temp_tmp = particles_tmp;
        particles_tmp = placeholder;
    }

}
}
