#include <stdio.h>
#include <pthread.h>

#include "lib/neryimg.h"
#include "lib/utils.h"

// those will be shared memory.
// pointers to them need to be on global scope so the threads can use them
ppm_image originalImage, result;

// this method will be run by each thread.
// receives an image slice range via parameters, and processes it
// w/ sobelImageSlice; reading and writing to shared memory.
// exits when done.
void *handleSlice(void *arg) {
  unsigned int *slice = (unsigned int *) arg;
  sobelImageSlice(originalImage, slice[0], slice[1], result);
  
  return NULL;
}

int main(int argc, char **argv) {
    // Checks number of arguments.
    if (argc != 4) fail("usage: ./sobel-thread <input ppm> <output ppm> <number of threads>");

    // Reads source image file.
    FILE* originalImageFile = fopen(argv[1], "rb");
    if(!originalImageFile) fail("Could not read original image file");

    // Loads source image.
    originalImage = get_ppm(originalImageFile);
    if(!originalImage) fail("Could not alloc PPM for the original image");
    to_greyscale(originalImage);

    // Allocates resulting image.
    result = allocSharedImage(originalImage->width, originalImage->height);
    fill_img(result, 0, 0, 0);

    unsigned int numThreads = *argv[3] - '0';
    pthread_t threads[numThreads];

    unsigned int **slices = getImageSliceRanges(originalImage, numThreads);

    unsigned int i;
    // Spawns a bunch of threads letting then run the handleSlice function.
    for (i = 0; i < numThreads; i++) {
        unsigned int* slice = slices[i]; // Passes a image slice as parameter.
        pthread_create(&threads[i], NULL, handleSlice, slice);
    }

    // Loops waiting for each thread to finish
    for (i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %ld joined.\n", threads[i]);
    }

    saveImage(result, argv[2]);

    return 0;
}
