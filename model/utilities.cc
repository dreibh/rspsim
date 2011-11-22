/* $Id$
 * --------------------------------------------------------------------------
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

#include "utilities.h"
#include "messages_m.h"


// ###### Get local node's transport address #################################
int getLocalAddress(cModule* module)
{
   cGate* destination = module->gate("toTransport")->getPathEndGate();
   if(destination) {
      cModule* destinationModule = destination->getOwnerModule();
      const int localAddress = destinationModule->par("interfaceAddress");
      return(localAddress);
   }
   return(0);
}


// ###### Return sprintf-formatted string ####################################
opp_string format(const char* formatString, ...)
{
   char    str[1024];
   va_list args;
   va_start(args, formatString);
   vsnprintf((char*)&str, sizeof(str), formatString, args);
   return(opp_string(str));
}
