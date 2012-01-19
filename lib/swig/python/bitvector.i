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

%extend Bitvector {
    int __len__() {
        return $self->size();
    }

    string __str__() {
        return $self->to_string();
    }

    bool __getitem__(int i) {
        return (*$self)[i];
    }

    void __setitem__(int i, bool v) {
        (*$self)[i] = v;
    }

    string as_bytes() {
        string bs((char *)($self->_data), ($self->_max >> 3)+1);
        return bs;
    }
}
