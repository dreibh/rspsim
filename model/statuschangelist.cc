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

#include <omnetpp.h>

#include "utilities.h"
#include "statuschangelist.h"


// ###### Constructor #######################################################
StatusChangeList::StatusChangeList()
{
   FirstStatusChange = NULL;
   LastStatusChange  = NULL;
   TimeBase          = 0.0;
}


// ###### Destructor ########################################################
StatusChangeList::~StatusChangeList()
{
   StatusChange* statusChange = FirstStatusChange;
   while(statusChange != NULL) {
      StatusChange* currentStatusChange = statusChange;
      statusChange = statusChange->Next;;
      delete currentStatusChange;
   }
   FirstStatusChange = NULL;
   LastStatusChange  = NULL;
}


// ###### Setup list ########################################################
void StatusChangeList::setup(const char* statusChangesList)
{
   while(*statusChangesList != 0x00) {
      statusChangesList = handleCommand(statusChangesList);
      while( (*statusChangesList == ' ') ||
             (*statusChangesList == ',') ) {
        statusChangesList++;
      }
   }
}


// ###### Print list ########################################################
void StatusChangeList::print()
{
   StatusChange* statusChange = FirstStatusChange;
   EV << "StatusChangeList:" << endl;
   while(statusChange != NULL) {
      EV << format(" - %6.6f %s\n", statusChange->Time.dbl(), statusChange->Command);
      statusChange = statusChange->Next;
   }
}


// ###### Read command and add it to list ###################################
const char* StatusChangeList::handleCommand(const char* p)
{
   StatusChange* statusChange = new StatusChange;
   statusChange->Next = NULL;

   while(*p == ' ') p++;
   size_t i = 0;
   while( (i < sizeof(statusChange->Command) - 1) &&  (*p >= 'a') &&  (*p <= 'z') ) {
      statusChange->Command[i++] = *p++;
   }
   statusChange->Command[i] = 0x00;

   if(*p++ != '@') {
      throw new cRuntimeError("Syntax error in statusChangesList parameter at \"%s\" - expected @-symbol!", p);
   }

   bool isAbsolute = true;
   if(*p == '+') {
      isAbsolute = false;
      p++;
   }

   int    n;
   double t;
   if(sscanf(p, "%lf%n", &t , &n) == 1) {
      statusChange->Time = (simtime_t)t;
      p = (const char*)&p[n];
   }
   else {
      throw new cRuntimeError("Syntax error in statusChangesList parameter at \"%s\" - expected value!", p);
      exit(1);
   }

   if(!isAbsolute) {
      TimeBase += statusChange->Time;
      statusChange->Time = TimeBase;
   }
   else {
      if(TimeBase > statusChange->Time) {
         throw new cRuntimeError("Time inconsistency in statusChangesList parameter at \"%s\" - expected value!", p);
      }
      TimeBase = statusChange->Time;
   }

   if(FirstStatusChange == NULL) {
      FirstStatusChange = statusChange;
   }
   if(LastStatusChange) {
      LastStatusChange->Next = statusChange;
   }
   LastStatusChange = statusChange;

   return(p);
}


// ###### Get time stamp for next action ####################################
simtime_t StatusChangeList::getNextActionTime(const simtime_t now) const
{
   if(FirstStatusChange) {
      return(std::max(0.0, FirstStatusChange->Time.dbl() - now.dbl()));
   }
   return(-1.0);
}


// ###### Pop top action ####################################################
StatusChange* StatusChangeList::pop()
{
   StatusChange* statusChange = FirstStatusChange;
   if(statusChange) {
      FirstStatusChange = statusChange->Next;
      if(FirstStatusChange == NULL) {
         LastStatusChange = NULL;
      }
   }
   return(statusChange);
}


// ###### Get time stamp for next expected action ###########################
simtime_t StatusChangeList::expectedAction(const simtime_t now, const char* command)
{
   if(FirstStatusChange) {
      if(!(strcmp(FirstStatusChange->Command, command))) {
         const simtime_t time = std::max(0.0, FirstStatusChange->Time.dbl() - now.dbl());
         delete pop();
         return(time);
      }
      throw cRuntimeError("Expected action \"%s\" but got action \"%s\"!\n",
                       command, FirstStatusChange->Command);
   }
   return(-1.0);
}
