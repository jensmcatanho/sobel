#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <sys/mman.h>

#include "neryimg.h"

/**
 * Allocates a shared memory block of the given size using
 * POSIX mmap. The block created is read/write-enabled,
 * and can be acessed by all child process of this process;
 * and other processes cannot get a pointer to it (it's anonymous)
 * @param size : size in bytes of the block
 */
void *createSharedMemoryBlock(size_t size);

/**
 * Prints an error message and exits the program with error code.
 * @param  message : a message string to be printed
 */
void *fail(char *message);

/**
 * Saves a given image buffer to a file.
 * @param image : a buffer containing a PPM image to be saved
 * @param file  : a filepath
 */
void saveImage(ppm_image image, char *filename);

/**
 * Saves a given image buffer to a debug file, for testing purposes,
 * as saveImage, but always saves to debug.ppm.
 * @param image : a buffer containing a PPM image
 */
void quickSaveImage(ppm_image image);

/**
 * Returns the size of a ppp_image's buffer.
 * That's the size ONLY of the buffer (buffer property),
 * not of the image type as a whole.
 * @param  image : a PPM image
 * @return unsigned long
 */
unsigned long sizeOfImageBuffer(ppm_image image);

/**
 * Returns an array of arrays representing the image vertically divided
 * by the numSlices argument.
 * eg. an image with 1200 width and 2 slices would yield:
 *   [[1,600], [600,1198]]
 *   that is, slices should be from 1 to 600 and 600 to 1198.
 * @param  image     : a PPM image
 * @param  numSlices : the number of slices
 * @return unsigned int[numSlices][2]
 */
unsigned int **getImageSliceRanges(ppm_image image, unsigned int numSlices);

/**
 * Adds a one-pixel thick box margin around a given ppm image buffer.
 * Take note that this increases the image size by two rows and collumns
 * @param  original : the original image
 * @return ppm_image
 */
ppm_image addMargins(ppm_image original);

/**
 * Applies the sobel transformation to a single pixel.
 * @param original : the original image
 * @param x        : input pixel's x
 * @param y        : input pixel's y
 * @param new      : target PPM image
 */
void transformPixelSobel(ppm_image original, unsigned int x, unsigned int y, ppm_image new);

/**
 * Applies the Sobel transform to a column slice of a given image.
 * This will be executed by each process.
 * @param original    : the original image
 * @param start       : start offset of the column slice
 * @param end         : end offset of the column slice
 * @param destination : destination image
 */
void sobelImageSlice(ppm_image original, unsigned int start, unsigned int end, ppm_image destination);

/**
 * Allocates a PPM image on shared memory.
 * Combines Nery's alloc_img logic with shared memory blocks.
 * @param  width  : image width
 * @param  height : image height
 * @return ppm_image : allocated image
 */
ppm_image allocSharedImage(unsigned int width, unsigned long height);

/**
 * Like Nery's get_ppm, but reads to a shared memory block.
 * Reads the given file pointer as a PPM and stores it in memory as ppm_image
 * @param  pf : the file (read permissions needed)
 * @return  ppm_image : the image
 */
ppm_image getPpmShared(FILE *pf);

#endif
