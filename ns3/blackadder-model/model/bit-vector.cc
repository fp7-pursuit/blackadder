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

#include "bit-vector.h"

NS_LOG_COMPONENT_DEFINE("BitVector");

namespace ns3 {

    void BitVector::finish_copy_constructor(const BitVector &o) {
        int nn = max_word();
        _data = new uint32_t[nn + 1];
        for (int i = 0; i <= nn; i++)
            _data[i] = o._data[i];
    }

    void BitVector::clear() {
        int nn = max_word();
        for (int i = 0; i <= nn; i++)
            _data[i] = 0;
    }

    bool BitVector::zero() const {
        int nn = max_word();
        for (int i = 0; i <= nn; i++)
            if (_data[i])
                return false;
        return true;
    }

    void BitVector::resize_to_max(int new_max, bool valid_n) {
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

    void BitVector::clear_last() {
        if (_max < 0)
            _data[0] = 0;
        else if ((_max & 0x1F) != 0x1F) {
            uint32_t mask = (1U << ((_max & 0x1F) + 1)) - 1;
            _data[_max >> 5] &= mask;
        }
    }

    BitVector & BitVector::operator=(const BitVector &o) {
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

    BitVector & BitVector::assign(int n, bool value) {
        resize(n);
        uint32_t bits = (value ? 0xFFFFFFFFU : 0U);
        int copy = (n > 32 ? max_word() : 0);
        for (int i = 0; i <= copy; i++)
            _data[i] = bits;
        if (value)
            clear_last();
        return *this;
    }

    void BitVector::negate() {
        int nn = max_word();
        uint32_t *data = _data;
        for (int i = 0; i <= nn; i++)
            data[i] = ~data[i];
        clear_last();
    }

    BitVector & BitVector::operator&=(const BitVector &o) {
        NS_ASSERT(o._max == _max);
        int nn = max_word();
        uint32_t *data = _data, *o_data = o._data;
        for (int i = 0; i <= nn; i++)
            data[i] &= o_data[i];
        return *this;
    }

    BitVector & BitVector::operator|=(const BitVector &o) {
        if (o._max > _max)
            resize(o._max + 1);
        int nn = max_word();
        uint32_t *data = _data, *o_data = o._data;
        for (int i = 0; i <= nn; i++)
            data[i] |= o_data[i];
        return *this;
    }

    BitVector & BitVector::operator^=(const BitVector &o) {
        NS_ASSERT(o._max == _max);
        int nn = max_word();
        uint32_t *data = _data, *o_data = o._data;
        for (int i = 0; i <= nn; i++)
            data[i] ^= o_data[i];
        return *this;
    }

    void BitVector::offset_or(const BitVector &o, int offset) {
        NS_ASSERT(offset >= 0 && offset + o._max <= _max);
        uint32_t bits_1st = offset & 0x1F;
        int my_pos = offset >> 5;
        int o_pos = 0;
        int my_max_word = max_word();
        int o_max_word = o.max_word();
        uint32_t *data = _data;
        const uint32_t *o_data = o._data;
        NS_ASSERT((o._max < 0 && o_data[0] == 0) || (o._max & 0x1F) == 0 || (o_data[o_max_word] & ((1U << ((o._max & 0x1F) + 1)) - 1)) == o_data[o_max_word]);

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

    void BitVector::or_with_difference(const BitVector &o, BitVector &diff) {
        NS_ASSERT(o._max == _max);
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

    bool BitVector::nonzero_intersection(const BitVector &o) const {
        int nn = o.max_word();
        if (nn > max_word())
            nn = max_word();
        const uint32_t *data = _data, *o_data = o._data;
        for (int i = 0; i <= nn; i++)
            if (data[i] & o_data[i])
                return true;
        return false;
    }

    void BitVector::swap(BitVector &x) {
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

    BitVector::BitVector(int n) : _max(n - 1), _data(&_f0), _f0(0), _f1(0) {
        NS_LOG_FUNCTION(this);
        NS_ASSERT(n >= 0);
        if (_max > MAX_INLINE_BIT)
            resize_to_max(_max, false);
    }

    BitVector::BitVector(int n, bool b) : _max(n - 1), _data(&_f0), _f0(0), _f1(0) {
        NS_ASSERT(n >= 0);
        if (_max > MAX_INLINE_BIT)
            resize_to_max(_max, false);
        if (b)
            assign(n, b);
    }

    BitVector::BitVector(const BitVector &o) : _max(o._max), _data(&_f0), _f0(o._data[0]), _f1(o._data[1]) {
        if (_max > MAX_INLINE_BIT)
            finish_copy_constructor(o);
    }

    BitVector::BitVector(std::string str, int n) : _max(n - 1), _data(&_f0), _f0(0), _f1(0) {
        NS_ASSERT(n >= 0);
        if (_max > MAX_INLINE_BIT)
            resize_to_max(_max, false);
        for (int i = 0; i < n; i++) {
            if (str[i] == '1') {
                (*this)[i] = true;
            }
        }
    }

    BitVector::BitVector(std::string &x) {
        unsigned int i = 0;
        _data = new uint32_t[x.length()];
        _max = x.length() - 1;
        memset(_data, 0, x.length());
        for (i = 0; i < x.length(); i++) {
            if (x.at(i) == '1') {
                (*this)[size() - i - 1] = true;
            }
        }
    }

    void BitVector::resize(int n) {
        NS_ASSERT(n >= 0);
        if (n - 1 > MAX_INLINE_BIT)
            resize_to_max(n - 1, true);
        _max = n - 1;
    }

    BitVector::Bit BitVector::operator[](int i) {
        NS_ASSERT(i >= 0 && i <= _max);
        return Bit(_data[i >> 5], i & 31);
    }

    bool BitVector::operator[](int i) const {
        NS_ASSERT(i >= 0 && i <= _max);
        return (_data[i >> 5] & (1U << (i & 31))) != 0;
    }

    BitVector::Bit BitVector::force_bit(int i) {
        NS_ASSERT(i >= 0);
        if (i > _max)
            resize(i + 1);
        return Bit(_data[i >> 5], i & 31);
    }

    BitVector & BitVector::operator-=(const BitVector &o) {
        return *this &= ~o;
    }

    BitVector BitVector::operator~() const {
        BitVector m = *this;
        m.negate();
        return m;
    }

    BitVector BitVector::operator&(const BitVector &o) const {
        BitVector m = *this;
        m &= o;
        return m;
    }

    BitVector BitVector::operator|(const BitVector &o) const {
        BitVector m = *this;
        m |= o;
        return m;
    }

    BitVector BitVector::operator^(const BitVector &o) const {
        BitVector m = *this;
        m ^= o;
        return m;
    }

    BitVector BitVector::operator-(const BitVector &o) const {
        return *this & ~o;
    }

    std::string BitVector::to_string() {
        std::string res("");
        for (int i = 0; i < size(); i++) {
            if (BitVector::Bit::unspecified_bool_type((*this)[size() - i - 1])) {
                res += '1';
            } else {
                res += '0';
            }
        }
        return res;
    }
}
