#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "png.h"


int main(void)
{
    png_t png_file = png_open("forest.png");
    printf(
        "\nCompressed Image Details:\nIdat Chunk Count: %i\nCompression: %i\nColor Type: %i\n",
        png_file.idat_chunk_vector.size,
        png_file.ihdr_chunk.compression,
        png_file.ihdr_chunk.type
    );
    png_image_t image = png_get_image(&png_file);
    printf("\nImage details:\n%i %i", image.width, image.height);
    return 0;
}