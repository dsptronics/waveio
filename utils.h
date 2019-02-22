/******************************************************************************

utils.h - utility functions prototypes for bit and byte manipulation

******************************************************************************/
/*-----------------------------------------------------------------------------
    Copyright DSPtronics 2019 - MIT License
-----------------------------------------------------------------------------*/
#ifndef UTILS_H
#define UTILS_H

int isBigEndian();
void reverseEndianness(const long long int size, void *value);
void toBigEndian(const long long int size, void *value);
void toLittleEndian(const long long int size, void* value);

#endif
