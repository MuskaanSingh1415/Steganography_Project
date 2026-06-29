#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"


/* Read and validate encode arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ptr;
 
    /* Store source image filename */
    encInfo->src_image_fname = argv[2];
 
    /* Check whether source image is .bmp */
    ptr = strrchr(encInfo->src_image_fname, '.');
 
    if (ptr == NULL || strcmp(ptr, ".bmp") != 0)
    {
        return e_failure;
    }
 
    /* Store secret filename */
    encInfo->secret_fname = argv[3];
 
    /* Store output filename */
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
 
    return e_success;
}
/*
 * Check whether the image has enough capacity
 * to store the secret data.
 */
/*
 * Check whether the image has enough capacity
 * to store the secret data.
 */
Status check_capacity(EncodeInfo *encInfo)
{
    /* Get total image capacity */
    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);

    /* Get secret file size */
    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret);

    /* Compare secret file size with image capacity */
    if (encInfo->size_secret_file <= encInfo->image_capacity)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
/*
 * Get image size
 * Input  : Source image file pointer
 * Output : Width × Height × 3
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    /* Move file pointer to width location */
    fseek(fptr_image, 18, SEEK_SET);

    /* Read image width */
    fread(&width, sizeof(int), 1, fptr_image);

    /* Read image height */
    fread(&height, sizeof(int), 1, fptr_image);

    /* Move pointer back to beginning of file */
    rewind(fptr_image);

    /* Return total image capacity */
    return (width * height * 3);
}
/*
 * Find the size of the secret file.
 */
/*
 * Get size of secret file
 */
uint get_file_size(FILE *fptr)
{
    uint size;

    /* Move to end of file */
    fseek(fptr, 0, SEEK_END);

    /* Get current position */
    size = ftell(fptr);

    /* Move back to beginning */
    rewind(fptr);

    return size;
}
/*
 * Copy first 54 bytes (BMP header)
 * from source image to destination image.
 */
Status copy_bmp_header(FILE *fptr_src_image,
                       FILE *fptr_dest_image)
{
    char header[54];

    fread(header, sizeof(char), 54, fptr_src_image);

    fwrite(header, sizeof(char), 54, fptr_dest_image);

    return e_success;
}
/*
 * Copy all remaining image bytes
 * after encoding is completed.
 */
Status copy_remaining_img_data(FILE *fptr_src,
                               FILE *fptr_dest)
{
    char ch;

    while (fread(&ch, sizeof(char), 1, fptr_src) == 1)
    {
        fwrite(&ch, sizeof(char), 1, fptr_dest);
    }

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    /* Open source BMP image */
    encInfo->fptr_src_image =
    fopen(encInfo->src_image_fname, "rb");

    if(encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr,
        "ERROR: Unable to open %s\n",
        encInfo->src_image_fname);

        return e_failure;
    }

    /* Open secret file */
    encInfo->fptr_secret =
    fopen(encInfo->secret_fname, "r");

    if(encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr,
        "ERROR: Unable to open %s\n",
        encInfo->secret_fname);

        return e_failure;
    }

    /* Open destination BMP image */
    encInfo->fptr_stego_image =
    fopen(encInfo->stego_image_fname, "wb");

    if(encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr,
        "ERROR: Unable to open %s\n",
        encInfo->stego_image_fname);

        return e_failure;
    }

    return e_success;
}


Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // Display status message
    printf("INFO : Encoding Magic String\n");

    /* Encode "#*" into the image */
    if (encode_data_to_image(magic_string,
                             strlen(magic_string),
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR : Failed to encode magic string\n");
        return e_failure;
    }

    printf("INFO : Magic String Encoded Successfully\n");

    return e_success;
}


Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    // holds 8 image bytes — one byte to carry each bit of 1 data byte
    char image_buffer[8];   

    /* encode size first, byte by byte, reusing the same image_buffer and encode_byte_tolsb */
    // size is an int = 4 bytes, encode each one

    for (int i = 0; i < sizeof(int); i++)   
    {
        // shift size right so the byte we want lands in the lowest 8 bits
        // i=0 → most significant byte of size
        // i=3 → least significant byte of size

        char size_byte = (size >> (24 - i * 8));
        
        // read next 8 bytes from source image
        // these 8 bytes will carry the 8 bits of size_byte

        fread(image_buffer, 1, 8, fptr_src_image);

         // hide size_byte inside the LSBs of image_buffer

        encode_byte_tolsb(size_byte, image_buffer);

        // write the modified 8 bytes into the stego image

        fwrite(image_buffer, 1, 8, fptr_stego_image);
    }
    // loop once for every byte in data

    for (int i = 0; i < size; i++) 
    {

        // read next 8 bytes from source image
        // these 8 bytes will carry the 8 bits of data[i]

        fread(image_buffer, 1, 8, fptr_src_image);  
 
        // hide data[i] inside the LSBs of image_buffer

        encode_byte_tolsb(data[i], image_buffer);  
  
        // write the modified 8 bytes into the stego image

        fwrite(image_buffer, 1, 8, fptr_stego_image); 
    }
    // signal that encoding completed without error

    return e_success;   
}

Status encode_byte_tolsb(char data, char *image_buffer)
{
    int i;
    int bit;

    /* Repeat for all 8 bits of one character */
    for(i = 0; i < 8; i++)
    {
        /*
         * Extract one bit from the character.
         *
         * First iteration extracts MSB.
         * Last iteration extracts LSB.
         */
        bit = (data >> (7 - i)) & 1;

        /*
         * Clear the existing LSB of the image byte
         * and insert the extracted bit.
         *
         * 0xFE = 11111110
         * '& 0xFE' clears the last bit.
         * '| bit' inserts the required bit.
         */
        image_buffer[i] = (image_buffer[i] & 0xFE) | bit;
    }

    return e_success;
}
/*
 * Function : do_encoding
 * Description : Controls the complete encoding process.
 */
Status do_encoding(EncodeInfo *encInfo)
{
    /* Open all required files */
    if (open_files(encInfo) == e_failure)
    {
        printf("ERROR : Unable to open files\n");
        return e_failure;
    }

    /* Check image capacity */
    if (check_capacity(encInfo) == e_failure)
    {
        printf("ERROR : Image capacity is not sufficient\n");
        return e_failure;
    }

    /* Copy BMP header */
    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR : Failed to copy BMP header\n");
        return e_failure;
    }

    /* Encode magic string */
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("ERROR : Failed to encode magic string\n");
        return e_failure;
    }

    /* These functions will be added later */
    /*
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
        return e_failure;

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
        return e_failure;

    if (encode_secret_file_data(encInfo) == e_failure)
        return e_failure;
    */

    /* Copy remaining image bytes */
    if (copy_remaining_img_data(encInfo->fptr_src_image,
                                encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR : Failed to copy remaining image data\n");
        return e_failure;
    }

    /* Close all opened files */
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}