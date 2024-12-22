#ifndef PB_JSON_H
#define PB_JSON_H

#include "json_macro.h"
#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Encodes a structure into a JSON string.
     * 
     * @param s The buffer to store the JSON string.
     * @param len The length of the buffer.
     * @param fields The message descriptor for the structure.
     * @param src_struct The structure to encode.
     * @return int Returns length of string on success, or a negative value on error.
     */
    int pbjson_encode(char *s, uint32_t len, const pbjson_msgdesc_t *fields, const void *src_struct);
    
    /**
     * @brief Decodes a JSON string into a Protocol Buffers structure.
     *
     * This function takes a JSON string and decodes it into the provided Protocol Buffers structure
     * based on the given message descriptor.
     *
     * @param s The JSON string to decode.
     * @param fields The message descriptor that describes the structure of the Protocol Buffers message.
     * @param src_struct A pointer to the structure where the decoded data will be stored.
     * @return An integer indicating the success or failure of the decoding process.
     *         Typically, 0 indicates success, while a negative value indicates an error.
     */
    int pbjson_decode(const char *s, const pbjson_msgdesc_t *fields, void *src_struct);

#ifdef __cplusplus
}
#endif

#endif // PB_JSON_H
