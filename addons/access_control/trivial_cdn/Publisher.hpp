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

#ifndef PUBLISHER_HPP
#define PUBLISHER_HPP

#include "BaseEP.hpp"
#include "iostream"
#include <string>

class Publisher:public CommChannel{
		private:
			BaseEP *ba;
		public: 
			bool IHaveFinish; //just a hack to finish normally
			/**
			 * It publishes the a publication by setting the ISP Id
			 * @param payload The ISP id
			 */ 
			void startPulish(std::string payload);
			/**
			 * Inherited from CommChannel
			 */
			void fromLowerLayer(RVEvent &ev);
};
#endif
