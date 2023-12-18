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
 * Copyright (C) 2003-2024 by Thomas Dreibholz
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
 * Contact: thomas.dreibholz@gmail.com
 */

#ifndef ABSTRACTCONTROLLER_H
#define ABSTRACTCONTROLLER_H

#include <omnetpp.h>

#include "utilities.h"


#define SCALAR_OUTPUT_VERSION 1000UL

class AbstractController
{
   public:
   AbstractController();
   virtual ~AbstractController();
   static AbstractController* getController();

   void initialize();
   void finish();
   void resetStatistics();


   public:
   // ====== Transport Node =================================================
   // Outgoing packets from application layer into the network
   unsigned long long GlobalENRPPackets;
   unsigned long long GlobalASAPPackets; // without ASAP_COOKIE/ASAP_COOKIE_ECHO
   unsigned long long GlobalCookies;     // only ASAP_COOKIE

   // ====== Registrar Statistics ===========================================
   unsigned long long GlobalStartupsWithMentor;
   unsigned long long GlobalStartupsAlone;
   unsigned long long GlobalRegistrations;
   unsigned long long GlobalDeregistrations;
   unsigned long long GlobalEndpointKeepAlivesSent;
   unsigned long long GlobalEndpointKeepAliveAcksReceived;
   unsigned long long GlobalEndpointKeepAliveTimeouts;
   unsigned long long GlobalLifetimeExpiries;
   unsigned long long GlobalHandleResolutions;
   unsigned long long GlobalRefusedHandleResolutions;
   unsigned long long GlobalEndpointUnreachables;
   unsigned long long GlobalRefusedEndpointUnreachables;
   unsigned long long GlobalHandleUpdates;
   unsigned long long GlobalRequestedPresences;
   unsigned long long GlobalPeerListRequests;
   unsigned long long GlobalHandleTableRequests;
   unsigned long long GlobalTakeoversStarted;
   unsigned long long GlobalTakeoversByConsent;
   unsigned long long GlobalTakeoversByTimeout;

   // ====== Pool Element Statistics ========================================
   double             GlobalUsedCapacity;
   double             GlobalWastedCapacity;

   // ====== Pool User Statistics ===========================================
   unsigned int       GlobalJobQueueLength;

   unsigned long long GlobalJobsQueued;
   unsigned long long GlobalJobsStarted;
   unsigned long long GlobalJobsCompleted;
   double             GlobalJobSizeQueued;
   double             GlobalJobSizeStarted;
   double             GlobalJobSizeCompleted;

   simtime_t          GlobalJobQueuingDelay;
   simtime_t          GlobalJobStartupDelay;
   simtime_t          GlobalJobProcessingTime;
   simtime_t          GlobalJobHandlingTime;
   simtime_t          GlobalJobGoodputTime;
   simtime_t          GlobalJobFailoverTime;

   cStdDev*           GlobalUtilizationStat;
   cStdDev*           GlobalJobQueuingDelayStat;
   cStdDev*           GlobalJobStartupDelayStat;
   cStdDev*           GlobalJobProcessingTimeStat;
   cStdDev*           GlobalJobHandlingTimeStat;

   unsigned long long GlobalServerSelections; // PU requests one entry
   unsigned long long GlobalUnreachables;
   unsigned long long GlobalFailovers;
   unsigned long long GlobalCalcAppRequests;
   unsigned long long GlobalCalcAppCookieEchoes;
   unsigned long long GlobalCalcAppAccepts;
   unsigned long long GlobalCalcAppRejects;

   // ====== Attacker Statistics ============================================
   unsigned long long GlobalAttackerIgnoredApplicationMessages;
};


#endif
