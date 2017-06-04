#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

#include "lib/neryimg.h"
#include "lib/utils.h"

int main(int argc, char **argv){
    // Checks number of arguments.
    if (argc != 4) fail("usage: ./sobel-process <input ppm> <output ppm> <number of processes>");
  
    // Reads source image file.
    FILE* originalImageFile = fopen(argv[1], "rb");
    if (!originalImageFile) fail("Could not read original image file");

    // Loads source image.
    ppm_image originalImage = get_ppm(originalImageFile);
    if (!originalImage) fail("Could not alloc PPM for the original image");

    // Adds margin to image.
    ppm_image wMargins = addMargins(originalImage);
    free_img(originalImage); // Frees memory from the original image.
    to_greyscale(wMargins); // Converts to grayscale.

    // Allocates resulting image.
    ppm_image result = allocSharedImage(wMargins->width, wMargins->height);
    fill_img(result, 0, 0, 0); // Fills it with black pixels.

    // Process control.
    unsigned short numProcesses = *argv[3] - '0';
    unsigned short unfinished = numProcesses;
    pid_t pids[numProcesses];

    // Section of the image each process will work on.
    unsigned int sliceSize = wMargins->width / numProcesses;
    // If the division is uneven, stores the remainder so we can add it later to the last slice.
    unsigned int remainderSlice = wMargins->width % numProcesses;
    unsigned short i;

    // Forks [numProcesses] processes.
    for (i = 0; i < numProcesses; i++) {
        pids[i] = fork(); // Adds the forked process PID to pids[].
        
        if (pids[i] == -1) fail("Failed at forking process");
        
        if (pids[i] == 0) {
            unsigned int start = i * sliceSize;
            unsigned int end = start + sliceSize;
            
            // If at the first slice, adds 1 to starting column.
            if (start == 0) start++;
            
            // If at last slice, adds the remainder columns in case the division was uneven.
            if (i >= numProcesses - 1) end += remainderSlice - 2;
            
            printf("Process %i handling columns %i to %i\n", getpid(), start, end);
            sobelImageSlice(wMargins, start, end, result);
            exit(0);
        }
    }
    
    while (unfinished) {
        pid_t pid;
        pid = wait(NULL);
        printf("%ld exited.\n", (long)pid);
        unfinished--;
    }
    
    saveImage(result, argv[2]);

    return 0;
}
