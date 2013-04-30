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

#include "ba_api.hh"

CLICK_DECLS

WritablePacket * bapi_publish_scope(String prefix, String id, uint8_t strategy){
  return bapi_makepacket(PUBLISH_SCOPE, prefix, id, strategy);
}

WritablePacket * bapi_publish_info(String prefix, String id, uint8_t strategy){
  return bapi_makepacket(PUBLISH_INFO, prefix, id, strategy);
}

WritablePacket * bapi_unpublish_scope(String prefix, String id, uint8_t strategy){
  return bapi_makepacket(UNPUBLISH_SCOPE, prefix, id, strategy);
}

WritablePacket * bapi_unpublish_info(String prefix, String id, uint8_t strategy){
  return bapi_makepacket(UNPUBLISH_INFO, prefix, id, strategy);
}

WritablePacket * bapi_makepacket(uint8_t type,String prefix, String id, uint8_t strategy){
  // Sizes
  unsigned char id_len = id.length()/PURSUIT_ID_LEN;
  unsigned char prefix_len = prefix.length()/PURSUIT_ID_LEN;

  WritablePacket *p = Packet::make(64,NULL,sizeof (type)+sizeof (id_len)+sizeof (prefix_len)+id.length()+prefix.length()+sizeof (strategy),0);
  
  memcpy(p->data(), &type, sizeof (type));
  
  if (prefix_len > 0){
    memcpy(p->data() + sizeof (type), &prefix_len, sizeof (prefix_len));
    memcpy(p->data() + sizeof (type) + sizeof (prefix_len), prefix.c_str(), prefix.length());
  
    memcpy(p->data() + sizeof (type) + sizeof (prefix_len) + prefix.length(),  &id_len, sizeof (id_len));
    memcpy(p->data() + sizeof (type) + sizeof (prefix_len) + prefix.length() + sizeof (id_len), id.c_str(), id.length());
  }else{
    memcpy(p->data() + sizeof (type), &id_len, sizeof (id_len));
    memcpy(p->data() + sizeof (type) + sizeof (id_len),  id.c_str(), id.length());
    memcpy(p->data() + sizeof (type) + sizeof (id_len) + id.length(), &prefix_len, sizeof (prefix_len));
  }
  
  memcpy(p->data() + sizeof (type) + sizeof (id_len) + id.length() + sizeof (prefix_len) + prefix.length(), &strategy, sizeof (strategy));
  
  
  
  
  
  return p;
}

WritablePacket * bapi_publish_data(String prefix, String id, uint8_t strategy, char * data, uint16_t datalen){
  // Sizes
  unsigned char id_len = (prefix.length()+id.length())/PURSUIT_ID_LEN;
  unsigned char type = PUBLISH_DATA;
  id=prefix+id;
  
  WritablePacket *p = Packet::make(45,NULL,sizeof (type)+sizeof (id_len)+id.length()+sizeof (strategy)+datalen,0);

  memcpy(p->data(), &type, sizeof (type));
  memcpy(p->data() + sizeof (type), &id_len, sizeof (id_len));
  memcpy(p->data() + sizeof (type) + sizeof (id_len),  id.c_str(), id.length());
  memcpy(p->data() + sizeof (type) + sizeof (id_len) + id.length() , &strategy, sizeof (strategy));
  
  
  if (data && datalen>0)
    memcpy(p->data() + sizeof (type) + sizeof (id_len) + id.length() + sizeof (strategy),data,datalen);
  
  return p;
}
CLICK_ENDDECLS
ELEMENT_REQUIRES(userlevel)
/* Actually this file compiles in the kernel as well. */
ELEMENT_PROVIDES(BA_API)
