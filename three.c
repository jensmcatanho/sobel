#include "lib/neryimg.c"
#include "lib/utils.c"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <omp.h>

int main(int numArgs, char **args){

  char* filePath = "./data/escola_artes_visuais.ppm";

  // reads source image file
  FILE* originalImageFile = fopen(filePath, "rb");
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

  quickSaveImage(result);
  return 0;
}
