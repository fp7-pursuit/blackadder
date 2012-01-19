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

#ifndef BLACKADDER_H
#define BLACKADDER_H

#include "blackadder_defs.h"

struct _ba_handle;
typedef struct _ba_handle *ba_handle;

typedef void *ba_event;

/*
 * We use C macro magic to wrap the publish_*(), unpublish_*(),
 * subscribe_*() and unsubscribe_*() functions.
 *
 * The parameters for these functions, except for publish_data(),
 * are the same.
 *
 * ba_<[un]publish|[un]subscribe>_<scope|info>(
 *     ba_handle ba,
 *     const char *id, unsigned int id_len,
 *     const char *prefix_id, unsigned int prefix_id_len,
 *     unsigned char strategy,
 *     void *str_opt, unsigned int str_opt_len)
 *
 * ba_publish_data(
 *     ba_handle ba,
 *     const char *id, unsigned int id_len,
 *     unsigned char strategy,
 *     void *str_opt, unsigned int str_opt_len,
 *     void *data, unsigned int data_len
 */

#define _BA_PARAMS                                                      \
    const char *id, unsigned int id_len,                                \
    const char *prefix_id, unsigned int prefix_id_len,                  \
    unsigned char strategy,                                             \
    void *str_opt, unsigned int str_opt_len

#define _BA_PARAMS_DATA                                                 \
    const char *id, unsigned int id_len,                                \
    unsigned char strategy,                                             \
    void *str_opt, unsigned int str_opt_len,                            \
    void *data, unsigned int data_len

#define _XX_BA_FUNC_DECL(ba_prefix, f_name)                             \
    void ba_prefix##_##f_name(ba_prefix##_handle ba, _BA_PARAMS)

#define _XX_BA_FUNC_DECL_DATA(ba_prefix, f_name)                        \
    void ba_prefix##_##f_name(ba_prefix##_handle ba, _BA_PARAMS_DATA)

#define _BA_FUNC_DECL(f_name)  _XX_BA_FUNC_DECL(ba, f_name)
#define _BA_FUNC_DECL_DATA(f_name)  _XX_BA_FUNC_DECL_DATA(ba, f_name)

#ifdef __cplusplus
#define _XX_BA_FUNC_DEF(ba_prefix, ba_type, f_name)                     \
    extern "C" _XX_BA_FUNC_DECL(ba_prefix, f_name) {                    \
        ba_type *_ba = (ba_type *)ba->instance;                         \
        string _id (id, id_len);                                        \
        string _prefix_id (prefix_id, prefix_id_len);                   \
        _ba->f_name(_id, _prefix_id, strategy, str_opt, str_opt_len);   \
    }

#define _XX_BA_FUNC_DEF_DATA(ba_prefix, ba_type, f_name)                \
    extern "C" _XX_BA_FUNC_DECL_DATA(ba_prefix, f_name) {               \
        ba_type *_ba = (ba_type *)ba->instance;                         \
        string _id (id, id_len);                                        \
        _ba->f_name(_id, strategy, str_opt, str_opt_len, data, data_len); \
    }

#define _BA_FUNC_DEF(f_name)  _XX_BA_FUNC_DEF(ba, Blackadder, f_name)
#define _BA_FUNC_DEF_DATA(f_name)  _XX_BA_FUNC_DEF_DATA(ba, Blackadder, f_name)

struct _ba_handle {
    Blackadder *instance;
};
#endif /* __cplusplus */

/*
 * Blackadder pub/sub functions.
 */
_BA_FUNC_DECL(publish_scope);
_BA_FUNC_DECL(publish_info);
_BA_FUNC_DECL(unpublish_scope);
_BA_FUNC_DECL(unpublish_info);
_BA_FUNC_DECL(subscribe_scope);
_BA_FUNC_DECL(subscribe_info);
_BA_FUNC_DECL(unsubscribe_scope);
_BA_FUNC_DECL(unsubscribe_info);

_BA_FUNC_DECL_DATA(publish_data);

/*
 * Other Blackadder instance functions.
 */
ba_handle ba_instance(int user_space);
void ba_delete(ba_handle ba);
void ba_disconnect(ba_handle ba);

void ba_get_event(ba_handle ba, ba_event ev);

/*
 * Accessor functions for the Event class.
 */
ba_event ba_event_new(void);
void ba_event_delete(ba_event ev);
void ba_event_type(ba_event ev, unsigned char **type);
void ba_event_id(ba_event ev, const char **id, unsigned int *id_len);
void ba_event_data(ba_event ev, void **data, unsigned int **data_len);

#endif /* BLACKADDER_H */
