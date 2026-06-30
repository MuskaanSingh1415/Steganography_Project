#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"


Status read_and_validate_decode_args(char *argv[],
                                     DecodeInfo *decInfo)
{
    if(argv[2] == NULL)
    {
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    if(argv[3] != NULL)
    {
        strcpy(decInfo->output_fname,
               argv[3]);
    }
    else
    {
        strcpy(decInfo->output_fname,
               "output");
    }

    return e_success;
}
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname,
              "rb");

    if(decInfo->fptr_stego_image == NULL)
    {
        printf("ERROR : Unable to open stego image\n");
        return e_failure;
    }

    return e_success;
}


/* Decode a single byte by reading the LSB of each of the 8 image bytes
   and rebuilding the original byte bit by bit */

Status decode_byte_from_lsb(char *image_buffer, char *data)
{
    int i;
    char result = 0;   // will hold the rebuilt byte, starts empty

    for (i = 0; i < 8; i++)   // loop through all 8 image bytes
    {
        result = result << 1;
        // shift result left by 1 bit
        // makes room at the bottom for the next bit to be inserted

        result = result | (image_buffer[i] & 1);
        // image_buffer[i] & 1 → extracts the LSB of this image byte
        // OR it into result → places that bit into the now-empty rightmost position
        // i=0 rebuilds the MSB first, i=7 rebuilds the LSB last
    }

    *data = result;
    // write the fully rebuilt byte into the address passed by the caller
    // this is how the decoded value gets sent back out

    return e_success;   // signal that this byte was decoded successfully
}


Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;
    int i;

    printf("INFO : Decoding Magic String\n");

    for(i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(image_buffer,
              1,
              8,
              decInfo->fptr_stego_image);

        decode_byte_from_lsb(image_buffer,
                             &ch);

        if(ch != MAGIC_STRING[i])
        {
            return e_failure;
        }
    }

    printf("INFO : Magic String Decoded Successfully\n");

    return e_success;
}

/*
 * Decode secret file extension size
 */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32];
    int size = 0;
    int i;

    printf("INFO : Decoding Secret File Extension Size\n");

    /* Read 32 image bytes */
    fread(image_buffer,
          1,
          32,
          decInfo->fptr_stego_image);

    /* Rebuild the original 32-bit integer */
    for (i = 0; i < 32; i++)
    {
        size = (size << 1) | (image_buffer[i] & 1);
    }

    decInfo->extn_size = size;

    printf("INFO : Extension Size Decoded = %d\n",
           decInfo->extn_size);

    return e_success;
}

/*
 * Decode secret file extension
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;
    int i;

    printf("INFO : Decoding Secret File Extension\n");

    /* Decode extn_size characters instead of hardcoded 4 */
    for (i = 0; i < decInfo->extn_size; i++)
    {
        fread(image_buffer,
              1,
              8,
              decInfo->fptr_stego_image);

        decode_byte_from_lsb(image_buffer,
                             &ch);

        decInfo->extn_secret_file[i] = ch;
    }

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    printf("INFO : Extension Decoded = %s\n",
           decInfo->extn_secret_file);

    return e_success;
}

/*
 * Decode secret file size
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char image_buffer[32];
    int size = 0;
    int i;

    printf("INFO : Decoding Secret File Size\n");

    /* Read 32 image bytes */
    fread(image_buffer,
          1,
          32,
          decInfo->fptr_stego_image);

    /* Rebuild the original 32-bit integer */
    for(i = 0; i < 32; i++)
    {
        size = (size << 1) |
               (image_buffer[i] & 1);
    }

    decInfo->size_secret_file = size;

    printf("INFO : Secret File Size = %ld\n",
           decInfo->size_secret_file);

    return e_success;
}
/*
 * Decode secret file data
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;
    long i;

    printf("INFO : Decoding Secret File Data\n");

    /* Decode all bytes of secret file */    
    for(i = 0;
        i < decInfo->size_secret_file;
        i++)
    {
        /* Read 8 image bytes */
        fread(image_buffer,
              1,
              8,
              decInfo->fptr_stego_image);

        /* Decode one character */
        decode_byte_from_lsb(image_buffer,
                             &ch);

        /* Write character into output file */
        fwrite(&ch,
               1,
               1,
               decInfo->fptr_output);
    }

    printf("INFO : Secret File Data Decoded Successfully\n");

    return e_success;
}
/*
 * Perform complete decoding
 */
Status do_decoding(DecodeInfo *decInfo)
{
    char filename[40];

    /* Open stego image */
    if(open_decode_files(decInfo) == e_failure)
    {
        printf("ERROR : Unable to open stego image\n");
        return e_failure;
    }

    /* Skip BMP header */
    fseek(decInfo->fptr_stego_image,
          54,
          SEEK_SET);

    /* Decode magic string */
    if(decode_magic_string(decInfo) == e_failure)
    {
        printf("ERROR : Invalid Stego Image\n");
        return e_failure;
    }

    /* Decode extension */
    if(decode_secret_file_extn(decInfo) == e_failure)
    {
        return e_failure;
    }

    /* Create output filename */
    strcpy(filename,
           decInfo->output_fname);

    strcat(filename,
           decInfo->extn_secret_file);

    /* Open output file */
    decInfo->fptr_output =
        fopen(filename,
              "wb");

    if(decInfo->fptr_output == NULL)
    {
        return e_failure;
    }

    /* Decode file size */
    if(decode_secret_file_size(decInfo) == e_failure)
    {
        return e_failure;
    }

    /* Decode file data */
    if(decode_secret_file_data(decInfo) == e_failure)
    {
        return e_failure;
    }

    printf("INFO : Decoding Successful\n");

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return e_success;
}