/******************************************************************************

utils.c - utility functions for bit and byte manipulation

******************************************************************************/
/*-----------------------------------------------------------------------------
    Copyright DSPtronics 2019 - MIT License
-----------------------------------------------------------------------------*/

// -------------------------------------------------- [ Section: Endianness ] -
int isBigEndian() {
    int test = 1;            // Multi-byte e.g. 0001
    char *p = (char*)&test;  //Single byte

    return p[0] == 0;        //First byte = 0?
}
void reverseEndianness(const long long int size, void *value){
    int i;
    char result[32];
    for( i=0; i<size; i+=1 ){
        result[i] = ((char*)value)[size-i-1];
    }
    for( i=0; i<size; i+=1 ){
        ((char*)value)[i] = result[i];
    }
}
void toBigEndian(const long long int size, void *value){
    char needsFix = !( (1 && isBigEndian()) || (0 && !isBigEndian()) );
    if( needsFix ){
        reverseEndianness(size,value);
    }
}
void toLittleEndian(const long long int size, void* value){
    char needsFix = !( (0 && isBigEndian()) || (1 && !isBigEndian()) );
    if( needsFix ){
        reverseEndianness(size,value);
    }
}
