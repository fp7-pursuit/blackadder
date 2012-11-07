/*
 * bitvector.{cc,hh} -- generic bit vector class
 * Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2002 International Computer Science Institute
 * Copyright (c) 2008 Meraki, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 *
 * ---------------------------------------------------------------------------
 *
 * Click LICENSE file
 * ===========================================================================
 *
 * (c) 1999-2009 Massachusetts Institute of Technology
 * (c) 2000-2009 Mazu Networks, Inc.
 * (c) 2001-2009 International Computer Science Institute
 * (c) 2004-2009 Regents of the University of California
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The name and trademarks of copyright holders may NOT be used in advertising
 * or publicity pertaining to the Software without specific, written prior
 * permission. Title to copyright in this Software and any associated
 * documentation will at all times remain with copyright holders.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * ---------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012  George Parisis and Dirk Trossen
 * See LICENSE and COPYING for more details.
 */

#include <stdint.h>
#include <iostream>
#include "bitvector.hpp"


Bitvector::Bitvector(string &x) {
    unsigned int i = 0;
    _data = new uint32_t[x.length()];
    _max = x.length() - 1;
    memset(_data, 0, x.length());
    for(i = 0; i < x.length(); i++) {
        if(x.at(i) == '1') {
            (*this)[size() - i - 1] = true;
        }
    }
}

Bitvector::Bitvector(const char *x) {
    unsigned int i = 0;
    _data = new uint32_t[strlen(x)];
    _max = strlen(x) - 1;
    memset(_data, 0, strlen(x));
    for(i = 0; i < strlen(x); i++) {
        if(x[i] == '1') {
            (*this)[size() - i - 1] = true;
        }
    }
}

void
Bitvector::finish_copy_constructor(const Bitvector &o) {
    int nn = max_word();
    _data = new uint32_t[nn + 1];
    for (int i = 0; i <= nn; i++)
        _data[i] = o._data[i];
}

void
Bitvector::clear() {
    int nn = max_word();
    for (int i = 0; i <= nn; i++)
        _data[i] = 0;
}

bool
Bitvector::zero() const {
    int nn = max_word();
    for (int i = 0; i <= nn; i++)
        if (_data[i])
            return false;
    return true;
}

void
Bitvector::resize_to_max(int new_max, bool valid_n) {
    int want_u = (new_max >> 5) + 1;
    int have_u = (valid_n ? max_word() : MAX_INLINE_WORD) + 1;
    if (have_u < MAX_INLINE_WORD + 1)
        have_u = MAX_INLINE_WORD + 1;
    if (want_u <= have_u)
        return;

    uint32_t *new_data = new uint32_t[want_u];
    memcpy(new_data, _data, have_u * sizeof (uint32_t));
    memset(new_data + have_u, 0, (want_u - have_u) * sizeof (uint32_t));
    if (_data != &_f0)
        delete[] _data;
    _data = new_data;
}

void
Bitvector::clear_last() {
    if ((_max < 0))
        _data[0] = 0;
    else if ((_max & 0x1F) != 0x1F) {
        uint32_t mask = (1U << ((_max & 0x1F) + 1)) - 1;
        _data[_max >> 5] &= mask;
    }
}

Bitvector &
        Bitvector::operator=(const Bitvector &o) {
    if (&o == this)
        /* nada */;
    else if (o.max_word() <= MAX_INLINE_WORD)
        memcpy(_data, o._data, 8);
    else {
        if (_data != &_f0)
            delete[] _data;
        _data = new uint32_t[o.max_word() + 1];
        memcpy(_data, o._data, (o.max_word() + 1) * sizeof (uint32_t));
    }
    _max = o._max;
    return *this;
}

Bitvector &
Bitvector::assign(int n, bool value) {
    resize(n);
    uint32_t bits = (value ? 0xFFFFFFFFU : 0U);
    // 24.Jun.2008 -- Even if n <= 0, at least one word must be set to "bits."
    // Otherwise assert(_max >= 0 || _data[0] == 0) will not hold.
    int copy = (n > 32 ? max_word() : 0);
    for (int i = 0; i <= copy; i++)
        _data[i] = bits;
    if (value)
        clear_last();
    return *this;
}

void
Bitvector::negate() {
    int nn = max_word();
    uint32_t *data = _data;
    for (int i = 0; i <= nn; i++)
        data[i] = ~data[i];
    clear_last();
}

Bitvector &
        Bitvector::operator&=(const Bitvector &o) {
    int nn = max_word();
    uint32_t *data = _data, *o_data = o._data;
    for (int i = 0; i <= nn; i++)
        data[i] &= o_data[i];
    return *this;
}

Bitvector &
        Bitvector::operator|=(const Bitvector &o) {
    if (o._max > _max)
        resize(o._max + 1);
    int nn = max_word();
    uint32_t *data = _data, *o_data = o._data;
    for (int i = 0; i <= nn; i++)
        data[i] |= o_data[i];
    return *this;
}

Bitvector &
        Bitvector::operator^=(const Bitvector &o) {
    int nn = max_word();
    uint32_t *data = _data, *o_data = o._data;
    for (int i = 0; i <= nn; i++)
        data[i] ^= o_data[i];
    return *this;
}

void
Bitvector::offset_or(const Bitvector &o, int offset) {
    uint32_t bits_1st = offset & 0x1F;
    int my_pos = offset >> 5;
    int o_pos = 0;
    int my_max_word = max_word();
    int o_max_word = o.max_word();
    uint32_t *data = _data;
    const uint32_t *o_data = o._data;

    while (true) {
        uint32_t val = o_data[o_pos];
        data[my_pos] |= (val << bits_1st);

        my_pos++;
        if (my_pos > my_max_word)
            break;

        if (bits_1st)
            data[my_pos] |= (val >> (32 - bits_1st));

        o_pos++;
        if (o_pos > o_max_word)
            break;
    }
}

void
Bitvector::or_with_difference(const Bitvector &o, Bitvector &diff) {
    if (diff._max != _max)
        diff.resize(_max + 1);
    int nn = max_word();
    uint32_t *data = _data, *diff_data = diff._data;
    const uint32_t *o_data = o._data;
    for (int i = 0; i <= nn; i++) {
        diff_data[i] = o_data[i] & ~data[i];
        data[i] |= o_data[i];
    }
}

bool
Bitvector::nonzero_intersection(const Bitvector &o) const {
    int nn = o.max_word();
    if (nn > max_word())
        nn = max_word();
    const uint32_t *data = _data, *o_data = o._data;
    for (int i = 0; i <= nn; i++)
        if (data[i] & o_data[i])
            return true;
    return false;
}

void
Bitvector::swap(Bitvector &x) {
    uint32_t u = _f0;
    _f0 = x._f0;
    x._f0 = u;

    u = _f1;
    _f1 = x._f1;
    x._f1 = u;

    int m = _max;
    _max = x._max;
    x._max = m;

    uint32_t *d = _data;
    _data = (x._data == &x._f0 ? &_f0 : x._data);
    x._data = (d == &_f0 ? &x._f0 : d);
}

string Bitvector::to_string() {
    string res;
    for (int i = 0; i < size(); i++) {
        if (Bitvector::Bit::unspecified_bool_type((*this)[size() - i - 1])) {
            res += '1';
        } else {
            res += '0';
        }
    }
    return res;
}
