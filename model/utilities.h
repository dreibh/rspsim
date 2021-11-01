/* --------------------------------------------------------------------------
 *
 *              //===//   //=====   //===//   //=====  //   //      //
 *             //    //  //        //    //  //       //   //=/  /=//
 *            //===//   //=====   //===//   //====   //   //  //  //
 *           //   \\         //  //             //  //   //  //  //
 *          //     \\  =====//  //        =====//  //   //      //  Version V
 *
 * ------------- An Open Source RSerPool Simulation for OMNeT++ -------------
 *
 * Copyright (C) 2003-2012 by Thomas Dreibholz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: dreibh@iem.uni-due.de
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <omnetpp.h>
#include <sys/time.h>

#include "messages_m.h"
#include "debug.h"


using namespace omnetpp;


#define handleUnexpectedMsg(msg) error("Received unexpected message %s", msg->getName())
#define handleUnexpectedMsgState(msg, fsm) error("Received unexpected message %s in state %s", msg->getName(), (fsm).getStateName())

#define handleIgnore(description, msg, state) \
   EV << (description) << "Ignoring " << (msg)->getName() << " from " \
      << ((SimplePacket*)(msg))->getSrcAddress() << ":" \
      << ((SimplePacket*)(msg))->getSrcPort() << " in state " \
      << (state).getStateName() << endl;


int getLocalAddress(cModule* module);
opp_string format(const char* formatString, ...);


// ###### Get current system time in microseconds ###########################
inline unsigned long long getMicroTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return(((unsigned long long)tv.tv_sec * (unsigned long long)1000000) + (unsigned long long)tv.tv_usec);
}


// ###### Colorize a module #################################################
inline void colorizeModule(cModule* module, const char* colorString = "")
{
   module->getDisplayString().setTagArg("i", 1, colorString);
}


// ###### WeightedStdDev class ##############################################
class WeightedStdDev : public cStdDev
{
   public:
   WeightedStdDev() : cStdDev(nullptr, true) {};
};

#endif
