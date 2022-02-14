#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOADBMP_IMPLEMENTATION
#include "loadbmp.h"

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
  fprintf(stderr,"clearing %p %d",p,sz);
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

static int encode(void) {

  // -----------------
  // OPEN ORIGINAL IMAGE
  unsigned char *im_data = NULL;
  unsigned int im_width = 0;
  unsigned int im_height = 0;

  unsigned int rv = 
    loadbmp_decode_file("./otter.bmp", &im_data, &im_width, &im_height, LOADBMP_RGB);

  if(rv) {
    fprintf(stderr, "unable to open input BMP file (rv=%d)", rv);
    return 1;
  }

  printf("[+] loaded BMP image is %dx%d\n", im_width, im_height);

  // -----------------
  // OPEN CLEAR TEXT
  const char * clear_text = "les loutres ca poutre OUI";

  // -----------------
  // ENCODE IMAGE DATA WITH CLEAR TEXT

  // estimate pixel count to fit clear text
  // (one bit per pixel color, 3 color by pixel)
  const int pcount = strlen(clear_text) * 8;
  if(pcount > im_width*im_height*3) {
    fprintf(stderr, "unable to fit clear text in image\n");
    return 2;
  }

  bit_depacker_t bd;
  bit_depacker_init(&bd, clear_text);

  // let's iterate over image pixels
  for(int y=0; y<im_height; y++)
  for(int x=0; x<im_width; x++) {

    // compute pixel index
    int idx = x + y*im_width;

    unsigned char bit = 0;
    for(int color_index=0; color_index<3; color_index++) {
      // get pixel color 
      unsigned char cvalue = im_data[3*idx + color_index];
      // depack one bit from clear text
      bit_depacker_next(&bd,&bit);
      // update pixel color with LSB set to clear text bit value
      im_data[3*idx + color_index] = (cvalue & 0xfe)   | (bit ? 0x01 : 0);
    }

  }

  // -----------------
  // SAVE ENCODED IMAGE
  rv = 
    loadbmp_encode_file("./encoded_otter.bmp", im_data, im_width, im_height, LOADBMP_RGB);

  if(rv) {
    fprintf(stderr, "unable to write to output BMP file (rv=%d)", rv);
    return 1;
  }

  printf("[+] output BMP image written\n");

  return 0;
}

static int decode(void) {

  // -----------------
  // OPEN ENCODED IMAGE
  unsigned char *im_data = NULL;
  unsigned int im_width = 0;
  unsigned int im_height = 0;

  unsigned int rv = 
    loadbmp_decode_file("./encoded_otter.bmp", &im_data, &im_width, &im_height, LOADBMP_RGB);

  if(rv) {
    fprintf(stderr, "unable to open input BMP file (rv=%d)", rv);
    return 1;
  }

  printf("[+] loaded BMP image is %dx%d\n", im_width, im_height);

  // -----------------
  // DECODE CLEAR TEXT FROM IMAGE DATA

  // allocate a clear text string big enough to fit maximum image decoded size
  const int clear_text_sz = (3*im_width * im_height)/8 + 1;
  char *decoded_clear_text = malloc(clear_text_sz);

  bit_packer_t bp;
  bit_packer_init(&bp, decoded_clear_text, sizeof(decoded_clear_text));

  // let's iterate over image pixels
  for(int y=0; y<im_height; y++)
  for(int x=0; x<im_width; x++) {

    // compute pixel index
    int idx = x + y*im_width;

    unsigned char bit = 0;
    for(int color_index=0; color_index<3; color_index++) {
      // get pixel color 
      unsigned char cvalue = im_data[3*idx + color_index];
      // push pixel color LSB to clear text
      bit_packer_push(&bp, cvalue & 0x01);
    }

  }

  // -----------------
  // SHOW CLEAR TEXT
  printf("[+] decoded clear text is:\n");
  printf("%s\n", decoded_clear_text);

  return 0;
}



int main(int argc, char **argv) {

  if(argc <= 1) {
    printf("Nor args\n");
    return 1;
  }

  if(!strcmp(argv[1], "-encode")) {
    printf("Encode\n");
    encode();
  } else if(!strcmp(argv[1], "-decode")) {
    printf("Decode\n");
    decode();
  } else {
    printf("Unkown args\n");
    return 1;
  }

  return 0;
}


