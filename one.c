#include "lib/neryimg.c"
#include "lib/utils.c"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

int main(int numArgs, char **args){

  // char* filePath = "./data/rio_map.ppm";
  //
  // FILE* originalImageFile = fopen(filePath, "rb");
  // if(!originalImageFile) fail("Could not read original image file");
  //
  // ppm_image originalImage = get_ppm(originalImageFile);
  // if(!originalImage) fail("Could not alloc PPM for the original image");
  //
  // ppm_image wMargins = addMargins(originalImage);
  //
  // to_greyscale(wMargins);
  // ppm_image result = alloc_img(wMargins->width, wMargins->height);
  // fill_img(result, 0,0,0);
  //
  // for(unsigned int i = 1; i < wMargins->width -2; i++){
  //   for(unsigned int j = 1; j < wMargins->height -2; j++){
  //     transformPixelSobel(wMargins, i, j, result);
  //   }
  // }
  //
  // quickSaveImage(result);
  //
  unsigned short nProcesses = 4;
  unsigned short waiting = nProcesses;
  pid_t pids[nProcesses];

  for(unsigned short i = 0; i < nProcesses; i++){
    pids[i] = fork();
    if(pids[i] == -1) fail("Failed at forking process");
    if(pids[i] == 0){
      printf("%i \n", i);
      exit(0);
    }
  }

  while(waiting){
    pid_t pid;
    pid = wait(NULL);
    printf("%ld exited \n", (long)pid);
    waiting--;
  }


  return 0;
}
