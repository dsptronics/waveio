/******************************************************************************

wave_test.c -  Test and demo driver for wavread and wavwrite functions

    wavread            Read .wav file
    wavwrite           Write .wav file

******************************************************************************/
/*-----------------------------------------------------------------------------
    Copyright DSPtronics 2019 - MIT License
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include "waveio.h"

//*****************************************************************************
// Sinusoidal Generator
void wavegen(float *y, float freq, float sampleRate, int size) {

    const float pi = 3.14159265358979;
    int k;

    for (k = 0; k < size; k++){
        y[k] = (float) sin(2*pi*k*freq/sampleRate);
    }
}
//*****************************************************************************
// Compare waves
int wavematch(float *wdataL, float *rdataL, float *wdataR, float *rdataR, int size, int bitsPerSample) {

    int res = 1;
    long long range = pow(2,(bitsPerSample-1));
    float e = (float) 2 / range;
    int k;

    for (k = 0; k < size; k++){
        if (fabs((wdataL[k]  - rdataL[k])) > e)  {
            printf("written= %.0f   read= %.0f\n", wdataL[k]*range, rdataL[k]*range);
            res = 0;
            break;
        }
    }
    return res;
}
//*****************************************************************************
// Test driver
int main(){

    // Set test wave parameters
    float sampleRate = 44100.0; // Sampling rate, Hz
    float freq = 440.0;         // Sinusoidal freq, Hertz
    float *wdataR;               // Right channel write data
    float *wdataL;               // Left channel write data
    float *rdataR;               // Right channel read data
    float *rdataL;               // Left channel read data
    int size = 1000;            // Sinusoidal length
    int numChannels = 2;        // Number of channels, 2 (L, R)
    int bitsPerSample = 32;     // Data word size: 32 bits

    // Allocate memory for data vectors
    wdataR = (float*) malloc(size*sizeof(float));
    wdataL = (float*) malloc(size*sizeof(float));
    rdataR = (float*) malloc(size*sizeof(float));
    rdataL = (float*) malloc(size*sizeof(float));

    // Fill vectors with sinusoidal data
    wavegen(wdataR, freq, sampleRate, size);
    wavegen(wdataL, 2*freq, sampleRate, size);

    // Write data to wave file
    wavwrite("mytest.wav", wdataL, wdataR, size, sampleRate, numChannels, bitsPerSample);

    // Read data from wave file
    wavread("mytest.wav", &rdataL, &rdataR, size, sampleRate, numChannels, bitsPerSample);

    displayData(rdataL, rdataR, size);

    // Check if read data matches written data
    if (wavematch(wdataL, rdataL, wdataR, rdataR, size, bitsPerSample)) {
        printf("\nVerification: SUCCESFUL\n");
    } else {
        printf("\nVerification: FAILED\n");
    }

    return 0;
}

