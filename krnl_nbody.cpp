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

#define BURST_J 20
#define DATA_II 8

/*
    Vector Addition Kernel Implementation 
    Arguments:
        in1   (input)     --> Input Vector1
        in2   (input)     --> Input Vector2
        out_r   (output)    --> Output Vector
        size  (input)     --> Size of Vector in Integer
*/


extern "C" {
void krnl_nbody(float *particles, 
                float *temp,
                int iterations){

    float* particles_tmp;
    float* temp_tmp;
    particles_tmp = particles;
    temp_tmp = temp;


    const float time_step = 0.01;
    const float G = 6.6743e-11f;
    const float min_cul_radius = 0.01;

    float BufP[BATCH_SIZE][5]; // batch several particles to calculate in parallel
    float BufF[BATCH_SIZE][2]; // batch the forces on the particles


    float BufJ[BURST_J][5]; //burst read particle_j to local buffer

    float Buf_Shift[BATCH_SIZE][DATA_II][2];

    #pragma HLS array_reshape dim=2 type=cyclic factor=2 variable=BufP
    #pragma HLS array_partition dim=1 type=complete variable=BufP

    #pragma HLS array_reshape dim=2 type=complete variable=BufF
    #pragma HLS array_partition dim=1 type=complete variable=BufF
     
    #pragma HLS array_partition dim=2 type=complete variable=BufJ

    #pragma HLS array_partition dim=0 type=complete variable=Buf_Shift
    



    
    TIME_STEP: for (int t = 0; t < ITERATIONS; t++){
        #pragma HLS pipeline off
        
        Pi: for (int i = 0; i < (INPUT_LENGTH*5); i += (BATCH_SIZE*5)){
            #pragma HLS pipeline off
            int curr_i;
            Load_Batch:for (int p = 0; p < BATCH_SIZE; p++){
                #pragma HLS unroll
                curr_i = i + p*5;
                BufP[p][0] = particles_tmp[curr_i];      //x
                BufP[p][1] = particles_tmp[curr_i+1];  //y
                BufP[p][2] = particles_tmp[curr_i+2];  //vx
                BufP[p][3] = particles_tmp[curr_i+3];  //vy
                BufP[p][4] = particles_tmp[curr_i+4];  //mass
                BufF[p][0] = 0;                     //force_x
                BufF[p][1] = 0;                     //force_y
            }

            for (int p = 0; p < BATCH_SIZE; p++){
                for (int s = 0; s < DATA_II; s++ ){
                    Buf_Shift[p][s][0] = 0;
                    Buf_Shift[p][s][1] = 0;
                }
            }
            
            
            Pj: for (int j = 0; j < (INPUT_LENGTH *5); j += (BURST_J * 5))
            {
                
                int curr_j;
                Fill_bufJ: for (int j0 = j; j0 < BURST_J; j0++){
                    curr_j = j + j0*5;
                    BufJ[j0][0] = particles_tmp[curr_j];
                    BufJ[j0][1] = particles_tmp[curr_j + 1];
                    BufJ[j0][2] = particles_tmp[curr_j + 2];
                    BufJ[j0][3] = particles_tmp[curr_j + 3];
                    BufJ[j0][4] = particles_tmp[curr_j + 4];
                }

                
                TILE_J: for (int j1 = 0; j1 < BURST_J; j1 ++){
                    #pragma HLS pipeline
                    float xj = BufJ[j1][0];
                    float yj = BufJ[j1][1];
                    float massj = BufJ[j1][4];
                    
                    
                    BATCH_FORCE: for (int b = 0; b < BATCH_SIZE; b++){
                        #pragma HLS unroll
                        // Calculate the distance between the two particles in 2D
                        if (i + b == j){continue;}

                        float dx = xj - BufP[b][0];
                        float dy = yj - BufP[b][1];

                        
                        float distance = sqrt(dx * dx + dy * dy);

                        
                        //  Define gravitational constant
                        //  Calculate the gravitational force in 2D

                        if (distance > min_cul_radius){
                            float force_magnitude = (G * BufP[b][4] * massj) / (distance * distance);
                            
                            // float force_magnitude_2 = force_magnitude_3 / G_scale;
                            // float force_magnitude_1 = force_magnitude_2 / G_scale;
                            // float force_magnitude_0 = force_magnitude_1 / G_scale;
                            // float force_magnitude = force_magnitude_1 / 100;



                            // Calculate force components in 2D
                            float rx = dx / distance;
                            float ry = dy / distance;

                            float force_x = force_magnitude * rx;
                            float force_y = force_magnitude * ry;

                            BufF[b][0] = force_x; //dependency in accumulation
                            BufF[b][1] = force_y; //dependency in accumulation

                            Buf_Shift[b][0][0] = force_x;
                            Buf_Shift[b][0][1] = force_y;
                            for (int s = DATA_II -1; s > 0; s --){
                                Buf_Shift[b][s][0] = Buf_Shift[b][s-1][0];
                                Buf_Shift[b][s][0] = Buf_Shift[b][s-1][0];
                            }
                            BufF[b][0] += Buf_Shift[b][DATA_II-1][0];
                            BufF[b][1] += Buf_Shift[b][DATA_II-1][0];
                        }
                        
                        
                    }
                }
                
                    
            }
            
            
            Update_Batch: for (int p = 0; p < BATCH_SIZE; p++){
                #pragma HLS unroll
                curr_i = i + p*5;
                float ax = BufF[p][0] / BufP[p][4];
                float ay = BufF[p][1] / BufP[p][4];
                // Update velocity in 2D using the calculated acceleration and time step
                temp_tmp[curr_i+2] = BufP[p][2] + ax * time_step;
                temp_tmp[curr_i+3] = BufP[p][3] + ay * time_step;
                temp_tmp[curr_i] = BufP[p][0] + temp_tmp[curr_i+2] * time_step;
                temp_tmp[curr_i+1] = BufP[p][1] + temp_tmp[curr_i+3] * time_step;
                temp_tmp[curr_i+4] = BufP[p][4];
            }
        }
        
        float *placeholder = temp_tmp;
        temp_tmp = particles_tmp;
        particles_tmp = placeholder;
    }

}
}
