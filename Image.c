#include "Image.h"
#include "utils.h"
#include <math.h>
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"


void Image_load(Image *img, const char *fname) {
    if((img->data = stbi_load(fname, &img->width, &img->height, &img->channels, 0)) != NULL) {
        img->size = img->width * img->height * img->channels;
        img->allocation_ = STB_ALLOCATED;
    }
}

void Image_create(Image *img, int width, int height, int channels, bool zeroed) {
    size_t size = width * height * channels;
    if(zeroed) {
        img->data = calloc(size, 1);
    } else {
        img->data = malloc(size);
    }

    if(img->data != NULL) {
        img->width = width;
        img->height = height;
        img->size = size;
        img->channels = channels;
        img->allocation_ = SELF_ALLOCATED;
    }
}

void Image_save(const Image *img, const char *fname) {
    // Check if the file name ends in one of the .jpg/.JPG/.jpeg/.JPEG or .png/.PNG
    if(str_ends_in(fname, ".jpg") || str_ends_in(fname, ".JPG") || str_ends_in(fname, ".jpeg") || str_ends_in(fname, ".JPEG")) {
        stbi_write_jpg(fname, img->width, img->height, img->channels, img->data, 100);
    } else if(str_ends_in(fname, ".png") || str_ends_in(fname, ".PNG")) {
        stbi_write_png(fname, img->width, img->height, img->channels, img->data, img->width * img->channels);
    } else {
        ON_ERROR_EXIT(false, "");
    }
}

void Image_free(Image *img) {
    if(img->allocation_ != NO_ALLOCATION && img->data != NULL) {
        if(img->allocation_ == STB_ALLOCATED) {
            stbi_image_free(img->data);
        } else {
            free(img->data);
        }
        img->data = NULL;
        img->width = 0;
        img->height = 0;
        img->size = 0;
        img->allocation_ = NO_ALLOCATION;
    }
}

void Image_to_gray(const Image *orig, Image *gray) {
    ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(gray, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(gray->data == NULL, "Error in creating the image");

    //loop through the image and convert each pixel to grayscale
    for(int i = 0; i < orig->size; i += orig->channels) {
        //calculate the average of the RGB values
        int avg = (orig->data[i] + orig->data[i + 1] + orig->data[i + 2]) / 3;
        //set the pixel to the average
        gray->data[i / orig->channels] = avg;
    }

}

thread_data_t* Generate_thread_data(int num_threads, Image *img, Image *gray) {
    thread_data_t *thread_data = malloc(num_threads * sizeof(thread_data_t));
    ON_ERROR_EXIT(thread_data == NULL, "Error in creating the thread data");

    int size = img->size / num_threads;
    int start = 0;
    for(int i = 0; i < num_threads; i++) {
        thread_data[i].rank = i;
        thread_data[i].start = start;
        thread_data[i].size = size;
        thread_data[i].img = img;
        thread_data[i].gray = gray;
        thread_data[i].channels = img->channels;
        start += size;
    }

    

    return thread_data;
} 

void* Help_image_to_gray(void *thread_data) {

    thread_data = (thread_data_t *)thread_data;

    int start = ((thread_data_t *)thread_data)->start;
    int size = ((thread_data_t *)thread_data)->size;
    int channels = ((thread_data_t *)thread_data)->channels;

    Image *img = ((thread_data_t *)thread_data)->img;
    uint8_t *data = img->data;

    Image *gray = ((thread_data_t *)thread_data)->gray;
    uint8_t *gray_data = gray->data;

    int i;
    for(i = start; i < start + size; i += channels) {
        int avg = (data[i] + data[i + 1] + data[i + 2]) / 3;
        gray_data[i / channels] = avg;
    }

    return NULL;
}

void paralel_image_to_gray(Image *orig, Image *gray) {
    ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(gray, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(gray->data == NULL, "Error in creating the image");
    
    //create the thread data
    thread_data_t *thread_data = Generate_thread_data(NUMBER_OF_THREADS, orig, gray);

    //create the threads
    int err;
    pthread_t threads[NUMBER_OF_THREADS];

    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        err = pthread_create(&threads[i], NULL, Help_image_to_gray, &thread_data[i]);

        if(err != 0) {
            ON_ERROR_EXIT(true, "Error in creating the thread");
        }
    }

    //wait for the threads to finish
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        err = pthread_join(threads[i], NULL);
    }


    
    //free the thread data
    free(thread_data);
}


