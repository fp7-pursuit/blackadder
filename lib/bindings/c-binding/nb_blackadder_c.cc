/*-
 * Copyright (C) 2011  Oy L M Ericsson Ab, NomadicLab
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of the
 * BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

#include <nb_blackadder.hpp>
extern "C" {
#include "nb_blackadder_c.h"
}

//using namespace std;

/*
 * We use C macro magic to wrap the publish_*(), unpublish_*(),
 * subscribe_*() and unsubscribe_*() functions.
 */

_NB_BA_FUNC_DEF(publish_scope)
_NB_BA_FUNC_DEF(publish_info)
_NB_BA_FUNC_DEF(unpublish_scope)
_NB_BA_FUNC_DEF(unpublish_info)
_NB_BA_FUNC_DEF(subscribe_scope)
_NB_BA_FUNC_DEF(subscribe_info)
_NB_BA_FUNC_DEF(unsubscribe_scope)
_NB_BA_FUNC_DEF(unsubscribe_info)

_NB_BA_FUNC_DEF_DATA(publish_data)

extern "C" nb_ba_handle
nb_ba_instance(int user_space)
{
    nb_ba_handle ba = new (nothrow) _nb_ba_handle;
    if (ba != NULL)
        ba->instance = NB_Blackadder::Instance(user_space);
    return ba;
}

extern "C" void
nb_ba_delete(nb_ba_handle ba)
{
    if (ba != NULL) {
        if (ba->instance != NULL) {
            delete ((NB_Blackadder *)ba->instance);
            ba->instance = NULL;
        }
        delete ba;
    }
}

extern "C" void
nb_ba_disconnect(nb_ba_handle ba)
{
    ((NB_Blackadder *)ba)->disconnect();
}

static nb_ba_callbacktype _nb_ba_cf = NULL;

static void
_nb_ba_callback(Event *ev)
{
    if (_nb_ba_cf != NULL)
        return _nb_ba_cf((ba_event)ev);
}

extern "C" void
nb_ba_set_callback(nb_ba_handle ba, nb_ba_callbacktype cf)
{
    _nb_ba_cf = cf;
    ((NB_Blackadder *)ba)->setCallback(_nb_ba_callback);
}

extern "C" void
nb_ba_selector(nb_ba_handle ba, void *arg)
{
    ((NB_Blackadder *)ba)->selector(arg);
}

extern "C" void
nb_ba_worker(nb_ba_handle ba, void *arg)
{
    ((NB_Blackadder *)ba)->worker(arg);
}

extern "C" void
nb_ba_signal_handler(nb_ba_handle ba, int sig)
{
    ((NB_Blackadder *)ba)->signal_handler(sig);
}

extern "C" void
nb_ba_join(nb_ba_handle ba)
{
    ((NB_Blackadder *)ba)->join();
}
