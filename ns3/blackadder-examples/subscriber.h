/*
 * Copyright (C) 2010-2012  George Parisis and Dirk Trossen
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

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/blackadder-module.h>

namespace ns3 {

    /**@brief A sample subscriber application. As all Blackadder NS3 applications, it must extend the PubSubApplication Class.
     * 
     */
    class Subscriber : public PubSubApplication {
    public:
        /**@brief As mandated by NS3..no attributes.
         */
        static TypeId GetTypeId(void);
        /**@brief Constructor: it initializes the single NS3 event in the code. The next time (and the only one) at which the subscriber will subscribe to root scope 00000000000.
         */
        Subscriber();
        virtual ~Subscriber();
        /**@brief The EventListener that is registered with the ServiceModel Class. It asynchronously called when a Blackadder Event is received.
         * It just prints the type of the Event.
         * 
         * @param ev a smart pointer to the received Event.
         */
        void EventHandler(Ptr<Event> ev);
    protected:
        void DoStart(void);
        virtual void DoDispose(void);
    private:
        virtual void StartApplication(void);
        virtual void StopApplication(void);

        void ScheduleTransmit(Time dt);
        void Send(void);

        Time m_interval;
        EventId m_Event;
    };

} // namespace ns3

#endif
