/*-
 * Copyright (C) 2012  Oy L M Ericsson Ab, NomadicLab
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

/**
 * @file ba_fnv.hpp
 * @brief Header file for some FNV hash functions.
 *        See Blackadder (Related Functions).
 *
 * See: http://isthe.com/chongo/tech/comp/fnv/
 */

/* Idea of using FNV in this context by Dimitris Syrivelis. */

#ifndef BA_FNV_HPP
#define BA_FNV_HPP

#include <algorithm>
#include <string>

typedef unsigned long long Fnv64_t;

/** @brief Computes the 64-bit FNV-1 hash of data. */
static inline Fnv64_t
fnv1_64(const unsigned char *data, unsigned int data_len)
{
    Fnv64_t hash = (Fnv64_t)14695981039346656037ULL;
    for (unsigned int i = 0; i < data_len; ++i) {
        hash *= (Fnv64_t)1099511628211ULL;
        hash ^= (Fnv64_t)data[i];
    }
    return (Fnv64_t)hash;
}

/** @brief Computes the 64-bit FNV-1a hash of data. */
static inline Fnv64_t
fnv1a_64(const unsigned char *data, unsigned int data_len)
{
    Fnv64_t hash = (Fnv64_t)14695981039346656037ULL;
    for(unsigned int i = 0; i < data_len; ++i) {
	hash ^= (Fnv64_t)data[i];
	hash *= (Fnv64_t)1099511628211ULL;
    }
    return (Fnv64_t)hash;
}

/** @brief Returns a hash integer as a char pointer. Note byte order issues. */
static inline char *
fnv_chars(Fnv64_t *hash)
{
    return (char *)hash;
}

/**
 * @relates Blackadder
 * @brief Computes the 64-bit FNV-1 hash of a string. Useful for generating
 *        identifiers matching PURSUIT_ID_LEN = 8.
 */
static inline std::string
fnv1_64_str(const std::string &data)
{
    Fnv64_t hash = fnv1_64((unsigned char *)data.c_str(), data.length());
    std::string result = std::string(fnv_chars(&hash), sizeof(hash));
    std::reverse(result.begin(), result.end());
    return result;
}

/**
 * @relates Blackadder
 * @brief Computes the 64-bit FNV-1a hash of a string. Useful for generating
 *        identifiers matching PURSUIT_ID_LEN = 8.
 */
static inline std::string
fnv1a_64_str(const std::string &data)
{
    Fnv64_t hash = fnv1a_64((unsigned char *)data.c_str(), data.length());
    std::string result = std::string(fnv_chars(&hash), sizeof(hash));
    std::reverse(result.begin(), result.end());
    return result;
}

#endif /* BA_FNV_HPP */
