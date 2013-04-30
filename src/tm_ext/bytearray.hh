/*
 * Copyright (C) 2012-2013  Andreas Bontozoglou
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

#ifndef BYTEARRAY_H 
#define BYTEARRAY_H

//#if !CLICK_LINUXMODULE && !CLICK_BSDMODULE
#include <inttypes.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string.h>
#include <float.h>
//endif

#include <stdio.h>

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/element.hh>
#include <click/glue.hh>
#include <click/string.hh>

// XXX: Should use Click's String instead of std::string

typedef uint8_t byte;

CLICK_DECLS

class ByteArray{
  private:
    uint64_t size;
    union DblConv{
      double d;
      byte bytes[sizeof(double)];
    };
  
  public:
    byte * data;
    
    ByteArray();
    ByteArray(const ByteArray &ba);
    ByteArray(uint64_t len);
    ByteArray(byte * cont, uint64_t len);
    ~ByteArray();
    
    // BASICS 
    uint64_t getSize();
    void clear();
    bool isEmpty();
    
    void zero();
    void one();
    void zero(uint64_t from, uint64_t to);
    
    void extend(uint64_t ext_size);
    void pretend(uint64_t ext_size);
    
    // BIT OPERATIONS
    static void showbits(uint64_t n, uint8_t len);
    static uint64_t getLowMask(uint8_t num);
    void setBits(byte b, uint8_t lownum, uint64_t offset_bytes, uint8_t offsetbits);
    void setBits(uint64_t b, uint8_t lownum, uint64_t offset_bytes, uint8_t offsetbits);
    void setBytes(byte * b, uint32_t num, uint64_t offset_bytes, uint8_t offsetbits);
    
    
    
    uint64_t getBits(uint8_t num, uint64_t offset_bytes, uint8_t offsetbits);
    void getBits(byte * b,uint8_t num, uint64_t offset_bytes, uint8_t offsetbits);
    byte getBits8_ORLESS(uint8_t num, uint64_t offset_bytes, uint8_t offsetbits);
    
    // VARIABLEs OPs
    void setBits(const std::string &str, uint64_t offset_bytes, uint8_t offsetbits);
    std::string getStrBits(uint8_t num, uint64_t offset_bytes, uint8_t offsetbits);
    
    // TODO: Add setBitsJava(double d....)
    /**
     * On the Java side, ByteArrayOutputStream used... for some reason Java 
     * has the bytes reversed on the doubles (Endian mismatch):
     * ie.
     * 0 0 0 0 0 0 0 1 (Java) = 1 0 0 0 0 0 0 0 (C/C++)
     */
    void setBits(double d, uint64_t offset_bytes, uint8_t offsetbits);
    double getDblBits(uint64_t offset_bytes, uint8_t offsetbits);
    double getDblBitsFromJava(uint64_t offset_bytes, uint8_t offsetbits);
    
    // OPERATORS
    ByteArray & operator=(const ByteArray &ba);
    const ByteArray operator+(const ByteArray &ba) const;
    bool operator==(const ByteArray &ba) const;
    bool operator!=(const ByteArray &ba) const;
    friend std::ostream & operator<<(std::ostream &out, const ByteArray &ba);
    
    
    
  public:
    void static copy(byte *d1,byte *d2, uint64_t d1_len);
    void static copy(byte *d1,byte *d2, uint64_t d1_len, uint64_t offset);
    
    static const ByteArray fromHexString(std::string);
    
};
CLICK_ENDDECLS
#endif
