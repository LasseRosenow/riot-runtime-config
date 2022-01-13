#include "cbor_example.h"

#include <stdio.h>
#include <cbor.h>

int cbor_example_run(void)
{
    // Encode
    uint8_t buf[16];
    CborEncoder encoder;

    cbor_encoder_init(&encoder, buf, sizeof(buf), 0);
    cbor_encode_int(&encoder, INT64_MAX);


    // Decode
    CborParser parser;
    CborValue value;

    //int result;

    cbor_parser_init(buf, sizeof(buf), 0, &parser, &value);
    CborType type = cbor_value_get_type(&value);

    printf("TYPE: ");

    switch (type) {
    case CborIntegerType: {
        int64_t result;
        cbor_value_get_int64(&value, &result);
        printf("CborIntegerType: %lld\n", result);
        break;
    }
    case CborByteStringType: {
        printf("CborByteStringType\n");
        break;
    }
    case CborTextStringType: {
        printf("CborTextStringType\n");
        break;
    }
    case CborArrayType: {
        printf("CborArrayType\n");
        break;
    }
    case CborMapType: {
        printf("CborMapType\n");
        break;
    }
    case CborTagType: {
        printf("CborTagType\n");
        break;
    }
    case CborSimpleType: {
        printf("CborSimpleType\n");
        break;
    }
    case CborBooleanType: {
        printf("CborBooleanType\n");
        break;
    }
    case CborNullType: {
        printf("CborNullType\n");
        break;
    }
    case CborUndefinedType: {
        printf("CborUndefinedType\n");
        break;
    }
    case CborHalfFloatType: {
        printf("CborHalfFloatType\n");
        break;
    }
    case CborFloatType: {
        printf("CborFloatType\n");
        break;
    }
    case CborDoubleType: {
        printf("CborDoubleType\n");
        break;
    }
    case CborInvalidType: {
        printf("CborInvalidType\n");
        break;
    }
    }

    //cbor_value_get_int(&value, &result);


    // Output
    //printf("CBOR: %d\n", result);
    return 0;
}
