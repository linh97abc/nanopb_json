#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PB_JSON_PROTO_HEADER_VERSION 40

#ifndef PB_STATIC_ASSERT
#if defined(__ICCARM__)
/* IAR has static_assert keyword but no _Static_assert */
#define PB_STATIC_ASSERT(COND, MSG) static_assert(COND, #MSG);
#elif defined(_MSC_VER) && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112)
/* MSVC in C89 mode supports static_assert() keyword anyway */
#define PB_STATIC_ASSERT(COND, MSG) static_assert(COND, #MSG);
#elif defined(PB_C99_STATIC_ASSERT)
/* Classic negative-size-array static assert mechanism */
#define PB_STATIC_ASSERT(COND, MSG) typedef char PB_STATIC_ASSERT_MSG(MSG, __LINE__, __COUNTER__)[(COND) ? 1 : -1];
#define PB_STATIC_ASSERT_MSG(MSG, LINE, COUNTER) PB_STATIC_ASSERT_MSG_(MSG, LINE, COUNTER)
#define PB_STATIC_ASSERT_MSG_(MSG, LINE, COUNTER) pb_static_assertion_##MSG##_##LINE##_##COUNTER
#elif defined(__cplusplus)
/* C++11 standard static_assert mechanism */
#define PB_STATIC_ASSERT(COND, MSG) static_assert(COND, #MSG);
#else
/* C11 standard _Static_assert mechanism */
#define PB_STATIC_ASSERT(COND, MSG) _Static_assert(COND, #MSG);
#endif
#endif

#define PBJSON_SUBMESSAGE_POINTER(msgname, type, prop) \
    _PBJSON_SUBMESSAGE_ITER_##type(msgname##_##prop##_MSGTYPE)
#define _PBJSON_SUBMESSAGE_ITER_MESSAGE(msgname) \
    _PBJSON_SUBMESSAGE_ITER2(msgname)
#define _PBJSON_SUBMESSAGE_ITER2(msgname) &msgname##_msg

#define _PBJSON_SUBMESSAGE_ITER_BOOL(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_ENUM(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_UENUM(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_FLOAT(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_DOUBLE(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_INT32(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_SINT32(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_SFIXED32(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_INT64(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_SINT64(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_SFIXED64(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_UINT32(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_FIXED32(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_UINT64(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_FIXED64(msgname) (void *)0
#define _PBJSON_SUBMESSAGE_ITER_STRING(msgname) (void *)0

#define PBJSON_GEN_OPTION_REQUIRED(struct_name, prop) 0, PBJSON_OPTION_SINGULAR
#define PBJSON_GEN_OPTION_SINGULAR(struct_name, prop) 0, PBJSON_OPTION_SINGULAR
#define PBJSON_GEN_OPTION_REPEATED(struct_name, prop) \
    (uint32_t)(uintptr_t)(&((struct_name *)0)->prop##_count), PBJSON_OPTION_REPEATED
#define PBJSON_GEN_OPTION_OPTIONAL(struct_name, prop) \
    (uint32_t)(uintptr_t)(&((struct_name *)0)->has_##prop), PBJSON_OPTION_OPTIONAL

#define PBJSON_GEN_ITEM_SIZE_REQUIRED(struct_name, prop) sizeof(((struct_name *)0)->prop)
#define PBJSON_GEN_ITEM_SIZE_SINGULAR(struct_name, prop) sizeof(((struct_name *)0)->prop)
#define PBJSON_GEN_ITEM_SIZE_REPEATED(struct_name, prop) sizeof(((struct_name *)0)->prop[0])
#define PBJSON_GEN_ITEM_SIZE_OPTIONAL(struct_name, prop) sizeof(((struct_name *)0)->prop)

#define PBJSON_GEN_ITER(struct_name, p1, option, type, prop, p3) \
    {                                                            \
        #prop,                                                   \
        PBJSON_SUBMESSAGE_POINTER(struct_name, type, prop),      \
        PBJSON_GEN_ITEM_SIZE_##option(struct_name, prop),        \
        (uint32_t)(uintptr_t)(&((struct_name *)0)->prop),        \
        PBJSON_GEN_OPTION_##option(struct_name, prop),           \
        PBJSON_##type##_TYPE,                                    \
    },

#define PBJSON_COUNT_ITER(struct_name, p1, option, type, prop, p3) +1

#define PBJSON_BIND(msgname, structname, width)                   \
    const pbjson_iter_t msgname##_field_info[] =                  \
        {msgname##_FIELDLIST(PBJSON_GEN_ITER, structname)};       \
    const pbjson_msgdesc_t msgname##_msg =                        \
        {                                                         \
            msgname##_field_info,                                 \
            0 msgname##_FIELDLIST(PBJSON_COUNT_ITER, structname), \
    };

#ifdef __cplusplus
extern "C"
{
#endif

    enum pbjson_type_enum
    {
        PBJSON_BOOL_TYPE = 0,
        PBJSON_ENUM_TYPE,
        PBJSON_UENUM_TYPE,
        PBJSON_FLOAT_TYPE,
        PBJSON_DOUBLE_TYPE,

        PBJSON_INT32_TYPE,
        PBJSON_SINT32_TYPE = PBJSON_INT32_TYPE,
        PBJSON_SFIXED32_TYPE = PBJSON_INT32_TYPE,

        PBJSON_INT64_TYPE,
        PBJSON_SINT64_TYPE = PBJSON_INT64_TYPE,
        PBJSON_SFIXED64_TYPE = PBJSON_INT64_TYPE,

        PBJSON_UINT32_TYPE,
        PBJSON_FIXED32_TYPE = PBJSON_UINT32_TYPE,

        PBJSON_UINT64_TYPE,
        PBJSON_FIXED64_TYPE = PBJSON_UINT64_TYPE,

        PBJSON_STRING_TYPE,
        PBJSON_MESSAGE_TYPE,

    };

    enum pbjson_option_enum
    {
        PBJSON_OPTION_SINGULAR,
        PBJSON_OPTION_REPEATED,
        PBJSON_OPTION_OPTIONAL,
    };

    typedef enum pbjson_type_enum pbjson_type_t;
    typedef enum pbjson_option_enum pbjson_option_t;

    typedef struct pbjson_iter_s pbjson_iter_t;
    typedef struct pbjson_msgdesc_s pbjson_msgdesc_t;

    struct pbjson_iter_s
    {
        const char *name;
        const pbjson_msgdesc_t *submsg;
        uint32_t item_size;
        uint32_t data_offset;
        uint32_t count_offset;
        pbjson_option_t option;
        pbjson_type_t data_type;
    };

    struct pbjson_msgdesc_s
    {
        const pbjson_iter_t *iter;
        uint32_t num_field;
    };

    typedef uint32_t pbjson_size_t;

    int pbjson_encode(char *s, uint32_t len, const pbjson_msgdesc_t *fields, const void *src_struct);
    int pbjson_decode(const char *s, const pbjson_msgdesc_t *fields, void *src_struct);

#ifdef __cplusplus
}
#endif
