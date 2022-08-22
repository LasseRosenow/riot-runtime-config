#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <assert.h>
#include <base64.h>
#include <kernel_defines.h>

#include "registry.h"

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(CONFIG_REGISTRY_USE_UINT64)
#include <fmt.h>
#endif /* CONFIG_REGISTRY_USE_INT64 || CONFIG_REGISTRY_USE_UINT64 */

int registry_convert_str_to_value(const char *src, void *dest, size_t dest_len,
                                  const registry_type_t dest_type)
{
    assert(src != NULL);
#if defined(CONFIG_REGISTRY_USE_INT64)
    int64_t val_i = 0;
#else /* CONFIG_REGISTRY_USE_INT64 */
    int32_t val_i = 0;
#endif

#if defined(CONFIG_REGISTRY_USE_UINT64)
    uint64_t val_u = 0;
#else /* CONFIG_REGISTRY_USE_UINT64 */
    uint32_t val_u = 0;
#endif

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    double val_f;
#elif defined(CONFIG_REGISTRY_USE_FLOAT32)
    float val_f;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 || CONFIG_REGISTRY_USE_FLOAT32 */

    char *eptr = 0;

    if (!src) {
        goto err;
    }

    switch (dest_type) {
    case REGISTRY_TYPE_STRING:
        val_i = strlen(src);
        if (val_i + 1 > dest_len) {
            goto err;
        }
        strcpy((char *)dest, src);
        break;

    case REGISTRY_TYPE_UINT8:
    case REGISTRY_TYPE_UINT16:
    case REGISTRY_TYPE_UINT32:
        val_u = strtoul(src, &eptr, 0);
        if (*eptr != '\0') {
            goto err;
        }
        else if (dest_type == REGISTRY_TYPE_UINT8) {
            if (val_u > UINT8_MAX) {
                goto err;
            }
            *(uint8_t *)dest = val_u;
        }
        else if (dest_type == REGISTRY_TYPE_UINT16) {
            if (val_u > UINT16_MAX) {
                goto err;
            }
            *(uint16_t *)dest = val_u;
        }
        else if (dest_type == REGISTRY_TYPE_UINT32) {
            *(uint32_t *)dest = val_u;
        }
        break;

#if defined(CONFIG_REGISTRY_USE_UINT64)
    case REGISTRY_TYPE_UINT64:
        val_u = strtoull(src, &eptr, 0);
        if (*eptr != '\0') {
            goto err;
        }
        *(uint64_t *)dest = val_u;
        break;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    case REGISTRY_TYPE_INT8:
    case REGISTRY_TYPE_INT16:
    case REGISTRY_TYPE_INT32:
    case REGISTRY_TYPE_BOOL:
        val_i = strtol(src, &eptr, 0);
        if (*eptr != '\0') {
            goto err;
        }
        if (dest_type == REGISTRY_TYPE_BOOL) {
            if (val_i < 0 || val_i > 1) {
                goto err;
            }
            *(bool *)dest = val_i;
        }
        else if (dest_type == REGISTRY_TYPE_INT8) {
            if (val_i < INT8_MIN || val_i > INT8_MAX) {
                goto err;
            }
            *(int8_t *)dest = val_i;
        }
        else if (dest_type == REGISTRY_TYPE_INT16) {
            if (val_i < INT16_MIN || val_i > INT16_MAX) {
                goto err;
            }
            *(int16_t *)dest = val_i;
        }
        else if (dest_type == REGISTRY_TYPE_INT32) {
            *(int32_t *)dest = val_i;
        }
        break;

#if defined(CONFIG_REGISTRY_USE_INT64)
    case REGISTRY_TYPE_INT64:
        val_i = strtoull(src, &eptr, 0);
        if (*eptr != '\0') {
            goto err;
        }
        *(int64_t *)dest = val_i;
        break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    case REGISTRY_TYPE_FLOAT32:
        val_f = strtof(src, &eptr);
        if (*eptr != '\0') {
            goto err;
        }
        *(float *)dest = val_f;
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    case REGISTRY_TYPE_FLOAT64:
        val_f = strtod(src, &eptr);
        if (*eptr != '\0') {
            goto err;
        }
        *(double *)dest = val_f;
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
    default:
        goto err;
    }
    return 0;
err:
    return -EINVAL;
}

int registry_convert_value_to_value(const registry_value_t *src, void *dest,
                                    const size_t dest_len, const registry_type_t dest_type)
{
    char string[REGISTRY_MAX_VAL_LEN];

    char *new_string =
        registry_convert_value_to_str(src, string, ARRAY_SIZE(string));

    if (new_string == NULL) {
        return -EINVAL;
    }

    int new_value_success = registry_convert_str_to_value(string, dest, dest_len, dest_type);

    return new_value_success;
}

int registry_convert_str_to_bytes(char *val_str, void *vp, size_t *len)
{
    assert(vp != NULL);
    char buf[base64_estimate_decode_size(REGISTRY_MAX_VAL_LEN)];
    size_t _len = 0;
    size_t val_len = strlen(val_str);

    base64_decode((unsigned char *)val_str, val_len, (void *)buf, &_len);

    if (_len > sizeof(buf)) {
        return -EINVAL;
    }

    if (base64_decode((unsigned char *)val_str, val_len, (void *)buf, &_len) !=
        BASE64_SUCCESS) {
        return -EINVAL;
    }

    if (_len > *len) {
        return -EINVAL;
    }

    memcpy(vp, (void *)buf, _len);
    *len = (int)_len;

    return 0;
}

char *registry_convert_value_to_str(const registry_value_t *src, char *dest,
                                    const size_t dest_len)
{
    assert(src != NULL);

#if defined(CONFIG_REGISTRY_USE_UINT64)
    int64_t val_u64 = 0;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

#if defined(CONFIG_REGISTRY_USE_INT64)
    int64_t val_i64 = 0;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(CONFIG_REGISTRY_USE_FLOAT32) || \
    defined(CONFIG_REGISTRY_USE_FLOAT64)
    size_t len;
#endif /* CONFIG_REGISTRY_USE_INT64 || CONFIG_REGISTRY_USE_FLOAT32 || CONFIG_REGISTRY_USE_FLOAT64 */

    uint32_t val_u = 0;
    int32_t val_i = 0;

    switch (src->type) {
    case REGISTRY_TYPE_STRING: {
        char *str_val = (char *)src->buf;

        if (strlen(str_val) <= dest_len) {
            strcpy(dest, str_val);
            return dest;
        }
        else {
            return NULL;
        }
    }

    case REGISTRY_TYPE_UINT8:
    case REGISTRY_TYPE_UINT16:
    case REGISTRY_TYPE_UINT32:
        if (src->type == REGISTRY_TYPE_UINT8) {
            val_u = *(uint8_t *)src->buf;
        }
        else if (src->type == REGISTRY_TYPE_UINT16) {
            val_u = *(uint16_t *)src->buf;
        }
        else if (src->type == REGISTRY_TYPE_UINT32) {
            val_u = *(uint32_t *)src->buf;
        }
        snprintf(dest, dest_len, "%" PRIu32, val_u);
        return dest;

#if defined(CONFIG_REGISTRY_USE_UINT64)
    case REGISTRY_TYPE_UINT64:
        val_u64 = *(uint64_t *)src->buf;
        len = fmt_u64_dec(NULL, val_u64);
        if (len > dest_len - 1) {
            return NULL;
        }
        fmt_u64_dec(dest, val_u64);
        dest[len] = '\0';
        return dest;
#endif /* CONFIG_REGISTRY_USE_INT64 */

    case REGISTRY_TYPE_INT8:
    case REGISTRY_TYPE_INT16:
    case REGISTRY_TYPE_INT32:
    case REGISTRY_TYPE_BOOL:
        if (src->type == REGISTRY_TYPE_INT8) {
            val_i = *(int8_t *)src->buf;
        }
        else if (src->type == REGISTRY_TYPE_INT16) {
            val_i = *(int16_t *)src->buf;
        }
        else if (src->type == REGISTRY_TYPE_INT32) {
            val_i = *(int32_t *)src->buf;
        }
        else if (src->type == REGISTRY_TYPE_BOOL) {
            val_i = *(bool *)src->buf;
        }
        snprintf(dest, dest_len, "%" PRId32, val_i);
        return dest;

#if defined(CONFIG_REGISTRY_USE_INT64)
    case REGISTRY_TYPE_INT64:
        val_i64 = *(int64_t *)src->buf;
        len = fmt_s64_dec(NULL, val_i64);
        if (len > dest_len - 1) {
            return NULL;
        }
        fmt_s64_dec(dest, val_i64);
        dest[len] = '\0';
        return dest;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    case REGISTRY_TYPE_FLOAT32:
        sprintf(dest, "%f", *(float *)src->buf);
        len = strlen(dest);
        if (len > dest_len - 1) {
            return NULL;
        }
        return dest;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    case REGISTRY_TYPE_FLOAT64:
        sprintf(dest, "%f", *(double *)src->buf);
        len = strlen(dest);
        if (len > dest_len - 1) {
            return NULL;
        }
        return dest;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

    default:
        return NULL;
    }

    return dest;
}

char *registry_convert_bytes_to_str(void *vp, size_t vp_len, char *buf, size_t buf_len)
{
    assert(vp != NULL);
    int res;
    char temp_buf[REGISTRY_MAX_VAL_LEN];
    size_t enc_len = 0;

    base64_encode(vp, vp_len, (unsigned char *)temp_buf, &enc_len);

    if (enc_len > REGISTRY_MAX_VAL_LEN) {
        return NULL;
    }

    res = base64_encode(vp, vp_len, (unsigned char *)temp_buf, &enc_len);

    if (res != BASE64_SUCCESS || enc_len > (buf_len - 1)) {
        return NULL;
    }

    memcpy(buf, (void *)temp_buf, enc_len);
    buf[enc_len] = '\0';

    return vp;
}
