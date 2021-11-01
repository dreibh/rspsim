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
 * Copyright (C) 2003-2022 by Thomas Dreibholz
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

#ifndef STATUSCHANGELIST_H
#define STATUSCHANGELIST_H

#include <omnetpp.h>

#include "utilities.h"


struct StatusChange {
   StatusChange* Next;
   char          Command[16];
   simtime_t     Time;
};

class StatusChangeList
{
   public:
   StatusChangeList();
   ~StatusChangeList();

   // ====== Methods ========================================================
   void setup(const char* statusChangesList);
   void print();
   simtime_t getNextActionTime(const simtime_t now) const;
   StatusChange* pop();
   simtime_t expectedAction(const simtime_t now, const char* command);

   // ====== Private data ===================================================
   private:
   const char* handleCommand(const char* p);

   StatusChange* FirstStatusChange;
   StatusChange* LastStatusChange;
   simtime_t     TimeBase;
};

#endif
