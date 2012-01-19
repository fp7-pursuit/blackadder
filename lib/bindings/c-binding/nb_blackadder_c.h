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

#ifndef NB_BLACKADDER_H
#define NB_BLACKADDER_H

#include "blackadder_c.h"

struct _nb_ba_handle;
typedef struct _nb_ba_handle *nb_ba_handle;
typedef void (*nb_ba_callbacktype)(ba_event);

/*
 * We use C macro magic to wrap the publish_*(), unpublish_*(),
 * subscribe_*() and unsubscribe_*() functions.
 *
 * The parameters for these functions, except for publish_data(),
 * are the same.
 *
 * nb_ba_<[un]publish|[un]subscribe>_<scope|info>(
 *     nb_ba_handle ba,
 *     const char *id, unsigned int id_len,
 *     const char *prefix_id, unsigned int prefix_id_len,
 *     unsigned char strategy,
 *     void *str_opt, unsigned int str_opt_len)
 *
 * nb_ba_publish_data(
 *     nb_ba_handle ba,
 *     const char *id, unsigned int id_len,
 *     unsigned char strategy,
 *     void *str_opt, unsigned int str_opt_len,
 *     void *data, unsigned int data_len
 */

#define _NB_BA_FUNC_DECL(f_name)  _XX_BA_FUNC_DECL(nb_ba, f_name)
#define _NB_BA_FUNC_DECL_DATA(f_name)  _XX_BA_FUNC_DECL_DATA(nb_ba, f_name)

#ifdef __cplusplus
#define _NB_BA_FUNC_DEF(f_name)  _XX_BA_FUNC_DEF(nb_ba, NB_Blackadder, f_name)
#define _NB_BA_FUNC_DEF_DATA(f_name)  _XX_BA_FUNC_DEF_DATA(nb_ba, NB_Blackadder, f_name)

struct _nb_ba_handle {
    NB_Blackadder *instance;
};
#endif /* __cplusplus */

/*
 * Blackadder pub/sub functions.
 */
_NB_BA_FUNC_DECL(publish_scope);
_NB_BA_FUNC_DECL(publish_info);
_NB_BA_FUNC_DECL(unpublish_scope);
_NB_BA_FUNC_DECL(unpublish_info);
_NB_BA_FUNC_DECL(subscribe_scope);
_NB_BA_FUNC_DECL(subscribe_info);
_NB_BA_FUNC_DECL(unsubscribe_scope);
_NB_BA_FUNC_DECL(unsubscribe_info);

_NB_BA_FUNC_DECL_DATA(publish_data);

/*
 * Other NB_Blackadder instance functions.
 */
nb_ba_handle nb_ba_instance(int user_space);
void nb_ba_delete(nb_ba_handle ba);
void nb_ba_disconnect(nb_ba_handle ba);

void nb_ba_set_callback(nb_ba_handle ba, nb_ba_callbacktype cf);
void nb_ba_selector(nb_ba_handle ba, void *arg);
void nb_ba_worker(nb_ba_handle ba, void *arg);
void nb_ba_signal_handler(nb_ba_handle ba, int sig);
void nb_ba_join(nb_ba_handle ba);

#endif /* NB_BLACKADDER_H */
