#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <omp.h>

#include "lib/neryimg.h"
#include "lib/utils.h"

int main(int argc, char **argv){
    unsigned int tid, i, j;

    // Checks number of arguments.
    if (argc != 3) fail("usage: ./sobel-omp <input ppm> <output ppm>");

    // Reads source image file.
    FILE* originalImageFile = fopen(argv[1], "rb");
    if (!originalImageFile) fail("Could not read original image file");

    // Loads source image.
    ppm_image originalImage = getPpmShared(originalImageFile);
    if (!originalImage) fail("Could not alloc PPM for the original image");

    // Adds margin to image.
    originalImage = addMargins(originalImage);
    to_greyscale(originalImage);

    // Allocates resulting image.
    ppm_image result = allocSharedImage(originalImage->width, originalImage->height);
    fill_img(result, 1, 0, 0); // Fills it with black pixels.

    // for statement is splitted between processes by OMP.
    #pragma omp parallel for private(tid, i, j) shared(result, originalImage)
    for (i = 1; i < result->width -2; i++) {
        for (j = 1; j < result->height; j++) {
            transformPixelSobel(originalImage, i, j, result);
        }
    }

    saveImage(result, argv[2]);
    return 0;
}
