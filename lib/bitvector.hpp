/*
* Copyright (C) 2010-2011  George Parisis and Dirk Trossen
* All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version
* 2 as published by the Free Software Foundation.
*
* Alternatively, this software may be distributed under the terms of
* the BSD license.
*
* See LICENSE and COPYING for more details.
*/

#ifndef BITVECTOR_HH
#define BITVECTOR_HH

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <string>

using namespace std;

class Bitvector {
public:
#ifndef SWIG
    class Bit;
#endif /* SWIG */
    Bitvector()
    : _max(-1), _data(&_f0), _f0(0), _f1(0) {
    }
    explicit Bitvector(int n);
    explicit Bitvector(bool bit)
    : _max(0), _data(&_f0), _f0(bit), _f1(0) {
    }
    Bitvector(int n, bool bit);
    Bitvector(const Bitvector &x);
    Bitvector(string &x);
    Bitvector(const char *);
    ~Bitvector() {
        if (_data != &_f0)
            delete[] _data;
    }
    int size() const {
        return _max + 1;
    }
    bool zero() const;
#ifndef SWIG
    typedef bool (Bitvector::*unspecified_bool_type)() const;
    operator unspecified_bool_type() const {
        return !zero() ? &Bitvector::zero : 0;
    }
    Bit operator[](int i);
    bool operator[](int i) const;
    Bit force_bit(int i);
#endif /* SWIG */
    void clear();
    void resize(int n);
    Bitvector &assign(int n, bool bit);
#ifndef SWIG
    Bitvector & operator=(const Bitvector &x);
    bool operator==(const Bitvector &x) const {
        if (_max != x._max)
            return false;
        else if (_max <= MAX_INLINE_BIT)
            return memcmp(_data, x._data, 8) == 0;
        else
            return memcmp(_data, x._data, (max_word() + 1)*4) == 0;
    }
    bool operator!=(const Bitvector &x) const {
        return !(*this == x);
    }
    inline Bitvector operator~() const;
    Bitvector operator&(const Bitvector &x) const;
    Bitvector operator|(const Bitvector &x) const;
    Bitvector operator^(const Bitvector &x) const;
    Bitvector operator-(const Bitvector &x) const;
#endif /* SWIG */
    void negate();
#ifndef SWIG
    Bitvector & operator&=(const Bitvector &x);
    Bitvector & operator|=(const Bitvector &x);
    Bitvector & operator^=(const Bitvector &x);
    Bitvector & operator-=(const Bitvector &x);
#endif /* SWIG */
    void offset_or(const Bitvector &x, int offset);
    void or_with_difference(const Bitvector &x, Bitvector &difference);
    bool nonzero_intersection(const Bitvector &x) const;
    void swap(Bitvector &x);
    typedef uint32_t data_word_type;
    enum {
        data_word_bits = 32
    };
    int max_word() const {
        return (_max < 0 ? -1 : _max >> 5);
    }
    data_word_type *data_words() {
        return _data;
    }
#ifndef SWIG
    const data_word_type *data_words() const {
        return _data;
    }
#endif
    string to_string();
    enum {
        MAX_INLINE_BIT = 63, MAX_INLINE_WORD = 1
    };
    int _max;
    uint32_t *_data;
    uint32_t _f0;
    uint32_t _f1;
    void finish_copy_constructor(const Bitvector &);
    void clear_last();
    void resize_to_max(int, bool);

};

#ifndef SWIG
class Bitvector::Bit {
public:
    Bit(Bitvector::data_word_type &w, int bit_offset)
    : _p(w), _mask(1U << bit_offset) {
    }

    typedef Bitvector::unspecified_bool_type unspecified_bool_type;
    inline operator unspecified_bool_type() const {
        return (_p & _mask) != 0 ? &Bitvector::zero : 0;
    }
    Bit & operator=(bool x) {
        if (x)
            _p |= _mask;
        else
            _p &= ~_mask;
        return *this;
    }
    Bit & operator=(const Bit &x) {
        if (x._p & x._mask)
            _p |= _mask;
        else
            _p &= ~_mask;
        return *this;
    }
    Bit & operator&=(bool x) {
        if (!x)
            _p &= ~_mask;
        return *this;
    }
    Bit & operator|=(bool x) {
        if (x)
            _p |= _mask;
        return *this;
    }
    Bit & operator^=(bool x) {
        if (x)
            _p ^= _mask;
        return *this;
    }
    Bit & operator-=(bool x) {
        if (x)
            _p &= ~_mask;
        return *this;
    }

private:

    uint32_t &_p;
    uint32_t _mask;

};

inline
Bitvector::Bitvector(int n)
: _max(n - 1), _data(&_f0), _f0(0), _f1(0) {
    if (_max > MAX_INLINE_BIT)
        resize_to_max(_max, false);
}

inline
Bitvector::Bitvector(int n, bool b)
: _max(n - 1), _data(&_f0), _f0(0), _f1(0) {
    if (_max > MAX_INLINE_BIT)
        resize_to_max(_max, false);
    if (b)
        assign(n, b);
}

inline
Bitvector::Bitvector(const Bitvector &o)
: _max(o._max), _data(&_f0), _f0(o._data[0]), _f1(o._data[1]) {
    if (_max > MAX_INLINE_BIT)
        finish_copy_constructor(o);
}

inline void
Bitvector::resize(int n) {
    if (n - 1 > MAX_INLINE_BIT)
        resize_to_max(n - 1, true);
    _max = n - 1;
}

inline Bitvector::Bit
Bitvector::operator[](int i) {
    return Bit(_data[i >> 5], i & 31);
}

inline bool
Bitvector::operator[](int i) const {
    return (_data[i >> 5] & (1U << (i & 31))) != 0;
}

inline Bitvector::Bit
Bitvector::force_bit(int i) {
    if (i > _max)
        resize(i + 1);
    return Bit(_data[i >> 5], i & 31);
}

inline Bitvector &
        Bitvector::operator-=(const Bitvector &o) {
    return *this &= ~o;
}

inline Bitvector
Bitvector::operator~() const {
    Bitvector m = *this;
    m.negate();
    return m;
}

inline Bitvector
Bitvector::operator&(const Bitvector &o) const {
    Bitvector m = *this;
    m &= o;
    return m;
}

inline Bitvector
Bitvector::operator|(const Bitvector &o) const {
    Bitvector m = *this;
    m |= o;
    return m;
}

inline Bitvector
Bitvector::operator^(const Bitvector &o) const {
    Bitvector m = *this;
    m ^= o;
    return m;
}

inline Bitvector
Bitvector::operator-(const Bitvector &o) const {
    return *this & ~o;
}

inline void click_swap(Bitvector &a, Bitvector &b) {
    a.swap(b);
}
#endif /* SWIG */

#endif

