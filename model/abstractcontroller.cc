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

#include <omnetpp.h>

#include "utilities.h"
#include "abstractcontroller.h"


// ###### Constructor #######################################################
AbstractController::AbstractController()
{
}


// ###### Destructor ########################################################
AbstractController::~AbstractController()
{
}


// ###### Initialize ########################################################
void AbstractController::initialize()
{
   GlobalUtilizationStat       = new cStdDev("GlobalUtilizationStat");
   GlobalJobQueuingDelayStat   = new cStdDev("GlobalJobQueuingDelayStat");
   GlobalJobStartupDelayStat   = new cStdDev("GlobalJobStartupDelayStat");
   GlobalJobProcessingTimeStat = new cStdDev("GlobalJobProcessingTimeStat");
   GlobalJobHandlingTimeStat   = new cStdDev("GlobalJobHandlingTimeStat");
   OPP_CHECK(GlobalUtilizationStat);
   OPP_CHECK(GlobalJobQueuingDelayStat);
   OPP_CHECK(GlobalJobStartupDelayStat);
   OPP_CHECK(GlobalJobProcessingTimeStat);
   OPP_CHECK(GlobalJobHandlingTimeStat);

   resetStatistics();
}


// ###### Clean-up ###########################################################
void AbstractController::finish()
{
   delete GlobalJobHandlingTimeStat;
   GlobalJobHandlingTimeStat = NULL;
   delete GlobalJobProcessingTimeStat;
   GlobalJobProcessingTimeStat = NULL;
   delete GlobalJobStartupDelayStat;
   GlobalJobStartupDelayStat = NULL;
   delete GlobalJobQueuingDelayStat;
   GlobalJobQueuingDelayStat = NULL;
   delete GlobalUtilizationStat;
   GlobalUtilizationStat = NULL;
}


// ###### Reset statistics ##################################################
void AbstractController::resetStatistics()
{
   GlobalUsedCapacity                  = 0.0;
   GlobalWastedCapacity                = 0.0;

   GlobalENRPPackets                   = 0;
   GlobalASAPPackets                   = 0;
   GlobalCookies                       = 0;

   GlobalStartupsWithMentor            = 0;
   GlobalStartupsAlone                 = 0;
   GlobalRegistrations                 = 0;
   GlobalDeregistrations               = 0;
   GlobalEndpointKeepAlivesSent        = 0;
   GlobalEndpointKeepAliveAcksReceived = 0;
   GlobalEndpointKeepAliveTimeouts     = 0;
   GlobalLifetimeExpiries              = 0;
   GlobalHandleResolutions             = 0;
   GlobalRefusedHandleResolutions      = 0;
   GlobalEndpointUnreachables          = 0;
   GlobalRefusedEndpointUnreachables   = 0;
   GlobalHandleUpdates                 = 0;
   GlobalRequestedPresences            = 0;
   GlobalPeerListRequests              = 0;
   GlobalHandleTableRequests           = 0;
   GlobalTakeoversStarted              = 0;
   GlobalTakeoversByConsent            = 0;
   GlobalTakeoversByTimeout            = 0;

   GlobalJobQueueLength                = 0;
   GlobalJobsQueued                    = 0;
   GlobalJobsStarted                   = 0;
   GlobalJobsCompleted                 = 0;
   GlobalJobSizeQueued                 = 0.0;
   GlobalJobSizeStarted                = 0.0;
   GlobalJobSizeCompleted              = 0.0;

   GlobalJobStartupDelay               = 0.0;
   GlobalJobQueuingDelay               = 0.0;
   GlobalJobProcessingTime             = 0.0;
   GlobalJobHandlingTime               = 0.0;
   GlobalJobGoodputTime                = 0.0;
   GlobalJobFailoverTime               = 0.0;

   GlobalUtilizationStat->clear();
   GlobalJobQueuingDelayStat->clear();
   GlobalJobStartupDelayStat->clear();
   GlobalJobProcessingTimeStat->clear();
   GlobalJobHandlingTimeStat->clear();

   GlobalServerSelections              = 0;
   GlobalUnreachables                  = 0;
   GlobalFailovers                     = 0;
   GlobalCalcAppRequests               = 0;
   GlobalCalcAppCookieEchoes           = 0;
   GlobalCalcAppAccepts                = 0;
   GlobalCalcAppRejects                = 0;

   GlobalAttackerIgnoredApplicationMessages = 0;
}


// ###### Get AbstractController singleton instance #########################
AbstractController* AbstractController::getController()
{
   cTopology topology;
   topology.extractByNedTypeName(cStringTokenizer("Controller").asVector());
   if(topology.getNumNodes() == 0) {
      return(NULL);
   }

   OPP_CHECK(topology.getNumNodes() == 1);
   AbstractController* controller = dynamic_cast<AbstractController*>(
                                       topology.getNode(0)->getModule());
   return(controller);
}
