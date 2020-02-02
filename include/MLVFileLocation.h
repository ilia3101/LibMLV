/* MLVFileLocation is a 64 bit type for storing where a block is in an MLV file,
 * and which file it is in (for split files). File index gets 8 bits, position
 * gets 56. This means max file size is 72 petabytes (enough) */
#ifndef _MLVFileLocation_h_
#define _MLVFileLocation_h_

#include <stdint.h>

/* The type */
typedef uint64_t MLVFileLocation_t;

/* Get and set file index */
#define MLVFileLocationGetFileIndex(Location) ((Location)>>56UL)
#define MLVFileLocationSetFileIndex(Location, Index) (((Location) & 0x00ffffffffffffffUL) | (((uint64_t)(Index)) << 56UL))

/* Get and set file position */
#define MLVFileLocationGetPosition(Location) ((Location) & 0x00ffffffffffffffUL)
#define MLVFileLocationSetPosition(Location, Position) (((Location) & 0xff00000000000000UL) | (((uint64_t)(Location)) & 0x00ffffffffffffffUL))

#endif