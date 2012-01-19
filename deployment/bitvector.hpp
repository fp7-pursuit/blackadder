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


#ifndef BITVECTOR_HPP
#define	BITVECTOR_HPP

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <string>

using namespace std;

/**@brief (Deployment Application) stolen from Click's bitvector
 */
class Bitvector {
public:

    class Bit;

    /** @brief Construct an empty Bitvector. */
    Bitvector()
    : _max(-1), _data(&_f0), _f0(0), _f1(0) {
    }

    /** @brief Construct an all-false Bitvector with @a n elements.
     * @pre @a n >= 0 */
    explicit Bitvector(int n);

    /** @brief Construct a @a bit-valued length-1 Bitvector. */
    explicit Bitvector(bool bit)
    : _max(0), _data(&_f0), _f0(bit), _f1(0) {
    }

    /** @brief Construct a @a bit-valued length-@a n Bitvector.
     * @pre @a n >= 0 */
    Bitvector(int n, bool bit);

    /** @brief Construct a Bitvector as a copy of @a x. */
    Bitvector(const Bitvector &x);

    Bitvector(string &x);

    /** @brief Destroy a Bitvector.
     *
     * All outstanding Bit objects become invalid. */
    ~Bitvector() {
        if (_data != &_f0)
            delete[] _data;
    }

    /** @brief Return the number of bits in the Bitvector. */
    int size() const {
        return _max + 1;
    }

    /** @brief Return true iff the Bitvector's bits are all false. */
    bool zero() const;

    typedef bool (Bitvector::*unspecified_bool_type)() const;

    /** @brief Return true iff the Bitvector's bits are all false.
     * @sa zero() */
    operator unspecified_bool_type() const {
        return !zero() ? &Bitvector::zero : 0;
    }


    /** @brief Return the bit at position @a i.
     * @pre 0 <= @a i < size() */
    Bit operator[](int i);

    /** @overload */
    bool operator[](int i) const;

    /** @brief Return the bit at position @a i, extending if necessary.
     *
     * If @a i >= size(), then the Bitvector is resize()d to length @a i+1,
     * which adds false bits to fill out the vector.
     *
     * @pre 0 <= @a i
     * @post @a i < size() */
    Bit force_bit(int i);


    /** @brief Set all bits to false. */
    void clear();

    /** @brief Resize the Bitvector to @a n bits.
     * @pre @a n >= 0
     *
     * Any bits added to the Bitvector are false. */
    void resize(int n);

    /** @brief Set the Bitvector to @a bit-valued length-@a n.
     * @pre @a n >= 0
     * @return *this */
    Bitvector &assign(int n, bool bit);

    /** @brief Set the Bitvector to a copy of @a x.
     * @return *this */
    Bitvector & operator=(const Bitvector &x);

    /** @brief Check bitvectors for equality. */
    bool operator==(const Bitvector &x) const {
        if (_max != x._max)
            return false;
        else if (_max <= MAX_INLINE_BIT)
            return memcmp(_data, x._data, 8) == 0;
        else
            return memcmp(_data, x._data, (max_word() + 1)*4) == 0;
    }

    /** @brief Check bitvectors for inequality. */
    bool operator!=(const Bitvector &x) const {
        return !(*this == x);
    }


    /** @brief Return the bitwise negation of this Bitvector. */
    inline Bitvector operator~() const;

    /** @brief Return the bitwise and of two bitvectors.
     * @pre @a x.size() == size() */
    Bitvector operator&(const Bitvector &x) const;

    /** @brief Return the bitwise or of two bitvectors.
     * @pre @a x.size() == size() */
    Bitvector operator|(const Bitvector &x) const;

    /** @brief Return the bitwise exclusive or of two bitvectors.
     * @pre @a x.size() == size() */
    Bitvector operator^(const Bitvector &x) const;

    /** @brief Return the bitwise subtraction of two bitvectors.
     * @pre @a x.size() == size()
     *
     * <code>x - y</code> is equivalent to <code>x & ~y</code>. */
    Bitvector operator-(const Bitvector &x) const;


    /** @brief Negate this Bitvector by flipping each of its bits. */
    void negate();

    /** @brief Modify this Bitvector by bitwise and with @a x.
     * @pre @a x.size() == size()
     * @return *this */
    Bitvector & operator&=(const Bitvector &x);

    /** @brief Modify this Bitvector by bitwise or with @a x.
     * @pre @a x.size() == size()
     * @return *this */
    Bitvector & operator|=(const Bitvector &x);

    /** @brief Modify this Bitvector by bitwise exclusive or with @a x.
     * @pre @a x.size() == size()
     * @return *this */
    Bitvector & operator^=(const Bitvector &x);

    /** @brief Modify this Bitvector by bitwise subtraction with @a x.
     * @pre @a x.size() == size()
     * @return *this
     *
     * Equivalent to <code>*this &= ~@a x</code>. */
    Bitvector & operator-=(const Bitvector &x);


    /** @brief Modify this Bitvector by bitwise or with an offset @a x.
     * @param x bitwise or operand
     * @param offset initial offset
     * @pre @a offset >= 0 && @a offset + @a x.size() <= size()
     *
     * Logically shifts @a x to start at position @a offset, then performs
     * a bitwise or.  <code>a.offset_or(b, offset)</code> is equivalent to:
     * @code
     * for (int i = 0; i < b.size(); ++i)
     *     a[offset+i] |= b[i];
     * @endcode */
    void offset_or(const Bitvector &x, int offset);

    /** @brief Modify this Bitvector by bitwise or, returning difference.
     * @param x bitwise or operand
     * @param[out] difference set to (@a x - old *this)
     * @pre @a x.size() == size()
     * @post @a difference.size() == size()
     * @post @a x | *this == *this
     * @post (@a difference & *this & @a x) == @a difference
     *
     * Same as operator|=, but additionally preserves any change in this
     * Bitvector.  Any newly set bits are returned in @a difference. */
    void or_with_difference(const Bitvector &x, Bitvector &difference);


    /** @brief Return whether this Bitvector and @a x have a common true bit.
     *
     * This Bitvector and @a x may have different sizes; the smaller is used. */
    bool nonzero_intersection(const Bitvector &x) const;


    /** @brief Swap the contents of this Bitvector and @a x. */
    void swap(Bitvector &x);


    /** @brief Data word type.
     *
     * Bitvectors are stored as arrays of data words, each containing
     * data_word_bits bits.  For special purposes it may be faster or easier
     * to manipulate data words directly. */
    typedef uint32_t data_word_type;

    enum {
        data_word_bits = 32
    };

    /** @brief Return the index of the maximum valid data word. */
    int max_word() const {
        return (_max < 0 ? -1 : _max >> 5);
    }

    /** @brief Return a pointer to this Bitvector's data words. */
    data_word_type *data_words() {
        return _data;
    }

    /** @overload */
    const data_word_type *data_words() const {
        return _data;
    }

    string to_string();

    //private:

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

/** @class Bitvector::Bit
  @brief (Deployment Application) stolen from Click's bitvector. A wrapper class that acts like a single bit.

  Bits are returned by modifiable Bitvectors' operator[].  They act like bools,
  but Bit operations actually index into individual bits in some shared word. */
class Bitvector::Bit {
public:

    /** @brief Construct a bit at offset @a bit_offset in data word @a w. */
    Bit(Bitvector::data_word_type &w, int bit_offset)
    : _p(w), _mask(1U << bit_offset) {
    }

    typedef Bitvector::unspecified_bool_type unspecified_bool_type;

    /** @brief Check if this bit is true. */
    inline operator unspecified_bool_type() const {
        return (_p & _mask) != 0 ? &Bitvector::zero : 0;
    }

    /** @brief Set this bit to @a x. */
    Bit & operator=(bool x) {
        if (x)
            _p |= _mask;
        else
            _p &= ~_mask;
        return *this;
    }

    /** @overload */
    Bit & operator=(const Bit &x) {
        if (x._p & x._mask)
            _p |= _mask;
        else
            _p &= ~_mask;
        return *this;
    }

    /** @brief Modify this bit by bitwise and with @a x. */
    Bit & operator&=(bool x) {
        if (!x)
            _p &= ~_mask;
        return *this;
    }

    /** @brief Modify this bit by bitwise or with @a x. */
    Bit & operator|=(bool x) {
        if (x)
            _p |= _mask;
        return *this;
    }

    /** @brief Modify this bit by bitwise exclusive or with @a x. */
    Bit & operator^=(bool x) {
        if (x)
            _p ^= _mask;
        return *this;
    }

    /** @brief Modify this bit by bitwise subtraction with @a x. */
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

#endif	/* BITVECTOR_HPP */

