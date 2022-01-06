#include "cbor_example.h"

#include <stdio.h>
#include <cbor.h>

int cbor_example_run(void)
{
    uint8_t buf[16];
    CborEncoder encoder;

    cbor_encoder_init(&encoder, buf, sizeof(buf), 0);
    cbor_encode_int(&encoder, 20);


    CborParser parser;
    CborValue value;
    int result;

    cbor_parser_init(buf, sizeof(buf), 0, &parser, &value);
    cbor_value_get_int(&value, &result);


    printf("CBOR: %d\n", result);


    return 0;
}
