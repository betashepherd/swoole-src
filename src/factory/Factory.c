/*
 +----------------------------------------------------------------------+
 | Swoole                                                               |
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
#include "Server.h"

int swFactory_create(swFactory *factory)
{
    factory->dispatch = swFactory_dispatch;
    factory->finish = swFactory_finish;
    factory->start = swFactory_start;
    factory->shutdown = swFactory_shutdown;
    factory->end = swFactory_end;
    factory->notify = swFactory_notify;

    factory->onTask = NULL;
    factory->onFinish = NULL;

    return SW_OK;
}

int swFactory_start(swFactory *factory)
{
    return SW_OK;
}

int swFactory_shutdown(swFactory *factory)
{
    return SW_OK;
}

int swFactory_dispatch(swFactory *factory, swDispatchData *task)
{
    factory->last_from_id = task->data.info.from_id;
    return swWorker_onTask(factory, &task->data);
}

int swFactory_notify(swFactory *factory, swDataHead *req)
{
    return swWorker_onTask(factory, (swEventData *) req);
}

int swFactory_end(swFactory *factory, int fd)
{
    swServer *serv = factory->ptr;
    if (serv->onClose != NULL)
    {
        serv->onClose(serv, fd, 0);
    }
    return SwooleG.main_reactor->close(SwooleG.main_reactor, fd);
}

int swFactory_finish(swFactory *factory, swSendData *resp)
{
    int ret = 0;

    resp->length = resp->info.len;
    ret = swReactorThread_send(resp);

    if (ret < 0)
    {
        swSysError("sendto to connection#%d failed.", resp->info.fd);
    }
    return ret;
}

int swFactory_check_callback(swFactory *factory)
{
    if (factory->onTask == NULL)
    {
        return SW_ERR;
    }
    if (factory->onFinish == NULL)
    {
        return SW_ERR;
    }
    return SW_OK;
}
