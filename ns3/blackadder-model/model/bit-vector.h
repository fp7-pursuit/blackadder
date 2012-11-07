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

#ifndef BIT_VECTOR_H
#define BIT_VECTOR_H

#include <string.h>
#include <string>
#include <iostream>
#include <ns3/simple-ref-count.h>
#include <ns3/ptr.h>
#include <ns3/abort.h>
#include <ns3/assert.h>
#include <ns3/log.h>


namespace ns3 {

    class BitVector : public SimpleRefCount<BitVector> {
    public:
        class Bit;

        /** @brief Construct an empty BitVector. */
        BitVector() : _max(-1), _data(&_f0), _f0(0), _f1(0) {
        }

        /** @brief Construct an all-false BitVector with @a n elements.
         * @pre @a n >= 0 */
        explicit BitVector(int n);

        /** @brief Construct a @a bit-valued length-1 BitVector. */
        explicit BitVector(bool bit) : _max(0), _data(&_f0), _f0(bit), _f1(0) {
        }

        /** @brief Construct a @a bit-valued length-@a n BitVector.
         * @pre @a n >= 0 */
        BitVector(int n, bool bit);

        /** @brief Construct a BitVector as a copy of @a x. */
        BitVector(const BitVector &x);

        BitVector(std::string str, int n);

        BitVector(std::string &x);

        /** @brief Destroy a BitVector.
         *
         * All outstanding Bit objects become invalid. */
        virtual ~BitVector() {
            if (_data != &_f0)
                delete[] _data;
        }

        /** @brief Return the number of bits in the BitVector. */
        int size() const {
            return _max + 1;
        }

        /** @brief Return true iff the BitVector's bits are all false. */
        bool zero() const;

        typedef bool (BitVector::*unspecified_bool_type)() const;

        /** @brief Return true iff the BitVector's bits are all false.
         * @sa zero() */
        operator unspecified_bool_type() const {
            return !zero() ? &BitVector::zero : 0;
        }


        /** @brief Return the bit at position @a i.
         * @pre 0 <= @a i < size() */
        Bit operator[](int i);

        /** @overload */
        bool operator[](int i) const;

        /** @brief Return the bit at position @a i, extending if necessary.
         *
         * If @a i >= size(), then the BitVector is resize()d to length @a i+1,
         * which adds false bits to fill out the vector.
         *
         * @pre 0 <= @a i
         * @post @a i < size() */
        Bit force_bit(int i);


        /** @brief Set all bits to false. */
        void clear();

        /** @brief Resize the BitVector to @a n bits.
         * @pre @a n >= 0
         *
         * Any bits added to the BitVector are false. */
        void resize(int n);

        /** @brief Set the BitVector to @a bit-valued length-@a n.
         * @pre @a n >= 0
         * @return *this */
        BitVector &assign(int n, bool bit);

        /** @brief Set the BitVector to a copy of @a x.
         * @return *this */
        BitVector & operator=(const BitVector &x);

        /** @brief Check bitvectors for equality. */
        bool operator==(const BitVector &x) const {
            if (_max != x._max)
                return false;
            else if (_max <= MAX_INLINE_BIT)
                return memcmp(_data, x._data, 8) == 0;
            else
                return memcmp(_data, x._data, (max_word() + 1)*4) == 0;
        }

        /** @brief Check bitvectors for inequality. */
        bool operator!=(const BitVector &x) const {
            return !(*this == x);
        }

        /** @brief Return the bitwise negation of this BitVector. */
        inline BitVector operator~() const;

        /** @brief Return the bitwise and of two bitvectors.
         * @pre @a x.size() == size() */
        BitVector operator&(const BitVector &x) const;

        /** @brief Return the bitwise or of two bitvectors.
         * @pre @a x.size() == size() */
        BitVector operator|(const BitVector &x) const;

        /** @brief Return the bitwise exclusive or of two bitvectors.
         * @pre @a x.size() == size() */
        BitVector operator^(const BitVector &x) const;

        /** @brief Return the bitwise subtraction of two bitvectors.
         * @pre @a x.size() == size()
         *
         * <code>x - y</code> is equivalent to <code>x & ~y</code>. */
        BitVector operator-(const BitVector &x) const;


        /** @brief Negate this BitVector by flipping each of its bits. */
        void negate();

        /** @brief Modify this BitVector by bitwise and with @a x.
         * @pre @a x.size() == size()
         * @return *this */
        BitVector & operator&=(const BitVector &x);

        /** @brief Modify this BitVector by bitwise or with @a x.
         * @pre @a x.size() == size()
         * @return *this */
        BitVector & operator|=(const BitVector &x);

        /** @brief Modify this BitVector by bitwise exclusive or with @a x.
         * @pre @a x.size() == size()
         * @return *this */
        BitVector & operator^=(const BitVector &x);

        /** @brief Modify this BitVector by bitwise subtraction with @a x.
         * @pre @a x.size() == size()
         * @return *this
         *
         * Equivalent to <code>*this &= ~@a x</code>. */
        BitVector & operator-=(const BitVector &x);


        /** @brief Modify this BitVector by bitwise or with an offset @a x.
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
        void offset_or(const BitVector &x, int offset);

        /** @brief Modify this BitVector by bitwise or, returning difference.
         * @param x bitwise or operand
         * @param[out] difference set to (@a x - old *this)
         * @pre @a x.size() == size()
         * @post @a difference.size() == size()
         * @post @a x | *this == *this
         * @post (@a difference & *this & @a x) == @a difference
         *
         * Same as operator|=, but additionally preserves any change in this
         * BitVector.  Any newly set bits are returned in @a difference. */
        void or_with_difference(const BitVector &x, BitVector &difference);


        /** @brief Return whether this BitVector and @a x have a common true bit.
         *
         * This BitVector and @a x may have different sizes; the smaller is used. */
        bool nonzero_intersection(const BitVector &x) const;


        /** @brief Swap the contents of this BitVector and @a x. */
        void swap(BitVector &x);


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

        /** @brief Return a pointer to this BitVector's data words. */
        data_word_type *data_words() {
            return _data;
        }

        /** @overload */
        const data_word_type *data_words() const {
            return _data;
        }

        std::string to_string();

        enum {
            MAX_INLINE_BIT = 63, MAX_INLINE_WORD = 1
        };

        int _max;
        uint32_t *_data;
        uint32_t _f0;
        uint32_t _f1;

        void finish_copy_constructor(const BitVector &);
        void clear_last();
        void resize_to_max(int, bool);
    };

    /**@brief (blackadder Core) Stolen by Click's bitvector. A wrapper class that acts like a single bit.
     * 
     * Bits are returned by modifiable Bitvectors' operator[].  They act like bools,
     * but Bit operations actually index into individual bits in some shared word. */
    class BitVector::Bit {
    public:

        /** @brief Construct a bit at offset @a bit_offset in data word @a w. */
        Bit(BitVector::data_word_type &w, int bit_offset)
        : _p(w), _mask(1U << bit_offset) {
        }

        typedef BitVector::unspecified_bool_type unspecified_bool_type;

        /** @brief Check if this bit is true. */
        inline operator unspecified_bool_type() const {
            return (_p & _mask) != 0 ? &BitVector::zero : 0;
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
}

#endif
