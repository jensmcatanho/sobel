#include "utils.h"

void *createSharedMemoryBlock(size_t size) {
    return mmap(NULL, size,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_SHARED,
        0, 0
    );
}

void *fail(char *message) {
    printf("FAILED due to: %s\n", message);
    exit(1);
}

void saveImage(ppm_image image, char *filename) {
   FILE *file = fopen(filename, "wb");
   output_ppm(file, image);
   fclose(file);
}

void quickSaveImage(ppm_image image) {
    saveImage(image, "debug.ppm");
}

unsigned long sizeOfImageBuffer(ppm_image image) {
    return image->width * image->height * sizeof(pixel);
}

unsigned int** getImageSliceRanges(ppm_image image, unsigned int numSlices) {
    unsigned int **slices = (unsigned int **)malloc(sizeof(*slices) * numSlices);
    unsigned int sliceSize = image->width / numSlices;
    unsigned int remains = image->width % numSlices;
    unsigned int start, end, i;
     
    for(i = 0; i < numSlices; i++) {
        start = i * sliceSize;
        end = start + sliceSize;
        
        if (start == 0) start++;
        if (i >= numSlices - 1) end += remains - 2;
        
        slices[i] = (unsigned int *)malloc(sizeof(*slices[i]) * 2);
        slices[i][0] = start;
        slices[i][1] = end;
    }
    
    return slices;
}

ppm_image addMargins(ppm_image original) {
    ppm_image result = alloc_img(original->width + 2, original->height + 2);
    fill_img(result, 0, 0, 0);
  
    unsigned int i, j;
    for (i = 0; i < result->width; i++) {
      for (j = 0; j < result->height; j++) {
        bool isMargin = (
          ( (i == 0) || (i == result->width - 1) ) ||
          ( (j == 0) || (j == result->height - 1) )
        );
      
        if (isMargin) {
          put_pixel_unsafe(result, i, j, 255, 0, 0);
        
        } else {
          int *pixel = (int *)GET_PIXEL(original, i - 1, j - 1);
          put_pixel_unsafe(result, i, j, pixel[0], pixel[1], pixel[2]);
        }
      }
    }
    
    return result;
}

void transformPixelSobel(ppm_image original, unsigned int x, unsigned int y, ppm_image new) {
    const char sobel_x[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    const char sobel_y[3][3] = { { 1, 2, 1}, { 0, 0, 0}, {-1,-2,-1} };

    unsigned char fa = GET_PIXEL(original, x - 1, y - 1)[0];
    unsigned char fb = GET_PIXEL(original,     x, y - 1)[0];
    unsigned char fc = GET_PIXEL(original, x + 1, y - 1)[0];
    unsigned char fd = GET_PIXEL(original, x - 1,     y)[0];
    unsigned char fe = GET_PIXEL(original,     x,     y)[0];
    unsigned char ff = GET_PIXEL(original, x + 1,     y)[0];
    unsigned char fg = GET_PIXEL(original, x - 1, y + 1)[0];
    unsigned char fh = GET_PIXEL(original,     x, y + 1)[0];
    unsigned char fi = GET_PIXEL(original, x + 1, y + 1)[0];

    short pixel_x = (sobel_x[0][0] * fa) +
                    (sobel_x[0][1] * fb) +
                    (sobel_x[0][2] * fc) +
                    (sobel_x[1][0] * fd) +
                    (sobel_x[1][1] * fe) +
                    (sobel_x[1][2] * ff) +
                    (sobel_x[2][0] * fg) +
                    (sobel_x[2][1] * fh) +
                    (sobel_x[2][2] * fi);

    short pixel_y = (sobel_y[0][0] * fa) +
                    (sobel_y[0][1] * fb) +
                    (sobel_y[0][2] * fc) +
                    (sobel_y[1][0] * fd) +
                    (sobel_y[1][1] * fe) +
                    (sobel_y[1][2] * ff) +
                    (sobel_y[2][0] * fg) +
                    (sobel_y[2][1] * fh) +
                    (sobel_y[2][2] * fi);

    short val = abs(pixel_x) + abs(pixel_y);
    unsigned char char_val = (255 - (unsigned char)(val));

    if (char_val > 200)
        char_val = 255;
    else if (char_val < 100)
        char_val = 0;

    put_pixel_unsafe(new, x, y, char_val, char_val, char_val);
}

void sobelImageSlice(ppm_image original, unsigned int start, unsigned int end, ppm_image destination) {
    unsigned int i, j;
  
    for (i = start; i < end; i++)
      for (j = 1; j < destination->height -2; j++)
        transformPixelSobel(original, i, j, destination);
}

ppm_image allocSharedImage(unsigned int width, unsigned long height) {
    ppm_image image = createSharedMemoryBlock(sizeof(image_t));
    
    image->buf = createSharedMemoryBlock(width * height * sizeof(pixel));
    image->width = width;
    image->height = height;
    image->high = 0;
    image->max_color = 255;
    
    return image;
}

ppm_image getPpmShared(FILE *pf) {
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
