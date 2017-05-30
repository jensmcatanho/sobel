#include "lib/neryimg.c"
#include "lib/utils.c"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

/**
 * applies the Sobel transform to a collumn slice of a given image.
 * this will be run by each process
 * @param original    : the original image
 * @param start       : start offset of the collumn slice
 * @param end         : end offset of the collumn slice
 * @param destination : destination image
 */
void sobelImageSlice(ppm_image original, unsigned int start, unsigned int end, ppm_image destination){
  for(unsigned int i = start; i < end; i++){
    for(unsigned int j = 1; j < destination->height -2; j++){
      transformPixelSobel(original, i, j, destination);
    }
  }
  char file[20];
  // sprintf(file, "debug%i.ppm", start);
  // saveImage(destination, file);
}

int main(int numArgs, char **args){

  char* filePath = "./data/escola_artes_visuais.ppm";

  // reads source image file
  FILE* originalImageFile = fopen(filePath, "rb");
    if(!originalImageFile) fail("Could not read original image file");

  // loads source image
  ppm_image originalImage = get_ppm(originalImageFile);
    if(!originalImage) fail("Could not alloc PPM for the original image");

  // adds margin to image
  ppm_image wMargins = addMargins(originalImage);
  free_img(originalImage); // frees memory from original image
  to_greyscale(wMargins); // converts to grayscale

  // allocates resulting image
  ppm_image result = allocSharedImage(wMargins->width, wMargins->height);
  fill_img(result, 0,0,0); // fills it with black pixels

  // Process control
  unsigned short numProcesses = 4;
  unsigned short waiting = numProcesses; //the number of unfinished processes
  pid_t pids[numProcesses]; //array which will have the PIDs of child processes

  //section of the image each process will work on
  unsigned int sliceSize = wMargins->width / numProcesses;
  unsigned int remainderSlice = wMargins->width % numProcesses; //if the division was uneven, stores the remainder so we can add it later to the last slice

  // forks [numProcesses] processes.
  for(unsigned short i = 0; i < numProcesses; i++){
    pids[i] = fork(); //adds the forked process PID to pids[]
    if(pids[i] == -1) fail("Failed at forking process");

    if(pids[i] == 0){
      unsigned int start = (i * sliceSize);
      unsigned int end = start + sliceSize;
      // if at the first slice, adds +1 to starting collumn
      if(start == 0) start++;
      // if at the last slice, adds the remainder collumns in case the slice division was uneven
      if(i >= numProcesses-1) end += remainderSlice -2;
      printf("process %i handling collumns %i-%i\n", getpid(), start, end);
      sobelImageSlice(wMargins, start, end, result);
      exit(0);
    }
  }

  while(waiting){
    pid_t pid;
    pid = wait(NULL);
    printf("%ld exited \n", (long)pid);
    waiting--;
  }

  quickSaveImage(result);

  return 0;
}
