/*
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: X11
*/
#include <stdlib.h>
#include <iostream>
#include "nbody.h"
#include <cstdlib>


// ********************************************************
int main()
{
    printf("Starting Nbody sims....\n");
    float *particles;
    float *original_particles;
    float *temp;
    particles = new float[INPUT_LENGTH * 5];
    if (particles == NULL)
    {
        printf("Error allocating particles\n");
        exit(EXIT_FAILURE);
    }

    
    original_particles = new float[INPUT_LENGTH * 5];
    if (original_particles == NULL)
    {
        printf("Error allocating original_particles\n");
        exit(EXIT_FAILURE);
    }
    
    // temp = new float[INPUT_LENGTH * 5];
    // if (temp == NULL)
    // {
    //     printf("Error allocating temp\n");
    //     exit(EXIT_FAILURE);
    // }

    

    for (int i = 0; i < INPUT_LENGTH * 5; i += 5){
        float tmp0 = (float(rand() % 200) / 200) * 100.0 - 50;
        float tmp1 = (float(rand() % 200) / 200) * 100.0 - 50;
        float tmp2 = (float(rand() % 200) / 200) * 100.0 - 50;
        float tmp3 = (float(rand() % 200) / 200) * 100.0 - 50;
        float tmp4 = (float(rand() % 200) / 200) * 100.0;

        particles[i] = tmp0;
        particles[i+1] = tmp1;
        particles[i+2] = tmp2;
        particles[i+3] = tmp3;
        particles[i+4] = tmp4;
        original_particles[i] = tmp0;
        original_particles[i+1] = tmp1;
        original_particles[i+2] = tmp2;
        original_particles[i+3] = tmp3;
        original_particles[i+4] = tmp4;

    }

    int iter = ITERATIONS;

    printf("Nbody Initialised particles!\n");
    krnl_nbody(particles, iter);

    bool calculation_performed = false;

    for (int i = 0; i < INPUT_LENGTH * 5; i += 5){
        for (int j = 0; j < 5; j ++){
            printf("%f  ", particles[i+j]);
            if (particles[i+j] != original_particles[i+j]){
                calculation_performed = true;
            }
        }
        printf("\n");
        //std::cout << std::endl;
        if (particles[i+4] != original_particles[i+4]){
            printf("mass diff!!!!!\n");
            return 1;
        }
    }

    if (calculation_performed){
        printf("Nbody ran successfully\n");
    } else{
        printf("calculation not done\n");
    }

    
    return 0;
//     int retval = 0, i, j;
//    FILE *fp;
//    fp = fopen("in.dat", "r");
//    // Read 640 entries from the File
//    for (i = 0; i < INPUT_LENGTH; i++)
//    {
//       float tmp1;
//       float tmp2;
//       float tmp3;
//       float tmp4;
//       float tmp5;
//       fscanf(fp, "%f %f %f %f %f", &tmp1, &tmp2, &tmp3, &tmp4, &tmp5);
//       particles[i].x = tmp1;
//       particles[i].y = tmp2;
//       particles[i].vx = tmp3;
//       particles[i].vy = tmp4;
//       particles[i].mass = tmp5;
//    }

//    fclose(fp);

//    int num_interations = 10;
//    fp = fopen("out.dat", "w");
//    printf("Nbody Initialised particles!\n");
//     //krnl_nbody(particles, temp, ITERATIONS);

//    for (i = 0; i < INPUT_LENGTH; i++)
//    {
//       fprintf(fp, "%f %f %f %f %f \n", temp[i].x, temp[i].y, temp[i].vx, temp[i].vy, temp[i].mass);
//    }

//     delete[] particles;
//     delete[] original_particles;
//     delete[] temp;
//     printf("Nbody ran through successfully!\n");
//     fclose(fp);
//     return 0;
}