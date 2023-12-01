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
void krnl_nbody(float *particles, 
                int iterations){

    

    const float time_step = 0.01f;
    const float G = 6.67e-11f;
    const float min_cul_radius = 0.001f;
    const float max_cul_radius = 0.25f;
    const float min_force = 0.0001f;
    const float min_force_n = -0.0001f;
    

    
    
    float particle_i_buff[INPUT_LENGTH][5];
    #pragma HLS array_partition dim=2 type=complete variable=particle_i_buff
    #pragma HL
    
    
    float bufF[INPUT_LENGTH][2];
    #pragma HLS array_partition dim=2 type=complete variable=bufF

    
    Read_particle: 
    for (int i = 0; i < INPUT_LENGTH*5; i += 5){
        particle_i_buff[i/5][0] = particles[i];
        particle_i_buff[i/5][1] = particles[i+1];
        particle_i_buff[i/5][2] = particles[i+2];
        particle_i_buff[i/5][3] = particles[i+3];
        particle_i_buff[i/5][4] = particles[i+4];
    }

    
    
    TIME: for (int t = 0; t < iterations; t++){
    #pragma HLS pipeline off

        Loop_k: for (int k = 0; k < INPUT_LENGTH; k++){
            #pragma HLS pipeline off
            float x_k = particle_i_buff[k][0];
            float y_k = particle_i_buff[k][1];
            float mass_k = particle_i_buff[k][4];

            float force_x;
            float force_y;
            Loop_i: for (int i = 0; i < INPUT_LENGTH; i++ ){
                if (i != k){
                    // compute force
                    float xi = particle_i_buff[i][0];
                    float yi = particle_i_buff[i][1];
                    float massi = particle_i_buff[i][4];

                    float dx = x_k - xi;
                    float dy = y_k - yi;
                    
                    float distance_sqr = (dx * dx) + (dy * dy);
                    
                    float distance = hls::sqrtf((dx * dx) + (dy * dy));
                
                    float force_magnitude = (G * massi * mass_k) / distance_sqr;
                    force_x = force_magnitude * (dx / distance);
                    force_y = force_magnitude * (dy / distance);
                    if (!hls::isnan(force_x)){
                        bufF[i][0] += force_x;
                    }

                    if (!hls::isnan(force_y)){
                        bufF[i][1] += force_y;
                    }
                
                    
                    
                }    
            }
        }
        

        Loop_u: for (int i = 0; i < INPUT_LENGTH; i++){
            float x = particle_i_buff[i][0];
            float y = particle_i_buff[i][1];
            float vx = particle_i_buff[i][2];
            float vy = particle_i_buff[i][3];
            float mass = particle_i_buff[i][4];
            float ax = bufF[i][0]/mass;
            float ay = bufF[i][1]/mass;
            float x_n = x + (vx * time_step);
            float y_n = y + (vy * time_step);
            float vx_n = vx + (ax * time_step);
            float vy_n = vy + (ay * time_step);
            
            particle_i_buff[i][0] = hls::isnan(x_n) ? x : x_n;
            particle_i_buff[i][1] = hls::isnan(y_n) ? y : y_n;
            particle_i_buff[i][2] = hls::isnan(vx_n) ? vx : vx_n;
            particle_i_buff[i][3] = hls::isnan(vy_n) ? vy : vy_n;
            //particle_i_buff[i][4] = mass;
        }
        
        
    }
    
    Write_particle: 
    for (int i = 0; i < INPUT_LENGTH*5; i += 5){
        particles[i] = particle_i_buff[i/5][0]; 
        particles[i+1] = particle_i_buff[i/5][1];
        particles[i+2] = particle_i_buff[i/5][2]; 
        particles[i+3] = particle_i_buff[i/5][3]; 
        
    }
}
}
