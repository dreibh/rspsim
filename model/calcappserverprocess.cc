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

#include <omnetpp.h>
#include "messages_m.h"
#include "calcappservermessages_m.h"


#include "statisticswriterinterface.h"
#include "abstractcontroller.h"
#include "statuschangelist.h"
#include "utilities.h"
#include "randomizer.h"
#include "handlespacemanagementwrapper.h"


class CalcAppServerJob : public CalcAppServerJob_Base
{
   public:
   CalcAppServerJob() {
   }
   CalcAppServerJob(const CalcAppServerJob& other) : CalcAppServerJob_Base() {
      operator=(other);
   }
   CalcAppServerJob& operator=(const CalcAppServerJob& other) {
      CalcAppServerJob_Base::operator=(other);
      return(*this);
   }
   virtual cOwnedObject* dup() {
      return new CalcAppServerJob(*this);
   }

   public:
   JobCompleteMessage*              JobCompleteTimer;
   JobKeepAliveTransmissionMessage* JobKeepAliveTransmissionTimer;
   JobKeepAliveTimeoutMessage*      JobKeepAliveTimeoutTimer;
   CookieTransmissionMessage*       CookieTransmissionTimer;
};

Register_Class(CalcAppServerJob);



class CalcAppServerProcess : public StatisticsWriterInterface,
                             public cSimpleModule
{
   // ====== Methods ========================================================
   virtual void initialize();
   virtual void finish();
   virtual void handleMessage(cMessage* msg);
   virtual void resetStatistics();
   virtual void writeStatistics();

   void startupService();
   void shutdownService();


   // ====== States ==========================================================
   private:
   enum {
      INIT               = 0,
      STARTUP_SERVICE    = FSM_Transient(1),
      RUN_SERVICE        = FSM_Steady(2),
      SHUTDOWN_SERVICE   = FSM_Steady(3),
      WAIT_FOR_RESTART   = FSM_Steady(4),
      FINISHED           = FSM_Steady(5)
   };
   cFSM State;


   // ====== Service Timers =================================================
   void startStartupTimer();
   void startShutdownTimer();
   void startRestartDelayTimer();


   // ====== CalcApp Protocol Timers ========================================
   void startJobCompleteTimer(CalcAppServerJob* job, const simtime_t duration);
   void stopJobCompleteTimer(CalcAppServerJob* job);
   void startJobKeepAliveTimeoutTimer(CalcAppServerJob* job);
   void stopJobKeepAliveTimeoutTimer(CalcAppServerJob* job);
   void startJobKeepAliveTransmissionTimer(CalcAppServerJob* job);
   void stopJobKeepAliveTransmissionTimer(CalcAppServerJob* job);
   void startCookieTransmissionTimer(CalcAppServerJob* job, const simtime_t duration);
   void stopCookieTransmissionTimer(CalcAppServerJob* job);
   void handleJobCompleteTimer(cMessage* msg);
   void handleJobKeepAliveTransmissionTimer(cMessage* msg);
   void handleJobKeepAliveTimeoutTimer(cMessage* msg);
   void handleCookieTransmissionTimer(cMessage* msg);


   // ====== CalcApp Protocol ===============================================
   void performPoolElementRegistration();
   void performPoolElementDeregistration(const bool cleanShutdown);
   void sendAbort(CalcAppServerJob* job);
   void sendCookie(CalcAppServerJob* job);
   void handleCalcAppRequest(CalcAppRequest* msg);
   void handleCalcAppCookieEcho(CalcAppCookieEcho* msg);
   void handleCalcAppKeepAlive(CalcAppKeepAlive* msg);
   void handleCalcAppKeepAliveAck(CalcAppKeepAliveAck* msg);
   opp_string getJobDescription(CalcAppServerJob* job);
   CalcAppServerJob* addJob(const unsigned int address,
                            const unsigned int port,
                            const unsigned int jobID,
                            const double       calculations,
                            const double       completed);
   CalcAppServerJob* findJob(const unsigned int address,
                             const unsigned int port,
                             const unsigned int jobID);
   void killAllJobs(const bool sendAbortMsg);
   void removeJob(CalcAppServerJob* job);
   void updateAccounting();
   void scheduleJobs();


   // ====== Parameters ======================================================
   double           ServiceCapacity;
   unsigned int     ServiceMaxJobs;
   unsigned int     SelectionPolicyType;
   unsigned int     SelectionPolicyWeight;
   unsigned int     SelectionPolicyWeightDPF;
   unsigned         SelectionPolicyLoadDegradation;
   unsigned int     SelectionPolicyLoadDPF;
   StatusChangeList ComponentStatusChanges;

   // ====== Timers ==========================================================
   cMessage*       StartupTimer;
   cMessage*       ShutdownTimer;
   cMessage*       RestartDelayTimer;


   // ====== Variables =======================================================
   unsigned int    Run;
   unsigned int    LocalPort;
   cQueue          JobQueue;
   double          CurrentLoad;
   simtime_t       LastPolicyInfoUpdate;
   simtime_t       LastAccounting;
   simtime_t       ServiceUptime;
   bool            ShuttingDown;

   double          TotalCalculations;
   unsigned int    TotalJobsAccepted;
   unsigned int    TotalJobsRejected;
   unsigned int    TotalPolicyUpdates;

   cOutVector*  LoadVector;

   opp_string   Description;
};

Define_Module(CalcAppServerProcess);



// ###### Initialize #########################################################
void CalcAppServerProcess::initialize()
{
   // ------ Initialize variables -------------------------------------------
   LoadVector = new cOutVector("LoadVector");
   OPP_CHECK(LoadVector);

   LocalPort = 1023;
   State.setName("State");
   Description = format("CalcAppServerProcess at %u:%u> ",
                        getLocalAddress(this), LocalPort);
   ComponentStatusChanges.setup(par("componentStatusChanges"));

   if(((opp_string)((const char*)par("servicePoolHandle"))).empty()) {
      throw new cRuntimeError("Bad pool handle!");
   }
   ServiceCapacity = par("serviceCapacity");
   if(ServiceCapacity < 1) {
      throw new cRuntimeError("serviceCapacity < 1!");
   }

   double minCapacityPerJob = par("serviceMinCapacityPerJob");
   if(minCapacityPerJob >= 1.0) {
      double targetMaxJobs = ServiceCapacity / minCapacityPerJob;
      if(targetMaxJobs < 1.0) {
         ServiceCapacity = minCapacityPerJob;
         targetMaxJobs   = 1.0;
         std::cerr << "WARNING: targetMaxJobs [minCapacityPerJob] => less than 1 job! Adapting ServiceCapacity to " << ServiceCapacity << std::endl;
      }
      ServiceMaxJobs = (unsigned int)rint(targetMaxJobs);
   }

   double maxJobs = par("serviceMaxJobs");
   if(maxJobs >= 1.0) {
      if(minCapacityPerJob >= 1.0) {
         throw new cRuntimeError("serviceMaxJobs and serviceMinCapacityPerJob may not be specified simultaneously!");
      }
      ServiceMaxJobs = (unsigned int)rint(maxJobs);
   }
   else {
      if(minCapacityPerJob < 1.0) {
         throw new cRuntimeError("Neither serviceMaxJobs nor serviceMinCapacityPerJob specified!");
      }
   }

   SelectionPolicyType = getPoolPolicyTypeByName(par("selectionPolicy"));
   if(SelectionPolicyType == PPT_UNDEFINED) {
      throw new cRuntimeError("Bad pool policy given!");
   }

   double policyWeight = par("selectionPolicyWeight");
   if(policyWeight < 0.0) {
      policyWeight = ServiceCapacity;

      if(SelectionPolicyType == PPT_WEIGHTED_ROUNDROBIN) {
         policyWeight /= 100000;
         if(policyWeight < 1.0) {
            policyWeight = 1.0;
         }
      }

      if(policyWeight > (double)PPV_MAX_WEIGHT) {
         throw new cRuntimeError("Auto selectionPolicyWeight calculation: serviceCapacity exceeds limits!");
      }
   }
   SelectionPolicyWeight = (unsigned int)rint(policyWeight);

   double policyLoadDegradation = par("selectionPolicyLoadDegradation");
   if(policyLoadDegradation < 0.0) {
      policyLoadDegradation = 1.0 / ServiceMaxJobs;
   }
   SelectionPolicyLoadDegradation = (unsigned int)rint((double)PPV_MAX_LOAD_DEGRADATION * policyLoadDegradation);
   if(SelectionPolicyLoadDegradation > PPV_MAX_LOAD_DEGRADATION) {
      SelectionPolicyLoadDegradation = PPV_MAX_LOAD_DEGRADATION;
   }

   const double policyLoadDPF = par("selectionPolicyLoadDPF");
   if( (policyLoadDPF < 0.0) || (policyLoadDPF > 1.0)) {
      throw new cRuntimeError("Bad selectionPolicyWeightDPF setting!");
   }
   SelectionPolicyLoadDPF = (unsigned int)rint((double)PPV_MAX_LOADDPF * policyLoadDPF);

   const double policyWeightDPF = par("selectionPolicyWeightDPF");
   if( (policyWeightDPF < 0.0) || (policyWeightDPF > 1.0)) {
      throw new cRuntimeError("Bad selectionPolicyWeightDPF setting!");
   }
   SelectionPolicyWeightDPF = (unsigned int)rint((double)PPV_MAX_WEIGHTDPF * policyWeightDPF);

   JobQueue.setName("CalcAppServerJobQueue");
   CurrentLoad          = 0.0;
   LastPolicyInfoUpdate = 0.0;
   ShuttingDown         = false;
   Run                  = 1;

   StartupTimer         = NULL;
   ShutdownTimer        = NULL;
   RestartDelayTimer    = NULL;

   // ------ Prepare startup ------------------------------------------------
   if((unsigned int)par("componentRuns") > 0) {
      colorizeModule(getParentModule(), "#ff0000");
      startStartupTimer();
   }
   else {
      FSM_Goto(State, FINISHED);
   }

   // ------ Reset statistics -----------------------------------------------
   LastAccounting = -1.0;
   resetStatistics();
}


// ###### Clean up ##########################################################
void CalcAppServerProcess::finish()
{
   delete LoadVector;
   LoadVector = NULL;
}


// ###### Reset statistics ##################################################
void CalcAppServerProcess::resetStatistics()
{
   if(LastAccounting >= 0.0) {
      updateAccounting();
      scheduleJobs();
   }

   LastAccounting     = simTime();

   ServiceUptime      = 0.0;
   TotalCalculations  = 0.0;
   TotalJobsAccepted  = 0;
   TotalJobsRejected  = 0;
   TotalPolicyUpdates = 0;
}


// ###### Write statistics ##################################################
void CalcAppServerProcess::writeStatistics()
{
   updateAccounting();
   scheduleJobs();

   const double utilization = (ServiceUptime.dbl() > 0.0) ? (TotalCalculations / (ServiceUptime.dbl() * ServiceCapacity)) : 0.0;

   recordScalar("CalcAppPE Service Uptime",                    ServiceUptime);
   recordScalar("CalcAppPE Service Capacity",                  ServiceCapacity);
   recordScalar("CalcAppPE Service Max Jobs",                  ServiceMaxJobs);
   recordScalar("CalcAppPE Selection Policy Weight",           SelectionPolicyWeight);
   recordScalar("CalcAppPE Selection Policy Load Degradation", SelectionPolicyLoadDegradation);
   recordScalar("CalcAppPE Selection Policy Load DPF",         SelectionPolicyLoadDPF);
   recordScalar("CalcAppPE Selection Policy Weight DPF",       SelectionPolicyWeightDPF);

   recordScalar("CalcAppPE Total Used Capacity",   TotalCalculations);
   recordScalar("CalcAppPE Total Wasted Capacity", (ServiceUptime.dbl() * ServiceCapacity) - TotalCalculations);
   recordScalar("CalcAppPE Total Jobs Accepted",   TotalJobsAccepted);
   recordScalar("CalcAppPE Total Jobs Rejected",   TotalJobsRejected);
   recordScalar("CalcAppPE Total Policy Updates",  TotalPolicyUpdates);
   recordScalar("CalcAppPE Utilization",
                utilization);

   AbstractController* controller = AbstractController::getController();
   if(controller) {
      controller->GlobalUtilizationStat->collect(utilization);
      controller->GlobalUsedCapacity   += TotalCalculations;
      controller->GlobalWastedCapacity += (ServiceUptime.dbl() * ServiceCapacity) - TotalCalculations;
   }
}


// ###### Get description string for CalcAppServerProcess module instance ###
opp_string CalcAppServerProcess::getJobDescription(CalcAppServerJob* job)
{
   return(format("job #%u from %u:%u [%1.0f/%1.0f]",
                 job->getJobID(), job->getClientAddress(), job->getClientPort(),
                 job->getJobCompletedCalculations(),
                 job->getJobCalculations()));
}


// ###### Start Startup timer ###############################################
void CalcAppServerProcess::startStartupTimer()
{
   OPP_CHECK(StartupTimer == NULL);
   StartupTimer = new cMessage("StartupTimer");
   simtime_t delay = ComponentStatusChanges.expectedAction(simTime(), "up");
   if(delay < 0.0) {
      delay = (simtime_t)par("componentStartupDelay");
   }
   scheduleAt(simTime() + delay, StartupTimer);
   ev << Description << "Scheduled startup in " << delay << "s" << endl;
}


// ###### Start Shutdown timer ##############################################
void CalcAppServerProcess::startShutdownTimer()
{
   OPP_CHECK(ShutdownTimer == NULL);
   ShutdownTimer = new cMessage("ShutdownTimer");
   simtime_t delay = ComponentStatusChanges.expectedAction(simTime(), "down");
   if(delay < 0.0) {
      delay = (simtime_t)par("componentUptime");
   }
   scheduleAt(simTime() + delay, ShutdownTimer);
}


// ###### Start Restart timer ###############################################
void CalcAppServerProcess::startRestartDelayTimer()
{
   OPP_CHECK(RestartDelayTimer == NULL);
   RestartDelayTimer = new cMessage("RestartDelayTimer");
   simtime_t delay = ComponentStatusChanges.expectedAction(simTime(), "up");
   if(delay < 0.0) {
      delay = (simtime_t)par("componentDowntime");
   }
   scheduleAt(simTime() + delay, RestartDelayTimer);
}


// ###### Start Job Complete timer ##########################################
void CalcAppServerProcess::startJobCompleteTimer(CalcAppServerJob* job, const simtime_t duration)
{
   OPP_CHECK(job->JobCompleteTimer == NULL);
   job->JobCompleteTimer = new JobCompleteMessage("JobCompleteTimer");
   job->JobCompleteTimer->setContextPointer((void*)job);

   // Controller's resetStatistics() call may schedule this timer!
   take(job->JobCompleteTimer);
   scheduleAt(simTime() + duration, job->JobCompleteTimer);
}


// ###### Stop Job Complete timer ###########################################
void CalcAppServerProcess::stopJobCompleteTimer(CalcAppServerJob* job)
{
   OPP_CHECK(job->JobCompleteTimer != NULL);
   delete cancelEvent(job->JobCompleteTimer);
   job->JobCompleteTimer = NULL;
}


// ###### Start Job KeepAlive Timeout timer #################################
void CalcAppServerProcess::startJobKeepAliveTimeoutTimer(CalcAppServerJob* job)
{
   OPP_CHECK(job->JobKeepAliveTimeoutTimer == NULL);
   job->JobKeepAliveTimeoutTimer = new JobKeepAliveTimeoutMessage("JobKeepAliveTimeoutTimer");
   job->JobKeepAliveTimeoutTimer->setContextPointer((void*)job);

   // Controller's resetStatistics() call may schedule this timer!
   take(job->JobKeepAliveTimeoutTimer);
   scheduleAt(simTime() + (simtime_t)par("serviceJobKeepAliveTimeout"),
              job->JobKeepAliveTimeoutTimer);
}


// ###### Stop Job KeepAlive Timeout timer ##################################
void CalcAppServerProcess::stopJobKeepAliveTimeoutTimer(CalcAppServerJob* job)
{
   OPP_CHECK(job->JobKeepAliveTimeoutTimer != NULL);
   delete cancelEvent(job->JobKeepAliveTimeoutTimer);
   job->JobKeepAliveTimeoutTimer = NULL;
}


// ###### Start Job KeepAlive Transmission timer ############################
void CalcAppServerProcess::startJobKeepAliveTransmissionTimer(CalcAppServerJob* job)
{
   OPP_CHECK(job->JobKeepAliveTransmissionTimer == NULL);
   job->JobKeepAliveTransmissionTimer = new JobKeepAliveTransmissionMessage("JobKeepAliveTransmissionTimer");
   job->JobKeepAliveTransmissionTimer->setContextPointer((void*)job);

   // Controller's resetStatistics() call may schedule this timer!
   take(job->JobKeepAliveTransmissionTimer);
   scheduleAt(simTime() + (simtime_t)par("serviceJobKeepAliveInterval"),
              job->JobKeepAliveTransmissionTimer);
}


// ###### Stop Job KeepAlive Transmission timer #############################
void CalcAppServerProcess::stopJobKeepAliveTransmissionTimer(CalcAppServerJob* job)
{
   OPP_CHECK(job->JobKeepAliveTransmissionTimer != NULL);
   delete cancelEvent(job->JobKeepAliveTransmissionTimer);
   job->JobKeepAliveTransmissionTimer = NULL;
}


// ###### Start Cookie Transmission timer ###################################
void CalcAppServerProcess::startCookieTransmissionTimer(CalcAppServerJob* job,
                                                        const simtime_t   duration)
{
   OPP_CHECK(job->CookieTransmissionTimer == NULL);
   job->CookieTransmissionTimer = new CookieTransmissionMessage("CookieTransmissionTimer");
   job->CookieTransmissionTimer->setContextPointer((void*)job);

   // Controller's resetStatistics() call may schedule this timer!
   take(job->CookieTransmissionTimer);
   scheduleAt(simTime() + duration, job->CookieTransmissionTimer);

}


// ###### Stop Cookie Transmission timer ####################################
void CalcAppServerProcess::stopCookieTransmissionTimer(CalcAppServerJob* job)
{
   OPP_CHECK(job->CookieTransmissionTimer != NULL);
   delete cancelEvent(job->CookieTransmissionTimer);
   job->CookieTransmissionTimer = NULL;
}


// ###### Start pool element registration ###################################
void CalcAppServerProcess::performPoolElementRegistration()
{
   ShuttingDown = false;

   cTransportParameter userTransportParameter;
   userTransportParameter.setAddress(getLocalAddress(this));
   userTransportParameter.setPort(LocalPort);

   cPoolPolicyParameter poolPolicyParameter;
   poolPolicyParameter.setPolicyType(SelectionPolicyType);
   poolPolicyParameter.setWeightDPF(SelectionPolicyWeightDPF);
   poolPolicyParameter.setWeight(SelectionPolicyWeight);
   poolPolicyParameter.setLoadDPF(SelectionPolicyLoadDPF);
   poolPolicyParameter.setLoadDegradation(SelectionPolicyLoadDegradation);
   poolPolicyParameter.setLoad(0);
   poolPolicyParameter.setDistance(0);
   LastPolicyInfoUpdate = 0.0;

   cPoolElementParameter poolElementParameter;
   PoolElementIdentifierType poolElementIdentifier = par("servicePoolElementIdentifier");
   if(poolElementIdentifier == 0) {
      poolElementIdentifier = 1 + (random32() % 0xfffffffe);
   }
   poolElementParameter.setIdentifier(poolElementIdentifier);
   poolElementParameter.setHomeRegistrarIdentifier(0);
   poolElementParameter.setRegistrationLife((unsigned int)rint(((simtime_t)par("serviceRegistrationLife")).dbl() * 1000.0));
   poolElementParameter.setUserTransportParameter(userTransportParameter);
   poolElementParameter.setPoolPolicyParameter(poolPolicyParameter);

   RegisterPoolElement* registration = new RegisterPoolElement("RegisterPoolElement");
   registration->setPoolHandle((const char*)par("servicePoolHandle"));
   registration->setPoolElementParameter(poolElementParameter);

   send(registration, "toASAP");
}


// ###### Start pool element deregistration #################################
void CalcAppServerProcess::performPoolElementDeregistration(const bool cleanShutdown)
{
   ShuttingDown = true;

   if(cleanShutdown) {
      DeregisterPoolElement* deregistration = new DeregisterPoolElement("DeregisterPoolElement");
      send(deregistration, "toASAP");
   }
   else {
      ResetPoolElement* reset = new ResetPoolElement("ResetPoolElement");
      send(reset, "toASAP");
   }
}


// ###### Send CALCAPP_ABORT ################################################
void CalcAppServerProcess::sendAbort(CalcAppServerJob* job)
{
   CalcAppAbort* jobAbort = new CalcAppAbort("CALCAPP_ABORT", CalcAppProtocol);
   jobAbort->setProtocol(CalcAppProtocol);
   jobAbort->setDstAddress(job->getClientAddress());
   jobAbort->setDstPort(job->getClientPort());
   jobAbort->setSrcPort(LocalPort);
   jobAbort->setJobID(job->getJobID());
   ev << Description << "Sending CALCAPP_ABORT for job "
      << getJobDescription(job) << " ..." << endl;
   send(jobAbort, "toTransport");
}


// ###### Handle CALCAPP_REQUEST message ####################################
void CalcAppServerProcess::handleCalcAppRequest(CalcAppRequest* msg)
{
   CalcAppServerJob* job = addJob(msg->getSrcAddress(), msg->getSrcPort(),
                                  msg->getJobID(),
                                  msg->getJobCalculations(), 0.0);
   if(job) {
      TotalJobsAccepted++;
      CalcAppAccept* success = new CalcAppAccept("CALCAPP_ACCEPT", CalcAppProtocol);
      success->setProtocol(CalcAppProtocol);
      success->setDstAddress(msg->getSrcAddress());
      success->setDstPort(msg->getSrcPort());
      success->setSrcPort(LocalPort);
      success->setJobID(msg->getJobID());
      ev << Description << "Sending CALCAPP_ACCEPT for job "
         << getJobDescription(job) << " ..." << endl;
      send(success, "toTransport");
   }
   else {
      TotalJobsRejected++;
      CalcAppReject* reject = new CalcAppReject("CALCAPP_REJECT", CalcAppProtocol);
      reject->setProtocol(CalcAppProtocol);
      reject->setDstAddress(msg->getSrcAddress());
      reject->setDstPort(msg->getSrcPort());
      reject->setSrcPort(LocalPort);
      reject->setJobID(msg->getJobID());
      ev << Description << "Sending CALCAPP_REJECT for request "
         << msg->getSrcAddress() << ":" << msg->getSrcPort() << " ..." << endl;
      send(reject, "toTransport");
   }
}


// ###### Handle ASAP_COOKIE_ECHO message (continue job) ####################
void CalcAppServerProcess::handleCalcAppCookieEcho(CalcAppCookieEcho* msg)
{
   CalcAppServerJob* job = addJob(msg->getSrcAddress(), msg->getSrcPort(),
                            msg->getCookie().getJobID(),
                            msg->getCookie().getJobCalculations(),
                            msg->getCookie().getJobCompletedCalculations());
   if(job) {
      TotalJobsAccepted++;
      CalcAppAccept* success = new CalcAppAccept("CALCAPP_ACCEPT", CalcAppProtocol);
      success->setProtocol(CalcAppProtocol);
      success->setDstAddress(msg->getSrcAddress());
      success->setDstPort(msg->getSrcPort());
      success->setSrcPort(LocalPort);
      success->setJobID(msg->getCookie().getJobID());
      ev << Description << "Sending CALCAPP_ACCEPT for job "
         << getJobDescription(job) << " ..." << endl;
      send(success, "toTransport");
   }
   else {
      TotalJobsRejected++;
      CalcAppReject* reject = new CalcAppReject("CALCAPP_REJECT", CalcAppProtocol);
      reject->setProtocol(CalcAppProtocol);
      reject->setDstAddress(msg->getSrcAddress());
      reject->setDstPort(msg->getSrcPort());
      reject->setSrcPort(LocalPort);
      reject->setJobID(msg->getCookie().getJobID());
      ev << Description << "Sending CALCAPP_REJECT for request from "
         << msg->getSrcAddress() << ":" << msg->getSrcPort() << " ..." << endl;
      send(reject, "toTransport");
   }
}


// ###### Handle CALCAPP_KEEP_ALIVE message #################################
void CalcAppServerProcess::handleCalcAppKeepAlive(CalcAppKeepAlive* msg)
{
   CalcAppServerJob* job = findJob(msg->getSrcAddress(), msg->getSrcPort(),
                      msg->getJobID());
   if(job) {
      CalcAppKeepAliveAck* keepAliveAck = new CalcAppKeepAliveAck("CALCAPP_KEEP_ALIVE_ACK", CalcAppProtocol);
      keepAliveAck->setProtocol(CalcAppProtocol);
      keepAliveAck->setDstAddress(msg->getSrcAddress());
      keepAliveAck->setDstPort(msg->getSrcPort());
      keepAliveAck->setSrcPort(LocalPort);
      keepAliveAck->setJobID(msg->getJobID());
      send(keepAliveAck, "toTransport");
   }
}


// ###### Handle CALCAPP_KEEP_ALIVE_ACK message #############################
void CalcAppServerProcess::handleCalcAppKeepAliveAck(CalcAppKeepAliveAck* msg)
{
   CalcAppServerJob* job = findJob(msg->getSrcAddress(), msg->getSrcPort(),
                                   msg->getJobID());
   if(job) {
      if(job->JobKeepAliveTimeoutTimer) {
         stopJobKeepAliveTimeoutTimer(job);
         startJobKeepAliveTransmissionTimer(job);
      }
   }
}


// ###### Handle JobCompleteTimer ###########################################
void CalcAppServerProcess::handleJobCompleteTimer(cMessage* msg)
{
   CalcAppServerJob* job = (CalcAppServerJob*)msg->getContextPointer();
   job->JobCompleteTimer = NULL;

   job->setJobCompletedAt(simTime());

   CalcAppComplete* completed = new CalcAppComplete("CALCAPP_COMPLETE", CalcAppProtocol);
   completed->setProtocol(CalcAppProtocol);
   completed->setDstAddress(job->getClientAddress());
   completed->setDstPort(job->getClientPort());
   completed->setSrcPort(LocalPort);
   completed->setJobID(job->getJobID());
   ev << Description << "Sending CALCAPP_COMPLETE for job "
      << getJobDescription(job) << " ..." << endl;
   send(completed, "toTransport");

   removeJob(job);
}


// ###### Handle JobKeepAliveTransmissionTimer ##############################
void CalcAppServerProcess::handleJobKeepAliveTransmissionTimer(cMessage* msg)
{
   CalcAppServerJob* job = (CalcAppServerJob*)msg->getContextPointer();
   job->JobKeepAliveTransmissionTimer = NULL;

   CalcAppKeepAlive* keepAlive = new CalcAppKeepAlive("CALCAPP_KEEP_ALIVE", CalcAppProtocol);
   keepAlive->setProtocol(CalcAppProtocol);
   keepAlive->setDstAddress(job->getClientAddress());
   keepAlive->setDstPort(job->getClientPort());
   keepAlive->setSrcPort(LocalPort);
   keepAlive->setJobID(job->getJobID());
   send(keepAlive, "toTransport");

   startJobKeepAliveTimeoutTimer(job);
}


// ###### Handle JobKeepAliveTimeoutTimer ###################################
void CalcAppServerProcess::handleJobKeepAliveTimeoutTimer(cMessage* msg)
{
   CalcAppServerJob* job = (CalcAppServerJob*)msg->getContextPointer();
   job->JobKeepAliveTimeoutTimer = NULL;

   sendAbort(job);
   removeJob(job);
}


// ###### Handle CookieTransmissionTimer ####################################
void CalcAppServerProcess::handleCookieTransmissionTimer(cMessage* msg)
{
   CalcAppServerJob* job = (CalcAppServerJob*)msg->getContextPointer();
   job->CookieTransmissionTimer = NULL;

   sendCookie(job);
}


// ###### Send cookie #######################################################
void CalcAppServerProcess::sendCookie(CalcAppServerJob* job)
{
   updateAccounting();
   job->setLastCookieCompletedCalculations(job->getJobCompletedCalculations());
   job->setLastCookieAt(simTime());
   scheduleJobs();  // Schedules next cookie transmission event!

   CalcAppCookieParameter cookieParameter;
   cookieParameter.setJobID(job->getJobID());
   cookieParameter.setJobCalculations(job->getJobCalculations());
   cookieParameter.setJobCompletedCalculations(job->getJobCompletedCalculations());

   CalcAppCookie* cookie = new CalcAppCookie("ASAP_COOKIE", ASAP);
   cookie->setProtocol(ASAP);
   cookie->setDstAddress(job->getClientAddress());
   cookie->setDstPort(job->getClientPort());
   cookie->setSrcPort(LocalPort);
   cookie->setCookie(cookieParameter);
   ev << Description << "Sending ASAP_COOKIE for job "
      << getJobDescription(job) << " ..." << endl;
   send(cookie, "toTransport");
}


// ###### Add new job #######################################################
CalcAppServerJob* CalcAppServerProcess::addJob(const unsigned int address,
                                               const unsigned int port,
                                               const unsigned int jobID,
                                               const double       calculations,
                                               const double       completed)
{
   if((unsigned int)JobQueue.length() >= ServiceMaxJobs) {
      ev << Description << "Server is full: rejected job " << jobID << " from "
         << address << ":" << port << endl;
      return(NULL);
   }
   const double serviceRejectProbability = (double)par("serviceRejectProbability");
   if((serviceRejectProbability > 0.0) &&
      (uniform(0.0, 1.0) < serviceRejectProbability)) {
      ev << Description << "Random reject: rejected job " << jobID << " from "
         << address << ":" << port << endl;
      return(NULL);
   }

   CalcAppServerJob* job = new CalcAppServerJob;
   job->setName("CalcAppServerJob");
   job->setJobID(jobID);
   job->setJobCalculations(calculations);
   job->setJobCompletedCalculations(completed);
   job->setJobStartedAt(simTime());
   job->setJobCompletedAt(-1.0);
   job->setClientAddress(address);
   job->setClientPort(port);
   job->setLastAccountingAt(simTime());
   job->setLastCookieCompletedCalculations(job->getJobCompletedCalculations());
   job->setLastCookieAt(simTime());

   job->JobCompleteTimer              = NULL;
   job->JobKeepAliveTransmissionTimer = NULL;
   job->JobKeepAliveTimeoutTimer      = NULL;
   job->CookieTransmissionTimer       = NULL;

   startJobKeepAliveTransmissionTimer(job);

   updateAccounting();
   JobQueue.insert(job);
   scheduleJobs();

   ev << Description << "Added job " << jobID << " from "
      << address << ":" << port << " - "
      << job->getJobCompletedCalculations() << " of " << job->getJobCalculations()
      << " calculations are already completed" << endl;
   return(job);
}


// ###### Remove job ########################################################
void CalcAppServerProcess::removeJob(CalcAppServerJob* job)
{
   updateAccounting();
   ev << Description << "Removing job " << job->getJobID()
      << " from " << job->getClientAddress() << ":" << job->getClientPort() << endl;
   if(job->CookieTransmissionTimer) {
      stopCookieTransmissionTimer(job);
   }
   if(job->JobKeepAliveTransmissionTimer) {
      stopJobKeepAliveTransmissionTimer(job);
   }
   if(job->JobKeepAliveTimeoutTimer) {
      stopJobKeepAliveTimeoutTimer(job);
   }
   delete JobQueue.remove(job);
   scheduleJobs();
}


// ###### Remove all jobs ###################################################
void CalcAppServerProcess::killAllJobs(const bool sendAbortMsg)
{
   for(;;) {
      cQueue::Iterator iterator(JobQueue);
      if(iterator.end()) {
         break;
      }
      CalcAppServerJob* job = (CalcAppServerJob*)iterator();
      if(sendAbortMsg) {
         sendCookie(job);
         sendAbort(job);
      }
      removeJob(job);
   }

   // For the case no job has been removed, updateAccounting() and scheduleJobs()
   // are still necessary to update ServiceUptime etc..
   updateAccounting();
   scheduleJobs();
}


// ###### Find job ##########################################################
CalcAppServerJob* CalcAppServerProcess::findJob(const unsigned int address,
                                                const unsigned int port,
                                                const unsigned int jobID)
{
   for(cQueue::Iterator iterator(JobQueue);!iterator.end();iterator++) {
      CalcAppServerJob* job = (CalcAppServerJob*)iterator();
      if((job->getJobID() == jobID) && (job->getClientAddress() == address) &&
         (job->getClientPort() == port)) {
         return(job);
      }
   }
   return(NULL);
}


// ###### Update completed calculations for all jobs ########################
void CalcAppServerProcess::updateAccounting()
{
   const simtime_t timeSinceLastAccounting = simTime() - LastAccounting;
   LastAccounting = simTime();
   if(State.getState() == FSM_Steady(RUN_SERVICE)) {
      ServiceUptime += timeSinceLastAccounting;
   }

   if(JobQueue.length() > 0) {
      const double powerPerJob = ServiceCapacity / JobQueue.length();
      ev << Description << "Update:" << endl;
      for(cQueue::Iterator iterator(JobQueue);!iterator.end();iterator++) {
         CalcAppServerJob* job = (CalcAppServerJob*)iterator();
         if(job->JobCompleteTimer) {
            stopJobCompleteTimer(job);
         }
         if(job->CookieTransmissionTimer) {
            stopCookieTransmissionTimer(job);
         }

         const simtime_t duration = simTime() - job->getLastAccountingAt();
         const double completed   = min(rint(duration.dbl() * powerPerJob),
                                        job->getJobCalculations() - job->getJobCompletedCalculations());
         job->setLastAccountingAt(simTime());
         job->setJobCompletedCalculations(job->getJobCompletedCalculations() + completed);
         if(job->getJobCalculations() - job->getJobCompletedCalculations() < -0.0001) {
            error("Negative value of operations to go for job %u\n", job->getJobID());
         }

         ev << "   " << getJobDescription(job)
            << format(" - update=%1.0f in %fs, toGo=%1.0f",
                      completed, duration.dbl(),
                      job->getJobCalculations() - job->getJobCompletedCalculations()) << endl;

         TotalCalculations += completed;
      }
      ev.printf("   Total calculations: %f\n", TotalCalculations);
      ev.printf("   Service runtime:    %f\n", ServiceUptime.dbl());
   }
}


// ###### Calculate job schedule ############################################
void CalcAppServerProcess::scheduleJobs()
{
   if(JobQueue.length() > 0) {
      const double powerPerJob = ServiceCapacity / JobQueue.length();

      ev << Description << "Schedule:" << endl;
      for(cQueue::Iterator iterator(JobQueue);!iterator.end();iterator++) {
         CalcAppServerJob* job = (CalcAppServerJob*)iterator();
         simtime_t duration = (job->getJobCalculations() - job->getJobCompletedCalculations()) / powerPerJob;

         startJobCompleteTimer(job, duration);

         const double    calculationsSinceLastCookie = job->getJobCompletedCalculations() - job->getLastCookieCompletedCalculations();
         const simtime_t timeSinceLastCookie         = simTime() - job->getLastCookieAt();
         const double nextByCalculations_dbl = ((double)par("serviceCookieMaxCalculations") - calculationsSinceLastCookie) / powerPerJob;
         const simtime_t nextByCalculations =
            (simtime_t)max(0.0, min(nextByCalculations_dbl, floor(simtime_t::getMaxTime().dbl())));
         const simtime_t nextByTime =
            max((simtime_t)0.0, (simtime_t)par("serviceCookieMaxTime") - timeSinceLastCookie);
         const simtime_t nextCookie = min(nextByCalculations, nextByTime);
         startCookieTransmissionTimer(job, nextCookie);

         ev << "   " << getJobDescription(job)
            << format(" - toGo=%1.0f, finish in %fs",
                      job->getJobCalculations() - job->getJobCompletedCalculations(), duration.dbl()) << endl;
      }
   }

   const double load = (double)JobQueue.length() / ServiceMaxJobs;
   LoadVector->record(load);
   if(fabs(CurrentLoad - load) > 0.000001) {
      CurrentLoad = load;
      if(ShuttingDown) {
         ev << Description << "Skipping PolicyUpdate since PE is shutting down" << endl;
      }
      else {
         if(PPT_IS_ADAPTIVE(SelectionPolicyType)) {
            const double selectionPolicyUpdateThreshold = par("selectionPolicyUpdateThreshold");
            ev << Description << "Load changed -> policy update: CurrentLoad="
               << CurrentLoad << ", LastPolicyInfoUpdate=" << LastPolicyInfoUpdate
               << " (PolicyUpdateThreshold " << selectionPolicyUpdateThreshold << ")"
               << endl;
/*
            std::cerr << Description << "Load changed -> policy update: CurrentLoad="
                     << CurrentLoad << ", LastPolicyInfoUpdate=" << LastPolicyInfoUpdate
                     << " (PolicyUpdateThreshold " << selectionPolicyUpdateThreshold << ")"
                     << std::endl;
*/

            if(fabs(CurrentLoad - LastPolicyInfoUpdate) > selectionPolicyUpdateThreshold) {
               TotalPolicyUpdates++;
               PolicyUpdate* policyUpdate = new PolicyUpdate("PolicyUpdate");
               policyUpdate->setReregisterImmediately((bool)par("selectionReregisterImmediatelyOnUpdate"));

               cPoolPolicyParameter poolPolicyParameter;
               poolPolicyParameter.setPolicyType(SelectionPolicyType);
               poolPolicyParameter.setWeight(SelectionPolicyWeight);
               poolPolicyParameter.setWeightDPF(SelectionPolicyWeightDPF);
               poolPolicyParameter.setLoadDegradation(SelectionPolicyLoadDegradation);
               poolPolicyParameter.setLoadDPF(SelectionPolicyLoadDPF);
               unsigned int policyLoad = (unsigned int)rint(CurrentLoad * (double)PPV_MAX_LOAD);
               if(policyLoad > PPV_MAX_LOAD) {
                  policyLoad = PPV_MAX_LOAD;
               }
               poolPolicyParameter.setLoad(policyLoad);
               LastPolicyInfoUpdate = CurrentLoad;

               poolPolicyParameter.setDistance(0);
               policyUpdate->setPoolPolicyParameter(poolPolicyParameter);
               send(policyUpdate, "toASAP");
            }
            else {
               ev << "Skipping update, difference="
                  << fabs(CurrentLoad - LastPolicyInfoUpdate)
                  << " below threshold " << selectionPolicyUpdateThreshold << endl;
            }
         }
      }
   }
}


// ###### Startup ###########################################################
void CalcAppServerProcess::startupService()
{
   ev << Description << "Registering and starting service ..." << endl;

   // ------ Bind to port ---------------------------------------------------
   LocalPort++;
   BindMessage* msg = new BindMessage("Bind");
   msg->setPort(LocalPort);
   send(msg, "toTransport");
   Description = format("CalcAppServerProcess at %u:%u> ",
                        getLocalAddress(this), LocalPort);

   // ------ Start registration ---------------------------------------------
   performPoolElementRegistration();
   LastAccounting = simTime();
}


// ###### Shutdown ##########################################################
void CalcAppServerProcess::shutdownService()
{
   const bool cleanShutdown = (uniform(0.0, 1.0) <= (double)par("componentCleanShutdownProbability"));

   ShutdownTimer = NULL;
   ev << Description << (cleanShutdown ? "clean" : "unclean") << " service shutdown ..." << endl;
   performPoolElementDeregistration(cleanShutdown);
   killAllJobs(cleanShutdown);
   LastAccounting = 0.0;

   // ------ Unbind port ----------------------------------------------------
   UnbindMessage* msg = new UnbindMessage("Unbind");
   msg->setPort(LocalPort);
   send(msg, "toTransport");
}


// ###### Handle message #####################################################
void CalcAppServerProcess::handleMessage(cMessage* msg)
{
   ev << Description << "Received message \"" << msg->getName()
      << "\" in state " << State.getStateName() << endl;

   FSM_Switch(State) {

      case FSM_Exit(INIT):
         if(msg == StartupTimer) {
            StartupTimer = NULL;
            FSM_Goto(State, STARTUP_SERVICE);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(STARTUP_SERVICE):
         startupService();
         FSM_Goto(State, RUN_SERVICE);
       break;

      case FSM_Exit(RUN_SERVICE):
         colorizeModule(getParentModule(), "#00ff00");
         if(dynamic_cast<CalcAppRequest*>(msg) != NULL) {
            handleCalcAppRequest(dynamic_cast<CalcAppRequest*>(msg));
         }
         else if(dynamic_cast<CalcAppCookieEcho*>(msg) != NULL) {
            handleCalcAppCookieEcho(dynamic_cast<CalcAppCookieEcho*>(msg));
         }
         else if(dynamic_cast<CalcAppKeepAlive*>(msg) != NULL) {
            handleCalcAppKeepAlive(dynamic_cast<CalcAppKeepAlive*>(msg));
         }
         else if(dynamic_cast<CalcAppKeepAliveAck*>(msg) != NULL) {
            handleCalcAppKeepAliveAck(dynamic_cast<CalcAppKeepAliveAck*>(msg));
         }
         else if(dynamic_cast<JobCompleteMessage*>(msg) != NULL) {
            handleJobCompleteTimer(msg);
         }
         else if(dynamic_cast<JobKeepAliveTransmissionMessage*>(msg) != NULL) {
            handleJobKeepAliveTransmissionTimer(msg);
         }
         else if(dynamic_cast<JobKeepAliveTimeoutMessage*>(msg) != NULL) {
            handleJobKeepAliveTimeoutTimer(msg);
         }
         else if(dynamic_cast<CookieTransmissionMessage*>(msg) != NULL) {
            handleCookieTransmissionTimer(msg);
         }
         else if(dynamic_cast<RegisterPoolElementAck*>(msg) != NULL)  {
            ev << Description << "Successfully registered as pool element" << endl;
            startShutdownTimer();
         }
         else if(msg == ShutdownTimer) {
            shutdownService();
            FSM_Goto(State, SHUTDOWN_SERVICE);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(SHUTDOWN_SERVICE):
         colorizeModule(getParentModule(), "#ff0000");
         if((dynamic_cast<DeregisterPoolElementAck*>(msg)) ||
            (dynamic_cast<ResetPoolElementAck*>(msg))) {
            if(Run < (unsigned int)par("componentRuns")) {
               Run++;
               startRestartDelayTimer();
               FSM_Goto(State, WAIT_FOR_RESTART);
            }
            else {
               ev << Description << "--- FINISHED ---" << endl;
               colorizeModule(getParentModule(), "");
               FSM_Goto(State, FINISHED);
            }
         }
         else if((dynamic_cast<CalcAppRequest*>(msg) != NULL)   ||
                 (dynamic_cast<CalcAppCookieEcho*>(msg)!= NULL) ||
                 (dynamic_cast<CalcAppKeepAlive*>(msg) != NULL) ||
                 (dynamic_cast<CalcAppKeepAliveAck*>(msg) != NULL)) {
            handleIgnore(Description, msg, State);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(WAIT_FOR_RESTART):
         if(msg == RestartDelayTimer) {
            RestartDelayTimer = NULL;
            ev << Description << "Restarting service ..." << endl;
            FSM_Goto(State, STARTUP_SERVICE);
         }
         else if((dynamic_cast<CalcAppRequest*>(msg) != NULL)   ||
                 (dynamic_cast<CalcAppCookieEcho*>(msg)!= NULL) ||
                 (dynamic_cast<CalcAppKeepAlive*>(msg) != NULL) ||
                 (dynamic_cast<CalcAppKeepAliveAck*>(msg) != NULL)) {
            handleIgnore(Description, msg, State);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;


      case FSM_Exit(FINISHED):
         handleIgnore(Description, msg, State);
       break;

   }

   delete msg;
}
