#include <stdio.h>
#include "encode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;

    if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
    {
        printf("ERROR : Invalid arguments\n");
        return 1;
    }

    if (do_encoding(&encInfo) == e_failure)
    {
        printf("ERROR : Encoding Failed\n");
        return 1;
    }

    printf("INFO : Encoding Successful\n");

    return 0;
}