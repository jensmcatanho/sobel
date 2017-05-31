#include "lib/neryimg.c"
#include "lib/utils.c"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <omp.h>

int main(int numArgs, char **args){
  if(numArgs != 3) fail("usage: ./three <input ppm> <output ppm>");

  // reads source image file
  FILE* originalImageFile = fopen(args[1], "rb");
    if(!originalImageFile) fail("Could not read original image file");

  // loads source image
  ppm_image originalImage = getPpmShared(originalImageFile);
    if(!originalImage) fail("Could not alloc PPM for the original image");

  // adds margin to image
  originalImage = addMargins(originalImage);
  to_greyscale(originalImage); // converts to grayscale

  // allocates resulting image
  ppm_image result = allocSharedImage(originalImage->width, originalImage->height);
  fill_img(result, 1,0,0); // fills it with black pixels

  unsigned int tid, i, j;
  // for will be split between processes by OMP
  #pragma omp parallel for private(tid, i, j) shared(result, originalImage)
  for(i = 1; i < result->width -2; i++){
    for(j = 1; j < result->height; j++){
      transformPixelSobel(originalImage, i, j, result);
    }
  }

  saveImage(result, args[2]);
  return 0;
}
