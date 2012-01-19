#include "ba_bitvector.hh"

CLICK_DECLS

void
BABitvector::finish_copy_constructor(const BABitvector &o) {
    int nn = max_word();
    _data = new uint32_t[nn + 1];
    for (int i = 0; i <= nn; i++)
        _data[i] = o._data[i];
}

void
BABitvector::clear() {
    int nn = max_word();
    for (int i = 0; i <= nn; i++)
        _data[i] = 0;
}

bool
BABitvector::zero() const {
    int nn = max_word();
    for (int i = 0; i <= nn; i++)
        if (_data[i])
            return false;
    return true;
}

void
BABitvector::resize_to_max(int new_max, bool valid_n) {
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
BABitvector::clear_last() {
    if (unlikely(_max < 0))
        _data[0] = 0;
    else if ((_max & 0x1F) != 0x1F) {
        uint32_t mask = (1U << ((_max & 0x1F) + 1)) - 1;
        _data[_max >> 5] &= mask;
    }
}

BABitvector &
        BABitvector::operator=(const BABitvector &o) {
#if CLICK_LINUXMODULE || CLICK_BSDMODULE
    // We might not have been initialized properly.
    if (!_data)
        _max = -1, _data = &_f0;
#endif
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

BABitvector &
BABitvector::assign(int n, bool value) {
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
BABitvector::negate() {
    int nn = max_word();
    uint32_t *data = _data;
    for (int i = 0; i <= nn; i++)
        data[i] = ~data[i];
    clear_last();
}

BABitvector &
        BABitvector::operator&=(const BABitvector &o) {
    assert(o._max == _max);
    int nn = max_word();
    uint32_t *data = _data, *o_data = o._data;
    for (int i = 0; i <= nn; i++)
        data[i] &= o_data[i];
    return *this;
}

BABitvector &
        BABitvector::operator|=(const BABitvector &o) {
    if (o._max > _max)
        resize(o._max + 1);
    int nn = max_word();
    uint32_t *data = _data, *o_data = o._data;
    for (int i = 0; i <= nn; i++)
        data[i] |= o_data[i];
    return *this;
}

BABitvector &
        BABitvector::operator^=(const BABitvector &o) {
    assert(o._max == _max);
    int nn = max_word();
    uint32_t *data = _data, *o_data = o._data;
    for (int i = 0; i <= nn; i++)
        data[i] ^= o_data[i];
    return *this;
}

void
BABitvector::offset_or(const BABitvector &o, int offset) {
    assert(offset >= 0 && offset + o._max <= _max);
    uint32_t bits_1st = offset & 0x1F;
    int my_pos = offset >> 5;
    int o_pos = 0;
    int my_max_word = max_word();
    int o_max_word = o.max_word();
    uint32_t *data = _data;
    const uint32_t *o_data = o._data;
    assert((o._max < 0 && o_data[0] == 0) || (o._max & 0x1F) == 0 || (o_data[o_max_word] & ((1U << ((o._max & 0x1F) + 1)) - 1)) == o_data[o_max_word]);

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
BABitvector::or_with_difference(const BABitvector &o, BABitvector &diff) {
    assert(o._max == _max);
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
BABitvector::nonzero_intersection(const BABitvector &o) const {
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
BABitvector::swap(BABitvector &x) {
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

String BABitvector::to_string() {
    String res;
    for (int i = 0; i < size(); i++) {
        if (BABitvector::Bit::unspecified_bool_type((*this)[size() - i - 1])) {
            res += '1';
        } else {
            res += '0';
        }
    }
    return String(res);
}

CLICK_ENDDECLS

ELEMENT_PROVIDES(BABitvector)
ELEMENT_PROVIDES(Bit)