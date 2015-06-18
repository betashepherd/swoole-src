/*
 +----------------------------------------------------------------------+
 | Swoole                                                               |
 +----------------------------------------------------------------------+
 | Copyright (c) 2012-2015 The Swoole Group                             |
 +----------------------------------------------------------------------+
 | This source file is subject to version 2.0 of the Apache license,    |
 | that is bundled with this package in the file LICENSE, and is        |
 | available through the world-wide-web at the following url:           |
 | http://www.apache.org/licenses/LICENSE-2.0.html                      |
 | If you did not receive a copy of the Apache2.0 license and are unable|
 | to obtain it through the world-wide-web, please send a note to       |
 | license@swoole.com so we can mail you a copy immediately.            |
 +----------------------------------------------------------------------+
 | Author: Tianfeng Han  <mikan.tenny@gmail.com>                        |
 +----------------------------------------------------------------------+
 */

#include "swoole.h"
#include "Connection.h"

/**
 * return the package total length
 */
int swProtocol_get_package_length(swProtocol *protocol, swConnection *conn, char *data, uint32_t size)
{
    uint16_t length_offset = protocol->package_length_offset;
    uint32_t body_length;
    /**
     * no have length field, wait more data
     */
    if (size < length_offset + protocol->package_length_size)
    {
        return 0;
    }
    body_length = swoole_unpack(protocol->package_length_type, data + length_offset);
    //Length error
    //Protocol length is not legitimate, out of bounds or exceed the allocated length
    if (body_length < 0 || body_length > protocol->package_max_length)
    {
        swWarn("invalid package, remote_addr=%s:%d, length=%d, size=%d.", swConnection_get_ip(conn), swConnection_get_port(conn), body_length, size);
        return SW_ERR;
    }
    //total package length
    return protocol->package_body_offset + body_length;
}

int swProtocol_split_package_by_eof(swProtocol *protocol, void *object, swString *buffer)
{
#if 0
    static count;
    count ++;
#endif

    char stack_buf[SW_BUFFER_SIZE_BIG];
    int eof_pos;
    if (buffer->length - buffer->offset < protocol->package_eof_len)
    {
        eof_pos = -1;
    }
    else
    {
        eof_pos = swoole_strnpos(buffer->str + buffer->offset, buffer->length - buffer->offset, protocol->package_eof, protocol->package_eof_len);
    }

    //swNotice("#[0] count=%d, length=%ld, size=%ld, offset=%ld", count, buffer->length, buffer->size, buffer->offset);

    //waiting for more data
    if (eof_pos < 0)
    {
        buffer->offset = buffer->length - protocol->package_eof_len;
        return buffer->length;
    }

    uint32_t length = buffer->offset + eof_pos + protocol->package_eof_len;
    //swNotice("#[4] count=%d, length=%d", count, length);
    protocol->onPackage(object, buffer->str, length);

    //there are remaining data
    if (length < buffer->length)
    {
        uint32_t remaining_length = buffer->length - length;
        char *remaining_data = buffer->str + length;
        //swNotice("#[5] count=%d, remaining_length=%d", count, remaining_length);

        while (1)
        {
            if (remaining_length < protocol->package_eof_len)
            {
                goto wait_more_data;
            }
            eof_pos = swoole_strnpos(remaining_data, remaining_length, protocol->package_eof, protocol->package_eof_len);
            if (eof_pos < 0)
            {
                wait_more_data:
                //swNotice("#[1] count=%d, remaining_length=%d, length=%d", count, remaining_length, length);
                memcpy(stack_buf, remaining_data, remaining_length);
                memcpy(buffer->str, stack_buf, remaining_length);
                buffer->length = remaining_length;
                buffer->offset = 0;
                return remaining_length;
            }
            else
            {
                length = eof_pos + protocol->package_eof_len;
                protocol->onPackage(object, remaining_data, length);
                //swNotice("#[2] count=%d, remaining_length=%d, length=%d", count, remaining_length, length);
                remaining_data += length;
                remaining_length -= length;
            }
        }
    }
    //swNotice("#[3] length=%ld, size=%ld, offset=%ld", buffer->length, buffer->size, buffer->offset);
    swString_clear(buffer);
    return 0;
}
