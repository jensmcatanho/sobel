#include <stdbool.h>
#include <sys/mman.h>

/**
 * allocates a shared memory block of the given size.
 * uses POSIX mmap. the block created is read/write-enabled,
 * and can be acessed by all child process of this process;
 * and other processes cannot get a pointer to it (it's annonymous)
 * @param size : size in bytes
 */
void* createSharedMemoryBlock(size_t size){
    return mmap(NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_SHARED,
        0, 0
    );
}

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
 * saves a given image buffer to a file.
 * @param image : a buffer cointaining a PPM image to be saved
 * @param file : a filepath
 * @return void
 */
void saveImage(ppm_image image, char* filename){
 FILE* file = fopen(filename, "wb");
 output_ppm(file, image);
 fclose(file);
}
/**
 * saves a given image buffer to a debug file, for testing purposes
 * as saveImage, but always saves to debug.ppm
 * @param image
 * @return void
 */
void quickSaveImage(ppm_image image){
  saveImage(image, "debug.ppm");
}

/**
 * returns the size of a ppp_image's buffer.
 * that's the size ONLY of the buffer (buf property),
 * not of the image type as a whole.
 * @param  image : a ppm_image
 * @return unsigned long : buf's size
 */
unsigned long sizeOfImageBuffer(ppm_image image){
  return image->width * image->height * sizeof(pixel);
}

/**
 * returns an array of arrays representing the image vertically divided
 * by the numSlices argument.
 * eg. an image with 1200 width and 2 slices would yield:
 *   [[1,600], [600,1198]]
 *   that is, slices should be from 1 to 600 and 600 to 1198.
 * @param  image     : a PPM image
 * @param  numSlices : the number of slices
 * @return unsigned int[numSlices][2]
 */
unsigned int** getImageSliceRanges(ppm_image image, unsigned int numSlices){
  unsigned int** slices = (unsigned int**) malloc(sizeof(*slices) * numSlices);
  unsigned int sliceSize = image->width / numSlices;
  unsigned int remains = image->width % numSlices;
  unsigned int start, end, i; unsigned int slice[2];
  for(i = 0; i < numSlices; i++){
    start = i * sliceSize;
    end = start + sliceSize;
    if(start == 0) start++;
    if(i >= numSlices-1) end += remains -2;
    slices[i] = (int*) malloc(sizeof(*slices[i]) *2);
    slices[i][0] = start; slices[i][1] = end;
  }
  return slices;
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
  unsigned int i, j;
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

/**
 * applies the Sobel transform to a collumn slice of a given image.
 * this will be run by each process
 * @param original    : the original image
 * @param start       : start offset of the collumn slice
 * @param end         : end offset of the collumn slice
 * @param destination : destination image
 */
void sobelImageSlice(ppm_image original, unsigned int start, unsigned int end, ppm_image destination){
  unsigned int i, j;
  for(i = start; i < end; i++){
    for(j = 1; j < destination->height -2; j++){
      transformPixelSobel(original, i, j, destination);
    }
  }
}

/**
 * allocates a PPM image on shared memory.
 * combines Nery's alloc_img logic w/ shared memory blocks
 * created w/ createSharedMemoryBlock.
 * @param  width  : image width
 * @param  height : image height
 * @return ppm_image : allocated image
 */
ppm_image allocSharedImage(unsigned int width, unsigned long height){
  ppm_image image;
  image = createSharedMemoryBlock(sizeof(image_t));
  image->buf = createSharedMemoryBlock(width * height * sizeof(pixel));
  image->width = width;
  image->height = height;
  image->high = 0;
  image->max_color = 255;
  return image;
}

/**
 * like Nery's get_ppm, but reads to a shared memory block.
 * reads the given file pointer as a PPM and stores it in memory as ppm_image
 * @param  pf : the file. read permissions needed
 * @return  ppm_image : the image
 */
ppm_image getPpmShared(FILE *pf)
{
        char buf[PPMREADBUFLEN], *t;
        ppm_image img;
        unsigned int w, h, d;
        int r;

        if (pf == NULL) return NULL;
        t = fgets(buf, PPMREADBUFLEN, pf);
        /* the code fails if the white space following "P6" is not '\n' */
        if ( (t == NULL) || ( strncmp(buf, "P6\n", 3) != 0 ) ) return NULL;
        do
        { /* Px formats can have # comments after first line */
           t = fgets(buf, PPMREADBUFLEN, pf);
           if ( t == NULL ) return NULL;
        } while ( strncmp(buf, "#", 1) == 0 );
        r = sscanf(buf, "%u %u", &w, &h);
        if ( r < 2 ) return NULL;

        r = fscanf(pf, "%u", &d);
        if ( (r < 1) || ( d != 255 ) ) return NULL;
        fseek(pf, 1, SEEK_CUR); /* skip one byte, should be whitespace */

        img = allocSharedImage(w, h);
        if ( img != NULL )
        {
            img->high = 0;
            size_t rd = fread(img->buf, sizeof(pixel), w*h, pf);
            if ( rd < w*h )
            {
               free_img(img);
               return NULL;
            }
            return img;
        }
        return NULL;
}
