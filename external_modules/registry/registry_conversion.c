#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <assert.h>
#include <base64.h>

#include "registry.h"

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(CONFIG_REGISTRY_USE_UINT64)
#include <fmt.h>
#endif /* CONFIG_REGISTRY_USE_INT64 || CONFIG_REGISTRY_USE_UINT64 */

int registry_value_from_str(char *val_str, registry_type_t type, void *vp,
                            int maxlen)
{
    assert(vp != NULL);
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

    if (!val_str) {
        goto err;
    }

    switch (type) {
        case REGISTRY_TYPE_STRING:
            val_i = strlen(val_str);
            if (val_i + 1 > maxlen) {
                goto err;
            }
            strcpy((char *)vp, val_str);
            break;

        case REGISTRY_TYPE_UINT8:
        case REGISTRY_TYPE_UINT16:
        case REGISTRY_TYPE_UINT32:
            val_u = strtoul(val_str, &eptr, 0);
            if (*eptr != '\0') {
                goto err;
            }
            else if (type == REGISTRY_TYPE_UINT8) {
                if (val_u > UINT8_MAX) {
                    goto err;
                }
                *(uint8_t *)vp = val_u;
            }
            else if (type == REGISTRY_TYPE_UINT16) {
                if (val_u > UINT16_MAX) {
                    goto err;
                }
                *(uint16_t *)vp = val_u;
            }
            else if (type == REGISTRY_TYPE_UINT32) {
                *(uint32_t *)vp = val_u;
            }
            break;

#if defined(CONFIG_REGISTRY_USE_UINT64)
        case REGISTRY_TYPE_UINT64:
            val_u = strtoull(val_str, &eptr, 0);
            if (*eptr != '\0') {
                goto err;
            }
            *(uint64_t *)vp = val_u;
            break;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

        case REGISTRY_TYPE_INT8:
        case REGISTRY_TYPE_INT16:
        case REGISTRY_TYPE_INT32:
        case REGISTRY_TYPE_BOOL:
            val_i = strtol(val_str, &eptr, 0);
            if (*eptr != '\0') {
                goto err;
            }
            if (type == REGISTRY_TYPE_BOOL) {
                if (val_i < 0 || val_i > 1) {
                    goto err;
                }
                *(bool *)vp = val_i;
            }
            else if (type == REGISTRY_TYPE_INT8) {
                if (val_i < INT8_MIN || val_i > INT8_MAX) {
                    goto err;
                }
                *(int8_t *)vp = val_i;
            }
            else if (type == REGISTRY_TYPE_INT16) {
                if (val_i < INT16_MIN || val_i > INT16_MAX) {
                    goto err;
                }
                *(int16_t *)vp = val_i;
            }
            else if (type == REGISTRY_TYPE_INT32) {
                *(int32_t *)vp = val_i;
            }
            break;

#if defined(CONFIG_REGISTRY_USE_INT64)
        case REGISTRY_TYPE_INT64:
            val_i = strtoull(val_str, &eptr, 0);
            if (*eptr != '\0') {
                goto err;
            }
            *(int64_t *)vp = val_i;
            break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
        case REGISTRY_TYPE_FLOAT32:
            val_f = strtof(val_str, &eptr);
            if (*eptr != '\0') {
                goto err;
            }
            *(float *)vp = val_f;
            break;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
        case REGISTRY_TYPE_FLOAT64:
            val_f = strtod(val_str, &eptr);
            if (*eptr != '\0') {
                goto err;
            }
            *(double *)vp = val_f;
            break;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
        default:
            goto err;
    }
    return 0;
    err:
    return -EINVAL;
}

int registry_bytes_from_str(char *val_str, void *vp, int *len)
{
    assert(vp != NULL);
    char buf[base64_estimate_decode_size(REGISTRY_MAX_VAL_LEN)];
    size_t _len = 0;
    int val_len = strlen(val_str);

    base64_decode((unsigned char *)val_str, val_len, (void *)buf, &_len);

    if (_len > sizeof(buf)) {
        return -EINVAL;
    }

    if (base64_decode((unsigned char *)val_str, val_len, (void *)buf, &_len) !=
        BASE64_SUCCESS) {
        return -EINVAL;
    }

    if ((int)_len > *len) {
        return -EINVAL;
    }

    memcpy(vp, (void *)buf, _len);
    *len = (int)_len;

    return 0;
}

char *registry_str_from_value(registry_type_t type, void *vp, char *buf,
                              int buf_len)
{
    assert(vp != NULL);

#if defined(CONFIG_REGISTRY_USE_UINT64)
    int64_t val_u64 = 0;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

#if defined(CONFIG_REGISTRY_USE_INT64)
    int64_t val_i64 = 0;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(CONFIG_REGISTRY_USE_FLOAT64)
    int len;
#endif /* CONFIG_REGISTRY_USE_INT64 || CONFIG_REGISTRY_USE_FLOAT32 || CONFIG_REGISTRY_USE_FLOAT64 */

    uint32_t val_u = 0;
    int32_t val_i = 0;

    switch(type) {
        case REGISTRY_TYPE_STRING: {
            char* str_val = (char *)vp;
        
            if (strlen(str_val) <= (size_t)buf_len) {
                strcpy(buf, str_val);
                return buf;
            }
            else {
                return NULL;
            }
        }

        case REGISTRY_TYPE_UINT8:
        case REGISTRY_TYPE_UINT16:
        case REGISTRY_TYPE_UINT32:
            if (type == REGISTRY_TYPE_UINT8) {
                val_u = *(uint8_t *)vp;
            }
            else if (type == REGISTRY_TYPE_UINT16) {
                val_u = *(uint16_t *)vp;
            }
            else if (type == REGISTRY_TYPE_UINT32) {
                val_u = *(uint32_t *)vp;
            }
            snprintf(buf, buf_len, "%" PRIu32, val_u);
            return buf;
            
#if defined(CONFIG_REGISTRY_USE_UINT64)
        case REGISTRY_TYPE_UINT64:
            val_u64 = *(uint64_t *)vp;
            len = fmt_u64_dec(NULL, val_u64);
            if (len > buf_len - 1) {
                return NULL;
            }
            fmt_u64_dec(buf, val_u64);
            buf[len] = '\0';
            return buf;
#endif /* CONFIG_REGISTRY_USE_INT64 */

        case REGISTRY_TYPE_INT8:
        case REGISTRY_TYPE_INT16:
        case REGISTRY_TYPE_INT32:
        case REGISTRY_TYPE_BOOL:
            if (type == REGISTRY_TYPE_INT8) {
                val_i = *(int8_t *)vp;
            }
            else if (type == REGISTRY_TYPE_INT16) {
                val_i = *(int16_t *)vp;
            }
            else if (type == REGISTRY_TYPE_INT32) {
                val_i = *(int32_t *)vp;
            }
            else if (type == REGISTRY_TYPE_BOOL) {
                val_i = *(bool *)vp;
            }
            snprintf(buf, buf_len, "%" PRId32, val_i);
            return buf;

#if defined(CONFIG_REGISTRY_USE_INT64)
        case REGISTRY_TYPE_INT64:
            val_i64 = *(int64_t *)vp;
            len = fmt_s64_dec(NULL, val_i64);
            if (len > buf_len - 1) {
                return NULL;
            }
            fmt_s64_dec(buf, val_i64);
            buf[len] = '\0';
            return buf;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
        case REGISTRY_TYPE_FLOAT32:
            sprintf(buf, "%f", *(float *)vp);
            len = strlen(buf);
            if (len > buf_len - 1) {
                return NULL;
            }
            return buf;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
        case REGISTRY_TYPE_FLOAT64:
            sprintf(buf, "%f", *(double *)vp);
            len = strlen(buf);
            if (len > buf_len - 1) {
                return NULL;
            }
            return buf;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

        default:
            return NULL;
    }

    return buf;
}

char *registry_str_from_bytes(void *vp, int vp_len, char *buf, int buf_len)
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

    if (res != BASE64_SUCCESS || (int)enc_len > (buf_len - 1)) {
        return NULL;
    }

    memcpy(buf, (void *)temp_buf, enc_len);
    buf[enc_len] = '\0';

    return vp;
}