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
#include "messages_m.h"

#include "statisticswriterinterface.h"
#include "abstractcontroller.h"
#include "utilities.h"
#include "handlespacemanagementwrapper.h"
#include "calcappqueuingclientmessages_m.h"

#undef min
#undef max


class CalcAppQueuingClientJob : public CalcAppQueuingClientJob_Base
{
   public:
   CalcAppQueuingClientJob() {
   }
   CalcAppQueuingClientJob(const CalcAppQueuingClientJob& other) : CalcAppQueuingClientJob_Base() {
      operator=(other);
   }
   CalcAppQueuingClientJob& operator=(const CalcAppQueuingClientJob& other) {
      CalcAppQueuingClientJob_Base::operator=(other); return(*this);
   }
   virtual cOwnedObject* dup() {
      return new CalcAppQueuingClientJob(*this);
   }
};

Register_Class(CalcAppQueuingClientJob);



class CalcAppQueuingClientProcess : public StatisticsWriterInterface,
                                    public cSimpleModule
{
   // ====== Methods ========================================================
   virtual void initialize();
   virtual void finish();
   virtual void handleMessage(cMessage* msg);
   virtual void resetStatistics();
   virtual void writeStatistics();

   void startNewJob();


   private:
   // ====== States =========================================================
   enum {
      INIT                               = 0,

      START_NEXT_JOB_TIMER               = FSM_Transient(1),
      WAIT_FOR_NEXT_JOB                  = FSM_Steady(2),

      START_NEW_JOB                      = FSM_Transient(3),

      SEND_SERVER_SELECTION_REQUEST      = FSM_Transient(4),
      WAIT_FOR_SERVER_SELECTION_RESPONSE = FSM_Steady(5),
      START_SERVER_SELECTION_RETRY_TIMER = FSM_Transient(6),
      WAIT_FOR_SERVER_SELECTION_RETRY    = FSM_Steady(7),

      SEND_JOB_REQUEST                   = FSM_Transient(8),
      WAIT_FOR_JOB_RESPONSE              = FSM_Steady(9),
      SEND_KEEP_ALIVE                    = FSM_Steady(10),
      FINISH_JOB                         = FSM_Transient(11),

      SEND_ENDPOINT_UNREACHABLE          = FSM_Transient(12),
      SEND_CACHE_PURGE                   = FSM_Transient(13),
      WAIT_FOR_JOB_RETRY                 = FSM_Steady(14),

      FINISHED                           = FSM_Steady(15)
   };
   cFSM State;


   // ====== CalcApp Timers =================================================
   void startServerSelectionRetryTimer();
   void stopServerSelectionRetryTimer();
   void startJobRetryTimer(simtime_t timeout = -1.0);
   void stopJobRetryTimer();
   void startJobRequestTimeoutTimer();
   void stopJobRequestTimeoutTimer();
   void startJobKeepAliveTimeoutTimer();
   void stopJobKeepAliveTimeoutTimer();
   void startJobKeepAliveTransmissionTimer();
   void stopJobKeepAliveTransmissionTimer();
   void startJobGeneratorTimer();
   void stopJobGeneratorTimer();
   void handleJobGeneratorTimer();

   // ====== CalcApp Protocol ===============================================
   void sendServerSelectionRequest();
   void handleServerSelectionSuccess(ServerSelectionSuccess* msg);
   bool handleCalcAppAccept(CalcAppAccept* msg);
   bool handleCalcAppReject(CalcAppReject* msg);
   bool handleCalcAppKeepAliveAck(CalcAppKeepAliveAck* msg);
   bool handleCalcAppKeepAlive(CalcAppKeepAlive* msg);
   bool handleCalcAppCookie(CalcAppCookie* msg);
   bool handleCalcAppComplete(CalcAppComplete* msg);
   bool handleCalcAppAbort(CalcAppAbort* msg);
   void sendCalcAppRequest();
   void sendCalcAppCookieEcho();
   void sendCalcAppKeepAlive();
   void sendEndpointUnreachable();
   void sendCachePurge();


   // ====== Parameters ======================================================
   unsigned int             Jobs;
   unsigned int             LastJobID;

   // ====== Timers ==========================================================
   cMessage*                StartupTimer;
   cMessage*                ServerSelectionRetryTimer;
   cMessage*                JobRetryTimer;
   cMessage*                JobRequestTimeoutTimer;
   cMessage*                JobKeepAliveTransmissionTimer;
   cMessage*                JobKeepAliveTimeoutTimer;
   cMessage*                JobGeneratorTimer;


   // ====== Variables ======================================================
   cQueue                   JobQueue;

   CalcAppQueuingClientJob* CurrentJob;
   cPoolElementParameter    CurrentPoolElement;
   opp_string               CurrentPoolElementDescription;
   bool                     HasCookie;
   CalcAppCookieParameter   Cookie;

   unsigned int             TotalJobsQueued;       // Jobs put into queue
   unsigned int             TotalJobsStarted;      // Jobs taken from queue
   unsigned int             TotalJobsCompleted;    // Jobs completed
   double                   TotalJobSizeQueued;
   double                   TotalJobSizeStarted;
   double                   TotalJobSizeCompleted;
   simtime_t                TotalJobRuntime;

   simtime_t                TotalJobQueuingDelay;
   simtime_t                TotalJobStartupDelay;
   simtime_t                TotalJobProcessingTime;
   simtime_t                TotalJobHandlingTime;
   simtime_t                TotalJobGoodputTime;   // CalcAppAccept -> CalcAppComplete, without failover time
   simtime_t                TotalJobFailoverTime;  // Failover time between CalcAppAccept -> CalcAppComplete

   unsigned int             TotalServerSelections;
   unsigned int             TotalUnreachables;
   unsigned int             TotalJobFailovers;
   unsigned int             TotalCalcAppRequests;
   unsigned int             TotalCalcAppCookieEchoes;
   unsigned int             TotalCalcAppAccepts;
   unsigned int             TotalCalcAppRejects;

   cStdDev*                 QueuingDelayStat;
   cStdDev*                 StartupDelayStat;
   cStdDev*                 ProcessingTimeStat;
   cStdDev*                 HandlingTimeStat;

   cOutVector*              QueueLengthVector;
   cOutVector*              QueuingDelayVector;
   cOutVector*              StartupDelayVector;
   cOutVector*              ProcessingTimeVector;
   cOutVector*              HandlingTimeVector;

   opp_string               Description;
};


Define_Module(CalcAppQueuingClientProcess);


// ###### Initialize ########################################################
void CalcAppQueuingClientProcess::initialize()
{
   // ------ Initialize variables -------------------------------------------
   State.setName("State");
   Description  = format("CalcAppQueuingClientProcess at %u:%u> ",
                         getLocalAddress(this), CalcAppClientPort);

   if(((opp_string)((const char*)par("servicePoolHandle"))).empty()) {
      throw new cRuntimeError("Bad pool handle!");
   }

   JobQueue.setName("CalcAppQueuingClientJobQueue");
   Jobs                          = 0;
   LastJobID                     = 0;
   CurrentJob                    = NULL;
   HasCookie                     = false;

   StartupTimer                  = NULL;
   ServerSelectionRetryTimer     = NULL;
   JobRetryTimer                 = NULL;
   JobRequestTimeoutTimer        = NULL;
   JobKeepAliveTransmissionTimer = NULL;
   JobKeepAliveTimeoutTimer      = NULL;
   JobGeneratorTimer             = NULL;

   QueuingDelayStat   = new cStdDev("QueuingDelayStat");
   StartupDelayStat   = new cStdDev("StartupDelayStat");
   ProcessingTimeStat = new cStdDev("ProcessingTimeStat");
   HandlingTimeStat   = new cStdDev("HandlingTimeStat");
   OPP_CHECK(QueuingDelayStat);
   OPP_CHECK(StartupDelayStat);
   OPP_CHECK(HandlingTimeStat);
   OPP_CHECK(ProcessingTimeStat);

   QueueLengthVector    = new cOutVector("QueueLengthVector");
   QueuingDelayVector   = new cOutVector("QueuingDelayVector");
   StartupDelayVector   = new cOutVector("StartupDelayVector");
   ProcessingTimeVector = new cOutVector("ProcessingTimeVector");
   HandlingTimeVector   = new cOutVector("HandlingTimeVector");
   OPP_CHECK(QueueLengthVector);
   OPP_CHECK(QueuingDelayVector);
   OPP_CHECK(StartupDelayVector);
   OPP_CHECK(HandlingTimeVector);
   OPP_CHECK(ProcessingTimeVector);
   QueueLengthVector->record(0);

   // ------ Bind to port ---------------------------------------------------
   BindMessage* msg = new BindMessage("Bind");
   msg->setPort(CalcAppClientPort);
   send(msg, "toTransport");

   // ------ Prepare startup ------------------------------------------------
   if((unsigned int)par("serviceJobCount") > 0) {
      colorizeModule(getParentModule(), "#00ff00");
      StartupTimer = new cMessage("StartupTimer");
      const simtime_t startupDelay = (simtime_t)par("componentStartupDelay");
      scheduleAt(simTime() + startupDelay, StartupTimer);
      EV << Description << "Scheduled startup in " << startupDelay << "s" << endl;
   }
   else {
      FSM_Goto(State, FINISHED);
   }

   // ------ Reset statistics -----------------------------------------------
   resetStatistics();
}


// ###### Clean up ##########################################################
void CalcAppQueuingClientProcess::finish()
{
   if(CurrentJob) {
      delete CurrentJob;
      CurrentJob = NULL;
   }
   delete QueuingDelayStat;
   QueuingDelayStat = NULL;
   delete StartupDelayStat;
   StartupDelayStat = NULL;
   delete HandlingTimeStat;
   HandlingTimeStat = NULL;
   delete ProcessingTimeStat;
   ProcessingTimeStat = NULL;

   delete QueueLengthVector;
   QueueLengthVector = NULL;

   delete QueuingDelayVector;
   QueuingDelayVector = NULL;
   delete StartupDelayVector;
   StartupDelayVector = NULL;
   delete HandlingTimeVector;
   HandlingTimeVector = NULL;
   delete ProcessingTimeVector;
   ProcessingTimeVector = NULL;
}


// ###### Reset statistics ##################################################
void CalcAppQueuingClientProcess::resetStatistics()
{
   QueuingDelayStat->clear();
   StartupDelayStat->clear();
   ProcessingTimeStat->clear();
   HandlingTimeStat->clear();

   TotalJobsStarted         = 0;
   TotalJobsQueued          = 0;
   TotalJobsCompleted       = 0;
   TotalJobSizeQueued       = 0.0;
   TotalJobSizeStarted      = 0.0;
   TotalJobSizeCompleted    = 0.0;

   TotalJobQueuingDelay     = 0.0;
   TotalJobStartupDelay     = 0.0;
   TotalJobProcessingTime   = 0.0;
   TotalJobHandlingTime     = 0.0;
   TotalJobGoodputTime      = 0.0;
   TotalJobFailoverTime     = 0.0;

   TotalServerSelections    = 0;
   TotalUnreachables        = 0;
   TotalJobFailovers        = 0;
   TotalCalcAppRequests     = 0;
   TotalCalcAppCookieEchoes = 0;
   TotalCalcAppAccepts      = 0;
   TotalCalcAppRejects      = 0;
}


// ###### Write statistics ##################################################
void CalcAppQueuingClientProcess::writeStatistics()
{
   recordScalar("CalcAppPU Total Jobs Queued",         TotalJobsQueued);
   recordScalar("CalcAppPU Total Jobs Started",        TotalJobsStarted);
   recordScalar("CalcAppPU Total Jobs Completed",      TotalJobsCompleted);
   recordScalar("CalcAppPU Total Job Size Queued",     TotalJobSizeQueued);
   recordScalar("CalcAppPU Total Job Size Started",    TotalJobSizeStarted);
   recordScalar("CalcAppPU Total Job Size Completed",  TotalJobSizeCompleted);

   recordScalar("CalcAppPU Total Job Queuing Delay",   TotalJobQueuingDelay);
   recordScalar("CalcAppPU Total Job Startup Delay",   TotalJobStartupDelay);
   recordScalar("CalcAppPU Total Job Processing Time", TotalJobProcessingTime);
   recordScalar("CalcAppPU Total Job Handling Time",   TotalJobHandlingTime);
   recordScalar("CalcAppPU Total Job Goodput Time",    TotalJobGoodputTime);
   recordScalar("CalcAppPU Total Job Failover Time",   TotalJobFailoverTime);

   recordScalar("CalcAppPU Average Queuing Delay",             QueuingDelayStat->getMean());
   recordScalar("CalcAppPU Average Queuing Delay Min",         QueuingDelayStat->getMin());
   recordScalar("CalcAppPU Average Queuing Delay Max",         QueuingDelayStat->getMax());
   recordScalar("CalcAppPU Average Queuing Delay Deviation",   QueuingDelayStat->getStddev());
   recordScalar("CalcAppPU Average Queuing Delay Variance",    QueuingDelayStat->getVariance());
   recordScalar("CalcAppPU Average Queuing Delay CV",          QueuingDelayStat->getStddev() * 100.0 / QueuingDelayStat->getMean());
   recordScalar("CalcAppPU Average Queuing Delay Samples",     QueuingDelayStat->getCount());

   recordScalar("CalcAppPU Average Startup Delay",             StartupDelayStat->getMean());
   recordScalar("CalcAppPU Average Startup Delay Min",         StartupDelayStat->getMin());
   recordScalar("CalcAppPU Average Startup Delay Max",         StartupDelayStat->getMax());
   recordScalar("CalcAppPU Average Startup Delay Deviation",   StartupDelayStat->getStddev());
   recordScalar("CalcAppPU Average Startup Delay Variance",    StartupDelayStat->getVariance());
   recordScalar("CalcAppPU Average Startup Delay CV",          StartupDelayStat->getStddev() * 100.0 / StartupDelayStat->getMean());
   recordScalar("CalcAppPU Average Startup Delay Samples",     StartupDelayStat->getCount());

   recordScalar("CalcAppPU Average Processing Time",           ProcessingTimeStat->getMean());
   recordScalar("CalcAppPU Average Processing Time Min",       ProcessingTimeStat->getMin());
   recordScalar("CalcAppPU Average Processing Time Max",       ProcessingTimeStat->getMax());
   recordScalar("CalcAppPU Average Processing Time Deviation", ProcessingTimeStat->getStddev());
   recordScalar("CalcAppPU Average Processing Time Variance",  ProcessingTimeStat->getVariance());
   recordScalar("CalcAppPU Average Processing Time CV",        ProcessingTimeStat->getStddev() * 100.0 / ProcessingTimeStat->getMean());
   recordScalar("CalcAppPU Average Processing Time Samples",   ProcessingTimeStat->getCount());

   recordScalar("CalcAppPU Average Handling Time",             HandlingTimeStat->getMean());
   recordScalar("CalcAppPU Average Handling Time Min",         HandlingTimeStat->getMin());
   recordScalar("CalcAppPU Average Handling Time Max",         HandlingTimeStat->getMax());
   recordScalar("CalcAppPU Average Handling Time Deviation",   HandlingTimeStat->getStddev());
   recordScalar("CalcAppPU Average Handling Time Variance",    HandlingTimeStat->getVariance());
   recordScalar("CalcAppPU Average Handling Time CV",          HandlingTimeStat->getStddev() * 100.0 / HandlingTimeStat->getMean());
   recordScalar("CalcAppPU Average Handling Time Samples",     HandlingTimeStat->getCount());

   recordScalar("CalcAppPU Total Server Selections",   TotalServerSelections);
   recordScalar("CalcAppPU Total Unreachables",        TotalUnreachables);
   recordScalar("CalcAppPU Total Failovers",           TotalJobFailovers);
   recordScalar("CalcAppPU Total CalcAppRequests",     TotalCalcAppRequests);
   recordScalar("CalcAppPU Total CalcAppCookieEchoes", TotalCalcAppCookieEchoes);
   recordScalar("CalcAppPU Total CalcAppAccepts",      TotalCalcAppAccepts);
   recordScalar("CalcAppPU Total CalcAppRejects",      TotalCalcAppRejects);

   AbstractController* controller = AbstractController::getController();
   if(controller) {
      controller->GlobalJobQueueLength      += JobQueue.getLength();
      controller->GlobalJobsQueued          += TotalJobsQueued;
      controller->GlobalJobsStarted         += TotalJobsStarted;
      controller->GlobalJobsCompleted       += TotalJobsCompleted;
      controller->GlobalJobSizeQueued       += TotalJobSizeQueued;
      controller->GlobalJobSizeStarted      += TotalJobSizeStarted;
      controller->GlobalJobSizeCompleted    += TotalJobSizeCompleted;

      controller->GlobalJobQueuingDelay     += TotalJobQueuingDelay;
      controller->GlobalJobStartupDelay     += TotalJobStartupDelay;
      controller->GlobalJobProcessingTime   += TotalJobProcessingTime;
      controller->GlobalJobHandlingTime     += TotalJobHandlingTime;
      controller->GlobalJobGoodputTime      += TotalJobGoodputTime;
      controller->GlobalJobFailoverTime     += TotalJobFailoverTime;

      controller->GlobalServerSelections    += TotalServerSelections;
      controller->GlobalUnreachables        += TotalUnreachables;
      controller->GlobalFailovers           += TotalJobFailovers;
      controller->GlobalCalcAppRequests     += TotalCalcAppRequests;
      controller->GlobalCalcAppCookieEchoes += TotalCalcAppCookieEchoes;
      controller->GlobalCalcAppAccepts      += TotalCalcAppAccepts;
      controller->GlobalCalcAppRejects      += TotalCalcAppRejects;
   }
}


// ###### Start Handle Resolution Retry timer ###############################
void CalcAppQueuingClientProcess::startServerSelectionRetryTimer()
{
   OPP_CHECK(ServerSelectionRetryTimer == NULL);
   ServerSelectionRetryTimer = new cMessage("ServerSelectionRetryTimer");
   scheduleAt(simTime() + (simtime_t)par("serviceHandleResolutionRetryDelay"),
              ServerSelectionRetryTimer);
}


// ###### Stop Handle Resolution Retry timer ################################
void CalcAppQueuingClientProcess::stopServerSelectionRetryTimer()
{
   OPP_CHECK(ServerSelectionRetryTimer != NULL);
   delete cancelEvent(ServerSelectionRetryTimer);
   ServerSelectionRetryTimer = NULL;
}


// ###### Start Job Retry timer #############################################
void CalcAppQueuingClientProcess::startJobRetryTimer(simtime_t timeout)
{
   OPP_CHECK(JobRetryTimer == NULL);
   JobRetryTimer = new cMessage("JobRetryTimer");
   if(timeout < 0.0) {
      timeout = (simtime_t)par("serviceJobRetryDelay");
   }
   scheduleAt(simTime() + timeout, JobRetryTimer);
}


// ###### Stop Job Retry timer ##############################################
void CalcAppQueuingClientProcess::stopJobRetryTimer()
{
   OPP_CHECK(JobRetryTimer != NULL);
   delete cancelEvent(JobRetryTimer);
   JobRetryTimer = NULL;
}


// ###### Start Job Request Timeout timer ###################################
void CalcAppQueuingClientProcess::startJobRequestTimeoutTimer()
{
   OPP_CHECK(JobRequestTimeoutTimer == NULL);
   JobRequestTimeoutTimer = new cMessage("JobRequestTimeout");
   scheduleAt(simTime() + (simtime_t)par("serviceJobRequestTimeout"),
              JobRequestTimeoutTimer);
}


// ###### Stop Job Request Timeout timer ####################################
void CalcAppQueuingClientProcess::stopJobRequestTimeoutTimer()
{
   OPP_CHECK(JobRequestTimeoutTimer != NULL);
   delete cancelEvent(JobRequestTimeoutTimer);
   JobRequestTimeoutTimer = NULL;
}


// ###### Start Job KeepAlive Timeout timer #################################
void CalcAppQueuingClientProcess::startJobKeepAliveTimeoutTimer()
{
   OPP_CHECK(JobKeepAliveTimeoutTimer == NULL);
   JobKeepAliveTimeoutTimer = new cMessage("JobKeepAliveTimeout");
   scheduleAt(simTime() + (simtime_t)par("serviceJobKeepAliveTimeout"),
              JobKeepAliveTimeoutTimer);
}


// ###### Stop Job KeepAlive Timeout timer ##################################
void CalcAppQueuingClientProcess::stopJobKeepAliveTimeoutTimer()
{
   OPP_CHECK(JobKeepAliveTimeoutTimer != NULL);
   delete cancelEvent(JobKeepAliveTimeoutTimer);
   JobKeepAliveTimeoutTimer = NULL;
}


// ###### Start Job KeepAlive Transmission timer ############################
void CalcAppQueuingClientProcess::startJobKeepAliveTransmissionTimer()
{
   OPP_CHECK(JobKeepAliveTransmissionTimer == NULL);
   JobKeepAliveTransmissionTimer = new cMessage("JobKeepAliveTransmissionTimer");
   scheduleAt(simTime() + (simtime_t)par("serviceJobKeepAliveInterval"),
              JobKeepAliveTransmissionTimer);
}


// ###### Stop Job KeepAlive Transmission timer #############################
void CalcAppQueuingClientProcess::stopJobKeepAliveTransmissionTimer()
{
   OPP_CHECK(JobKeepAliveTransmissionTimer != NULL);
   delete cancelEvent(JobKeepAliveTransmissionTimer);
   JobKeepAliveTransmissionTimer = NULL;
}


// ###### Start Next Job timer ##############################################
void CalcAppQueuingClientProcess::startJobGeneratorTimer()
{
   OPP_CHECK(JobGeneratorTimer == NULL);
   JobGeneratorTimer = new cMessage("JobGeneratorTimer");
   const simtime_t jobInterval = par("serviceJobInterval");
   scheduleAt(simTime() + jobInterval, JobGeneratorTimer);
   EV << Description << "Waiting " << jobInterval << "s before starting next job ..." << endl;
}


// ###### Stop Next Job timer ###############################################
void CalcAppQueuingClientProcess::stopJobGeneratorTimer()
{
   OPP_CHECK(JobGeneratorTimer != NULL);
   delete cancelEvent(JobGeneratorTimer);
   JobGeneratorTimer = NULL;
}


// ###### Handle Next Job timer #############################################
void CalcAppQueuingClientProcess::handleJobGeneratorTimer()
{
   CalcAppQueuingClientJob* job = new CalcAppQueuingClientJob;
   OPP_CHECK(job != NULL);

   job->setJobID(++LastJobID);
   job->setJobQueuedAt(simTime());
   job->setJobStartedAt(-1.0);
   job->setJobAcceptedAt(-1.0);
   job->setJobCalculations((double)par("serviceJobSize"));
   job->setLastGoodputAt(-1.0);
   job->setLastGoodputCalculations(0);
   job->setTotalJobGoodputTime(0.0);
   job->setTotalJobFailoverTime(0.0);

   job->setServerSelections(0);
   job->setUnreachables(0);
   job->setFailovers(0);
   job->setCalcAppRequests(0);
   job->setCalcAppCookieEchoes(0);
   job->setCalcAppAccepts(0);
   job->setCalcAppRejects(0);

   JobQueue.insert(job);
   QueueLengthVector->record(JobQueue.getLength());
   TotalJobsQueued++;
   TotalJobSizeQueued += job->getJobCalculations();

   EV << Description << "Queued job #" << job->getJobID() << " with "
      << job->getJobCalculations() << " calculations" << endl;

   startJobGeneratorTimer();
}


// ###### Send ServerSelection request #####################################
void CalcAppQueuingClientProcess::sendServerSelectionRequest()
{
   ServerSelectionRequest* handleResolutionRequest = new ServerSelectionRequest("ServerSelectionRequest");
   handleResolutionRequest->setPoolHandle((const char*)par("servicePoolHandle"));
   send(handleResolutionRequest, "toASAP");
}


// ###### Handle ServerSelection response ##################################
void CalcAppQueuingClientProcess::handleServerSelectionSuccess(ServerSelectionSuccess* msg)
{
   CurrentPoolElement            = msg->getPoolElementParameter();

   // printf("PE: %u:%u\n",CurrentPoolElement.getUserTransportParameter().getAddress(),
   //                      CurrentPoolElement.getUserTransportParameter().getPort());

   CurrentPoolElementDescription = getPoolElementParameterDescription(CurrentPoolElement);
};


// ###### Send EndpointUnreachable ##########################################
void CalcAppQueuingClientProcess::sendEndpointUnreachable()
{
   EndpointUnreachable* endpointUnreachable = new EndpointUnreachable("EndpointUnreachable");
   endpointUnreachable->setPoolHandle((const char*)par("servicePoolHandle"));
   endpointUnreachable->setIdentifier(CurrentPoolElement.getIdentifier());
   send(endpointUnreachable, "toASAP");
}


// ###### Send CachePurge ###################################################
void CalcAppQueuingClientProcess::sendCachePurge()
{
   CachePurge* cachePurge = new CachePurge("CachePurge");
   cachePurge->setPoolHandle((const char*)par("servicePoolHandle"));
   cachePurge->setIdentifier(CurrentPoolElement.getIdentifier());
   send(cachePurge, "toASAP");
}


// ###### Send CALCAPP_REQUEST message (start job) ##########################
void CalcAppQueuingClientProcess::sendCalcAppRequest()
{
   CalcAppRequest* calcAppRequest = new CalcAppRequest("CALCAPP_REQUEST", CalcAppProtocol);
   calcAppRequest->setProtocol(CalcAppProtocol);
   calcAppRequest->setDstAddress(CurrentPoolElement.getUserTransportParameter().getAddress());
   calcAppRequest->setDstPort(CurrentPoolElement.getUserTransportParameter().getPort());
   calcAppRequest->setSrcPort(CalcAppClientPort);
   calcAppRequest->setJobID(CurrentJob->getJobID());
   calcAppRequest->setJobCalculations(CurrentJob->getJobCalculations());
   send(calcAppRequest, "toTransport");
}


// ###### Send ASAP_COOKIE_ECHO message (continue job) ######################
void CalcAppQueuingClientProcess::sendCalcAppCookieEcho()
{
   CalcAppCookieEcho* calcAppCookieEcho = new CalcAppCookieEcho("ASAP_COOKIE_ECHO", ASAP);
   calcAppCookieEcho->setProtocol(ASAP);
   calcAppCookieEcho->setDstAddress(CurrentPoolElement.getUserTransportParameter().getAddress());
   calcAppCookieEcho->setDstPort(CurrentPoolElement.getUserTransportParameter().getPort());
   calcAppCookieEcho->setSrcPort(CalcAppClientPort);
   calcAppCookieEcho->setCookie(Cookie);
   send(calcAppCookieEcho, "toTransport");
}


// ###### Send CALCAPP_KEEP_ALIVE message ###################################
void CalcAppQueuingClientProcess::sendCalcAppKeepAlive()
{
   CalcAppKeepAlive* calcAppKeepAlive = new CalcAppKeepAlive("CALCAPP_KEEP_ALIVE", CalcAppProtocol);
   calcAppKeepAlive->setProtocol(CalcAppProtocol);
   calcAppKeepAlive->setDstAddress(CurrentPoolElement.getUserTransportParameter().getAddress());
   calcAppKeepAlive->setDstPort(CurrentPoolElement.getUserTransportParameter().getPort());
   calcAppKeepAlive->setSrcPort(CalcAppClientPort);
   calcAppKeepAlive->setJobID(CurrentJob->getJobID());
   send(calcAppKeepAlive, "toTransport");
}


// ###### Handle CALCAPP_ACCEPT message #####################################
bool CalcAppQueuingClientProcess::handleCalcAppAccept(CalcAppAccept* msg)
{
   if((msg->getSrcAddress() == CurrentPoolElement.getUserTransportParameter().getAddress()) &&
      (msg->getSrcPort() == CurrentPoolElement.getUserTransportParameter().getPort()) &&
      (msg->getJobID() == CurrentJob->getJobID())) {
      EV << Description << "Job #" << CurrentJob->getJobID()
         << " accepted from pool element " << CurrentPoolElementDescription << endl;
      stopJobRequestTimeoutTimer();
      startJobKeepAliveTransmissionTimer();

      CurrentJob->setCalcAppAccepts(CurrentJob->getCalcAppAccepts() + 1);
      TotalCalcAppAccepts++;
      if(CurrentJob->getJobAcceptedAt() < 0.0) {
         CurrentJob->setJobAcceptedAt(simTime());
         const simtime_t startupDelay = CurrentJob->getJobAcceptedAt() - CurrentJob->getJobStartedAt();
         TotalJobStartupDelay += startupDelay;
         StartupDelayStat->collect(startupDelay);
         AbstractController* controller = AbstractController::getController();
         if(controller) {
            controller->GlobalJobStartupDelayStat->collect(startupDelay);
         }
      }
      else {
         const simtime_t failoverTime = simTime() - CurrentJob->getLastGoodputAt();
         CurrentJob->setTotalJobFailoverTime(CurrentJob->getTotalJobFailoverTime() +
                                             failoverTime);
         TotalJobFailoverTime += failoverTime;
      }
      CurrentJob->setLastGoodputAt(simTime());

      return(true);
   }
   else {
      EV << Description << "Ignoring CALCAPP_ACCEPT with invalid source or JobID" << endl;
      return(false);
   }
}


// ###### Handle CALCAPP_REJECT message #####################################
bool CalcAppQueuingClientProcess::handleCalcAppReject(CalcAppReject* msg)
{
   if((msg->getSrcAddress() == CurrentPoolElement.getUserTransportParameter().getAddress()) &&
      (msg->getSrcPort() == CurrentPoolElement.getUserTransportParameter().getPort()) &&
      (msg->getJobID() == CurrentJob->getJobID())) {
      EV << Description << "Job #" << CurrentJob->getJobID()
         << " rejected from pool element " << CurrentPoolElementDescription << endl;
      stopJobRequestTimeoutTimer();
      return(true);
   }
   else {
      EV << Description << "Ignoring CALCAPP_REJECT with invalid source or JobID" << endl;
      return(false);
   }
}


// ###### Handle CALCAPP_ABORT message ######################################
bool CalcAppQueuingClientProcess::handleCalcAppAbort(CalcAppAbort* msg)
{
   if((msg->getSrcAddress() == CurrentPoolElement.getUserTransportParameter().getAddress()) &&
      (msg->getSrcPort() == CurrentPoolElement.getUserTransportParameter().getPort()) &&
      (msg->getJobID() == CurrentJob->getJobID())) {
      EV << Description << "Job #" << CurrentJob->getJobID()
         << " aborted from pool element " << CurrentPoolElementDescription << endl;
      if(JobRequestTimeoutTimer) {
         stopJobRequestTimeoutTimer();
      }
      if(JobKeepAliveTimeoutTimer) {
         stopJobKeepAliveTimeoutTimer();
      }
      if(JobKeepAliveTransmissionTimer) {
         stopJobKeepAliveTransmissionTimer();
      }
      return(true);
   }
   else {
      EV << Description << "Ignoring CALCAPP_ABORT with invalid source or JobID" << endl;
      return(false);
   }
}


// ###### Handle CALCAPP_KEEP_ALIVE_ACK message #############################
bool CalcAppQueuingClientProcess::handleCalcAppKeepAliveAck(CalcAppKeepAliveAck* msg)
{
   if((msg->getSrcAddress() == CurrentPoolElement.getUserTransportParameter().getAddress()) &&
      (msg->getSrcPort() == CurrentPoolElement.getUserTransportParameter().getPort()) &&
      (msg->getJobID() == CurrentJob->getJobID())) {
      EV << Description << "JobKeepAliveAck for job #"
         << CurrentJob->getJobID() << " received -> pool element "
         << CurrentPoolElementDescription << " is still alive" << endl;
      stopJobKeepAliveTimeoutTimer();
      startJobKeepAliveTransmissionTimer();
      return(true);
   }
   else {
      EV << Description << "Ignoring CALCAPP_KEEP_ALIVE_ACK with invalid source or JobID" << endl;
      return(false);
   }
}


// ###### Handle CALCAPP_KEEP_ALIVE message #################################
bool CalcAppQueuingClientProcess::handleCalcAppKeepAlive(CalcAppKeepAlive* msg)
{
   if((msg->getSrcAddress() == CurrentPoolElement.getUserTransportParameter().getAddress()) &&
      (msg->getSrcPort() == CurrentPoolElement.getUserTransportParameter().getPort()) &&
      (msg->getJobID() == CurrentJob->getJobID())) {
      EV << Description << "JobKeepAlive for job #"
         << CurrentJob->getJobID() << " received from pool element "
         << CurrentPoolElementDescription << " -> sending CALCAPP_KEEP_ALIVE_ACK" << endl;
      CalcAppKeepAliveAck* keepAliveAck = new CalcAppKeepAliveAck("CALCAPP_KEEP_ALIVE_ACK", CalcAppProtocol);
      keepAliveAck->setProtocol(CalcAppProtocol);
      keepAliveAck->setDstAddress(CurrentPoolElement.getUserTransportParameter().getAddress());
      keepAliveAck->setDstPort(CurrentPoolElement.getUserTransportParameter().getPort());
      keepAliveAck->setSrcPort(CalcAppClientPort);
      keepAliveAck->setJobID(CurrentJob->getJobID());
      send(keepAliveAck, "toTransport");
      return(true);
   }
   else {
      EV << Description << "Ignoring CALCAPP_KEEP_ALIVE with invalid source or JobID" << endl;
      return(false);
   }
}


// ###### Handle ASAP_COOKIE message (CalcApp job cookie) ###################
bool CalcAppQueuingClientProcess::handleCalcAppCookie(CalcAppCookie* msg)
{
   if((msg->getSrcAddress() == CurrentPoolElement.getUserTransportParameter().getAddress()) &&
      (msg->getSrcPort() == CurrentPoolElement.getUserTransportParameter().getPort()) &&
      (msg->getCookie().getJobID() == CurrentJob->getJobID())) {
      EV << Description << "Cookie for job #"
         << CurrentJob->getJobID() << " received from pool element "
         << CurrentPoolElementDescription << ": completed "
         << msg->getCookie().getJobCompletedCalculations() << " of "
         << msg->getCookie().getJobCalculations() << endl;
      HasCookie = true;
      Cookie = msg->getCookie();

      // A cookie has been received from the server. The calculations up to
      // this cookie are now save -> add this time to goodput.
      const simtime_t goodputTime  = simTime() - CurrentJob->getLastGoodputAt();
      const double    goodputCalcs = msg->getCookie().getJobCompletedCalculations() - CurrentJob->getLastGoodputCalculations();
      CurrentJob->setTotalJobGoodputTime(CurrentJob->getTotalJobGoodputTime() +
                                      goodputTime);
      CurrentJob->setLastGoodputAt(simTime());
      CurrentJob->setLastGoodputCalculations(msg->getCookie().getJobCompletedCalculations());
      TotalJobGoodputTime += goodputTime;
      TotalJobSizeCompleted += goodputCalcs;

      return(true);
   }
   else {
      EV << Description << "Ignoring ASAP_COOKIE with invalid source or JobID" << endl;
      return(false);
   }
}


// ###### Start new job #####################################################
void CalcAppQueuingClientProcess::startNewJob()
{
   HasCookie = false;
   CurrentJob = (CalcAppQueuingClientJob*)JobQueue.pop();
   OPP_CHECK(CurrentJob);

   TotalJobsStarted++;
   TotalJobSizeStarted += CurrentJob->getJobCalculations();

   CurrentJob->setJobStartedAt(simTime());
   QueueLengthVector->record(JobQueue.getLength());

   const simtime_t queuingDelay = simTime() - CurrentJob->getJobQueuedAt();
   TotalJobQueuingDelay += queuingDelay;
   QueuingDelayStat->collect(queuingDelay);
   AbstractController* controller = AbstractController::getController();
   if(controller) {
      controller->GlobalJobQueuingDelayStat->collect(queuingDelay);
   }
}


// ###### Handle CALCAPP_COMPLETE message ###################################
bool CalcAppQueuingClientProcess::handleCalcAppComplete(CalcAppComplete* msg)
{
   if((msg->getSrcAddress() == CurrentPoolElement.getUserTransportParameter().getAddress()) &&
      (msg->getSrcPort() == CurrentPoolElement.getUserTransportParameter().getPort()) &&
      (msg->getJobID() == CurrentJob->getJobID())) {
      EV << Description << "Job #" << CurrentJob->getJobID() << " is complete!" << endl;
      if(JobKeepAliveTransmissionTimer) {
         stopJobKeepAliveTransmissionTimer();
      }
      if(JobKeepAliveTimeoutTimer) {
         stopJobKeepAliveTimeoutTimer();
      }

      // ====== Update local statistics =====================================
      const simtime_t goodputTime  = simTime() - CurrentJob->getLastGoodputAt();
      const double jobGoodputCalcs = CurrentJob->getJobCalculations() - CurrentJob->getLastGoodputCalculations();
      CurrentJob->setTotalJobGoodputTime(CurrentJob->getTotalJobGoodputTime() + goodputTime);
      TotalJobGoodputTime += goodputTime;

      const simtime_t queuingDelay   = CurrentJob->getJobStartedAt() - CurrentJob->getJobQueuedAt();
      const simtime_t startupDelay   = CurrentJob->getJobAcceptedAt() - CurrentJob->getJobStartedAt();
      const simtime_t processingTime = simTime() - CurrentJob->getJobAcceptedAt();
      const simtime_t handlingTime   = simTime() - CurrentJob->getJobQueuedAt();

      EV << Description << "--- Job finished after handling="
         << handlingTime << "s: "
         << "queuing=" << queuingDelay.dbl() << "s, "
         << "startup=" << startupDelay.dbl() << "s, "
         << "processing=" << processingTime.dbl() << "s, "
         << "goodputTime=" << CurrentJob->getTotalJobGoodputTime().dbl() << "s;   "
         << "failoverTime=" << CurrentJob->getTotalJobFailoverTime().dbl() << "s, "
         << "failovers=" << CurrentJob->getFailovers() << ", "
         << "unrchs=" << CurrentJob->getUnreachables() << ";   "
         << "hres=" << CurrentJob->getServerSelections() << " "
         << "ca.reqs=" << CurrentJob->getCalcAppRequests() << ", "
         << "ca.ce=" << CurrentJob->getCalcAppCookieEchoes() << ", "
         << "ca.acc=" << CurrentJob->getCalcAppAccepts() << ", "
         << "ca.rej=" << CurrentJob->getCalcAppRejects() << ", "
         << "---"
         << endl;

      TotalJobsCompleted++;
      TotalJobSizeCompleted  += jobGoodputCalcs;
      TotalJobProcessingTime += processingTime;
      TotalJobHandlingTime   += handlingTime;

      ProcessingTimeStat->collect(processingTime);
      HandlingTimeStat->collect(handlingTime);
      ProcessingTimeVector->record(processingTime);
      HandlingTimeVector->record(handlingTime);

      AbstractController* controller = AbstractController::getController();
      if(controller) {
         controller->GlobalJobProcessingTimeStat->collect(processingTime);
         controller->GlobalJobHandlingTimeStat->collect(handlingTime);
      }

      // ====== Do some checks ==============================================
      OPP_CHECK(fabs(CurrentJob->getTotalJobGoodputTime() + CurrentJob->getTotalJobFailoverTime() - processingTime) < 0.001);
      OPP_CHECK(fabs(queuingDelay + startupDelay + CurrentJob->getTotalJobGoodputTime() + CurrentJob->getTotalJobFailoverTime() - handlingTime) < 0.001);
      OPP_CHECK(CurrentJob->getUnreachables() <= CurrentJob->getCalcAppRequests() + CurrentJob->getCalcAppCookieEchoes());
      OPP_CHECK(CurrentJob->getFailovers() < CurrentJob->getCalcAppRequests() + CurrentJob->getCalcAppCookieEchoes());
      OPP_CHECK(CurrentJob->getCalcAppRequests() + CurrentJob->getCalcAppCookieEchoes() <= CurrentJob->getServerSelections());

      delete CurrentJob;
      CurrentJob = NULL;

      return(true);
   }
   else {
      EV << Description << "Ignoring CALCAPP_COMPLETE with invalid source or JobID" << endl;
      return(false);
   }
}


// ###### Handle message ####################################################
void CalcAppQueuingClientProcess::handleMessage(cMessage* msg)
{
   FSM_Switch(State) {
      case FSM_Exit(INIT):
         if(msg == StartupTimer) {
            StartupTimer = NULL;
            FSM_Goto(State, START_NEXT_JOB_TIMER);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(START_NEXT_JOB_TIMER):
         colorizeModule(getParentModule(), "#ffff55");
         EV << Description << "Startup" << endl;
         startJobGeneratorTimer();
         FSM_Goto(State, WAIT_FOR_NEXT_JOB);
       break;

      case FSM_Exit(WAIT_FOR_NEXT_JOB):
         if(msg == JobGeneratorTimer) {
            JobGeneratorTimer = NULL;
            handleJobGeneratorTimer();
            FSM_Goto(State, START_NEW_JOB);
         }
         else {
            handleIgnore(Description, msg, State);
         }
       break;

      case FSM_Exit(START_NEW_JOB):
         startNewJob();
         EV << Description << "Starting job #" << CurrentJob->getJobID() << " ..." << endl;
         FSM_Goto(State, SEND_SERVER_SELECTION_REQUEST);
       break;

      case FSM_Exit(SEND_SERVER_SELECTION_REQUEST):
         EV << Description << "Sending ServerSelectionRequest ..." << endl;
         CurrentJob->setServerSelections(CurrentJob->getServerSelections() + 1);
         TotalServerSelections++;
         sendServerSelectionRequest();
         FSM_Goto(State, WAIT_FOR_SERVER_SELECTION_RESPONSE);
       break;

      case FSM_Exit(WAIT_FOR_SERVER_SELECTION_RESPONSE):
         if(msg == JobGeneratorTimer) {
            JobGeneratorTimer = NULL;
            handleJobGeneratorTimer();
         }
         else if(dynamic_cast<ServerSelectionSuccess*>(msg)) {
            handleServerSelectionSuccess((ServerSelectionSuccess*)msg);
            EV << Description << "Handle resolution successfull, selected pool element "
               << CurrentPoolElementDescription << endl;
            FSM_Goto(State, SEND_JOB_REQUEST);
         }
         else if(dynamic_cast<ServerSelectionFailure*>(msg)) {
            EV << Description << "Handle resolution failed, no pool element found" << endl;
            FSM_Goto(State, START_SERVER_SELECTION_RETRY_TIMER);
         }
         else {
            handleIgnore(Description, msg, State);
         }
       break;

      case FSM_Exit(START_SERVER_SELECTION_RETRY_TIMER):
         EV << Description << "Waiting for handle resolution retry ..." << endl;
         startServerSelectionRetryTimer();
         FSM_Goto(State, WAIT_FOR_SERVER_SELECTION_RETRY);
       break;

      case FSM_Exit(WAIT_FOR_SERVER_SELECTION_RETRY):
         if(msg == JobGeneratorTimer) {
            JobGeneratorTimer = NULL;
            handleJobGeneratorTimer();
         }
         else if(msg == ServerSelectionRetryTimer) {
            ServerSelectionRetryTimer = NULL;
            EV << Description << "Retrying handle resolution ..." << endl;
            FSM_Goto(State, SEND_SERVER_SELECTION_REQUEST);
         }
         else {
            handleIgnore(Description, msg, State);
         }
       break;

      case FSM_Exit(SEND_JOB_REQUEST):
         if(HasCookie) {
            EV << Description << "Sending CookieEcho for job #"
               << CurrentJob->getJobID() << " to pool element "
               << CurrentPoolElementDescription << " ..." << endl;
            sendCalcAppCookieEcho();
            CurrentJob->setCalcAppCookieEchoes(CurrentJob->getCalcAppCookieEchoes() + 1);
            TotalCalcAppCookieEchoes++;
         }
         else {
            EV << Description << "Sending JobRequest for job #"
               << CurrentJob->getJobID() << " to pool element "
               << CurrentPoolElementDescription << " ..." << endl;
            sendCalcAppRequest();
            CurrentJob->setCalcAppRequests(CurrentJob->getCalcAppRequests() + 1);
            TotalCalcAppRequests++;
         }
         startJobRequestTimeoutTimer();
         FSM_Goto(State, WAIT_FOR_JOB_RESPONSE);
       break;

      case FSM_Exit(WAIT_FOR_JOB_RESPONSE):
         if(msg == JobGeneratorTimer) {
            JobGeneratorTimer = NULL;
            handleJobGeneratorTimer();
         }
         else if(msg == JobRequestTimeoutTimer) {
            JobRequestTimeoutTimer = NULL;
            EV << Description << "Sending job request for job #"
               << CurrentJob->getJobID() << " to pool element "
               << CurrentPoolElementDescription << " timed out -> reporing unreachability" << endl;
            colorizeModule(getParentModule(), "#d1a944");
            startJobRetryTimer(0.0);   // Retry immediately!
            FSM_Goto(State, SEND_ENDPOINT_UNREACHABLE);
         }
         else if(dynamic_cast<CalcAppAccept*>(msg)) {
            colorizeModule(getParentModule(), "#00ff00");
            if(handleCalcAppAccept((CalcAppAccept*)msg)) {
               FSM_Goto(State, SEND_KEEP_ALIVE);
            }
         }
         else if(dynamic_cast<CalcAppReject*>(msg)) {
            if(handleCalcAppReject((CalcAppReject*)msg)) {
               colorizeModule(getParentModule(), "#0000ff");
               CurrentJob->setCalcAppRejects(CurrentJob->getCalcAppRejects() + 1);
               TotalCalcAppRejects++;
               startJobRetryTimer();   // Retry after clientJobRetryDelay.
               FSM_Goto(State, SEND_CACHE_PURGE);
            }
         }
         else {
            handleIgnore(Description, msg, State);
         }
       break;

      case FSM_Exit(SEND_ENDPOINT_UNREACHABLE):
         EV << Description << "Reporting unreachability of pool element "
            << CurrentPoolElementDescription << " ..." << endl;
         CurrentJob->setUnreachables(CurrentJob->getUnreachables() + 1);
         TotalUnreachables++;
         sendEndpointUnreachable();
         OPP_CHECK(JobRetryTimer != NULL);
         FSM_Goto(State, WAIT_FOR_JOB_RETRY);
       break;

      case FSM_Exit(SEND_CACHE_PURGE):
         EV << Description << "Cache purging of pool element "
            << CurrentPoolElementDescription << " ..." << endl;
         sendCachePurge();
         OPP_CHECK(JobRetryTimer != NULL);
         FSM_Goto(State, WAIT_FOR_JOB_RETRY);
       break;

       case FSM_Exit(WAIT_FOR_JOB_RETRY):
          if(msg == JobGeneratorTimer) {
             JobGeneratorTimer = NULL;
             handleJobGeneratorTimer();
          }
          else if(msg == JobRetryTimer) {
             JobRetryTimer = NULL;
             FSM_Goto(State, SEND_SERVER_SELECTION_REQUEST);
          }
          else {
             handleIgnore(Description, msg, State);
          }
       break;

      case FSM_Exit(SEND_KEEP_ALIVE):
         if(msg == JobGeneratorTimer) {
            JobGeneratorTimer = NULL;
            handleJobGeneratorTimer();
         }
         else if(msg == JobKeepAliveTransmissionTimer) {
            JobKeepAliveTransmissionTimer = NULL;
            EV << Description << "Sending JobKeepAlive for job #"
               << CurrentJob->getJobID() << " to pool element " << CurrentPoolElementDescription << endl;
            sendCalcAppKeepAlive();
            startJobKeepAliveTimeoutTimer();
         }
         else if(msg == JobKeepAliveTimeoutTimer) {
            JobKeepAliveTimeoutTimer = NULL;
            EV << Description << "JobKeepAliveAck for job #"
               << CurrentJob->getJobID() << " timed out -> pool element "
               << CurrentPoolElementDescription << " is dead!" << endl;
            colorizeModule(getParentModule(), "#ffff55");
            CurrentJob->setFailovers(CurrentJob->getFailovers() + 1);
            TotalJobFailovers++;
            startJobRetryTimer(0.0);   // Retry immediately!
            FSM_Goto(State, SEND_ENDPOINT_UNREACHABLE);
         }
         else if(dynamic_cast<CalcAppKeepAlive*>(msg)) {
            handleCalcAppKeepAlive((CalcAppKeepAlive*)msg);
         }
         else if(dynamic_cast<CalcAppKeepAliveAck*>(msg)) {
            handleCalcAppKeepAliveAck((CalcAppKeepAliveAck*)msg);
         }
         else if(dynamic_cast<CalcAppCookie*>(msg)) {
            handleCalcAppCookie((CalcAppCookie*)msg);
         }
         else if(dynamic_cast<CalcAppAbort*>(msg)) {
            if(handleCalcAppAbort((CalcAppAbort*)msg)) {
               EV << Description << "CalcAppAbort for job #"
                  << CurrentJob->getJobID() << " timed out -> pool element "
                  << CurrentPoolElementDescription << " is shutting down!" << endl;
               colorizeModule(getParentModule(), "#0000ff");
               CurrentJob->setFailovers(CurrentJob->getFailovers() + 1);
               TotalJobFailovers++;
               startJobRetryTimer(0.0);   // Retry immediately!
               FSM_Goto(State, SEND_CACHE_PURGE);
            }
         }
         else if(dynamic_cast<CalcAppComplete*>(msg)) {
            if(handleCalcAppComplete((CalcAppComplete*)msg)) {
               Jobs++;
               colorizeModule(getParentModule(), "#ffff55");
               if(Jobs < (unsigned int)par("serviceJobCount")) {
                  FSM_Goto(State, FINISH_JOB);
               }
               else {
                  stopJobGeneratorTimer();
                  colorizeModule(getParentModule());
                  FSM_Goto(State, FINISHED);
               }
            }
         }
         else {
            handleIgnore(Description, msg, State);
         }
       break;

      case FSM_Exit(FINISH_JOB):
         colorizeModule(getParentModule(), "#ffff55");
         if(JobQueue.isEmpty()) {
            EV << Description << "Waiting for next job ..." << endl;
            FSM_Goto(State, WAIT_FOR_NEXT_JOB);
         }
         else {
            EV << Description << "There are already jobs in the queue -> starting a new job immediately" << endl;
            FSM_Goto(State, START_NEW_JOB);
         }
       break;


      case FSM_Exit(FINISHED):
         handleUnexpectedMsgState(msg, State);
       break;
   }

   delete msg;
}
