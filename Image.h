#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define NUMBER_OF_THREADS 4

enum allocation_type {
    NO_ALLOCATION, SELF_ALLOCATED, STB_ALLOCATED
};

typedef struct {
    int width;
    int height;
    int channels;
    size_t size;
    uint8_t *data;
    enum allocation_type allocation_;
} Image;

typedef struct {
    int rank;
    int start; // the start in the for loop
    int size; // the size of the for loop
    int channels;
    Image *img; 
    Image *gray;
} thread_data_t;

void Image_load(Image *img, const char *fname);
void Image_create(Image *img, int width, int height, int channels, bool zeroed);
void Image_save(const Image *img, const char *fname);
void Image_free(Image *img);
void Image_to_gray(const Image *orig, Image *gray);
thread_data_t* Generate_thread_data(int num_threads, Image *img, Image *gray);
void* Help_image_to_gray(void *thread_data);
void paralel_image_to_gray(Image *orig, Image *gray);
