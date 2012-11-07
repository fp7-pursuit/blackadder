/*-
 * Copyright (C) 2012  Mobile Multimedia Laboratory, AUEB
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

#ifndef SUBSCRIBER_HPP
#define SUBSCRIBER_HPP

#include "BaseEP.hpp"
#include "iostream"
#include <string>

class Subscriber:public CommChannel{
		private:
			BaseEP *ba;
		public: 
			/**
			 * It stores if the subscription is a channel or not
			 */
			 bool isChannel;
			  
			bool IHaveFinish; //just a hack to finish normally
			/**
			 * Inherited from CommChannel
			 */ 
			void fromLowerLayer(RVEvent &ev);
			/**
			 * It publishes the a publication by setting the ISP Id
			 * @param payload The ISP id
			 */  
			void startSubscribe(std::string payload);
};
#endif
