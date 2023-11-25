#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PNG_PRINT_CHUNK_SIGNATURE

#define PNG_SIGNATURE 0x89504E470D0A1A0A
#define PNG_IHDR_SIGNATURE 0x49484452
#define PNG_IDAT_SIGNATURE 0x49444154
#define PNG_IEND_SIGNATURE 0x49454E44




typedef struct png_ihdr_chunk_t {
    int32_t chunk_size;
    int32_t signature;
    int32_t width;
    int32_t height;
    int8_t bits;
    int8_t type;
    int8_t compression;
    int8_t filter;
    int8_t interlace;
    int32_t crc;
} png_ihdr_chunk_t;

typedef struct png_idat_chunk_t {
    int32_t chunk_size;
    int32_t signature;
    int8_t compression;
    int8_t type;
    int8_t *data;
    int32_t checksum;
    int32_t crc;
} png_idat_chunk_t;

#define PNG_IDAT_EMPTY (png_idat_chunk_t){0, 0, 0, 0, NULL, 0, 0}




// Vector implementation to store multiple instances of IDAT chunk

typedef struct png_vector_t {
    int32_t size;
    png_idat_chunk_t *idat_chunks;
} png_vector_t;

void png_vector_add(png_vector_t *vector, png_idat_chunk_t idat_chunk)
{
    png_idat_chunk_t *new_idat_chunks = (png_idat_chunk_t *) malloc(sizeof(png_idat_chunk_t) * (vector->size + 1));

    for(int32_t i = 0; i < vector->size; ++i) {
        new_idat_chunks[i] = vector->idat_chunks[i];
    } new_idat_chunks[vector->size] = idat_chunk;
    vector->size++;
    free(vector->idat_chunks);
    vector->idat_chunks = new_idat_chunks;
}

png_idat_chunk_t png_vector_get(png_vector_t *vector, int32_t index)
{
    if(index >= 0 && index < vector->size)
        return vector->idat_chunks[index];
    
    return PNG_IDAT_EMPTY;
}

//




typedef struct png_t {
    png_ihdr_chunk_t ihdr_chunk;
    png_vector_t idat_chunk_vector;
} png_t;

typedef struct png_image_t {
    int32_t width;
    int32_t height;
    void *pixels;
} png_image_t;

#define PNG_EMPTY (png_t){{0}, {0}}
#define PNG_IMAGE_EMPTY (png_image_t){0, 0, NULL}




int32_t swap_endian_32t(int32_t le_int)
{
    int32_t be_int = 0;
    int8_t size = sizeof(le_int);
    for(int8_t i = 0; i < size; ++i) {
        ((int8_t *) &be_int)[i] = ((int8_t *) &le_int)[size - 1 - i];
    }
    return be_int;
}

int64_t swap_endian_64t(int64_t le_int)
{
    int64_t be_int = 0;
    int8_t size = sizeof(le_int);
    for(int8_t i = 0; i < size; ++i) {
        ((int8_t *) &be_int)[i] = ((int8_t *) &le_int)[size - 1 - i];
    }
    return be_int;
}

png_t png_open(const char *filepath)
{
    FILE *file = fopen(filepath, "rb");

    png_t png_file = PNG_EMPTY;

    if(!file) {
        fprintf(stderr, "FILE_STREAM_ERROR: Couldn't open a file!\n");
        exit(-1);
    }
    
    uint64_t file_signature = 0;
    fread(&file_signature, sizeof(file_signature), 1, file);
    file_signature = swap_endian_64t(file_signature);
    if(file_signature != PNG_SIGNATURE) {
        fprintf(stderr, "PNG_SIGNATURE_ERROR: Wrong signature of file!");
        exit(-1);
    }

    int32_t chunk_size = 0;
    int32_t signature = 0;
    int8_t running = 1;
    while(signature != PNG_IEND_SIGNATURE && running) {
        fread(&chunk_size, sizeof(chunk_size), 1, file);
        fread(&signature, sizeof(signature), 1, file);
        chunk_size = swap_endian_32t(chunk_size);
        signature = swap_endian_32t(signature);
        if(chunk_size == 0) {
            fseek(file, 4, SEEK_CUR);
            continue;
        }
        switch(signature) {
            case PNG_IHDR_SIGNATURE: {

                #ifdef PNG_PRINT_CHUNK_SIGNATURE
                printf("%i IHDR\n", chunk_size);
                #endif

                png_file.ihdr_chunk.chunk_size = chunk_size;
                png_file.ihdr_chunk.signature = signature;

                fread(&png_file.ihdr_chunk.width, sizeof(png_file.ihdr_chunk.width), 1, file);
                fread(&png_file.ihdr_chunk.height, sizeof(png_file.ihdr_chunk.height), 1, file);
                fread(&png_file.ihdr_chunk.bits, sizeof(png_file.ihdr_chunk.bits), 1, file);
                fread(&png_file.ihdr_chunk.type, sizeof(png_file.ihdr_chunk.type), 1, file);
                fread(&png_file.ihdr_chunk.compression, sizeof(png_file.ihdr_chunk.compression), 1, file);
                fread(&png_file.ihdr_chunk.filter, sizeof(png_file.ihdr_chunk.filter), 1, file);
                fread(&png_file.ihdr_chunk.interlace, sizeof(png_file.ihdr_chunk.interlace), 1, file);
                fread(&png_file.ihdr_chunk.crc, sizeof(png_file.ihdr_chunk.crc), 1, file);

                png_file.ihdr_chunk.width = swap_endian_32t(png_file.ihdr_chunk.width);
                png_file.ihdr_chunk.height = swap_endian_32t(png_file.ihdr_chunk.height);
                png_file.ihdr_chunk.crc = swap_endian_32t(png_file.ihdr_chunk.crc);
                break;
            }
            case PNG_IDAT_SIGNATURE: {

                #ifdef PNG_PRINT_CHUNK_SIGNATURE
                printf("%i IDAT\n", chunk_size);
                #endif
                
                png_idat_chunk_t idat_chunk = PNG_IDAT_EMPTY;
                idat_chunk.chunk_size = chunk_size;
                idat_chunk.signature = signature;

                fread(&idat_chunk.compression, sizeof(idat_chunk.compression), 1, file);
                fread(&idat_chunk.type, sizeof(idat_chunk.type), 1, file);

                int32_t data_size = sizeof(int8_t) * (chunk_size - sizeof(idat_chunk.compression) - sizeof(idat_chunk.type) - sizeof(idat_chunk.checksum));
                idat_chunk.data = (int8_t *) malloc(data_size);
                fread(idat_chunk.data, data_size, 1, file);

                fread(&idat_chunk.checksum, sizeof(idat_chunk.checksum), 1, file);
                fread(&idat_chunk.crc, sizeof(idat_chunk.crc), 1, file);

                idat_chunk.crc = swap_endian_32t(idat_chunk.crc);

                // printf("%02x\n%02x\n", idat_chunk.compression & 0xFF, idat_chunk.type & 0xFF);

                png_vector_add(&png_file.idat_chunk_vector, idat_chunk);
                break;
            }
            default: {
                #ifdef PNG_PRINT_CHUNK_SIGNATURE
                printf(
                    "%i %c%c%c%c\n",
                    chunk_size,
                    (signature >> 24) & 0xFF,
                    (signature >> 16) & 0xFF,
                    (signature >> 8) & 0xFF,
                    (signature >> 0) & 0xFF
                );
                #endif
                fseek(file, chunk_size + 4, SEEK_CUR);
            }
        }   
    }

    return png_file;
}

png_image_t png_get_image(png_t *png)
{
    
    return (png_image_t){
        png->ihdr_chunk.width,
        png->ihdr_chunk.height,
        NULL
    };
}
