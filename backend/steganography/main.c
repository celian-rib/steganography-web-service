#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define LOADBMP_IMPLEMENTATION
#include "loadbmp.h"


#include "libspng/spng/spng.h"

typedef struct {
  int bit_index;
  const char* p;
  
} bit_depacker_t;

static void bit_depacker_init(bit_depacker_t* bd, const char* p) {
  bd->bit_index = 0;
  bd->p = p;
}

/** @brief Depack string bit by bit
 * @return true if depacking can continue, false otherwise */
static int bit_depacker_next(bit_depacker_t* bd, unsigned char* bit) {
  
  // check if last byte is reached
  if(*(bd->p) == '\0') {
    return 0;
  }

  // read bit at bit_index in current byte
  unsigned char byte = *(bd->p);
  *bit = byte & (1 << bd->bit_index);

  // increment bit index, advance byte pointer on overflow
  bd->bit_index++;
  if(bd->bit_index > 7) {
    bd->bit_index = 0;
    bd->p++;
  }

  return 1;
}


typedef struct {
  int bit_index;
  char *p;
}bit_packer_t;

static void bit_packer_init(bit_packer_t* bp, char* p, int sz) {
  bp->bit_index = 0;
  bp->p = p;
  // clear all bytes 
  memset(p, 0, sz);
}

static void bit_packer_push(bit_packer_t* bp, unsigned char bit) {
  // pack bit to current byte
  const int shift = bp->bit_index;
  if(bit) {
    *(bp->p) = *(bp->p) | (1<<shift);
  }

  // increment bit index, advance byte pointer on overflow
  bp->bit_index++;
  if(bp->bit_index > 7) {
    bp->bit_index = 0;
    bp->p++;
  }
}

void help(const char *name) {
  printf("%s -encode input.png \"clear text\" output.png\n", name);
  printf("%s -decode input.png\n", name);
}

typedef struct {
  uint8_t *buffer;
  unsigned int width;
  unsigned int height;
  uint8_t depth;
} image_t;

int steganography_decode_text(image_t *image, char * buffer) {
  // -----------------
  // DECODE CLEAR TEXT FROM IMAGE DATA

  // allocate a clear text string big enough to fit maximum image decoded size
  const int clear_text_sz = (image->depth*image->width * image->height)/8 + 1;
  //buffer = malloc(clear_text_sz);
  bit_packer_t bp;
  bit_packer_init(&bp, buffer, sizeof(buffer));

  // let's iterate over image pixels
  for(int y=0; y<image->height; y++)
    for(int x=0; x<image->width; x++) {

    // compute pixel index
    int idx = x + y*image->width;

    unsigned char bit = 0;
    for(int color_index=0; color_index<image->depth; color_index++) {
      // get pixel color 
      unsigned char cvalue = image->buffer[image->depth*idx + color_index];
      // push pixel color LSB to clear text
      bit_packer_push(&bp, cvalue & 0x01);
    }

  }
  return 0;

}

int steganography_encode_text(image_t *image, const char * text) {
  // -----------------
  // ENCODE IMAGE DATA WITH CLEAR TEXT

  // estimate pixel count to fit clear text
  // (one bit per pixel color, 3 color by pixel)
  const int pcount = strlen(text) * 8;
  if(pcount > image->width*image->height*3) {
    fprintf(stderr, "unable to fit clear text in image\n");
    return -2;
  }

  bit_depacker_t bd;
  bit_depacker_init(&bd, text);

  // let's iterate over image pixels
  for(int y=0; y<image->height; y++)
    for(int x=0; x<image->width; x++) {

      // compute pixel index
      int idx = x + y*image->width;

      unsigned char bit = 0;
      for(int color_index=0; color_index<image->depth; color_index++) {
        // get pixel color 
        unsigned char cvalue = image->buffer[image->depth*idx + color_index];
        // depack one bit from clear text
        bit_depacker_next(&bd,&bit);
        // update pixel color with LSB set to clear text bit value
        image->buffer[image->depth*idx + color_index] = (cvalue & 0xfe)   | (bit ? 0x01 : 0);
      }

  }
}

void print_image(image_t* image) {
  printf("Image is %dx%dx%d\n", image->width, image->height, image->depth);
}

int load_bmp(const char *infile, image_t * image) {

  unsigned int rv = 
    loadbmp_decode_file(infile, &(image->buffer), &(image->width), &(image->height), LOADBMP_RGB);
  image->depth = 3;

  if(rv) {
    fprintf(stderr, "unable to open input BMP file %s (rv=%d)\n", infile, rv);
    return -1;
  }
  print_image(image);
  return 0;
}

int save_bmp(image_t *image, const char * outfile) {
  if(image->depth != 3) {
    fprintf(stderr, "invalid pixel depth %d\n", image->depth);
    return -2;
  }
  return loadbmp_encode_file(outfile, image->buffer, image->width, image->height, LOADBMP_RGB);
}



int load_png(const char *infile, image_t * image) {
  FILE *png;
  png = fopen(infile, "rb");
  if(png == NULL) {
    fprintf(stderr, "Could not read png %s\n", infile);
    return -2;
  }
  spng_ctx *ctx = NULL;
  ctx = spng_ctx_new(0);
  if(ctx == NULL)
  {
      fprintf(stderr, "spng_ctx_new() failed\n");
      return -2;
  }
  /* Ignore and don't calculate chunk CRC's */
  spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

  /* Set memory usage limits for storing standard and unknown chunks,
     this is important when reading untrusted files! */
  size_t limit = 1024 * 1024 * 64;
  spng_set_chunk_limits(ctx, limit, limit);

  /* Set source PNG */
  spng_set_png_file(ctx, png); /* or _buffer(), _stream() */

  struct spng_ihdr ihdr;
  int ret = spng_get_ihdr(ctx, &ihdr);

  if(ret)
  {
      fprintf(stderr, "spng_get_ihdr() error: %s\n", spng_strerror(ret));
      return -2;
  }


  struct spng_plte plte = {0};
  ret = spng_get_plte(ctx, &plte);

  if(ret && ret != SPNG_ECHUNKAVAIL)
  {
      fprintf(stderr, "spng_get_plte() error: %s\n", spng_strerror(ret));
      return -2;
  }

  if(!ret) printf("palette entries: %u\n", plte.n_entries);


  size_t image_size, image_width;

  /* Output format, does not depend on source PNG format except for
     SPNG_FMT_PNG, which is the PNG's format in host-endian or
     big-endian for SPNG_FMT_RAW.
     Note that for these two formats <8-bit images are left byte-packed */
  int fmt = SPNG_FMT_PNG;

  /* With SPNG_FMT_PNG indexed color images are output as palette indices,
     pick another format to expand them. */
  if(ihdr.color_type == SPNG_COLOR_TYPE_INDEXED) fmt = SPNG_FMT_RGB8;

  ret = spng_decoded_image_size(ctx, fmt, &image_size);

  if(ret) return -2;

  image->buffer = malloc(image_size);

  if(image->buffer == NULL) return -2;

  /* Decode the image in one go */
  ret = spng_decode_image(ctx, image->buffer, image_size, SPNG_FMT_RGB8, 0);
  image->width = ihdr.width;
  image->height = ihdr.height;
  image->depth = 3;

  print_image(image);
  return 0;
}

int save_png(image_t *image, const char * outfile)
{
    int fmt;
    int ret = 0;
    spng_ctx *ctx = NULL;
    struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */

    /* Creating an encoder context requires a flag */
    ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    /* Encode to internal buffer managed by the library */
    spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

    /* Alternatively you can set an output FILE* or stream with spng_set_png_file() or spng_set_png_stream() */



    /* Set image properties, this determines the destination image format */
    ihdr.width = image->width;
    ihdr.height = image->height;
    ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR;
    ihdr.bit_depth = 8;
    /* Valid color type, bit depth combinations: https://www.w3.org/TR/2003/REC-PNG-20031110/#table111 */

    spng_set_ihdr(ctx, &ihdr);

    /* When encoding fmt is the source format */
    /* SPNG_FMT_PNG is a special value that matches the format in ihdr */
    fmt = SPNG_FMT_PNG;

    /* SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker */
    ret = spng_encode_image(ctx, image->buffer, image->width*image->height*image->depth, fmt, SPNG_ENCODE_FINALIZE);

    if(ret)
    {
        fprintf(stderr, "spng_encode_image() error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        return -2;
    }
    size_t png_size;
    void *png_buf = NULL;

    /* Get the internal buffer of the finished PNG */
    png_buf = spng_get_png_buffer(ctx, &png_size, &ret);
    if(png_buf == NULL)
    {
        fprintf(stderr, "spng_get_png_buffer() error: %s\n", spng_strerror(ret));
    }


    FILE *png;
    png = fopen(outfile, "wb");
    if(png == NULL) {
      fprintf(stderr, "Could not open write png %s\n", outfile);
      return -2;
    }

    //fwrite(png_buf, sizeof(uint8_t), png_size, png);
    int res = fwrite(png_buf, sizeof(uint8_t), png_size, png);
    if(res != png_size) {
      fprintf(stderr, "fwrite error: %d written != %ld\n", res, png_size);
    }
    fclose(png);

    /* User owns the buffer after a successful call */
    free(png_buf);
    spng_ctx_free(ctx);

    return ret;
}

int load_image(const char* file, image_t* image) {
  if(strstr(file, ".bmp") != NULL) {
    int res = load_bmp(file, image);
    if(res < 0) {
      return -2;
    }
  } else if(strstr(file, ".png") != NULL) {
    int res = load_png(file, image);
    if(res < 0) {
      return -2;
    }

  } else {
    fprintf(stderr, "input format not supported\n");
    return -2;
  }

}

int steganography_encode(const char* infile, const char* text, const char* outfile){
  image_t img;
  if(load_image(infile, &img) <0) {
    return -2;
  }
  if(steganography_encode_text(&img, text) < 0) {
    fprintf(stderr, "Could not encode text\n");
    return -2;
  }
  if(strstr(outfile, ".bmp") != NULL) {
    int res = save_bmp(&img, outfile);
    if(res < 0) {
      fprintf(stderr, "Could not save bmp\n");
      return -2;
    }
  } else if(strstr(outfile, ".png") != NULL) {
    int res = save_png(&img, outfile);
    if(res < 0) {
      fprintf(stderr, "Could not save png\n");
      return -2;
    }
  } else {
    fprintf(stderr, "output format not supported\n");
    return -2;
  }

  printf("Text encoded to %s\n", outfile);

  return 0;
}

int steganography_decode(const char* infile, char *text){ 
  image_t img;
  if(load_image(infile, &img) <0) {
    return -2;
  }
  if(steganography_decode_text(&img, text) < 0) {
    fprintf(stderr, "Could not decode text\n");
    return -2;
  }
  return 0;
}


int main(int argc, char **argv) {

  if(argc <2) {
    fprintf(stderr, "Invalid argument count\n");
    help(argv[0]);
    return -1;
  }

  if(strcmp(argv[1], "-encode") == 0) {
    if(argc != 5) {
      fprintf(stderr, "Invalid paramers for encode\n");
      help(argv[0]);
      return -1;
    }
    return steganography_encode(argv[2], argv[3], argv[4]);
  } else if(strcmp(argv[1], "-decode") == 0) {
    if(argc != 3) {
      fprintf(stderr, "Invalid paramers for decode\n");
      help(argv[0]);
      return -1;
    }
    char text[255];
    int res = steganography_decode(argv[2], text);
    if(res < 0) {
      return res;
    }
    printf("%s\n",text);
    return 0;
  } else {
    fprintf(stderr, "Invalid paramter %s\n", argv[1]);
    return -1;
  }
  return 0;
}


