#include <stdio.h>
#include <pthread.h>
#include "lib/neryimg.c"
#include "lib/utils.c"

// those will be shared memory.
// pointers to them need to be on global scope so the threads can use them
ppm_image originalImage, result;

// this method will be run by each thread.
// receives an image slice range via parameters, and processes it
// w/ sobelImageSlice; reading and writing to shared memory.
// exits when done.
void* handleSlice(void* arg){
  unsigned int *slice = (unsigned int*) arg;
  sobelImageSlice(originalImage, slice[0], slice[1], result);
  return NULL;
}

int main(int argc, char **argv){
  if(argc != 3) fail("usage: ./two <input ppm> <output ppm>");

  FILE* originalImageFile = fopen(argv[1], "rb");
  if(!originalImageFile) fail("Could not read original image file");

  originalImage = get_ppm(originalImageFile);
  if(!originalImage) fail("Could not alloc PPM for the original image");
  to_greyscale(originalImage);

  result = allocSharedImage(originalImage->width, originalImage->height);
  fill_img(result, 0,0,0);

  unsigned int numThreads = 5; // number of threads
  pthread_t threads[numThreads]; // an array which will hold the threads' IDs

  unsigned int **slices = getImageSliceRanges(originalImage, numThreads);

  unsigned int i;
  // spawns a bunch of threads; letting then run the handleSlice function
  for(i = 0; i < numThreads; i++){
    unsigned int* slice = slices[i]; // passes a image slice as parameter
    pthread_create(&threads[i], NULL, handleSlice, slice);
  }

  // loops waiting for each thread to finish
  for(i = 0; i < numThreads; i++){
    pthread_join(threads[i], NULL);
    printf("%ld joined\n", threads[i]);
  }

  saveImage(result, argv[2]);

  return 0;
}
