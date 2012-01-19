#ifndef CLICK_BA_BITVECTOR_HH
#define CLICK_BA_BITVECTOR_HH


#include <click/config.h>
#include <click/element.hh>
#include <click/string.hh>

CLICK_DECLS

/** @brief (blackadder Core) Stolen by Click's bitvector. 
 * Can now directly access the _data value and print it
 */
class BABitvector {
public:

    class Bit;

    /** @brief Construct an empty BABitvector. */
    BABitvector()
    : _max(-1), _data(&_f0), _f0(0), _f1(0) {
    }

    /** @brief Construct an all-false BABitvector with @a n elements.
     * @pre @a n >= 0 */
    explicit BABitvector(int n);

    /** @brief Construct a @a bit-valued length-1 BABitvector. */
    explicit BABitvector(bool bit)
    : _max(0), _data(&_f0), _f0(bit), _f1(0) {
    }

    /** @brief Construct a @a bit-valued length-@a n BABitvector.
     * @pre @a n >= 0 */
    BABitvector(int n, bool bit);

    /** @brief Construct a BABitvector as a copy of @a x. */
    BABitvector(const BABitvector &x);

    /** @brief Destroy a BABitvector.
     *
     * All outstanding Bit objects become invalid. */
    ~BABitvector() {
        if (_data != &_f0)
            delete[] _data;
    }

    /** @brief Return the number of bits in the BABitvector. */
    int size() const {
        return _max + 1;
    }

    /** @brief Return true iff the BABitvector's bits are all false. */
    bool zero() const;

    typedef bool (BABitvector::*unspecified_bool_type)() const;

    /** @brief Return true iff the BABitvector's bits are all false.
     * @sa zero() */
    operator unspecified_bool_type() const {
        return !zero() ? &BABitvector::zero : 0;
    }


    /** @brief Return the bit at position @a i.
     * @pre 0 <= @a i < size() */
    Bit operator[](int i);

    /** @overload */
    bool operator[](int i) const;

    /** @brief Return the bit at position @a i, extending if necessary.
     *
     * If @a i >= size(), then the BABitvector is resize()d to length @a i+1,
     * which adds false bits to fill out the vector.
     *
     * @pre 0 <= @a i
     * @post @a i < size() */
    Bit force_bit(int i);


    /** @brief Set all bits to false. */
    void clear();

    /** @brief Resize the BABitvector to @a n bits.
     * @pre @a n >= 0
     *
     * Any bits added to the BABitvector are false. */
    void resize(int n);

    /** @brief Set the BABitvector to @a bit-valued length-@a n.
     * @pre @a n >= 0
     * @return *this */
    BABitvector &assign(int n, bool bit);

    /** @brief Set the BABitvector to a copy of @a x.
     * @return *this */
    BABitvector & operator=(const BABitvector &x);

    /** @brief Check bitvectors for equality. */
    bool operator==(const BABitvector &x) const {
        if (_max != x._max)
            return false;
        else if (_max <= MAX_INLINE_BIT)
            return memcmp(_data, x._data, 8) == 0;
        else
            return memcmp(_data, x._data, (max_word() + 1)*4) == 0;
    }

    /** @brief Check bitvectors for inequality. */
    bool operator!=(const BABitvector &x) const {
        return !(*this == x);
    }


    /** @brief Return the bitwise negation of this BABitvector. */
    inline BABitvector operator~() const;

    /** @brief Return the bitwise and of two bitvectors.
     * @pre @a x.size() == size() */
    BABitvector operator&(const BABitvector &x) const;

    /** @brief Return the bitwise or of two bitvectors.
     * @pre @a x.size() == size() */
    BABitvector operator|(const BABitvector &x) const;

    /** @brief Return the bitwise exclusive or of two bitvectors.
     * @pre @a x.size() == size() */
    BABitvector operator^(const BABitvector &x) const;

    /** @brief Return the bitwise subtraction of two bitvectors.
     * @pre @a x.size() == size()
     *
     * <code>x - y</code> is equivalent to <code>x & ~y</code>. */
    BABitvector operator-(const BABitvector &x) const;


    /** @brief Negate this BABitvector by flipping each of its bits. */
    void negate();

    /** @brief Modify this BABitvector by bitwise and with @a x.
     * @pre @a x.size() == size()
     * @return *this */
    BABitvector & operator&=(const BABitvector &x);

    /** @brief Modify this BABitvector by bitwise or with @a x.
     * @pre @a x.size() == size()
     * @return *this */
    BABitvector & operator|=(const BABitvector &x);

    /** @brief Modify this BABitvector by bitwise exclusive or with @a x.
     * @pre @a x.size() == size()
     * @return *this */
    BABitvector & operator^=(const BABitvector &x);

    /** @brief Modify this BABitvector by bitwise subtraction with @a x.
     * @pre @a x.size() == size()
     * @return *this
     *
     * Equivalent to <code>*this &= ~@a x</code>. */
    BABitvector & operator-=(const BABitvector &x);


    /** @brief Modify this BABitvector by bitwise or with an offset @a x.
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
    void offset_or(const BABitvector &x, int offset);

    /** @brief Modify this BABitvector by bitwise or, returning difference.
     * @param x bitwise or operand
     * @param[out] difference set to (@a x - old *this)
     * @pre @a x.size() == size()
     * @post @a difference.size() == size()
     * @post @a x | *this == *this
     * @post (@a difference & *this & @a x) == @a difference
     *
     * Same as operator|=, but additionally preserves any change in this
     * BABitvector.  Any newly set bits are returned in @a difference. */
    void or_with_difference(const BABitvector &x, BABitvector &difference);


    /** @brief Return whether this BABitvector and @a x have a common true bit.
     *
     * This BABitvector and @a x may have different sizes; the smaller is used. */
    bool nonzero_intersection(const BABitvector &x) const;


    /** @brief Swap the contents of this BABitvector and @a x. */
    void swap(BABitvector &x);


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

    /** @brief Return a pointer to this BABitvector's data words. */
    data_word_type *data_words() {
        return _data;
    }

    /** @overload */
    const data_word_type *data_words() const {
        return _data;
    }

    String to_string();

    //private:

    enum {
        MAX_INLINE_BIT = 63, MAX_INLINE_WORD = 1
    };

    int _max;
    uint32_t *_data;
    uint32_t _f0;
    uint32_t _f1;

    void finish_copy_constructor(const BABitvector &);
    void clear_last();
    void resize_to_max(int, bool);

};

/**@brief (blackadder Core) Stolen by Click's bitvector. A wrapper class that acts like a single bit.
 * 
 * Bits are returned by modifiable Bitvectors' operator[].  They act like bools,
 * but Bit operations actually index into individual bits in some shared word. */
class BABitvector::Bit {
public:

    /** @brief Construct a bit at offset @a bit_offset in data word @a w. */
    Bit(BABitvector::data_word_type &w, int bit_offset)
    : _p(w), _mask(1U << bit_offset) {
    }

    typedef BABitvector::unspecified_bool_type unspecified_bool_type;

    /** @brief Check if this bit is true. */
    inline operator unspecified_bool_type() const {
        return (_p & _mask) != 0 ? &BABitvector::zero : 0;
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
BABitvector::BABitvector(int n)
: _max(n - 1), _data(&_f0), _f0(0), _f1(0) {
    assert(n >= 0);
    if (_max > MAX_INLINE_BIT)
        resize_to_max(_max, false);
}

inline
BABitvector::BABitvector(int n, bool b)
: _max(n - 1), _data(&_f0), _f0(0), _f1(0) {
    assert(n >= 0);
    if (_max > MAX_INLINE_BIT)
        resize_to_max(_max, false);
    if (b)
        assign(n, b);
}

inline
BABitvector::BABitvector(const BABitvector &o)
: _max(o._max), _data(&_f0), _f0(o._data[0]), _f1(o._data[1]) {
    if (_max > MAX_INLINE_BIT)
        finish_copy_constructor(o);
}

inline void
BABitvector::resize(int n) {
    assert(n >= 0);
    if (n - 1 > MAX_INLINE_BIT)
        resize_to_max(n - 1, true);
    _max = n - 1;
}

inline BABitvector::Bit
BABitvector::operator[](int i) {
    assert(i >= 0 && i <= _max);
    return Bit(_data[i >> 5], i & 31);
}

inline bool
BABitvector::operator[](int i) const {
    assert(i >= 0 && i <= _max);
    return (_data[i >> 5] & (1U << (i & 31))) != 0;
}

inline BABitvector::Bit
BABitvector::force_bit(int i) {
    assert(i >= 0);
    if (i > _max)
        resize(i + 1);
    return Bit(_data[i >> 5], i & 31);
}

inline BABitvector &
        BABitvector::operator-=(const BABitvector &o) {
    return *this &= ~o;
}

inline BABitvector
BABitvector::operator~() const {
    BABitvector m = *this;
    m.negate();
    return m;
}

inline BABitvector
BABitvector::operator&(const BABitvector &o) const {
    BABitvector m = *this;
    m &= o;
    return m;
}

inline BABitvector
BABitvector::operator|(const BABitvector &o) const {
    BABitvector m = *this;
    m |= o;
    return m;
}

inline BABitvector
BABitvector::operator^(const BABitvector &o) const {
    BABitvector m = *this;
    m ^= o;
    return m;
}

inline BABitvector
BABitvector::operator-(const BABitvector &o) const {
    return *this & ~o;
}

inline void click_swap(BABitvector &a, BABitvector &b) {
    a.swap(b);
}

CLICK_ENDDECLS
#endif

