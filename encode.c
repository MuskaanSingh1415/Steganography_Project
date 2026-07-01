#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"


/* Read and validate encode arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char ch1, ch2;

    /* Store source image filename */
    encInfo->src_image_fname = argv[2];

    /* Check if source image is provided */
    if (argv[2] == NULL)
    {
        printf("ERROR : Source image not provided\n");
        return e_failure;
    }

    /* Check if secret file is provided */
    if (argv[3] == NULL)
    {
        printf("ERROR : Secret file not provided\n");
        return e_failure;
    }

    /* Store secret file name */
    encInfo->secret_fname = argv[3];

    /* Store output file name */
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }

    /* Open all files first */
    if (open_files(encInfo) == e_failure)
    {
        printf("ERROR : Unable to open files\n");
        return e_failure;
    }

    /* Read first two bytes from already opened source image */
    fread(&ch1, ONE_BYTE, ONE_ELEMENT, encInfo->fptr_src_image);
    fread(&ch2, ONE_BYTE, ONE_ELEMENT, encInfo->fptr_src_image);

    /* BMP files always start with 'B' and 'M' */
    if (ch1 != 'B' || ch2 != 'M')
    {
        printf("ERROR : Invalid BMP file\n");
        return e_failure;
    }

    /* Move file pointer back to beginning */
    rewind(encInfo->fptr_src_image);

    return e_success;
}
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
    fread(&width, sizeof(int),ONE_ELEMENT , fptr_image);

    /* Read image height */
    fread(&height, sizeof(int), ONE_ELEMENT, fptr_image);

    /* Move pointer back to beginning of file */
    rewind(fptr_image);

    /* Return total image capacity */
    return (width * height * 3);
}

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

    while (fread(&ch, sizeof(char), ONE_ELEMENT, fptr_src) == 1)
    {
        fwrite(&ch, sizeof(char), ONE_ELEMENT, fptr_dest);
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

 /*
 * Encode secret file extension size
 * Encodes the length of the extension string (e.g. 4 for ".txt")
 * so the decoder knows how many characters to read back
 */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char size_buf[sizeof(int)];   // holds the 4 bytes of size
    int i;

    printf("INFO : Encoding Secret File Extension Size\n");

    /* Break size into 4 bytes and store in buffer */
    for (i = 0; i < sizeof(int); i++)
    {
        /* Extract one byte of size at a time */
        /* i=0 → most significant byte */
        /* i=3 → least significant byte */
        size_buf[i] = (size >> (24 - i * 8)) ;
    }

    /* Encode all 4 bytes into image using encode_data_to_image */
    if (encode_data_to_image(size_buf,
                             sizeof(int),
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR : Failed to encode extension size\n");
        return e_failure;
    }

    printf("INFO : Extension Size Encoded Successfully\n");

    return e_success;
}

/*
 * Encode secret file extension
 * Example : .txt
 */
Status encode_secret_file_extn(const char *file_extn,
                               EncodeInfo *encInfo)
{
    printf("INFO : Encoding Secret File Extension\n");

    if(encode_data_to_image(file_extn,
                            strlen(file_extn),
                            encInfo->fptr_src_image,
                            encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR : Failed to encode extension\n");
        return e_failure;
    }

    printf("INFO : Secret File Extension Encoded Successfully\n");

    return e_success;
}

/*
 * Encode data into image.
 * Each character is stored using 8 image bytes.
 */
Status encode_data_to_image(const char *data,
                            int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    /* Stores 8 bytes read from source image */
    char image_buffer[EIGHT_ELEMENT];

    /* Encode one character at a time */
    for(int i = 0; i < size; i++)
    {
        /* Read 8 image bytes from source image */
        fread(image_buffer, ONE_BYTE, EIGHT_ELEMENT, fptr_src_image);

        /*
         * Hide one character inside these 8 bytes.
         * Each bit of the character is stored in
         * the LSB of one image byte.
         */
        encode_byte_tolsb(data[i], image_buffer);

        /* Write modified bytes to stego image */
        fwrite(image_buffer, ONE_BYTE, EIGHT_ELEMENT, fptr_stego_image);
    }

    /* Encoding completed successfully */
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
 * Encode secret file size
 * File size is stored using 32 image bytes
 */
/*
 * Encode secret file size
 * File size is stored using 32 image bytes
 */
Status encode_secret_file_size(long file_size,
                               EncodeInfo *encInfo)
{
    char size_buf[sizeof(int)];
    int i;

    printf("INFO : Encoding Secret File Size\n");

    /* Break size into 4 bytes, big-endian, to match the decoder
       and to match encode_secret_file_extn_size's format */
    for (i = 0; i < sizeof(int); i++)
    {
        size_buf[i] = (char)(file_size >> (24 - i * 8));
    }

    if (encode_data_to_image(size_buf,
                             sizeof(int),
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    printf("INFO : File Size Encoded Successfully\n");

    return e_success;
}
/*
 * Encode secret file data
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    printf("INFO : Encoding Secret File Data\n");

    rewind(encInfo->fptr_secret);

    while(fread(&ch, ONE_BYTE, ONE_ELEMENT,
                encInfo->fptr_secret) == 1)
    {
        if(encode_data_to_image(&ch,
                                1,
                                encInfo->fptr_src_image,
                                encInfo->fptr_stego_image)
                                == e_failure)
        {
            return e_failure;
        }
    }

    printf("INFO : Secret File Data Encoded Successfully\n");

    return e_success;
}
/*
 * Function : do_encoding
 * Description : Controls the complete encoding process.
 */
Status do_encoding(EncodeInfo *encInfo)
{
    /* No need to call open_files here anymore */
    /* It is already called inside read_and_validate_encode_args */

    char *extn;

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
    /* Get extension from secret file */
    extn = strrchr(encInfo->secret_fname, '.');

    if (extn == NULL)
    {
        printf("ERROR : Secret file has no extension\n");
        return e_failure;
    }

    /* Store extension */
    strcpy(encInfo->extn_secret_file, extn);

    /* Encode extension size */
    if (encode_secret_file_extn_size(
            strlen(encInfo->extn_secret_file),
            encInfo) == e_failure)
    {
        printf("ERROR : Failed to encode extension size\n");
        return e_failure;
    }

    /* Encode extension */
    if (encode_secret_file_extn(
            encInfo->extn_secret_file,
            encInfo) == e_failure)
    {
        printf("ERROR : Failed to encode extension\n");
        return e_failure;
    }

    /* Encode secret file size */
    if (encode_secret_file_size(
            encInfo->size_secret_file,
            encInfo) == e_failure)
    {
        printf("ERROR : Failed to encode file size\n");
        return e_failure;
    }

    /* Encode secret file data */
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR : Failed to encode secret file data\n");
        return e_failure;
    }

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