#include "lib/neryimg.c"
#include "lib/utils.c"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

int main(int numArgs, char **args){
  if(numArgs != 3) fail("usage: ./one <input ppm> <output ppm>");
  // reads source image file
  FILE* originalImageFile = fopen(args[1], "rb");
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
  unsigned short numProcesses = 5;
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

  saveImage(result, args[2]);

  return 0;
}
