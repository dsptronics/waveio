# waveio
Wave file (.wav) reader and writer (C code)

# usage
    // Write data to wave file
    wavwrite("mytest.wav", wdataL, wdataR, size, sampleRate, numChannels, bitsPerSample)
    
     // Read data from wave file
    wavread("mytest.wav", &rdataL, &rdataR, size, sampleRate, numChannels, bitsPerSample)
    
Support for one or two channels (L, R)
