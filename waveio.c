/******************************************************************************

waveio.c - Wave file .wav read/write functions

    wavread            Read .wav file
    wavwrite           Write .wav file
    sec2time           Convert seconds to HH:MM:SS.mmm

    plus other helper functions

******************************************************************************/
/*-----------------------------------------------------------------------------
    Copyright DSPtronics 2019 - MIT License
-----------------------------------------------------------------------------*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "waveio.h"
#include "utils.h"
#define TRUE 1
#define FALSE 0

//*****************************************************************************
// Covert seconds to HH:MM:SS.mmm
char* sec2time(float totalseconds)
{

    int hours, minutes, seconds, milliseconds, diff;
    char *svalue;
    svalue = (char*) malloc(14);

    hours = totalseconds / 3600;

    diff = (totalseconds - hours * 3600);

    minutes =  diff / 60;

    diff = (diff - minutes * 60);

    seconds = diff;

    milliseconds = (int) round((totalseconds - floor(totalseconds)) * 1000);

    sprintf(svalue, "%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
    return svalue;

}
//*****************************************************************************
// Translate wave format type to string
char* getWaveFormatType(int audioFormat)
{

    char *format_name;
    format_name = (char*) malloc(14);

    strcpy(format_name,"");

    if (audioFormat == 1)
       strcpy(format_name,"PCM");
    else if (audioFormat == 6)
      strcpy(format_name, "A-law");
    else if (audioFormat == 7)
      strcpy(format_name, "Mu-law");

    return format_name;

}
//*****************************************************************************
// Convert little endian to big endian 4 byte int
int buffer4ToInt(unsigned char *buffer4)
{
    return buffer4[0] |
          (buffer4[1]<<8) |
          (buffer4[2]<<16) |
          (buffer4[3]<<24);
}
//*****************************************************************************
// Convert little endian to big endian 2 byte int
int buffer2ToInt(unsigned char *buffer2)
{
    return buffer2[0] |
          (buffer2[1]<<8);
}
//*****************************************************************************
// Display .wav file header info
void displayHeader(struct HEADER *header)
{

    printf("Wave File Header Info:\n");
    printf("----------------------\n");
    printf("1-4\t%.4s\n", header->riff);
    printf("5-8 \tOverall size: %u bytes (%.2f KB) \n", header->overall_size, (float) header->overall_size/1024);
    printf("9-12 \tWave marker: %.4s\n", header->wave);
    printf("13-16 \tFmt marker: %s\n", header->fmt_chunk_marker);
    printf("17-20 \tLength of Fmt header: %u \n", header->length_of_fmt);
    printf("21-22 \tFormat type: %u (%s) \n", header->audioFormat, getWaveFormatType(header->audioFormat));
    printf("23-24 \tChannels: %u \n", header->numChannels);
    printf("25-28 \tSample rate: %u\n", header->sampleRate);
    printf("29-32 \tByte Rate: %u , Bit Rate: %u\n", header->byteRate, header->byteRate*8);
    printf("33-34 \tBlock Alignment: %u \n", header->blockAlign);
    printf("35-36 \tBits per sample: %u \n", header->bitsPerSample);
    printf("37-40 \tData Marker: %.4s \n", header->data_chunk_header);
    printf("41-44 \tSize of data chunk: %u \n", header->data_size);

    printf("\n");
    long num_samples = (8 * header->data_size) / (header->numChannels * header->bitsPerSample);
    printf("Number of samples: %lu \n", num_samples);
    long size_of_each_sample = (header->numChannels * header->bitsPerSample) / 8;
    printf("Size of each sample: %ld bytes\n", size_of_each_sample);
    float duration_in_seconds = (float) header->overall_size / header->byteRate;
    printf("Approx.Duration in seconds= %f\n", duration_in_seconds);
    printf("Approx.Duration in h:m:s= %s\n", sec2time(duration_in_seconds));

}
//*****************************************************************************
// Read data from .wav file
int wavread(char* filename,
            float **dataL, float **dataR,
            int size,
            int sampleRate,
            int numChannels,
            int bitsPerSample
            )
{
    // Local variables
    struct HEADER header;
    unsigned char buffer4[4];
    unsigned char buffer2[2];
    char* seconds_to_time(float seconds);
    FILE *ptr;
    int read = 0;

    if (numChannels <= 0) {
        printf("Number of channels = 0, nothing to read.\n");
        return 1;
    } else if (numChannels > 2) {
        printf("Only 1 or 2 channels supported.\n");
        return 1;
    }

    // Open wave file
    ptr = fopen(filename, "rb");
    if (ptr == NULL) {
        return 1;
     }

    // Read wave file header
    read = fread(header.riff, sizeof(header.riff), 1, ptr);
    header.riff[4] = '\0';
    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    header.overall_size = buffer4ToInt(buffer4);
    read = fread(header.wave, sizeof(header.wave), 1, ptr);
    read = fread(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker), 1, ptr);
    header.fmt_chunk_marker[3] = '\0';
    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    header.length_of_fmt = buffer4ToInt(buffer4);
    read = fread(buffer2, sizeof(buffer2), 1, ptr);
    header.audioFormat = buffer2ToInt(buffer2);
    read = fread(buffer2, sizeof(buffer2), 1, ptr);
    header.numChannels = buffer2ToInt(buffer2);
    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    header.sampleRate = buffer4ToInt(buffer4);
    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    header.byteRate  = buffer4ToInt(buffer4);
    read = fread(buffer2, sizeof(buffer2), 1, ptr);
    header.blockAlign = buffer2ToInt(buffer2);
    read = fread(buffer2, sizeof(buffer2), 1, ptr);
    header.bitsPerSample = buffer2ToInt(buffer2);
    read = fread(header.data_chunk_header, sizeof(header.data_chunk_header), 1, ptr);
    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    header.data_size = buffer4ToInt(buffer4);

    // Calculate no.of samples
    long num_samples = (8 * header.data_size) / (header.numChannels * header.bitsPerSample);
    long size_of_each_sample = (header.numChannels * header.bitsPerSample) / 8;


    // Create vectors to store the audio samples
    *dataR = (float*) realloc(*dataR,num_samples*sizeof(float));
    *dataL = (float*) realloc(*dataL,num_samples*sizeof(float));

     // Read each sample from data chunk if PCM
    if (header.audioFormat == 1) { // PCM
            long i =0;
            unsigned char data_buffer[size_of_each_sample];
            int  size_is_correct = TRUE;

            // Verification: make sure that the bytes-per-sample is completely divisible by num.of numChannels
            long bytes_in_each_channel = (size_of_each_sample / header.numChannels);
            if ((bytes_in_each_channel  * header.numChannels) != size_of_each_sample) {
                printf("Error: %ld x %ud <> %ld\n", bytes_in_each_channel, header.numChannels, size_of_each_sample);
                size_is_correct = FALSE;
            }

            if (size_is_correct) {
                // Valid amplitude range for values based on word size (bits per sample)
                long low_limit = 0l;
                long high_limit = 0l;

                switch (header.bitsPerSample) {
                    case 8:
                        low_limit = -128;
                        high_limit = 127;
                        break;
                    case 16:
                        low_limit = -32768;
                        high_limit = 32767;
                        break;
                    case 32:
                        low_limit = -2147483648;
                        high_limit = 2147483647;
                        break;
                }

                for (i =0; i < num_samples; i++) {

                    read = fread(data_buffer, sizeof(data_buffer), 1, ptr);
                    if (read == 1) {

                        // Read all channels
                        unsigned int  xchannels = 0;
                        int data_in_channel = 0;
                        int coffset = 0;

                        for (xchannels = 0; xchannels < header.numChannels; xchannels ++ ) {

                            // Convert data from little endian to big endian based on bytes in each channel sample
                            if (bytes_in_each_channel == 4) {
                              data_in_channel =     (data_buffer[0+coffset]) |
                                                    (data_buffer[1+coffset]<<8) |
                                                    (data_buffer[2+coffset]<<16) |
                                                    (data_buffer[3+coffset]<<24);
                                                    coffset = coffset + 4;
                            }
                            else if (bytes_in_each_channel == 2) {
                                data_in_channel = data_buffer[0+coffset] |
                                                    (data_buffer[1+coffset] << 8);
                                                    coffset = coffset + 2;
                            }
                            else if (bytes_in_each_channel == 1) {
                                data_in_channel = data_buffer[0+coffset];
                                coffset = coffset + 1;
                            }

                            if (xchannels == 0){
                                   (*dataL)[i] = (float)data_in_channel/high_limit;
                            } else {
                                    (*dataR)[i] = (float)data_in_channel/high_limit;
                            }

                            // Check if value is within range
                            if (data_in_channel < low_limit || data_in_channel > high_limit)
                                printf("**value out of range\n");

                        }

                    }
                    else {
                        printf("Error reading file. %d bytes\n", read);
                        break;
                    }

                }  //for (i =0...
            } //if(size...
    } //if (header...

    // Memory  cleanup
    fclose(ptr);
    free(filename);

    // Debug
    //displayHeader(&header);

     return 0;

}
//*****************************************************************************
// Build .wav file header
WaveHeader makeWaveHeader(int const sampleRate, short int const numChannels, short int const bitsPerSample ){
    WaveHeader myHeader;

    // RIFF WAVE Header
    myHeader.riff[0] = 'R';
    myHeader.riff[1] = 'I';
    myHeader.riff[2] = 'F';
    myHeader.riff[3] = 'F';
    myHeader.wave[0] = 'W';
    myHeader.wave[1] = 'A';
    myHeader.wave[2] = 'V';
    myHeader.wave[3] = 'E';

    // Format subchunk
    myHeader.fmt_chunk_marker[0] = 'f';
    myHeader.fmt_chunk_marker[1] = 'm';
    myHeader.fmt_chunk_marker[2] = 't';
    myHeader.fmt_chunk_marker[3] = ' ';
    myHeader.audioFormat = 1; // FOR PCM
    myHeader.numChannels = numChannels; // 1 for MONO, 2 for stereo
    myHeader.sampleRate = sampleRate; // e.g. 44100 hertz
    myHeader.bitsPerSample = bitsPerSample; //
    myHeader.byteRate = myHeader.sampleRate * myHeader.numChannels * myHeader.bitsPerSample / 8;
    myHeader.blockAlign = myHeader.numChannels * myHeader.bitsPerSample/8;

    // Data subchunk
    myHeader.data_chunk_header[0] = 'd';
    myHeader.data_chunk_header[1] = 'a';
    myHeader.data_chunk_header[2] = 't';
    myHeader.data_chunk_header[3] = 'a';

    // Metrics
    myHeader.overall_size = 4+8+16+8+0;
    myHeader.length_of_fmt = 16;
    myHeader.data_size = 0;

    return myHeader;
}
//*****************************************************************************
// New Wave Structure
Wave makeWave(int const sampleRate, short int const numChannels, short int const bitsPerSample){
    Wave myWave;
    myWave.header = makeWaveHeader(sampleRate,numChannels,bitsPerSample);
    return myWave;
}
//*****************************************************************************
// Wave Structure Destructor, release data memory
void waveDestroy( Wave* wave ){
    free( wave->data );
}
//*****************************************************************************
// Wave Duration
void waveSetDuration( Wave* wave, const float seconds ){
    long long int totalBytes = (long long int)(wave->header.byteRate*seconds);
    wave->data = (char*)malloc(totalBytes);
    wave->index = 0;
    wave->size = totalBytes;
    wave->nSamples = (long long int) wave->header.numChannels * wave->header.sampleRate * seconds;
    wave->header.overall_size = 4+8+16+8+totalBytes;
    wave->header.data_size = totalBytes;
}
//*****************************************************************************
// Add sample (or samples if more than one channel)
void waveAddSample( Wave* wave, const float* samples ){
    int i;
    short int sample8bit;
    int sample16bit;
    long int sample32bit;
    char* sample;
    if( wave->header.bitsPerSample == 8 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample8bit = (short int) (127+127.0*samples[i]);

            toLittleEndian(1, (void*) &sample8bit);
            sample = (char*)&sample8bit;
            (wave->data)[ wave->index ] = sample[0];
            wave->index += 1;
        }
    }
    if( wave->header.bitsPerSample == 16 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample16bit = (int) (32767*samples[i]);

            toLittleEndian(2, (void*) &sample16bit);
            sample = (char*)&sample16bit;
            wave->data[ wave->index + 0 ] = sample[0];
            wave->data[ wave->index + 1 ] = sample[1];
            wave->index += 2;
        }
    }
    if( wave->header.bitsPerSample == 32 ){
        for( i=0; i<wave->header.numChannels; i+= 1){
            sample32bit = (long int) ((pow(2,32-1)-1)*samples[i]);

            toLittleEndian(4, (void*) &sample32bit);
            sample = (char*)&sample32bit;
            wave->data[ wave->index + 0 ] = sample[0];
            wave->data[ wave->index + 1 ] = sample[1];
            wave->data[ wave->index + 2 ] = sample[2];
            wave->data[ wave->index + 3 ] = sample[3];
            wave->index += 4;
        }
    }
}
//*****************************************************************************
// Write wave to .wav file
void waveToFile( Wave* wave, const char* filename ){

    // Before you read it, make sure all numbers are little endian
    toLittleEndian(sizeof(int), (void*)&(wave->header.overall_size));
    toLittleEndian(sizeof(int), (void*)&(wave->header.length_of_fmt));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.audioFormat));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.numChannels));
    toLittleEndian(sizeof(int), (void*)&(wave->header.sampleRate));
    toLittleEndian(sizeof(int), (void*)&(wave->header.byteRate));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.blockAlign));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.bitsPerSample));
    toLittleEndian(sizeof(int), (void*)&(wave->header.data_size));

    // Open the wave file, write header, then write data
    FILE *file;
    file = fopen(filename, "wb");
    fwrite( &(wave->header), sizeof(WaveHeader), 1, file );
    fwrite( (void*)(wave->data), sizeof(char), wave->size, file );
    fclose( file );

    // Convert back to system endian-ness
    toLittleEndian(sizeof(int), (void*)&(wave->header.overall_size));
    toLittleEndian(sizeof(int), (void*)&(wave->header.length_of_fmt));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.audioFormat));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.numChannels));
    toLittleEndian(sizeof(int), (void*)&(wave->header.sampleRate));
    toLittleEndian(sizeof(int), (void*)&(wave->header.byteRate));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.blockAlign));
    toLittleEndian(sizeof(short int), (void*)&(wave->header.bitsPerSample));
    toLittleEndian(sizeof(int), (void*)&(wave->header.data_size));
}
//*****************************************************************************
// [-1, ]] saturation
static inline float satVal(float val){return (val > 1) ? 1 : (val < -1) ? -1:val;}
//*****************************************************************************
// Main call to write .wav file
int wavwrite(char* filename, float *dataR, float *dataL,
              int size,
              int sampleRate,
              int numChannels,
              int bitsPerSample
              )
{
    if (numChannels <= 0) {
        printf("Number of channels = 0, nothing to write.");
        return 1;
    } else if (numChannels > 2) {
        printf("Only 1 or 2 channels supported.\n");
        return 1;
    }

    // Set wave parameters
    float duration;
    float frame[2];
    duration = (float) size / sampleRate;
    Wave mySound = makeWave((int)sampleRate,numChannels,bitsPerSample);
    waveSetDuration( &mySound, duration );


    // Add data samples to build wave
    int i;
    for(i=0; i<size; i+=1 ){
        //frame = &data[i];
        frame[0] = satVal(dataR[i]);
        frame[1] = satVal(dataL[i]);
        waveAddSample( &mySound, frame );
    }

    // Write it to a file and clean up when done
    waveToFile( &mySound, filename);
    waveDestroy( &mySound );

    return 0;

}
//*****************************************************************************
// Dump wave data
void displayData(float *dataL, float *dataR, int size)
{
    int k;

    printf("Left Channel \t Right Channel\n");
    for ( k = 0; k < size ; k++) {
        printf("%f\t%f\n", dataL[k], dataR[k]);
    }
}

