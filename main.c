#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./header/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./header/stb_image_write.h"

unsigned char *uc_arrayNew_1d(int _size)
{
    return (unsigned char *)calloc(_size, sizeof(unsigned char));
}

// convert to grey color (1 channel)
unsigned char* grey_conversion(unsigned char* image, int w, int h, int c)
{
    unsigned char* grey = uc_arrayNew_1d(w*h);
    float r, g, b;
    for(int i=0; i<h; i++)
    {
        for(int j=0; j<w; j++)
        {
            r = image[i*w*c + j*c + 0];
            g = image[i*w*c + j*c + 1];
            b = image[i*w*c + j*c + 2];
            grey[i*w+j] = (r+g+b)/3.0;
        }
    }
    return grey;
}


void cosine_similarity(unsigned char* template_grey, int w, int h, unsigned char* sample_grey, int W, int H, unsigned char* sample)
{
    // initialize
    int h_limit = 400-h;
    int w_limit = 530-w;

    float cosine[h_limit * w_limit];

    float max = cosine[0];
    for(int a=88; a<h_limit; a++)
    {
        for(int b=225; b<w_limit; b++)
        {
            float dot_product = 0.0; 
            float template_len = 0.0; 
            float sample_len = 0.0;
            for(int i=0, I=a;  i<h  ; i++, I++)
            {
                for(int j=0, J=b;  j<w  ; j++, J++)
                {
                    dot_product += template_grey[i * w + j] * sample_grey[I * W + J];
                    template_len += template_grey[i * w + j] * template_grey[i * w + j];
                    sample_len += sample_grey[I * W + J] * sample_grey[I * W + J];
                }
            }

            cosine[a*w_limit+b] = dot_product / (sqrt(template_len) * sqrt(sample_len));
            if(cosine[a*w_limit+b] > max)
                max = cosine[a*w_limit+b];
        }
    }

    
    // update template and draw
    for(int a=88; a<h_limit; a++)
    {
        for(int b=225; b<w_limit; b++)
        {
            // verify
            if(cosine[a * w_limit + b] == max)
            {
                
                for(int i=0, I=a;  i<h  ; i++, I++)
                {   
                    for(int j=0, J=b;  j<w  ; j++, J++)
                    {
                        // update the template
                        template_grey[i * w + j] = sample_grey[I * W + J];


                        // draw
                        if(I == a || I == a + h - 1 || J == b || J == b + w - 1)
                        {
                            // sample_grey[I * W + J] = 0;
                            sample[I * W * 3 + J * 3 + 0] = 0;
                            sample[I * W * 3 + J * 3 + 1] = 0;
                            sample[I * W * 3 + J * 3 + 2] = 0;
                        }

                    }
                }

            }
        }
    }
    
}


int main ()
{
    // load the template
    char path_template[] = "./template.jpg";
    int w, h, c;
    unsigned char *template = stbi_load(path_template, &w, &h, &c, 0);


    // convert to the template to grey color
    char path_template_grey[] = "./template_grey.jpg";
    unsigned char* template_grey = grey_conversion(template, w, h, c);
    stbi_write_png(path_template_grey, w, h, 1, template_grey, w);



    char path[500];
    for(int i=0; i<63; i++)
    {
        // Load 63 images
        sprintf(path, "./images/img%d.jpg", i);
        int W, H, C; 
        unsigned char *sample = stbi_load(path, &W, &H, &C, 0);

        // check
        if (template == NULL || sample == NULL)
        {
            printf("\nError in loading the image\n");
            exit(1);
        }

        // convert all 63 images to grey color and 
        sprintf(path, "./images_grey/img%d.jpg", i);
        unsigned char* sample_grey = grey_conversion(sample, W, H, C);


        // save them all to the folder "images_grey"
        stbi_write_png(path, W, H, 1, sample_grey, W);


        // the result path
        sprintf(path, "./result/result%d.jpg", i);

        // using cosine similarity to track the object and update the template
        cosine_similarity(template_grey, w, h, sample_grey, W, H, sample);


        // save the image as the updated sample (color or gray is up to you)
        stbi_write_png(path, W, H, 3, sample, W*3);


        // update the template
        sprintf(path, "./update_template/template_grey%d.jpg", i);
        stbi_write_png(path, w, h, 1, template_grey, w);
    }

    return 0;
}
