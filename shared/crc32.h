/*
CRCs are calculated using standard CRC methods with pre and post conditioning, 
as defined by ISO 3309 [ISO-3309] or ITU-T V.42 [ITU-V42]. The CRC polynomial employed is 
x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1

The 32-bit CRC register is initialized to all 1's, and then the data from each byte 
is processed from the least significant bit (1) to the most significant bit (128). 
After all the data bytes are processed, the CRC register is inverted (its ones complement is taken).
This value is transmitted (stored in the file) MSB first. For the purpose of separating into 
bytes and ordering, the least significant bit of the 32-bit CRC is defined to be the 
coefficient of the x^31 term. 

Practical calculation of the CRC always employs a precalculated table to greatly accelerate 
the computation. 
*/

#ifndef NULL
#define NULL 0L
#endif

/* Return the CRC of the bytes buf[0..len-1]. */
// NOTE: endianness matters, for example if your data is big endian, 
// and you are on a little endian machine, you will need to reverse 
// the ordering of the bytes before using this function.
unsigned long P3DCRC32(const unsigned char* const buf, int len);

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */
// NOTE: to begin a running crc, initialize crc to all 1s (0xffffffffL)
unsigned long P3DUpdateCRC32(unsigned long crc, const unsigned char* const buf, unsigned int len);


// if DYNAMIC_CRC_TABLE was defined at compile time, then it checks whether 
//      crc_table_empty is set and if not, it creates the lookup table.
// otherwise it just returns the predefined lookup table.
const unsigned long * P3DGetCRC32Table();