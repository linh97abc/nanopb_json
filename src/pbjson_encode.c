/**
 * @file pbjson_encode.c
 * @brief JSON encoding functions for nanopb structures.
 *
 * This file contains functions to encode nanopb structures into JSON format.
 * It provides functions to encode various data types and structures into JSON strings.
 */

#include <pb/json.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <limits.h>

/**
 * @struct pbjson_ostream_s
 * @brief Structure representing the JSON output stream.
 *
 * @var pbjson_ostream_s::s
 * Pointer to the output buffer.
 * @var pbjson_ostream_s::max_size
 * Maximum size of the output buffer.
 * @var pbjson_ostream_s::bytes_written
 * Number of bytes written to the output buffer.
 * @var pbjson_ostream_s::is_wait_first_key
 * Flag indicating if the first key is being waited for.
 */
struct pbjson_ostream_s
{
    char *s;
    uint32_t max_size;
    uint32_t bytes_written;
    bool is_wait_first_key;
};

typedef struct pbjson_ostream_s pbjson_ostream_t;

/**
 * @brief Copies a string from source to destination.
 *
 * @param dst Destination buffer.
 * @param src Source string.
 * @return Pointer to the end of the destination string.
 */
static char *strcopy(char *dst, const char *src);

/**
 * @brief Writes a character to the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param c Character to write.
 * @return 0 on success, -1 on error.
 */
static int pbjson_ostream_put_char(pbjson_ostream_t *stream, char c);

/**
 * @brief Writes a key to the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param s Key string to write.
 * @return 0 on success, -1 on error.
 */
static int pbjson_ostream_put_key(pbjson_ostream_t *stream, const char *s);

/**
 * @brief Writes a string value to the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param s String value to write.
 * @return Number of bytes written on success, -1 on error.
 */
static int pbjson_ostream_put_string(pbjson_ostream_t *stream, const char *s);

/**
 * @brief Writes a boolean value to the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param val Boolean value to write.
 * @return Number of bytes written on success, -1 on error.
 */
static int pbjson_ostream_put_bool(pbjson_ostream_t *stream, bool val);

/**
 * @brief Writes an enum value to the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param item_size Size of the enum item.
 * @param data Pointer to the enum data.
 * @return Number of bytes written on success, -1 on error.
 */
static int pbjson_ostream_put_enum(pbjson_ostream_t *stream, uint32_t item_size, const void *data);

/**
 * @brief Writes an unsigned enum value to the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param item_size Size of the unsigned enum item.
 * @param data Pointer to the unsigned enum data.
 * @return Number of bytes written on success, -1 on error.
 */
static int pbjson_ostream_put_uenum(pbjson_ostream_t *stream, uint32_t item_size, const void *data);

/**
 * @brief Encodes a dictionary (object) into the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param fields Pointer to the message descriptor.
 * @param src_struct Pointer to the source structure.
 * @return 0 on success, -1 on error.
 */
static int pbjson_encode_dict(pbjson_ostream_t *stream, const pbjson_msgdesc_t *fields, const void *src_struct);

/**
 * @brief Encodes an array into the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param key Pointer to the key descriptor.
 * @param count Number of elements in the array.
 * @param src_struct Pointer to the source structure.
 * @return 0 on success, -1 on error.
 */
static int pbjson_encode_array(pbjson_ostream_t *stream, const pbjson_iter_t *key, uint32_t count, const void *src_struct);

/**
 * @brief Checks if a structure has a specific key.
 *
 * @param key Pointer to the key descriptor.
 * @param src_struct Pointer to the source structure.
 * @return true if the structure has the key, false otherwise.
 */
static bool pbjson_struct_has_key(const pbjson_iter_t *key, const void *src_struct);

/**
 * @brief Encodes a value into the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param key Pointer to the key descriptor.
 * @param data_offset Pointer to the data offset.
 * @return 0 on success, -1 on error.
 */
static int pbjson_encode_value(pbjson_ostream_t *stream, const pbjson_iter_t *key, const void *data_offset);

/**
 * @brief Encodes a key-value pair into the JSON output stream.
 *
 * @param stream Pointer to the JSON output stream.
 * @param key Pointer to the key descriptor.
 * @param src_struct Pointer to the source structure.
 * @return 0 on success, -1 on error.
 */
static int pbjson_encode_key(pbjson_ostream_t *stream, const pbjson_iter_t *key, const void *src_struct);


static char *strcopy(char *dst, const char *src)
{
    while (*src != '\0')
    {
        *dst = *src;
        dst++;
        src++;
    }

    return dst;
}

static int pbjson_ostream_put_char(pbjson_ostream_t *stream, char c)
{
    if (stream->bytes_written >= stream->max_size)
    {
        return -1;
    }

    *stream->s = c;
    stream->s++;
    stream->bytes_written++;
    return (stream->bytes_written >= stream->max_size) ? -1 : 0;
    
}

static int pbjson_ostream_put_key(pbjson_ostream_t *stream, const char *s)
{

    if (!stream->is_wait_first_key)
    {
        int err = pbjson_ostream_put_char(stream, ',');
        if (err)
            return err;
    }

    if (stream->bytes_written >= stream->max_size)
    {
        return -1;
    }
    

    int len = snprintf(stream->s, stream->max_size - stream->bytes_written, "\"%s\":", s);
    if (len <= 0)
        return -1;
    stream->bytes_written += len;
    stream->s += len;
    return 0;
}

static int pbjson_ostream_put_string(pbjson_ostream_t *stream, const char *s)
{

    size_t temp_len = strlen(s);
    if (temp_len > INT_MAX)
    {
        return -1; // or any other error handling mechanism
    }
    int len = (int)temp_len;

    len += 2;

    uint32_t newLen = stream->bytes_written + len;

    if (newLen >= stream->max_size)
    {
        return -1;
    }

    char *dst = stream->s;

    *dst = '"';
    dst++;

    dst = strcopy(dst, s);

    *dst = '"';

    return len;
}

static int pbjson_ostream_put_bool(pbjson_ostream_t *stream, bool val)
{
    uint32_t len = (val ? 4 : 5);
    uint32_t new_len = stream->bytes_written + len;

    if (new_len >= stream->max_size)
    {
        return -1;
    }

    strcopy(stream->s, val ? "true" : "false");

    return len;
}

static int pbjson_ostream_put_enum(pbjson_ostream_t *stream, uint32_t item_size, const void *data)
{
    int val;

    switch (item_size)
    {
#if INT_MAX > INT16_MAX
    case sizeof(int):
        val = *(const unsigned *)data;
        break;
#endif

    case sizeof(int16_t):
        val = *(const int16_t *)data;
        break;
    case sizeof(int8_t):
        val = *(const int8_t *)data;
        break;

    default:
        return -1;
    }

    return snprintf(stream->s, stream->max_size - stream->bytes_written, "%d", val);
}

static int pbjson_ostream_put_uenum(pbjson_ostream_t *stream, uint32_t item_size, const void *data)
{
    unsigned val;

    switch (item_size)
    {
#if UINT_MAX > INT16_MAX
    case sizeof(int):
        val = *(const unsigned *)data;
        break;
#endif
    case sizeof(uint16_t):
        val = *(const uint16_t *)data;
        break;
    case sizeof(uint8_t):
        val = *(const uint8_t *)data;
        break;

    default:
        return -1;
    }

    return snprintf(stream->s, stream->max_size - stream->bytes_written, "%u", val);
}

static int pbjson_encode_dict(pbjson_ostream_t *stream, const pbjson_msgdesc_t *fields, const void *src_struct)
{
    int err;
    err = pbjson_ostream_put_char(stream, '{');
    if (err)
        return err;

    stream->is_wait_first_key = true;

    unsigned i = 0;
    for (; i < fields->num_field; i++)
    {
        err = pbjson_encode_key(stream, &fields->iter[i], src_struct);
        stream->is_wait_first_key = false;
        if (err)
            return err;
    }

    err = pbjson_ostream_put_char(stream, '}');

    return err;
}

static int pbjson_encode_array(pbjson_ostream_t *stream, const pbjson_iter_t *key, uint32_t count, const void *src_struct)
{
    int err;
    err = pbjson_ostream_put_char(stream, '[');
    if (err)
        return err;

    unsigned i = 0;
    const char *pSrc = (const char *)src_struct;

    for (; i < count; i++)
    {
        if (key->data_type == PBJSON_MESSAGE_TYPE)
        {
            err = pbjson_encode_dict(stream, key->submsg, pSrc);
        }
        else
        {

            err = pbjson_encode_value(stream, key, pSrc);
        }

        if (err)
        {
            return err;
        }

        if (i != count - 1)
        {
            err = pbjson_ostream_put_char(stream, ',');
            if (err)
                return err;
        }

        pSrc += key->item_size;
    }

    err = pbjson_ostream_put_char(stream, ']');

    return err;
}

static bool pbjson_struct_has_key(const pbjson_iter_t *key, const void *src_struct)
{
    if (key->option == PBJSON_OPTION_OPTIONAL)
    {
        const bool *pHasMsg = (const bool *)(const void *)(((const char *)src_struct) + key->count_offset);
        return *pHasMsg;
    }

    return true;
}

static int pbjson_encode_value(pbjson_ostream_t *stream, const pbjson_iter_t *key, const void *data_offset)
{
    int len;

    if(stream->bytes_written >= stream->max_size)
    {
        return -1;
    }

    switch (key->data_type)
    {
    case PBJSON_STRING_TYPE:
        len = pbjson_ostream_put_string(stream, (const char *)data_offset);
        break;
    case PBJSON_BOOL_TYPE:
        len = pbjson_ostream_put_bool(stream, *(const bool *)data_offset);
        break;
    case PBJSON_INT32_TYPE:
        len = snprintf(stream->s, stream->max_size - stream->bytes_written, "%" PRId32, *(const int32_t *)data_offset);
        break;
    case PBJSON_INT64_TYPE:
        len = snprintf(stream->s, stream->max_size - stream->bytes_written, "%" PRId64, *(const int64_t *)data_offset);
        break;
    case PBJSON_UINT32_TYPE:
        len = snprintf(stream->s, stream->max_size - stream->bytes_written, "%" PRIu32, *(const uint32_t *)data_offset);
        break;
    case PBJSON_UINT64_TYPE:
        len = snprintf(stream->s, stream->max_size - stream->bytes_written, "%" PRIu64, *(const uint64_t *)data_offset);
        break;

    case PBJSON_FLOAT_TYPE:
        len = snprintf(stream->s, stream->max_size - stream->bytes_written, "%f", *(const float *)data_offset);
        break;
    case PBJSON_DOUBLE_TYPE:
        len = snprintf(stream->s, stream->max_size - stream->bytes_written, "%lf", *(const double *)data_offset);
        break;

    case PBJSON_ENUM_TYPE:
        len = pbjson_ostream_put_enum(stream, key->item_size, data_offset);
        break;

    case PBJSON_UENUM_TYPE:
        len = pbjson_ostream_put_uenum(stream, key->item_size, data_offset);
        break;

    default:
        len = -1;
        break;
    }

    if (len <= 0)
        return -1;

    stream->bytes_written += len;
    stream->s += len;
    return 0;
}

static int pbjson_encode_key(pbjson_ostream_t *stream, const pbjson_iter_t *key, const void *src_struct)
{
    if (!pbjson_struct_has_key(key, src_struct))
    {
        return 0;
    }

    int err;
    err = pbjson_ostream_put_key(stream, key->name);
    if (err)
        return err;

    err = -1;

    const void *data_offset = (const void *)(((const char *)src_struct) + key->data_offset);

    if (key->option == PBJSON_OPTION_REPEATED)
    {
        const uint32_t *pdata_cout = (const uint32_t *)(const void *)(((const char *)src_struct) + key->count_offset);
        err = pbjson_encode_array(stream, key, *pdata_cout, data_offset);
    }
    else if (key->data_type == PBJSON_MESSAGE_TYPE)
    {
        err = pbjson_encode_dict(stream, key->submsg, data_offset);
    }

    else
    {
        err = pbjson_encode_value(stream, key, data_offset);
    }

    return err;
}

int pbjson_encode(char *s, uint32_t len, const pbjson_msgdesc_t *fields, const void *src_struct)
{
    if (len < 3)
    {
        return -1;
    }
    
    pbjson_ostream_t stream;
    stream.s = s;
    stream.max_size = len - 1;
    stream.bytes_written = 0;

    int err = pbjson_encode_dict(&stream, fields, src_struct);
    *stream.s = '\0';

    if (err)
    {
        return err;
    }

    return stream.bytes_written;
}
