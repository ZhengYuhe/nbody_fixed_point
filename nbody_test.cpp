/*
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: X11
*/

#include "nbody.h"


// ********************************************************
int main()
{
    printf("Starting Nbody sims....\n");
    fixed_t *particles;
    fixed_t *original_particles;
    fixed_t *temp;
    particles = new fixed_t[INPUT_LENGTH * 5];
    if (particles == NULL)
    {
        printf("Error allocating particles\n");
        exit(EXIT_FAILURE);
    }

    
    original_particles = new fixed_t[INPUT_LENGTH * 5];
    if (original_particles == NULL)
    {
        printf("Error allocating original_particles\n");
        exit(EXIT_FAILURE);
    }
    
    temp = new fixed_t[INPUT_LENGTH * 5];
    if (temp == NULL)
    {
        printf("Error allocating temp\n");
        exit(EXIT_FAILURE);
    }

    fixed_t tmp0 = 18.643;
    fixed_t tmp1 = 15.213;
    fixed_t tmp2 = 18.740;
    fixed_t tmp3 = 18.5;
    fixed_t tmp4 = 1.127;

    fixed_t t0 = 8.9;
    fixed_t t1 = 2.2;
    fixed_t t2 = 1.005;
    fixed_t t3 = 4.5;
    

    fixed_t t = 2; 

    for (int i = 0; i < INPUT_LENGTH * 5; i += 5){
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

        tmp0 = tmp0+t0;
        tmp1 = tmp1+t1;
        tmp2 = tmp2*t2;
        tmp3 = tmp3+t3;
        if (i % 25 == 0){
            tmp0 = tmp0 / t;
            tmp1 = tmp1 / t;
            tmp2 = tmp2 / t;
            tmp3 = tmp3 / t;
        }

        if (i % 100 == 0 ){
            tmp4 = tmp4 + t;
        }
    }

    int iter = ITERATIONS;

    printf("Nbody Initialised particles!\n");
    krnl_nbody(particles, temp, iter);

    bool calculation_performed = false;

    for (int i = 0; i < INPUT_LENGTH * 5; i += 5){
        for (int j = 0; j < 4; j ++){
            if (particles[i+j] != original_particles[i+j]){
                calculation_performed = true;
            }
        }
        if (particles[i+4] != original_particles[i+4]){
            std::cout << "mass difference at particle" << i << std::endl;
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