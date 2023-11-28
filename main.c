#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "png.h"

int main(void)
{
    png_t png_file = png_open("images/test.png");
    printf(
        "\nCompressed Image Details:\nIdat Chunk Count: %i\nCompression: %i\nFilter: %i\nColor Type: %i\n",
        png_file.idat_chunk_vector.size,
        png_file.ihdr_chunk.compression,
        png_file.ihdr_chunk.filter,
        png_file.ihdr_chunk.type
    );
    png_idat_chunk_t schunk = png_file.idat_chunk_vector.idat_chunks[0];
    int32_t data_size = sizeof(int8_t) * (schunk.chunk_size - sizeof(schunk.compression) - sizeof(schunk.type) - sizeof(schunk.checksum));
    printf("Data size: %i\n", data_size);
    char arr[9] = "";
    for(int32_t i = 0; i < data_size; ++i) {
        bin((char *)arr, schunk.data[i] & 0xFF);
        printf("0x%02x - 0b%s\n", schunk.data[i] & 0xFF, arr);
    }
    printf("\n");
    png_image_t image = png_get_image(&png_file);
    printf("\nImage details:\nWidth: %i\nHeight: %i\nPixel Array: %s\n", image.width, image.height, image.pixels ? "NOT NULL" : "NULL");
    return 0;
}