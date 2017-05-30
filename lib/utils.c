#include <stdbool.h>

/**
 * prints an error message and exists the program w/ error code.
 * @param  message : a message string, which will be printed
 * @return void
 */
void* fail(char* message){
  printf("FAILED due to: %s \n", message);
  exit(1);
}

/**
 * quickly saves a given image buffer to ./debug.ppm, for testing purposes.
 * @param image : a buffer cointaining a PPM image to be saved
 * @return void
 */
void quickSaveImage(ppm_image image){
  FILE* file = fopen("debug.ppm", "wb");
  output_ppm(file, image);
  fclose(file);
}

/**
 * adds a one-pixel thick box margin around a given ppm image buffer.
 * take note that this increases the image size by two rows and collumns
 * @param  original
 * @return ppm_image : the image w/ added margins
 */
ppm_image addMargins(ppm_image original){
  ppm_image result = alloc_img(original->width +2, original->height +2);
  fill_img(result, 0,0,0);
  for (unsigned int i = 0; i < result->width; i++) {
    for (unsigned int j = 0; j < result->height; j++) {
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
        int* pixel = (int*) GET_PIXEL(original, i-1, j-1);
        put_pixel_unsafe(result,
          i, j,
          pixel[0], pixel[1], pixel[2]
        );
      }
    }
  }
  return result;
}
