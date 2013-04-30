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
#include "bytearray.hh" 

using namespace std;

CLICK_DECLS

// CONSTRUCTORS 
// ------------------------------------------------
ByteArray::ByteArray(){
  data=0;
  size=0;
}

ByteArray::ByteArray(const ByteArray &ba){
  size=ba.size;
  data=new byte[size];
  
  for (uint64_t i=0; i<size; i++)
    data[i]=ba.data[i];
}

ByteArray::ByteArray(uint64_t len){
  size=len;
  data=new byte[size];
  zero();
}


ByteArray::ByteArray(byte * cont, uint64_t len){
  size=len;
  data=new byte[size];
  
  for (uint64_t i=0; i<size; i++)
    data[i]=cont[i];
}

ByteArray::~ByteArray(){
  if (data)
    delete [] data;
  size=0;
}


// BASICS
// ------------------------------------------------
uint64_t 
ByteArray::getSize(){return size;}

void 
ByteArray::clear(){
  size=0;
  delete [] data;
}

bool 
ByteArray::isEmpty(){
  return (size==0);
}

void 
ByteArray::zero(){
  for (uint64_t i=0; i<size; i++)
    data[i]=0;
}

void 
ByteArray::one(){
  for (uint64_t i=0; i<size; i++)
    data[i]=1;
}


void 
ByteArray::zero(uint64_t from, uint64_t to){
  
  // Swap
  if (from > to) {
    uint64_t tmp=to;
    to=from;
    from=tmp;
  }
  
  // Checks
  if (to > size) return;
  
  for (uint64_t i=from; i<to; i++)
    data[i]=0;
}

void 
ByteArray::copy(byte *d1,byte *d2, uint64_t d1_len){
  for (uint64_t i=0; i<d1_len; i++)
    d2[i]=d1[i];
}

void 
ByteArray::copy(byte *d1,byte *d2, uint64_t d1_len, uint64_t offset){
  for (uint64_t i=offset; i<d1_len; i++)
    d2[i]=d1[i-offset];
}

void 
ByteArray::extend(uint64_t ext_size){
  
//   // Copy old to temp
//   byte tmp[size];
//   if (size>0){
//     copy(data, tmp, size);
//     
//     delete [] data;
//   }
//   
//   data = new byte[size+ext_size];
//   
//   // Copy old back
//   if (size>0)
//     copy(tmp, data, size);
//   
//   zero(size,size+ext_size);
//   
//   size+=ext_size;
  
  byte * newdata = new byte[size+ext_size];
  
  // Copy old to temp
  if (size>0){
    copy(data, newdata, size);
    delete [] data;
  }
  
  data = newdata;
  size+=ext_size;
}

void 
ByteArray::pretend(uint64_t ext_size){
  byte tmp[size];
  copy(data, tmp, size);
  
  delete [] data;
  data = new byte[size+ext_size];
  
  copy(tmp, data, size, ext_size);
  zero(0, ext_size);
  
  size+=ext_size;
}




// BIT OPERATIONS
// ------------------------------------------------
void 
ByteArray::showbits(uint64_t B, uint8_t len)
{
  int16_t i ;
  uint64_t k , mask;

  for( i =len-1 ; i >= 0 ; i--)
  {
     mask = 1L << i;
     k = B & mask;
     if( k == 0)
        cout<<"0";
     else
        cout<<"1";
     
     if (i%8 == 0)
       cout<<" ";
  }
  cout<<" ";
}

uint64_t 
ByteArray::getLowMask(uint8_t num){
  if (num>64) return 0x00;
  uint64_t res=0;
  for (uint8_t i=0; i<num; i++){
    uint8_t bit=1;
    bit<<=i;
    res|=bit;
  }
  
  return res;
}


void 
ByteArray::setBits(byte b, uint8_t lownum, uint64_t offset_bytes, uint8_t offsetbits){
  // Check bits
  if (lownum>8 || offsetbits>8) return;
  if (offset_bytes>size-1) return;
  // Check
  if (lownum/8+offset_bytes > size) return;
  
  // Ensure 0z
  uint8_t mask=getLowMask(lownum); 
  b&=mask;
				// ex. 0000 0101 lownum=3 mask: 0000 0111
  b<<=(8-lownum); 		// 1010 0000
  mask<<=(8-lownum); 		// 1110 0000
  
  if (lownum+offsetbits<=8){
    b>>=offsetbits;		// offset=3 -> 0010 1000
    mask>>=offsetbits;		// offset=3 -> 0011 1000
    mask=~mask;			// 1100 0111
    
    // Zero these bits
    data[offset_bytes] &=mask;
    // Set them back
    data[offset_bytes] |=b;
  }
  else{
    uint16_t b2 = b;		// 0 0 1010 0000
    uint16_t mask2=mask;	// 0 0 1110 0000
    
    
    b2<<=8;			// 1010 0000 0000 0000
    mask2<<=8;			// 1110 0000 0000 0000
    
    
    b2>>=offsetbits;		// offset=7 -> 0000 0010 1000 0000
    mask2>>=offsetbits;		// offset=7 -> 0000 0011 1000 0000
    mask2=~mask2;		// 1111 1100 0111 1111
    
    uint8_t mask2a = mask2>>8;		// 1111 1100
    uint8_t mask2b = mask2&0x00FF;	// 0111 1111
    
    uint8_t b2a = b2>>8;		// 0000 0010
    uint8_t b2b = b2&0x00FF;		// 1000 0000
    
    // Zero bits
    data[offset_bytes]   &=mask2a;
    data[offset_bytes+1] &=mask2b;
    
    // Set them back
    data[offset_bytes]   |=b2a;
    data[offset_bytes+1] |=b2b;
  }
}

void 
ByteArray::setBits(uint64_t b, uint8_t lownum, uint64_t offset_bytes, uint8_t offsetbits){
  
  // Check
  if (lownum/8+offset_bytes > size) return;
  
  uint8_t to=((lownum % 8 == 0) ? (lownum/8) : (lownum/8 + 1));
  uint8_t from=(8 - to);
  for (uint8_t i=from; i<8; i++){
    if (i==0 && lownum%8 > 0){
      uint8_t bits=lownum%8;
      byte cur=(byte)(b>>((to-1)*8));
      setBits(cur, bits, offset_bytes, offsetbits);
      offsetbits+=bits;
      if (offsetbits>8) {
	offset_bytes++;
	offsetbits=offsetbits%8;
      }

      continue;
    }
    

    uint64_t tmp_b=b<<(i*8);
    byte cur=(byte)(tmp_b>>((8-1)*8));
    setBits(cur, (byte) 8, offset_bytes, offsetbits);
    offset_bytes++;
  }
}

void 
ByteArray::setBytes(byte * b, uint32_t num, uint64_t offset_bytes, uint8_t offsetbits){
  
  // Check
  if (num+offset_bytes > size) return;
  
  
  
  for (uint32_t i=0; i<num; i++){
    setBits(b[i], 8, offset_bytes, offsetbits);
    offset_bytes++;
  }
}


//
// Get-------
//
//

byte 
ByteArray::getBits8_ORLESS(uint8_t num, uint64_t offset_bytes, uint8_t offsetbits){
  byte res=0;
  // Check
  if (num > 8 ) return res;
  
  if (num+offsetbits<=8){
    
    res=data[offset_bytes];
    res<<=offsetbits;
    res>>=(8-num);
    
  }else{
    
    res=data[offset_bytes];	// ex. offset=4, num=6, 1111 1001 | 1101 1111 NOTE: VERY BAD EXAMPLE
    res<<=offsetbits;		// 1001 0000
    res>>=(offsetbits);		// 0000 1001

    res<<=(num-(8-offsetbits));	// 0010 0100
    res|= (data[offset_bytes+1]>>(16-num-offsetbits)); 	// 1101 1111 >> 6 = 0000 0011
								// res = 0010 0111
    
  }
  
  return res;
}

uint64_t 
ByteArray::getBits(uint8_t num, uint64_t offset_bytes, uint8_t offsetbits){
  uint64_t res=0x00L;
  uint8_t to=((num % 8 == 0) ? (num/8) : (num/8 + 1));
  
  for (uint8_t i=0; i<to; i++){
    
    // Not full byte...
    if (i==0 && num%8 > 0){
      uint8_t bits=num%8;
      res=getBits8_ORLESS(bits, offset_bytes, offsetbits);
      offsetbits+=bits;

      if (offsetbits>8) {
	offset_bytes++;
	offsetbits=offsetbits%8;
      }
      
      continue;
    }

    
    // Full Byte here
    res<<=8;
    res|=getBits8_ORLESS(8, offset_bytes, offsetbits);
    offset_bytes++;
    
  }
  
  return res;
}

void 
ByteArray::getBits(byte * b, uint8_t num, uint64_t offset_bytes, uint8_t offsetbits){

  uint8_t to=((num % 8 == 0) ? (num/8) : (num/8 + 1));
  // Check
  if (num/8+offset_bytes > size) return;
  
  for (uint8_t i=0; i<to; i++){
    
    // Not full byte...
    if (i==0 && num%8 > 0){
      uint8_t bits=num%8;
      b[i]=getBits8_ORLESS(bits, offset_bytes, offsetbits);
      offsetbits+=bits;

      if (offsetbits>8) {
	offset_bytes++;
	offsetbits=offsetbits%8;
      }
      continue;
    }

    
    // Full Byte here
    b[i]=getBits8_ORLESS(8, offset_bytes, offsetbits);
    offset_bytes++;
    
  }
  
}

// VARIABLE
// --------------------------------------------------
void 
ByteArray::setBits(const string &str, uint64_t offset_bytes, uint8_t offsetbits){
  
  // Check 
  if (str.size()+offset_bytes > size) return;
  
  for (uint8_t i=0; i<str.size(); i++){
    
    byte cur=(byte)str[i];
    setBits(cur, 8, offset_bytes, offsetbits);
    offset_bytes++;
  }
}

string 
ByteArray::getStrBits(uint8_t num, uint64_t offset_bytes, uint8_t offsetbits){
  if (num+offset_bytes > size) return "";
  string res="";
  
  for (uint8_t i=0; i<num; i++){  
    // Full Byte here
    res+=(char)getBits8_ORLESS(8, offset_bytes, offsetbits);
    offset_bytes++;
    
  }
  
  return res;
}

void 
ByteArray::setBits(double d, uint64_t offset_bytes, uint8_t offsetbits){
  // Check
  if (sizeof(d)+offset_bytes > size) return;
  
  DblConv dc;
  dc.d=d;
  
  setBytes(dc.bytes, sizeof(d), offset_bytes, offsetbits);
  
}

double 
ByteArray::getDblBits(uint64_t offset_bytes, uint8_t offsetbits){
  
  uint8_t num=sizeof(double);
  
  if (sizeof(num)+offset_bytes > size) return DBL_MAX;
  byte b[num];
  
  DblConv dc;
  dc.d=0.0;
  
  getBits(b, num*8, offset_bytes, offsetbits);
  
  for (int i=0; i<num; i++){
    dc.bytes[i]=b[i];
  }
  
  return dc.d;
}


double 
ByteArray::getDblBitsFromJava(uint64_t offset_bytes, uint8_t offsetbits){
  
  uint8_t num=sizeof(double);
  
  if (sizeof(num)+offset_bytes > size) return DBL_MAX;
  byte b[num];
  
  DblConv dc;
  dc.d=0.0;
  
  getBits(b, num*8, offset_bytes, offsetbits);
  
  for (int i=0; i<num; i++){
    dc.bytes[7-i]=b[i];
  }
  
  return dc.d;
}



const ByteArray 
ByteArray::fromHexString(std::string hexstr){
  // every 2 chars in string is one array entry
  uint32_t items = hexstr.length() / 2;

  // create array
  uint8_t* strdata = new uint8_t[items];

  for(uint32_t i = 0; i < items; ++i)
  {
	  // extract each hex pair as a substring
	  std::string val = hexstr.substr(i * 2, 2);

	  // convert hex string into unsigned int (uint)
	  // and store in the array
	  //std::istringstream(val) >> std::hex >> strdata[i];
	  
	  sscanf (val.c_str(),"%X",(unsigned int *) &strdata[i]);
	  
// 	  cout<<val<< std::hex<<"  0x"<<(int)strdata[i]<< std::dec<<" - "<<strdata[i]<<" ->";
// 	  showbits(strdata[i],8);
// 	  cout<<endl;
  }

  ByteArray res(strdata,items);
  
  // free memory for array
  delete[] strdata;
  
  return res;
}


// OPERATORS
// ------------------------------------------------
ByteArray &
ByteArray::operator=(const ByteArray &ba){
  if (data)
    delete [] data;
  
  size=ba.size;
  data=new byte[size];
  
  for (uint64_t i=0; i<size; i++)
    data[i]=ba.data[i];
  
  return *this;
}


const ByteArray
ByteArray::operator+(const ByteArray &ba) const{
  
  ByteArray result = *this;
  if (&ba==NULL) return result;
  
  int size_old = size;
  result.extend(ba.size);
  
  for (uint64_t i=0; i<ba.size; i++)
    result.data[size_old+i]=ba.data[i];
  
  return result;
  
}

bool
ByteArray::operator==(const ByteArray &ba) const{
  
  if (size!=ba.size) return false;
  
  for (uint64_t i=0; i<size; i++){
    if (data[i]!=ba.data[i]) 
      return false;
  }
  
  return true;
}

bool
ByteArray::operator!=(const ByteArray &ba) const{
 
  return !(*this == ba);
}

ostream & operator<<(ostream &out, const ByteArray &ba){
  cout.setf ( ios::hex, ios::basefield );
  for (uint64_t i=0; i<ba.size; i++){
    out<<"0x"<<hex;
    if (ba.data[i]< 16) out<<0;
    
    out<<(int)ba.data[i]<<" ";
  }
  out<<dec<<endl;
  
  return out;
}


CLICK_ENDDECLS
ELEMENT_REQUIRES(userlevel)
ELEMENT_PROVIDES(ByteArray)
