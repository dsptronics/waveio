/******************************************************************************

waveio.h - function prototypes and structures for
           wave file .wav read/write functions

******************************************************************************/
/*-----------------------------------------------------------------------------
    Copyright DSPtronics 2019 - MIT License
-----------------------------------------------------------------------------*/

//*****************************************************************************
// Wave file header info
struct HEADER {
	unsigned char riff[4];						// RIFF string label
	unsigned int overall_size	;				// overall size of file in bytes
	unsigned char wave[4];						// WAVE string label
	unsigned char fmt_chunk_marker[4];			// fmt string with trailing null char
	unsigned int length_of_fmt;					// length of the format data
	short int audioFormat;					    // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	short int numChannels;						// number of channels
	unsigned int sampleRate;					// sampling rate (blocks per second)
	unsigned int byteRate;						// SampleRate * NumChannels * BitsPerSample/8
	short int blockAlign;					    // NumChannels * BitsPerSample/8
	short int bitsPerSample;				    // bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned char data_chunk_header[4];		    // DATA string or FLLR string labels
	unsigned int data_size;						// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk of data that will be read
};

typedef struct HEADER WaveHeader;
//*****************************************************************************
// Wave Structure
typedef struct Wave {
    WaveHeader header;
    char* data;
    long long int index;
    long long int size;
    long long int nSamples;
} Wave;

WaveHeader makeWaveHeader(int const sampleRate, short int const numChannels, short int const bitsPerSample );
Wave makeWave(int const sampleRate, short int const numChannels, short int const bitsPerSample);
void waveDestroy( Wave* wave );
void waveSetDuration( Wave* wave, const float seconds );
void waveAddSample( Wave* wave, const float* samples );
void waveToFile( Wave* wave, const char* filename );
int wavwrite(char* filename, float *dataR, float *dataL,
              int size,
              int sampleRate,
              int numChannels,
              int bitsPerSample
              );
void displayData(float *dataL, float *dataR, int size);
int wavread(char* filename, float **dataL, float **dataR, int size, int sampleRate, int numChannels, int bitsPerSample);
