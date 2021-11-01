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
#include <fstream>
#include <time.h>
#include "messages_m.h"

#include "abstractcontroller.h"
#include "statisticswriterinterface.h"
#include "utilities.h"


class Controller : public AbstractController,
                   public cSimpleModule
{
   // ====== Methods =========================================================
   virtual void initialize();
   virtual void finish();
   virtual void handleMessage(cMessage* msg);

   void writeStatistics();

   // ====== Variables =======================================================
   private:
   unsigned long long StartTime;
   cMessage*          ResetStatisticsTimer;
   cMessage*          WriteStatisticsTimer;
};

Define_Module(Controller);


// ###### Constructor ########################################################
void Controller::initialize()
{
   AbstractController::initialize();

   StartTime = getMicroTime();
   ResetStatisticsTimer = NULL;
   WriteStatisticsTimer = NULL;

   const simtime_t statisticsResetTime = (simtime_t)par("statisticsResetTime");
   if(statisticsResetTime >= 0.0) {
      EV << "Reset statistics at: " << statisticsResetTime.dbl() << "s" << endl;
      ResetStatisticsTimer = new cMessage("ResetStatisticsMessage");
      scheduleAt(statisticsResetTime, ResetStatisticsTimer);
   }

   const simtime_t statisticsWriteTime = (simtime_t)par("statisticsWriteTime");
   if(statisticsWriteTime >= 0.0) {
      EV << "Write statistics at: " << statisticsWriteTime.dbl() << "s" << endl;
      WriteStatisticsTimer = new cMessage("WriteStatisticsMessage");
      scheduleAt(statisticsWriteTime, WriteStatisticsTimer);
   }
}


// ###### Clean-up ###########################################################
void Controller::finish()
{
   AbstractController::finish();
}


// ###### Destructor #########################################################
void Controller::writeStatistics()
{
   recordScalar("Version", SCALAR_OUTPUT_VERSION);

   recordScalar("Simulation Run CPU Time",
                (double)rint(getMicroTime() - StartTime) / 1000000.0);

   recordScalar("TransportNode Global ASAP Packets",                  GlobalASAPPackets);
   recordScalar("TransportNode Global ENRP Packets",                  GlobalENRPPackets);
   recordScalar("TransportNode Global Cookies",                       GlobalCookies);

   recordScalar("Registrar Global Startups With Mentor",              GlobalStartupsWithMentor);
   recordScalar("Registrar Global Startups Alone",                    GlobalStartupsAlone);
   recordScalar("Registrar Global Registrations",                     GlobalRegistrations);
   recordScalar("Registrar Global Deregistrations",                   GlobalDeregistrations);
   recordScalar("Registrar Global Endpoint Keep Alives Sent",         GlobalEndpointKeepAlivesSent);
   recordScalar("Registrar Global Endpoint Keep Alive Acks Received", GlobalEndpointKeepAliveAcksReceived);
   recordScalar("Registrar Global Endpoint Keep Alive Timeouts",      GlobalEndpointKeepAliveTimeouts);
   recordScalar("Registrar Global Lifetime Expiries",                 GlobalLifetimeExpiries);
   recordScalar("Registrar Global Handle Resolutions",                GlobalHandleResolutions);
   recordScalar("Registrar Global Refused Handle Resolutions",        GlobalRefusedHandleResolutions);
   recordScalar("Registrar Global Endpoint Unreachables",             GlobalEndpointUnreachables);
   recordScalar("Registrar Global Refused Endpoint Unreachables",     GlobalRefusedEndpointUnreachables);
   recordScalar("Registrar Global Handle Updates",                    GlobalHandleUpdates);
   recordScalar("Registrar Global Requested Presences",               GlobalRequestedPresences);
   recordScalar("Registrar Global Peer List Requests",                GlobalPeerListRequests);
   recordScalar("Registrar Global Handle Table Requests",             GlobalHandleTableRequests);
   recordScalar("Registrar Global Takeovers Started",                 GlobalTakeoversStarted);
   recordScalar("Registrar Global Takeovers By Consent",              GlobalTakeoversByConsent);
   recordScalar("Registrar Global Takeovers By Timeout",              GlobalTakeoversByTimeout);

   recordScalar("CalcAppPE Global Provided Capacity",                 GlobalUsedCapacity + GlobalWastedCapacity);
   recordScalar("CalcAppPE Global Used Capacity",                     GlobalUsedCapacity);
   recordScalar("CalcAppPE Global Wasted Capacity",                   GlobalWastedCapacity);

   recordScalar("CalcAppPU Global Job Queue Length",    GlobalJobQueueLength);
   recordScalar("CalcAppPU Global Jobs Queued",         GlobalJobsQueued);
   recordScalar("CalcAppPU Global Jobs Started",        GlobalJobsStarted);
   recordScalar("CalcAppPU Global Jobs Completed",      GlobalJobsCompleted);
   recordScalar("CalcAppPU Global Job Size Queued",     GlobalJobSizeQueued);
   recordScalar("CalcAppPU Global Job Size Started",    GlobalJobSizeStarted);
   recordScalar("CalcAppPU Global Job Size Completed",  GlobalJobSizeCompleted);

   recordScalar("CalcAppPU Global Job Queuing Delay",   GlobalJobQueuingDelay);
   recordScalar("CalcAppPU Global Job Startup Delay",   GlobalJobStartupDelay);
   recordScalar("CalcAppPU Global Job Processing Time", GlobalJobProcessingTime);
   recordScalar("CalcAppPU Global Job Handling Time",   GlobalJobHandlingTime);
   recordScalar("CalcAppPU Global Job Goodput Time",    GlobalJobGoodputTime);
   recordScalar("CalcAppPU Global Job Failover Time",   GlobalJobFailoverTime);

   recordScalar("CalcAppPU Global Server Selections",   GlobalServerSelections);
   recordScalar("CalcAppPU Global Unreachables",        GlobalUnreachables);
   recordScalar("CalcAppPU Global Failovers",           GlobalFailovers);
   recordScalar("CalcAppPU Global CalcAppRequests",     GlobalCalcAppRequests);
   recordScalar("CalcAppPU Global CalcAppCookieEchoes", GlobalCalcAppCookieEchoes);
   recordScalar("CalcAppPU Global CalcAppAccepts",      GlobalCalcAppAccepts);
   recordScalar("CalcAppPU Global CalcAppRejects",      GlobalCalcAppRejects);


   recordScalar("System Utilization",
                (GlobalUsedCapacity > 0) ? (GlobalUsedCapacity / (GlobalUsedCapacity + GlobalWastedCapacity)) : 0.0);
   recordScalar("System Average Overhead Per Calculation",
                (GlobalJobSizeCompleted > 0.0) ? ((GlobalASAPPackets + GlobalENRPPackets) / GlobalJobSizeCompleted) : -1.0);
   recordScalar("System Average Startup Speed",
                (GlobalJobStartupDelay > 0) ? GlobalJobSizeCompleted / GlobalJobStartupDelay.dbl() : 0.0);
   recordScalar("System Average Processing Speed",
                (GlobalJobProcessingTime > 0) ? GlobalJobSizeCompleted / GlobalJobProcessingTime.dbl() : 0.0);
   recordScalar("System Average Handling Speed",
                (GlobalJobHandlingTime > 0) ? GlobalJobSizeCompleted / GlobalJobHandlingTime.dbl() : 0.0);

   /*
      IMPORTANT: System Utilization is:
                    GlobalUsedCapacity / (GlobalUsedCapacity + GlobalWastedCapacity)
                 System Average Utilization is:
                    The *unweighted* average over all PE utilizations!
                    This leads to strange results in case of heterogeneous capacities!
   */

   recordScalar("System Average Utilization",               GlobalUtilizationStat->getMean());
   recordScalar("System Average Utilization Min",           GlobalUtilizationStat->getMin());
   recordScalar("System Average Utilization Max",           GlobalUtilizationStat->getMax());
   recordScalar("System Average Utilization Deviation",     GlobalUtilizationStat->getStddev());
   recordScalar("System Average Utilization Variance",      GlobalUtilizationStat->getVariance());
   recordScalar("System Average Utilization CV",            GlobalUtilizationStat->getStddev() * 100.0 / GlobalUtilizationStat->getMean());
   recordScalar("System Average Utilization Samples",       GlobalUtilizationStat->getCount());

   recordScalar("System Average Queuing Delay",             GlobalJobQueuingDelayStat->getMean());
   recordScalar("System Average Queuing Delay Min",         GlobalJobQueuingDelayStat->getMin());
   recordScalar("System Average Queuing Delay Max",         GlobalJobQueuingDelayStat->getMax());
   recordScalar("System Average Queuing Delay Deviation",   GlobalJobQueuingDelayStat->getStddev());
   recordScalar("System Average Queuing Delay Variance",    GlobalJobQueuingDelayStat->getVariance());
   recordScalar("System Average Queuing Delay CV",          GlobalJobQueuingDelayStat->getStddev() * 100.0 / GlobalJobQueuingDelayStat->getMean());
   recordScalar("System Average Queuing Delay Samples",     GlobalJobQueuingDelayStat->getCount());

   recordScalar("System Average Startup Delay",             GlobalJobStartupDelayStat->getMean());
   recordScalar("System Average Startup Delay Min",         GlobalJobStartupDelayStat->getMin());
   recordScalar("System Average Startup Delay Max",         GlobalJobStartupDelayStat->getMax());
   recordScalar("System Average Startup Delay Deviation",   GlobalJobStartupDelayStat->getStddev());
   recordScalar("System Average Startup Delay Variance",    GlobalJobStartupDelayStat->getVariance());
   recordScalar("System Average Startup Delay CV",          GlobalJobStartupDelayStat->getStddev() * 100.0 / GlobalJobStartupDelayStat->getMean());
   recordScalar("System Average Startup Delay Samples",     GlobalJobStartupDelayStat->getCount());

   recordScalar("System Average Processing Time",           GlobalJobProcessingTimeStat->getMean());
   recordScalar("System Average Processing Time Min",       GlobalJobProcessingTimeStat->getMin());
   recordScalar("System Average Processing Time Max",       GlobalJobProcessingTimeStat->getMax());
   recordScalar("System Average Processing Time Deviation", GlobalJobProcessingTimeStat->getStddev());
   recordScalar("System Average Processing Time Variance",  GlobalJobProcessingTimeStat->getVariance());
   recordScalar("System Average Processing Time CV",        GlobalJobProcessingTimeStat->getStddev() * 100.0 / GlobalJobProcessingTimeStat->getMean());
   recordScalar("System Average Processing Time Samples",   GlobalJobProcessingTimeStat->getCount());

   recordScalar("System Average Handling Time",             GlobalJobHandlingTimeStat->getMean());
   recordScalar("System Average Handling Time Min",         GlobalJobHandlingTimeStat->getMin());
   recordScalar("System Average Handling Time Max",         GlobalJobHandlingTimeStat->getMax());
   recordScalar("System Average Handling Time Deviation",   GlobalJobHandlingTimeStat->getStddev());
   recordScalar("System Average Handling Time Variance",    GlobalJobHandlingTimeStat->getVariance());
   recordScalar("System Average Handling Time CV",          GlobalJobHandlingTimeStat->getStddev() * 100.0 / GlobalJobHandlingTimeStat->getMean());
   recordScalar("System Average Handling Time Samples",     GlobalJobHandlingTimeStat->getCount());

   recordScalar("Global Attacker Ignored Application Messages", GlobalAttackerIgnoredApplicationMessages);
}


// ###### Handle timer message ###############################################
void Controller::handleMessage(cMessage* msg)
{
   EV << "Controller: Received message \"" << msg->getName() << "\"" << endl;

   const std::vector<std::string> statisticsForModules = cStringTokenizer("RegistrarProcess CalcAppServerProcess CalcAppQueuingClientProcess AttackerProcess TransportNode").asVector();

   if(msg == ResetStatisticsTimer) {
      ResetStatisticsTimer = NULL;
      EV << "--- Reset Statistics ---" << endl;

      // Reset statistics of all modules
      cTopology topology;
      topology.extractByNedTypeName(statisticsForModules);
      for(int i = 0;i < topology.getNumNodes();i++) {
         StatisticsWriterInterface* swi = dynamic_cast<StatisticsWriterInterface*>(topology.getNode(i)->getModule());
         if(swi) {
            swi->resetStatistics();
         }
      }

      // Finally, reset Controller's global statistics
      resetStatistics();
   }
   else if(msg == WriteStatisticsTimer) {
      WriteStatisticsTimer = NULL;
      EV << "--- Writing Statistics ---" << endl;

      // Write statistics of all modules (except the Controller!)
      cTopology topology;
      topology.extractByNedTypeName(statisticsForModules);
      const opp_string prefix = format("%f ", simTime().dbl());
      for(int i = 0;i < topology.getNumNodes();i++) {
         StatisticsWriterInterface* swi = dynamic_cast<StatisticsWriterInterface*>(topology.getNode(i)->getModule());
         if(swi) {
            // printf("<%s>\n",topology.getNode(i)->getModule()->getName());
            swi->writeStatistics();
            // puts("---");
         }
      }

      // Finally, write Controller's global statistics
      writeStatistics();
   }

   delete msg;
}
