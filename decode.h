#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include "types.h"

/* Structure to store decoding information */

typedef struct _DecodeInfo
{
    /* Stego Image */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output File */
    char output_fname[30];
    FILE *fptr_output;

    /* Secret file details */
    char extn_secret_file[10];
    long size_secret_file;
    int extn_size; 

} DecodeInfo;


/* Function Prototypes */

/* Read and validate arguments */
Status read_and_validate_decode_args(char *argv[],
                                     DecodeInfo *decInfo);

/* Open files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode process */
Status do_decoding(DecodeInfo *decInfo);

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo);

/* decode extension size*/
Status decode_secret_file_extn_size(DecodeInfo *decInfo);
/* Decode extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode one byte */
Status decode_byte_from_lsb(char *image_buffer,char *data);

/* Decode size */
/*Status decode_size_from_lsb(int *size,
                            char *image_buffer);*/

#endif