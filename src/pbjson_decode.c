#include <pb/json.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct pbjson_parser_s
{
    const char *s;
} pbjson_parser_t;

static int pbjson_decode_value(pbjson_parser_t *parser, const pbjson_iter_t *key, void *dst);
static int pbjson_decode_dict(pbjson_parser_t *parser, const pbjson_msgdesc_t *fields, void *dst, void *p_has_msg);

static int pbjson_find_first_char(pbjson_parser_t *parser)
{
    while ((*parser->s == ' ') || (*parser->s == '\n') || (*parser->s == '\t'))
    {
        parser->s++;
        if (*parser->s == '\0')
        {
            return -1;
        }
    }

    return 0;
}

static int pbjson_jumpto_first_char(pbjson_parser_t *parser, char c)
{
    int err = pbjson_find_first_char(parser);
    if (err)
    {
        return err;
    }

    if (*parser->s == c)
    {
        parser->s++;
        return 0;
    }

    return -1;
}

static int pbjson_check_brace(const char *s)
{
    char buff_brace[64];
    unsigned buff_brace_cur_i = 0;
    bool is_string_open = false;

    buff_brace[0] = ' ';

    while (*s != '\0')
    {
        if (is_string_open)
        {
            if (*s == '"')
            {
                is_string_open = false;
            }

            s++;
            continue;
        }

        switch (*s)
        {
        case '{':
            buff_brace_cur_i++;
            if (buff_brace_cur_i >= sizeof(buff_brace))
            {
                return -1;
            }
            buff_brace[buff_brace_cur_i] = '{';
            break;

        case '}':
            if (buff_brace[buff_brace_cur_i] != '{')
            {
                return -1;
            }
            else
            {
                buff_brace_cur_i--;
            }

            break;

        case '[':
            buff_brace_cur_i++;
            if (buff_brace_cur_i >= sizeof(buff_brace))
            {
                return -1;
            }
            buff_brace[buff_brace_cur_i] = '[';
            break;

        case ']':
            if (buff_brace[buff_brace_cur_i] != '[')
            {
                return -1;
            }
            else
            {
                buff_brace_cur_i--;
            }
            break;

        case '"':
            is_string_open = true;
            break;

        default:
            break;
        }

        s++;
    }

    return (buff_brace_cur_i == 0) ? 0 : -1;
}

static int pbjson_check_obj_empty(pbjson_parser_t *parser, char close_brace)
{
    const char *s = parser->s;
    while (true)
    {
        if (*s == '\0')
        {
            return -1;
        }

        if (*s == close_brace)
        {
            parser->s = s;
            parser->s++;
            return 1;
        }

        if (*s != ' ')
        {
            return 0;
        }
        s++;
    }
}

static int pbjson_decode_array(pbjson_parser_t *parser, const pbjson_iter_t *key, void *dst)
{
    int err;
    err = pbjson_jumpto_first_char(parser, '[');

    if (err)
    {
        return err;
    }

    uint32_t count = 0;
    int list_empty_stt = pbjson_check_obj_empty(parser, ']');

    if (list_empty_stt < 0)
    {
        return list_empty_stt;
    }

    if (list_empty_stt == 0)
    {

        char *data = (char *)dst + key->data_offset;

        while (true)
        {
            if (key->data_type == PBJSON_MESSAGE_TYPE)
            {
                err = pbjson_decode_dict(parser, key->submsg, data, NULL);
            }
            else
            {

                err = pbjson_decode_value(parser, key, data);
            }

            if (err)
                return err;

            err = pbjson_find_first_char(parser);

            if (err)
            {
                return err;
            }

            count++;

            if (*parser->s == ']')
            {
                parser->s++;
                break;
            }

            if (*parser->s != ',')
            {
                return -1;
            }

            parser->s++;
            data += key->item_size;
        }
    }

    char *count_offset = ((char *)dst) + key->count_offset;
    *(uint32_t *)(void *)count_offset = count;

    return 0;
}

static int pbjson_get_string(pbjson_parser_t *parser, const pbjson_iter_t *key, char *dst)
{
    if (*parser->s != '"')
    {
        return -1;
    }

    parser->s++;

    for (uint32_t i = 0; i < key->item_size; i++)
    {
        if (*parser->s == '"')
        {
            parser->s++;
            *dst = '\0';
            return 0;
        }

        if (*parser->s == '\0')
        {
            *dst = '\0';
            return -1;
        }

        *dst = *parser->s;
        dst++;
        parser->s++;
    }

    return 0;
}

static int pbjson_get_bool(pbjson_parser_t *parser, const pbjson_iter_t *key, bool *dst)
{
    bool val;
    if (!strncmp("true", parser->s, 4))
    {
        val = true;
        parser->s += 4;
    }
    else if (!strncmp("false", parser->s, 5))
    {
        val = false;
        parser->s += 5;
    }
    else
    {
        return -1;
    }

    *dst = val;

    return 0;
}

static int pbjson_get_number(pbjson_parser_t *parser, pbjson_type_t type, void *dst)
{

    char *end_ptr;

    switch (type)
    {
    case PBJSON_FLOAT_TYPE:
        *(float *)dst = strtof(parser->s, &end_ptr);
        break;

    case PBJSON_DOUBLE_TYPE:
        *(double *)dst = strtod(parser->s, &end_ptr);
        break;

    case PBJSON_INT32_TYPE:
        *(int32_t *)dst = strtol(parser->s, &end_ptr, 10);
        break;

    case PBJSON_INT64_TYPE:
        *(int64_t *)dst = strtoll(parser->s, &end_ptr, 10);
        break;

    case PBJSON_UINT32_TYPE:
        *(uint32_t *)dst = strtoul(parser->s, &end_ptr, 10);
        break;

    case PBJSON_UINT64_TYPE:
        *(uint64_t *)dst = strtoull(parser->s, &end_ptr, 10);
        break;
    default:
        end_ptr = (char *)parser->s;
        break;
    }

    if (parser->s == end_ptr)
    {
        return -1;
    }

    parser->s = end_ptr;

    return 0;
}

static int pbjson_get_enum(pbjson_parser_t *parser, const pbjson_iter_t *key, void *dst)
{
    int32_t number;
    int err = pbjson_get_number(parser, PBJSON_INT32_TYPE, &number);

    if (err)
    {
        return err;
    }

    switch (key->item_size)
    {
    case 1:
        *(int8_t *)dst = (int8_t)number;
        break;

    case 2:
        *(int16_t *)dst = (int16_t)number;
        break;

    case 4:
        *(int32_t *)dst = (int32_t)number;
        break;

    default:
        return -1;
    }
    return 0;
}

static int pbjson_get_uenum(pbjson_parser_t *parser, const pbjson_iter_t *key, void *dst)
{
    uint32_t number;
    int err = pbjson_get_number(parser, PBJSON_UINT32_TYPE, &number);

    if (err)
    {
        return err;
    }

    switch (key->item_size)
    {
    case 1:
        *(uint8_t *)dst = (uint8_t)number;
        break;

    case 2:
        *(uint16_t *)dst = (uint16_t)number;
        break;

    case 4:
        *(uint32_t *)dst = (uint32_t)number;
        break;

    default:
        return -1;
    }
    return 0;
}

static int pbjson_decode_value(pbjson_parser_t *parser, const pbjson_iter_t *key, void *dst)
{
    int err;
    err = pbjson_find_first_char(parser);

    if (err)
    {
        return err;
    }

    switch (key->data_type)
    {
    case PBJSON_STRING_TYPE:
        err = pbjson_get_string(parser, key, (char *)dst);
        break;

    case PBJSON_BOOL_TYPE:
        err = pbjson_get_bool(parser, key, (bool *)dst);
        break;

    case PBJSON_FLOAT_TYPE:
    case PBJSON_DOUBLE_TYPE:
    case PBJSON_INT32_TYPE:
    case PBJSON_INT64_TYPE:
    case PBJSON_UINT32_TYPE:
    case PBJSON_UINT64_TYPE:
        err = pbjson_get_number(parser, key->data_type, dst);
        break;

    case PBJSON_ENUM_TYPE:
        err = pbjson_get_enum(parser, key, dst);
        break;
    case PBJSON_UENUM_TYPE:
        err = pbjson_get_uenum(parser, key, dst);
        break;

    default:
        err = -1;
        break;
    }

    return err;
}

static int pbjson_check_key(pbjson_parser_t *parser, const char *key)
{
    const char *s = parser->s;

    while (true)
    {
        if (*key == '\0')
        {
            if (*s != '"')
            {
                return -1;
            }
            else
            {
                break;
            }
        }

        if (*key != *s)
            return -1;

        key++;
        s++;
    }

    s++;
    parser->s = s;
    return 0;
}

static int pbjson_discard_value(pbjson_parser_t *parser)
{
    unsigned n_brace_open = 0;
    bool is_string_open = false;
    bool is_end = false;

    while ((*parser->s != '\0') && !is_end)
    {
        if (is_string_open)
        {
            if (*parser->s == '"')
            {
                is_string_open = false;
            }

            parser->s++;
            continue;
        }

        switch (*parser->s)
        {
        case '{':
        case '[':
            n_brace_open++;
            break;

        case ']':
        case '}':
            if (n_brace_open == 0)
            {
                is_end = true;
            }
            else
            {
                n_brace_open--;
            }

            break;

        case '"':
            is_string_open = true;
            break;

        case ',':
            if (n_brace_open == 0)
            {
                is_end = true;
            }
            else
            {
                return -1;
            }

            break;

        default:
            break;
        }

        parser->s++;
    }

    return is_end ? 0 : -1;
}

static int pbjson_decode_key(pbjson_parser_t *parser, const pbjson_msgdesc_t *fields, void *dst)
{
    int err;
    err = pbjson_jumpto_first_char(parser, '"');

    if (err)
    {
        return err;
    }

    const pbjson_iter_t *piter = NULL;
    for (unsigned i = 0; i < fields->num_field; i++)
    {
        err =
            pbjson_check_key(parser, fields->iter[i].name);
        if (err == 0)
        {
            piter = &fields->iter[i];
            break;
        }
    }

    if (!piter)
    {
        while (*parser->s != '"')
        {
            if (*parser->s == '\0')
                return -1;
            parser->s++;
        }

        parser->s++;
    }

    err = pbjson_jumpto_first_char(parser, ':');

    if (err)
    {
        return err;
    }

    if (!piter)
    {
        return pbjson_discard_value(parser);
    }

    if (piter->option == PBJSON_OPTION_REPEATED)
    {
        return pbjson_decode_array(parser, piter, dst);
    }

    char *data = (char *)dst + piter->data_offset;
    if (piter->data_type == PBJSON_MESSAGE_TYPE)
    {

        return pbjson_decode_dict(parser, piter->submsg, data, (char *)dst + piter->count_offset);
    }

    if (piter->option == PBJSON_OPTION_OPTIONAL)
    {
        char *pHasMsg = (char *)dst + piter->count_offset;
        *(bool *)pHasMsg = true;
    }

    return pbjson_decode_value(parser, piter, data);
}

static int pbjson_decode_dict(pbjson_parser_t *parser, const pbjson_msgdesc_t *fields, void *dst, void *p_has_msg)
{
    int err;

    err = pbjson_jumpto_first_char(parser, '{');

    if (err)
    {
        return err;
    }

    int dict_empty_stt = pbjson_check_obj_empty(parser, '}');

    if (dict_empty_stt < 0)
    {
        return dict_empty_stt;
    }

    if (dict_empty_stt != 0)
    {
        if (p_has_msg)
        {
            *(bool *)p_has_msg = false;

            return 0;
        }

        return -1;
    }

    if (p_has_msg)
    {
        *(bool *)p_has_msg = true;
    }

    while (true)
    {
        err = pbjson_decode_key(parser, fields, dst);

        if (err)
        {
            return err;
        }

        err = pbjson_find_first_char(parser);

        if (err)
        {
            return err;
        }

        if (*parser->s == '}')
        {
            parser->s++;
            break;
        }
        else if (*parser->s != ',')
        {
            return -1;
        }

        parser->s++;
    }

    return 0;
}

int pbjson_decode(const char *s, const pbjson_msgdesc_t *fields, void *dst)
{

    pbjson_parser_t parser;
    parser.s = s;

    int err = pbjson_check_brace(s);

    if (err)
    {
        return err;
    }

    return pbjson_decode_dict(&parser, fields, dst, NULL);
}