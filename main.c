// Example of using the Image library

#include "Image.h"
#include "utils.h"



int main(void) {
    Image img_sky, img_shapes;

    Image_load(&img_sky, "./Input_images/sky.jpg");
    ON_ERROR_EXIT(img_sky.data == NULL, "Error in loading the image");
    Image_load(&img_shapes, "./Input_images/Shapes.png");
    ON_ERROR_EXIT(img_shapes.data == NULL, "Error in loading the image");

    // Convert the images to gray
    Image img_sky_gray, img_shapes_gray;
    paralel_image_to_gray(&img_sky, &img_sky_gray);
    paralel_image_to_gray(&img_shapes, &img_shapes_gray);


    // Save images
    Image_save(&img_sky_gray, "./Output_images/sky_gray.jpg");
    Image_save(&img_shapes_gray, "./Output_images/Shapes_gray.png");
    printf("Images saved successfully\n");

    // Release memory
    Image_free(&img_sky);
    Image_free(&img_sky_gray);

    Image_free(&img_shapes);
    Image_free(&img_shapes_gray);
}
