#include "lib/neryimg.c"
#include "lib/utils.c"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

void writePipe(int P[2], void* content, unsigned int size){
  close(P[0]);
  write(P[1], content, size);
  close(P[1]);
}

void* readPipe(int P[2], int bufferSize){
  int bytesRead;
  void* buffer = createSharedMemoryBlock(bufferSize);
  close(P[1]);
  bytesRead = read(P[0], buffer, bufferSize);
  close(P[0]);
  return buffer;
}

ppm_image getSliceFragment(ppm_image image, unsigned int start, unsigned int end){
  unsigned int i, j;
  unsigned int width = end - start;
  unsigned int numSlices = image->width/(width);
  unsigned int sliceNum = end/(width);

  printf("%i-%i %i \n", start, end, width);

  ppm_image fragment = alloc_img(width, image->height);
  for(i = start; i < end; i++){
    for(j = 0; j < image->height; j++){
      int* pixel = (int*) GET_PIXEL(image, i, j);
      put_pixel_unsafe(fragment, i, j - (sliceNum -1),
        pixel[0], pixel[1], pixel[2]
      );
    }
  }

  // fill_img(fragment, 255,0,0);
  //
  // for(i = start -1; i < end+1; i++){
  //   for(j = 0; j < image->height+2; j++){
  //     bool isOutsideMargin = (
  //       // (i == 0 || i == image->width +2) ||
  //       (j == 0 || j == image->height+1)
  //     );
  //     if(!isOutsideMargin){
  //       int* pixel = (int*) GET_PIXEL(image, i, j);
  //       put_pixel_unsafe(fragment, i, j,
  //         pixel[0], pixel[1], pixel[2]
  //       );
  //     }
  //   }
  // }

  return fragment;
}

ppm_image addMarginsToFragment(ppm_image original, ppm_image fragment, unsigned int start, unsigned int end){
  unsigned int width = end - start;
  unsigned int numSlices = original->width/width;
  unsigned int sliceNum = end/width;
  ppm_image result = alloc_img(fragment->width +2, original->height+2);
  fill_img(result, 0,0,0);
  unsigned i, j;
  for (i = 0; i < result->width; i++) {
    for (j = 0; j < result->height; j++) {
      bool isMargin = (
        ( (i==0) || (i==result->width-1) ) ||
        ( (j==0) || (j==result->height-1) )
      );
      if(isMargin) {
        put_pixel_unsafe(result,
          i, j,
          255, 0, 0
        );
      } else {
        // int* pixel = (int*) GET_PIXEL(fragment, i, j);
        int* pixel = (int*) GET_PIXEL(fragment, start+i-numSlices+2, j);
        put_pixel_unsafe(result,
          i, j,
          pixel[0], pixel[1], pixel[2]
        );
      }
    }
  }
  return result;
}

int main(int numArgs, char **args){
  unsigned int i;
  char* filename = "data/rio_night.ppm";

  FILE* originalImageFile = fopen(filename, "rb");
    if(!originalImageFile) fail("Could not read original image file");
  ppm_image originalImage = get_ppm(originalImageFile);
    if(!originalImage) fail("Could not alloc PPM for the original image");

  // to_greyscale(originalImage); // converts to grayscale
// printf("%ld \n", sizeOfImageBuffer(originalImage));
  int duct[2];

  unsigned short numProcesses = 4;
  unsigned short waiting = numProcesses; //the number of unfinished processes
  pid_t pids[numProcesses]; //array which will have the PIDs of child processes

  ppm_image fragments[numProcesses];
  unsigned int** slices = getImageSliceRanges(originalImage, numProcesses, false);
  for(i = 0; i < numProcesses; i++){
      ppm_image frag = getSliceFragment(originalImage, slices[i][0], slices[i][1]);
      fragments[i] = addMarginsToFragment(originalImage, frag, slices[i][0], slices[i][1]);
      char outfile[20]; sprintf(outfile, "s_%i.ppm", slices[i][0]);
      saveImage(fragments[i], outfile);
  }

  // printf("%i %i", originalImage->width, slices[4][1]);

  // for(unsigned short i = 0; i < numProcesses; i++){
  //   pipe(duct);
  //   pids[i] = fork();
  //   if(pids[i] == -1) fail("Forking failed");
  //   if(pids[i] == 0){
  //     // ppm_image img = alloc_img(originalImage->width, originalImage->height);
  //     // img->buf = readPipe(duct, sizeOfImageBuffer(originalImage));
  //     // char filename[20];
  //     // sprintf(filename, "%i.ppm", i);
  //     // saveImage(img, filename);
  //     exit(0);
  //   } else {
  //     // writePipe(duct, originalImage->buf, sizeOfImageBuffer(originalImage));
  //   }
  // }
  //
  // while(waiting){
  //   pid_t pid;
  //   pid = wait(NULL);
  //   //printf("%ld exited \n", (long)pid);
  //   waiting--;
  // }

  // unsigned int tid, i, j;
  // // for will be split between processes by OMP
  // #pragma omp parallel for private(tid, i, j) shared(result, originalImage)
  // for(i = 1; i < result->width -2; i++){
  //   for(j = 1; j < result->height; j++){
  //     transformPixelSobel(originalImage, i, j, result);
  //   }
  // }
  //
  // quickSaveImage(result);
  return 0;
}
