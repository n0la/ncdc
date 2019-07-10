#include <dc/gateway.h>
#include "internal.h"

static uint8_t *
websocket_mask(uint8_t key[4], uint8_t const *pload, size_t psize)
{
    size_t i = 0;
    uint8_t *ret = calloc(psize, sizeof(char));

    for (i = 0; i < psize; i++) {
        ret[i] = pload[i] ^ key[i % 4];
    }

    return ret;
}

uint8_t *
dc_gateway_makeframe(uint8_t const *d, size_t data_len,
                     uint8_t type, size_t *outlen)
{
    uint8_t *data = NULL;
    uint8_t *full_data;
    uint32_t len_size = 1;
    uint8_t mkey[4] = { 0x12, 0x34, 0x56, 0x78 };

    data = websocket_mask(mkey, d, data_len);

    if (data_len > 125) {
        if (data_len <= UINT16_MAX) {
            len_size += 2;
        } else {
            len_size += 8;
        }
    }

    full_data = calloc(1 + data_len + len_size + 4, sizeof(uint8_t));

    if (type == 0) {
        type = 129;
    }

    full_data[0] = type;

    if (data_len <= 125) {
        full_data[1] = data_len | 0x80;
    } else if (data_len <= G_MAXUINT16) {
        uint16_t be_len = GUINT16_TO_BE(data_len);
        full_data[1] = 126 | 0x80;
        memmove(full_data + 2, &be_len, 2);
    } else {
        guint64 be_len = GUINT64_TO_BE(data_len);
        full_data[1] = 127 | 0x80;
        memmove(full_data + 2, &be_len, 8);
    }

    memmove(full_data + (1 + len_size), &mkey, 4);
    memmove(full_data + (1 + len_size + 4), data, data_len);

    *outlen = 1 + data_len + len_size + 4;
    free(data);

    return full_data;
}

size_t
dc_gateway_parseframe(uint8_t const *data, size_t datalen,
                      uint8_t *type, uint8_t **outdata, size_t *outlen)
{
    size_t ret = 0;
    uint8_t t = 0, l = 0;
    size_t idx = 0, data_len = 0;
    uint8_t *buf = NULL;

    goto_if_true(data == NULL || datalen == 0, cleanup);

    goto_if_true(datalen <= idx, cleanup);
    t = data[idx];

    switch (t) {
    case GATEWAY_FRAME_TEXT_DATA:
    case GATEWAY_FRAME_DISCONNECT:
    case GATEWAY_FRAME_PING:
    case GATEWAY_FRAME_PONG:
        break;

    default: return false;
    }

    ++idx;
    goto_if_true(datalen <= idx, cleanup);
    l = data[idx];

    if (l <= 125) {
        data_len = l;
        ++idx;
    } else if (l == 126) {
        /* read an uint16_t from the data
         */
        uint16_t len = 0;

        ++idx;
        goto_if_true(datalen <= idx, cleanup);
        memcpy(&len, data+idx, sizeof(len));

        data_len = GUINT16_TO_BE(len);
        idx += sizeof(len);
    } else if (l == 127) {
        /* read an uint16_t from the data
         */
        uint64_t len = 0;

        ++idx;
        goto_if_true(datalen <= idx, cleanup);
        memcpy(&len, data+idx, sizeof(len));

        data_len = GUINT64_TO_BE(len);
        idx += sizeof(len);
    }

    goto_if_true(datalen < (idx+data_len), cleanup);
    buf = calloc(data_len, sizeof(uint8_t));
    goto_if_true(buf == NULL, cleanup);

    memcpy(buf, data+idx, data_len);

    if (type != NULL) {
        *type = t;
    }

    if (outdata != NULL) {
        *outdata = buf;
        *outlen = data_len;
        buf = NULL;
    }

    ret = idx + data_len;

cleanup:

    free(buf);

    return ret;
}
