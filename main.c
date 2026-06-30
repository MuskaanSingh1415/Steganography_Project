#include <stdio.h>
#include <string.h>

#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    /* Check minimum arguments */
    if(argc < 3)
    {   
        printf("Usage:\n");
        printf("./stego -e <source.bmp> <secret.txt> [stego.bmp]\n");
        printf("./stego -d <stego.bmp> [output_file]\n");
        return 1;
    }

    /* Encoding Operation */
    if(strcmp(argv[1], "-e") == 0)
    {
        EncodeInfo encInfo;

        if(read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("ERROR : Invalid Encode Arguments\n");
            return 1;
        }

        if(do_encoding(&encInfo) == e_failure)
        {
            printf("ERROR : Encoding Failed\n");
            return 1;
        }

        printf("INFO : Encoding Successful\n");
    }

    /* Decoding Operation */
    else if(strcmp(argv[1], "-d") == 0)
    {
        DecodeInfo decInfo;

        if(read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            printf("ERROR : Invalid Decode Arguments\n");
            return 1;
        }

        if(do_decoding(&decInfo) == e_failure)
        {
            printf("ERROR : Decoding Failed\n");
            return 1;
        }

        printf("INFO : Decoding Successful\n");
    }

    /* Invalid Option */
    else
    {
        printf("ERROR : Invalid Option\n");
        printf("Use:\n");
        printf("-e for Encoding\n");
        printf("-d for Decoding\n");
        return 1;
    }

    return 0;
}