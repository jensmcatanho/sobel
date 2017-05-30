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

void transformPixelSobel(ppm_image original, unsigned int x, unsigned int y, ppm_image new){

  const char sobel_x[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
  const char sobel_y[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};

  unsigned char fa = GET_PIXEL(original, x-1,y-1)[0];
  unsigned char fb = GET_PIXEL(original, x,y-1)[0];
  unsigned char fc = GET_PIXEL(original, x+1,y-1)[0];
  unsigned char fd = GET_PIXEL(original, x-1,y)[0];
  unsigned char fe = GET_PIXEL(original, x,y)[0];
  unsigned char ff = GET_PIXEL(original, x+1,y)[0];
  unsigned char fg = GET_PIXEL(original, x-1,y+1)[0];
  unsigned char fh = GET_PIXEL(original, x,y+1)[0];
  unsigned char fi = GET_PIXEL(original, x+1,y+1)[0];

  short pixel_x = (sobel_x[0][0] * fa ) + (sobel_x[0][1] * fb ) + (sobel_x[0][2] * fc) + (sobel_x[1][0] * fd) + (sobel_x[1][1] * fe) + (sobel_x[1][2] * ff) + (sobel_x[2][0] * fg) + (sobel_x[2][1] * fh) + (sobel_x[2][2] * fi);

  short pixel_y = (sobel_y[0][0] * fa ) + (sobel_y[0][1] * fb ) + (sobel_y[0][2] * fc) + (sobel_y[1][0] * fd) + (sobel_y[1][1] * fe) + (sobel_y[1][2] * ff) + (sobel_y[2][0] * fg) + (sobel_y[2][1] * fh) + (sobel_y[2][2] * fi);

  short val = abs(pixel_x) + abs(pixel_y);
  unsigned char char_val = (255-(unsigned char)(val));

  if(char_val > 200)
      char_val = 255;
  else if(char_val < 100)
      char_val = 0;

  put_pixel_unsafe(new, x, y, char_val, char_val, char_val);
}
