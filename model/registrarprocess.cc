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
#include <algorithm>
#include "messages_m.h"

#include "statisticswriterinterface.h"
#include "abstractcontroller.h"
#include "statuschangelist.h"
#include "utilities.h"
#include "handlespacemanagementwrapper.h"



class RegistrarProcess : public StatisticsWriterInterface,
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
   void addStaticPeer(const unsigned int peerAddress);
   void printHandlespace();
   void beginNormalOperation(const bool initializedFromMentor);
   unsigned int randomizeMaxHandleResolutionItems(const unsigned int maxHandleResolutionItems,
                                                  const char*        poolHandle);

   void updateNumberStatistics();

   // ====== States =========================================================
   private:
   enum {
      INIT             = 0,
      STARTUP_SERVICE  = FSM_Transient(1),
      RUN_SERVICE      = FSM_Steady(2),
      SHUTDOWN_SERVICE = FSM_Transient(3),
      WAIT_FOR_RESTART = FSM_Steady(4),
      FINISHED         = FSM_Steady(5)
   };
   cFSM State;


   // ====== Registrar Timers ===============================================
   void startStartupTimer();
   void startShutdownTimer();
   void startRestartDelayTimer();

   // ====== ENRP Timers ====================================================
   void startPeerHeartbeatCycleTimer();
   void stopPeerHeartbeatCycleTimer();
   void startLastHeardTimeoutTimer(cPeerListNode* node);
   void stopLastHeardTimeoutTimer(cPeerListNode* node);
   void startResponseTimeoutTimer(cPeerListNode* node);
   void stopResponseTimeoutTimer(cPeerListNode* node);
   void startTakeoverExpiryTimer(cPeerListNode* node);
   void stopTakeoverExpiryTimer(cPeerListNode* node);
   void handlePeerHeartbeatCycleTimer();
   void handleLastHeardTimeoutTimer(cPeerListNode* node);
   void handleResponseTimeoutTimer(cPeerListNode* node);
   void handleTakeoverExpiryTimer(cPeerListNode* node);

   // ====== ASAP Timers ====================================================
   void startMentorDiscoveryTimeoutTimer();
   void stopMentorDiscoveryTimeoutTimer();
   void startEndpointKeepAliveTransmissionTimer(cPoolElement* poolElement);
   void stopEndpointKeepAliveTransmissionTimer(cPoolElement* poolElement);
   void startEndpointKeepAliveTimeoutTimer(cPoolElement* poolElement);
   void stopEndpointKeepAliveTimeoutTimer(cPoolElement* poolElement);
   void handleEndpointKeepAliveTransmissionTimer(cPoolElement* poolElement);
   void handleEndpointKeepAliveTimeoutTimer(cPoolElement* poolElement);
   void handleLifetimeExpiryTimer(cPoolElement* poolElement);
   void startLifetimeExpiryTimer(cPoolElement* poolElement);
   void stopLifetimeExpiryTimer(cPoolElement* poolElement);


   // ====== ASAP Protocol ==================================================
   inline bool inStartupPhase() const {
      return(MentorDiscoveryTimeoutTimer != NULL);
   }
   void handleASAPRegistration(ASAPRegistration* msg);
   void handleASAPDeregistration(ASAPDeregistration* msg);
   void handleASAPHandleResolution(ASAPHandleResolution* msg);
   void sendASAPEndpointKeepAlive(const cPoolElement* poolElement,
                                  const bool          homeFlag);
   void handleASAPEndpointUnreachable(ASAPEndpointUnreachable* msg);
   void handleASAPEndpointKeepAliveAck(ASAPEndpointKeepAliveAck* msg);


   // ====== ENRP Protocol ==================================================
   void sendENRPListRequest(cPeerListNode* node);
   void handleENRPListRequest(ENRPListRequest* msg);
   bool handleENRPListResponse(ENRPListResponse* msg);
   void sendENRPHandleTableRequest(cPeerListNode* node,
                                   const bool     ownChildrenOnly);
   void handleENRPHandleTableRequest(ENRPHandleTableRequest* msg);
   bool handleENRPHandleTableResponse(ENRPHandleTableResponse* msg);

   void sendENRPPresence(const cPeerListNode* node,
                         const bool           replyRequired);
   void handleENRPPresence(ENRPPresence* msg);
   void sendENRPHandleUpdates(const cPoolElement* poolElement,
                              const unsigned int  updateAction);
   void handleENRPHandleUpdate(ENRPHandleUpdate* msg);

   void sendENRPInitTakeovers(const unsigned int targetServerID);
   void handleENRPInitTakeover(ENRPInitTakeover* msg);
   void sendENRPInitTakeoverAck(const unsigned int targetServerID,
                                const unsigned int receiverServerID,
                                const unsigned int receiverServerAddress,
                                const unsigned int receiverServerPort);
   void handleENRPInitTakeoverAck(ENRPInitTakeoverAck* msg);
   void sendENRPTakeoverServers(const unsigned int targetServerID);
   void handleENRPTakeoverServer(ENRPTakeoverServer* msg);
   void finishTakeover(cPeerListNode* node);

   // ====== Parameters =====================================================
   unsigned int               MyIdentifier;


   // ====== Timers =========================================================
   cMessage*                  StartupTimer;
   cMessage*                  ShutdownTimer;
   cMessage*                  RestartDelayTimer;
   cMessage*                  MentorDiscoveryTimeoutTimer;
   cMessage*                  PeerHeartbeatCycleTimer;


   // ====== Variables ======================================================
   unsigned int               TotalStartupsWithMentor;
   unsigned int               TotalStartupsAlone;
   unsigned int               TotalRegistrations;
   unsigned int               TotalDeregistrations;
   unsigned int               TotalHandleResolutions;
   unsigned int               TotalRefusedHandleResolutions;
   unsigned int               TotalEndpointUnreachables;
   unsigned int               TotalRefusedEndpointUnreachables;
   unsigned int               TotalEndpointKeepAliveTimeouts;
   unsigned int               TotalEndpointKeepAlivesSent;
   unsigned int               TotalEndpointKeepAliveAcksReceived;
   unsigned int               TotalLifetimeExpiries;
   unsigned int               TotalHandleUpdates;
   unsigned int               TotalRequestedPresences;
   unsigned int               TotalPeerListRequests;
   unsigned int               TotalHandleTableRequests;
   unsigned int               TotalTakeoversStarted;
   unsigned int               TotalTakeoversByConsent;
   unsigned int               TotalTakeoversByTimeout;

   unsigned int               Run;
   unsigned int               LocalAddress;
   cPoolHandlespace*          Handlespace;
   cPeerList*                 PeerList;
   cPoolUserList              UserList;
   ServerInformationParameter OwnServerInfo;
   opp_string                 Description;
   StatusChangeList           ComponentStatusChanges;
   unsigned int               MentorServerID;

   cOutVector*                PoolElementCountVector;
   cOutVector*                OwnedPoolElementCountVector;

   WeightedStdDev             NumberOfPoolsStat;
   WeightedStdDev             NumberOfPEsStat;
   WeightedStdDev             NumberOfOwnedPEsStat;
   WeightedStdDev             NumberOfPeersStat;
   simtime_t                  LastNumberUpdate;
   size_t                     NumberOfPools;
   size_t                     NumberOfPEs;
   size_t                     NumberOfOwnedPEs;
   size_t                     NumberOfPeers;
};


Define_Module(RegistrarProcess);


// ###### Initialize ########################################################
void RegistrarProcess::initialize()
{
   // ------ Initialize variables -------------------------------------------
   State.setName("State");
   MyIdentifier = par("registrarIdentifier");
   Description  = format("RegistrarProcess at %u:%u [id=%u]> ",
                         getLocalAddress(this), RegistrarPort, MyIdentifier);

   Handlespace = new cPoolHandlespace(MyIdentifier);
   OPP_CHECK(Handlespace);
   PeerList = new cPeerList(Handlespace, MyIdentifier);
   OPP_CHECK(PeerList);

   StartupTimer                = NULL;
   ShutdownTimer               = NULL;
   RestartDelayTimer           = NULL;
   MentorDiscoveryTimeoutTimer = NULL;
   PeerHeartbeatCycleTimer     = NULL;

   Run                         = 1;
   LocalAddress                = getLocalAddress(this);
   ComponentStatusChanges.setup(par("componentStatusChanges"));
   MentorServerID              = UNDEFINED_REGISTRAR_IDENTIFIER;

   OwnServerInfo.setServerID(MyIdentifier);
   OwnServerInfo.setAddress(getLocalAddress(this));
   OwnServerInfo.setPort(RegistrarPort);

   PoolElementCountVector = new cOutVector("PoolElementCount");
   OPP_CHECK(PoolElementCountVector);
   PoolElementCountVector->record(0);

   OwnedPoolElementCountVector = new cOutVector("OwnedPoolElementCount");
   OPP_CHECK(OwnedPoolElementCountVector);
   OwnedPoolElementCountVector->record(0);

   // ------ Create peer table ----------------------------------------------
   const char*  staticRegistrarsList = par("enrpStaticPeersList");
   unsigned int registrarAddress;
   int          n;
   while(staticRegistrarsList[0] != 0x00) {
      if(sscanf(staticRegistrarsList, "%u%n", &registrarAddress, &n) == 1) {
         addStaticPeer(registrarAddress);
         staticRegistrarsList = (const char*)&staticRegistrarsList[n];
         while( (*staticRegistrarsList == ' ') ||
                (*staticRegistrarsList == ',') ) {
           staticRegistrarsList++;
         }
      }
      else {
         break;
      }
   }

   EV << Description;
   PeerList->print();

   // ------ Bind to port ---------------------------------------------------
   BindMessage* msg = new BindMessage("Bind");
   msg->setPort(RegistrarPort);
   send(msg, "toTransport");

   // ------ Prepare startup ------------------------------------------------
   if((unsigned int)par("componentRuns") > 0) {
      colorizeModule(getParentModule(), "#ff0000");
      startStartupTimer();
   }
   else {
      FSM_Goto(State, FINISHED);
   }

   // ------ Reset statistics -----------------------------------------------
   resetStatistics();
}


// ###### Clean up ##########################################################
void RegistrarProcess::finish()
{
   delete PoolElementCountVector;
   PoolElementCountVector = NULL;
   delete OwnedPoolElementCountVector;
   OwnedPoolElementCountVector = NULL;
   delete PeerList;
   PeerList = NULL;
   Handlespace->clear();
   delete Handlespace;
   Handlespace = NULL;
}


// ###### Reset statistics ##################################################
void RegistrarProcess::resetStatistics()
{
   TotalStartupsWithMentor            = 0;
   TotalStartupsAlone                 = 0;
   TotalRegistrations                 = 0;
   TotalDeregistrations               = 0;
   TotalHandleResolutions             = 0;
   TotalRefusedHandleResolutions      = 0;
   TotalEndpointUnreachables          = 0;
   TotalRefusedEndpointUnreachables   = 0;
   TotalEndpointKeepAliveTimeouts     = 0;
   TotalEndpointKeepAlivesSent        = 0;
   TotalEndpointKeepAliveAcksReceived = 0;
   TotalLifetimeExpiries              = 0;
   TotalHandleUpdates                 = 0;
   TotalRequestedPresences            = 0;
   TotalPeerListRequests              = 0;
   TotalHandleTableRequests           = 0;
   TotalTakeoversStarted              = 0;
   TotalTakeoversByConsent            = 0;
   TotalTakeoversByTimeout            = 0;

   NumberOfPoolsStat.clear();
   NumberOfPEsStat.clear();
   NumberOfOwnedPEsStat.clear();
   NumberOfPeersStat.clear();
   LastNumberUpdate = simTime();
   NumberOfPools    = Handlespace->getPools();
   NumberOfPEs      = Handlespace->getPoolElements();
   NumberOfOwnedPEs = Handlespace->getOwnedPoolElements();
   NumberOfPeers    = PeerList->getPeers();
}


// ###### Write statistics ##################################################
void RegistrarProcess::writeStatistics()
{
   recordScalar("Registrar Total Startups With Mentor",             TotalStartupsWithMentor);
   recordScalar("Registrar Total Startups Alone",                   TotalStartupsAlone);
   recordScalar("Registrar Total Registrations",                    TotalRegistrations);
   recordScalar("Registrar Total Deregistrations",                  TotalDeregistrations);
   recordScalar("Registrar Total Handle Resolutions",               TotalHandleResolutions);
   recordScalar("Registrar Total Refused Handle Resolutions",       TotalRefusedHandleResolutions);
   recordScalar("Registrar Total Endpoint Unreachables",            TotalEndpointUnreachables);
   recordScalar("Registrar Total Refused Endpoint Unreachables",    TotalRefusedEndpointUnreachables);
   recordScalar("Registrar Total Endpoint Keep Alives Sent",        TotalEndpointKeepAlivesSent);
   recordScalar("Registrar Total Endpoint Keep Alive Ack Received", TotalEndpointKeepAliveAcksReceived);
   recordScalar("Registrar Total Endpoint Keep Alive Timeouts",     TotalEndpointKeepAliveTimeouts);
   recordScalar("Registrar Total Lifetime Expiries",                TotalLifetimeExpiries);
   recordScalar("Registrar Total Handle Updates",                   TotalHandleUpdates);
   recordScalar("Registrar Total Requested Presences",              TotalRequestedPresences);
   recordScalar("Registrar Total Peer List Requests",               TotalPeerListRequests);
   recordScalar("Registrar Total Handle Table Requests",            TotalHandleTableRequests);
   recordScalar("Registrar Total Takeovers Started",                TotalTakeoversStarted);
   recordScalar("Registrar Total Takeovers By Consent",             TotalTakeoversByConsent);
   recordScalar("Registrar Total Takeovers By Timeout",             TotalTakeoversByTimeout);

   recordScalar("Registrar Average Number Of Pools", NumberOfPoolsStat.getMean());
   recordScalar("Registrar Average Number Of Pool Elements", NumberOfPEsStat.getMean());
   recordScalar("Registrar Average Number Of Owned Pool Elements", NumberOfOwnedPEsStat.getMean());
   recordScalar("Registrar Average Number Of Peers", NumberOfPeersStat.getMean());

   AbstractController* controller = AbstractController::getController();
   if(controller) {
      controller->GlobalStartupsWithMentor            += TotalStartupsWithMentor;
      controller->GlobalStartupsAlone                 += TotalStartupsAlone;
      controller->GlobalRegistrations                 += TotalRegistrations;
      controller->GlobalDeregistrations               += TotalDeregistrations;
      controller->GlobalHandleResolutions             += TotalHandleResolutions;
      controller->GlobalRefusedHandleResolutions      += TotalRefusedHandleResolutions;
      controller->GlobalEndpointUnreachables          += TotalEndpointUnreachables;
      controller->GlobalRefusedEndpointUnreachables   += TotalRefusedEndpointUnreachables;
      controller->GlobalEndpointKeepAlivesSent        += TotalEndpointKeepAlivesSent;
      controller->GlobalEndpointKeepAliveAcksReceived += TotalEndpointKeepAliveAcksReceived;
      controller->GlobalEndpointKeepAliveTimeouts     += TotalEndpointKeepAliveTimeouts;
      controller->GlobalLifetimeExpiries              += TotalLifetimeExpiries;
      controller->GlobalHandleUpdates                 += TotalHandleUpdates;
      controller->GlobalRequestedPresences            += TotalRequestedPresences;
      controller->GlobalPeerListRequests              += TotalPeerListRequests;
      controller->GlobalHandleTableRequests           += TotalHandleTableRequests;
      controller->GlobalTakeoversStarted              += TotalTakeoversStarted;
      controller->GlobalTakeoversByConsent            += TotalTakeoversByConsent;
      controller->GlobalTakeoversByTimeout            += TotalTakeoversByTimeout;
   }
}


// ###### Update handlespace/peer number statistics #########################
void RegistrarProcess::updateNumberStatistics()
{
   const simtime_t elapsedTime = simTime() - LastNumberUpdate;
   const double    elapsed     = elapsedTime.dbl();
   if(elapsed > 0) {
      NumberOfPoolsStat.collectWeighted(NumberOfPools, elapsed);
      NumberOfPEsStat.collectWeighted(NumberOfPEs, elapsed);
      NumberOfOwnedPEsStat.collectWeighted(NumberOfOwnedPEs, elapsed);
      NumberOfPeersStat.collectWeighted(NumberOfPeers, elapsed);

      NumberOfPools    = Handlespace->getPools();
      NumberOfPEs      = Handlespace->getPoolElements();
      NumberOfOwnedPEs = Handlespace->getOwnedPoolElements();
      NumberOfPeers    = PeerList->getPeers();

      LastNumberUpdate = simTime();
   }
}


// ###### Start Startup timer ###############################################
void RegistrarProcess::startStartupTimer()
{
   OPP_CHECK(StartupTimer == NULL);
   StartupTimer = new cMessage("StartupTimer");
   simtime_t delay = ComponentStatusChanges.expectedAction(simTime(), "up");
   if(delay < 0.0) {
      delay = (simtime_t)par("componentStartupDelay");
   }
   scheduleAt(simTime() + delay, StartupTimer);
   EV << Description << "Scheduled startup in " << delay << "s" << endl;
}


// ###### Start Shutdown timer ##############################################
void RegistrarProcess::startShutdownTimer()
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
void RegistrarProcess::startRestartDelayTimer()
{
   OPP_CHECK(RestartDelayTimer == NULL);
   RestartDelayTimer = new cMessage("RestartDelayTimer");
   simtime_t delay = ComponentStatusChanges.expectedAction(simTime(), "up");
   if(delay < 0.0) {
      delay = (simtime_t)par("componentDowntime");
   }
   scheduleAt(simTime() + delay, RestartDelayTimer);
}


// ###### Start Peer Heartbeat Cycle timer ##################################
void RegistrarProcess::startPeerHeartbeatCycleTimer()
{
   OPP_CHECK(PeerHeartbeatCycleTimer == NULL);
   PeerHeartbeatCycleTimer = new cMessage("PeerHeartbeatCycleTimer");
   scheduleAt(simTime() + (simtime_t)par("enrpPeerHeartbeatCycle"), PeerHeartbeatCycleTimer);
}


// ###### Stop Peer Heartbeat Cycle timer ###################################
void RegistrarProcess::stopPeerHeartbeatCycleTimer()
{
   OPP_CHECK(PeerHeartbeatCycleTimer != NULL);
   delete cancelEvent(PeerHeartbeatCycleTimer);
   PeerHeartbeatCycleTimer = NULL;
}


// ###### Start MentorDiscoveryTimeout timer ################################
void RegistrarProcess::startMentorDiscoveryTimeoutTimer()
{
   OPP_CHECK(MentorDiscoveryTimeoutTimer == NULL);
   MentorDiscoveryTimeoutTimer = new cMessage("MentorDiscoveryTimeoutTimer");
   scheduleAt(simTime() + (simtime_t)par("registrarMentorDiscoveryTimeout"), MentorDiscoveryTimeoutTimer);
}

// ###### Stop MentorDiscoveryTimeout timer #################################
void RegistrarProcess::stopMentorDiscoveryTimeoutTimer()
{
   OPP_CHECK(MentorDiscoveryTimeoutTimer != NULL);
   delete cancelEvent(MentorDiscoveryTimeoutTimer);
   MentorDiscoveryTimeoutTimer = NULL;
}


// ###### Start Endpoint Keep Alive Transmission timer ######################
void RegistrarProcess::startEndpointKeepAliveTransmissionTimer(cPoolElement* poolElement)
{
   OPP_CHECK(poolElement->EndpointKeepAliveTransmissionTimer == NULL);
   OPP_CHECK(poolElement->EndpointKeepAliveTimeoutTimer == NULL);
   OPP_CHECK(poolElement->LifetimeExpiryTimer == NULL);
   poolElement->EndpointKeepAliveTransmissionTimer = new EndpointKeepAliveTransmissionMessage("EndpointKeepAliveTransmissionTimer");
   poolElement->EndpointKeepAliveTransmissionTimer->setContextPointer((void*)poolElement);
   scheduleAt(simTime() + (simtime_t)par("asapEndpointKeepAliveInterval"), poolElement->EndpointKeepAliveTransmissionTimer);
}


// ###### Stop Endpoint Keep Alive Transmission timer #######################
void RegistrarProcess::stopEndpointKeepAliveTransmissionTimer(cPoolElement* poolElement)
{
   OPP_CHECK(poolElement->EndpointKeepAliveTransmissionTimer);
   OPP_CHECK(poolElement->EndpointKeepAliveTimeoutTimer == NULL);
   OPP_CHECK(poolElement->LifetimeExpiryTimer == NULL);
   delete cancelEvent(poolElement->EndpointKeepAliveTransmissionTimer);
   poolElement->EndpointKeepAliveTransmissionTimer = NULL;
}


// ###### Start Endpoint Keep Alive Timeout timer ###########################
void RegistrarProcess::startEndpointKeepAliveTimeoutTimer(cPoolElement* poolElement)
{
   OPP_CHECK(poolElement->EndpointKeepAliveTransmissionTimer == NULL);
   OPP_CHECK(poolElement->EndpointKeepAliveTimeoutTimer == NULL);
   OPP_CHECK(poolElement->LifetimeExpiryTimer == NULL);
   poolElement->EndpointKeepAliveTimeoutTimer = new EndpointKeepAliveTimeoutMessage("EndpointKeepAliveTimeoutTimer");
   poolElement->EndpointKeepAliveTimeoutTimer->setContextPointer((void*)poolElement);
   scheduleAt(simTime() + (simtime_t)par("asapEndpointKeepAliveTimeout"), poolElement->EndpointKeepAliveTimeoutTimer);
}


// ###### Stop Endpoint Keep Alive Timeout timer ############################
void RegistrarProcess::stopEndpointKeepAliveTimeoutTimer(cPoolElement* poolElement)
{
   OPP_CHECK(poolElement->EndpointKeepAliveTransmissionTimer == NULL);
   OPP_CHECK(poolElement->EndpointKeepAliveTimeoutTimer);
   OPP_CHECK(poolElement->LifetimeExpiryTimer == NULL);
   delete cancelEvent(poolElement->EndpointKeepAliveTimeoutTimer);
   poolElement->EndpointKeepAliveTimeoutTimer = NULL;
}


// ###### Start Endpoint Keep Alive Timeout timer ###########################
void RegistrarProcess::startLifetimeExpiryTimer(cPoolElement* poolElement)
{
   OPP_CHECK(poolElement->EndpointKeepAliveTransmissionTimer == NULL);
   OPP_CHECK(poolElement->EndpointKeepAliveTimeoutTimer == NULL);
   OPP_CHECK(poolElement->LifetimeExpiryTimer == NULL);
   poolElement->LifetimeExpiryTimer = new LifetimeExpiryMessage("LifetimeExpiryTimer");
   poolElement->LifetimeExpiryTimer->setContextPointer((void*)poolElement);
   scheduleAt(simTime() + (poolElement->getRegistrationLife() / 1000.0),
              poolElement->LifetimeExpiryTimer);
}


// ###### Stop Endpoint Keep Alive Timeout timer ############################
void RegistrarProcess::stopLifetimeExpiryTimer(cPoolElement* poolElement)
{
   OPP_CHECK(poolElement->EndpointKeepAliveTransmissionTimer == NULL);
   OPP_CHECK(poolElement->EndpointKeepAliveTimeoutTimer == NULL);
   OPP_CHECK(poolElement->LifetimeExpiryTimer);
   delete cancelEvent(poolElement->LifetimeExpiryTimer);
   poolElement->LifetimeExpiryTimer = NULL;
}


// ###### Handle Endpoint Keep Alive Transmission timer #####################
void RegistrarProcess::handleEndpointKeepAliveTransmissionTimer(cPoolElement* poolElement)
{
   poolElement->EndpointKeepAliveTransmissionTimer = NULL;
   sendASAPEndpointKeepAlive(poolElement, false);
   startEndpointKeepAliveTimeoutTimer(poolElement);
}


// ###### Handle Endpoint Keep Alive Timeout timer ##########################
void RegistrarProcess::handleEndpointKeepAliveTimeoutTimer(cPoolElement* poolElement)
{
   poolElement->EndpointKeepAliveTimeoutTimer = NULL;
   TotalEndpointKeepAliveTimeouts++;

   EV << Description << "Pool element " << getPoolElementDescription(*poolElement)
      << " has not answered, removing it" << endl;
   sendENRPHandleUpdates(poolElement, DEL_PE);
   if(poolElement->EndpointKeepAliveTransmissionTimer) {
      stopEndpointKeepAliveTransmissionTimer(poolElement);
   }
   if(poolElement->EndpointKeepAliveTimeoutTimer) {
      stopEndpointKeepAliveTimeoutTimer(poolElement);
   }
   if(poolElement->LifetimeExpiryTimer) {
      stopLifetimeExpiryTimer(poolElement);
   }
   if(Handlespace->deregisterPoolElement(poolElement) != 0) {
      EV << Description << "Failed to remove pool element "
         << getPoolElementDescription(*poolElement)
         << " from handlespace!" << endl;
   }
   PoolElementCountVector->record(Handlespace->getPoolElements());
   OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());
}


// ###### Handle Endpoint Keep Alive Timeout timer ##########################
void RegistrarProcess::handleLifetimeExpiryTimer(cPoolElement* poolElement)
{
   poolElement->LifetimeExpiryTimer = NULL;
   TotalLifetimeExpiries++;

   EV << Description << "Pool element " << getPoolElementDescription(*poolElement)
      << "'s  lifetime has expired, removing it" << endl;
   if(Handlespace->deregisterPoolElement(poolElement) != 0) {
      EV << Description << "Failed to remove pool element "
         << getPoolElementDescription(*poolElement)
         << " from handlespace!" << endl;
   }
   PoolElementCountVector->record(Handlespace->getPoolElements());
   OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());
}


// ###### Start LastHeardTimeout timer ######################################
void RegistrarProcess::startLastHeardTimeoutTimer(cPeerListNode* node)
{
   OPP_CHECK(node->LastHeardTimeoutTimer == NULL);
   OPP_CHECK(node->ResponseTimeoutTimer == NULL);
   OPP_CHECK(node->TakeoverExpiryTimer == NULL);
   node->LastHeardTimeoutTimer = new LastHeardTimeoutMessage("LeastHeardTimeoutTimer");
   node->LastHeardTimeoutTimer->setContextPointer((void*)node);
   scheduleAt(simTime() + (simtime_t)par("enrpMaxTimeLastHeared"), node->LastHeardTimeoutTimer);
}


// ###### Stop LastHeardTimeout timer #######################################
void RegistrarProcess::stopLastHeardTimeoutTimer(cPeerListNode* node)
{
   OPP_CHECK(node->LastHeardTimeoutTimer != NULL);
   OPP_CHECK(node->ResponseTimeoutTimer == NULL);
   OPP_CHECK(node->TakeoverExpiryTimer == NULL);
   delete cancelEvent(node->LastHeardTimeoutTimer);
   node->LastHeardTimeoutTimer = NULL;
}


// ###### Start ResponseTimeout timer #######################################
void RegistrarProcess::startResponseTimeoutTimer(cPeerListNode* node)
{
   OPP_CHECK(node->LastHeardTimeoutTimer == NULL);
   OPP_CHECK(node->ResponseTimeoutTimer == NULL);
   OPP_CHECK(node->TakeoverExpiryTimer == NULL);
   node->ResponseTimeoutTimer = new ResponseTimeoutMessage("ResponseTimeoutTimer");
   node->ResponseTimeoutTimer->setContextPointer((void*)node);
   scheduleAt(simTime() + (simtime_t)par("enrpMaxTimeNoResponse"), node->ResponseTimeoutTimer);
}


// ###### Stop ResponseTimeout timer ########################################
void RegistrarProcess::stopResponseTimeoutTimer(cPeerListNode* node)
{
   OPP_CHECK(node->LastHeardTimeoutTimer == NULL);
   OPP_CHECK(node->ResponseTimeoutTimer != NULL);
   OPP_CHECK(node->TakeoverExpiryTimer == NULL);
   delete cancelEvent(node->ResponseTimeoutTimer);
   node->ResponseTimeoutTimer = NULL;
}


// ###### Start TakeoverExpiry timer ########################################
void RegistrarProcess::startTakeoverExpiryTimer(cPeerListNode* node)
{
   OPP_CHECK(node->LastHeardTimeoutTimer == NULL);
   OPP_CHECK(node->ResponseTimeoutTimer == NULL);
   OPP_CHECK(node->TakeoverExpiryTimer == NULL);
   OPP_CHECK(node->Takeover != NULL);
   node->TakeoverExpiryTimer = new TakeoverExpiryMessage("TakeoverExpiryTimer");
   node->TakeoverExpiryTimer->setContextPointer((void*)node);
   scheduleAt(simTime() + (simtime_t)par("enrpTakeoverExpiry"), node->TakeoverExpiryTimer);
}


// ###### Stop TakeoverExpiry timer #########################################
void RegistrarProcess::stopTakeoverExpiryTimer(cPeerListNode* node)
{
   OPP_CHECK(node->LastHeardTimeoutTimer == NULL);
   OPP_CHECK(node->ResponseTimeoutTimer == NULL);
   OPP_CHECK(node->TakeoverExpiryTimer != NULL);
   delete cancelEvent(node->TakeoverExpiryTimer);
   node->TakeoverExpiryTimer = NULL;
}


// ###### Handle Peer Heartbeat Cycle timer #################################
void RegistrarProcess::handlePeerHeartbeatCycleTimer()
{
   EV << Description << "Sending Peer Heartbeats ..." << endl;
   PeerList->print();

   cPeerListNode* node = PeerList->getFirstPeerListNode();
   while(node != NULL) {
      sendENRPPresence(node, false);
      node = PeerList->getNextPeerListNode(node);
   }

   startPeerHeartbeatCycleTimer();
}


// ###### Handle LastHeardTimeout timer #####################################
void RegistrarProcess::handleLastHeardTimeoutTimer(cPeerListNode* node)
{
   node->LastHeardTimeoutTimer = NULL;
   EV << Description << "LastHeard timeout for peer " << node->getIdentifier()
      << " -> requesting Presence" << endl;

   if(hasGUI()) {
      char str[256];
      snprintf((char*)&str, sizeof(str), "Peer %u (%u:%u) seems to be dead. Requesting ENRP Presence.",
               node->getIdentifier(), node->getAddress(), node->getPort());
      getParentModule()->bubble(str);
   }

   sendENRPPresence(node, true);
   startResponseTimeoutTimer(node);
}


// ###### Handle ResponseTimeout timer ######################################
void RegistrarProcess::handleResponseTimeoutTimer(cPeerListNode* node)
{
   node->ResponseTimeoutTimer = NULL;
   EV << Description << "Response timeout for peer " << node->getIdentifier()
      << " -> starting takeover" << endl;
   TotalTakeoversStarted++;

   if(hasGUI()) {
      char str[256];
      snprintf((char*)&str, sizeof(str), "Starting takeover of %u (%u:%u)",
               node->getIdentifier(), node->getAddress(), node->getPort());
      getParentModule()->bubble(str);
   }

   node->Takeover = new cTakeoverProcess(node->getIdentifier(), PeerList);
   startTakeoverExpiryTimer(node);
   sendENRPInitTakeovers(node->getIdentifier());
}


// ###### Handle TakeoverExpiry timer #######################################
void RegistrarProcess::handleTakeoverExpiryTimer(cPeerListNode* node)
{
   node->TakeoverExpiryTimer = NULL;
   EV << Description << "Takeover expiry for peer " << node->getIdentifier()
      << ". -> assuming consent for takeover." << endl;
   TotalTakeoversByTimeout++;
   finishTakeover(node);
}


// ###### Add static peer to peer list ######################################
void RegistrarProcess::addStaticPeer(const unsigned int peerAddress)
{
   if(peerAddress != LocalAddress) {
      ServerInformationParameter serverInformationParameter;
      serverInformationParameter.setServerID(0);
      serverInformationParameter.setAddress(peerAddress);
      serverInformationParameter.setPort(RegistrarPort);

      cPeerListNode* node;
      OPP_CHECK(PeerList->registerPeerListNode(serverInformationParameter, node) == RSPERR_OKAY);
   }
}


// ###### Dump handlespace ##################################################
void RegistrarProcess::printHandlespace()
{
   EV << Description << endl
      << "===== Handlespace Dump ======" << endl;
   Handlespace->print();
   EV << "===========================" << endl;
}


// ###### Handle ASAP_REGISTRATION message ##################################
void RegistrarProcess::handleASAPRegistration(ASAPRegistration* msg)
{
   TotalRegistrations++;

   // ====== Create response ================================================
   ASAPRegistrationResponse* response = new ASAPRegistrationResponse("ASAP_REGISTRATION_RESPONSE", ASAP);
   response->setProtocol(ASAP);
   response->setDstAddress(msg->getSrcAddress());
   response->setSrcPort(RegistrarPort);
   response->setDstPort(msg->getSrcPort());
   response->setPoolHandle(msg->getPoolHandle());
   response->setIdentifier(msg->getPoolElementParameter().getIdentifier());

   if( (!inStartupPhase()) || ((bool)par("asapNoServiceDuringStartup") == false) ) {
      msg->getPoolElementParameter().setHomeRegistrarIdentifier(MyIdentifier);
      if(msg->getSrcAddress() != msg->getPoolElementParameter().getUserTransportParameter().getAddress()) {
         error("Registration for address %u from address %u!",
               msg->getPoolElementParameter().getUserTransportParameter().getAddress(), msg->getSrcAddress());
      }

      // ====== Set distance ===================================================
      const simtime_t delay = simTime() - msg->getTimestamp();
      msg->getPoolElementParameter().getPoolPolicyParameter().setDistance((unsigned int)ceil(1000.0 * delay.dbl()));

      // ====== Register pool element ==========================================
      cPoolElement* poolElement;
      bool          updated;
      response->setError(Handlespace->registerPoolElement(msg->getPoolHandle(),
                                                          msg->getPoolElementParameter(),
                                                          msg->getSrcAddress(),
                                                          msg->getSrcPort(),
                                                          poolElement, updated));
      if(response->getError() == 0) {
         EV << Description << "Added or updated pool element "
            << getPoolElementDescription(*poolElement) << endl;
         sendENRPHandleUpdates(poolElement, ADD_PE);
         if(poolElement->LifetimeExpiryTimer) {
            stopLifetimeExpiryTimer(poolElement);
         }
         if((poolElement->EndpointKeepAliveTransmissionTimer == NULL) &&
            (poolElement->EndpointKeepAliveTimeoutTimer == NULL)) {
            EV << Description << "Pool element is new -> "
               << "starting endpoint keep alive transmission timer ..." << endl;
            startEndpointKeepAliveTransmissionTimer(poolElement);
         }
      }
      else {
         EV << Description << "Cannot add new pool element" << endl;
      }

      PoolElementCountVector->record(Handlespace->getPoolElements());
      OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());
      printHandlespace();
   }
   else {
      EV << Description << "Rejecting registration request in startup phase" << endl;
      response->setRejectFlag(true);
   }

   response->setTimestamp(simTime());
   send(response, "toTransport");
}


// ###### Handle ASAP_DEREGISTRATION message ################################
void RegistrarProcess::handleASAPDeregistration(ASAPDeregistration* msg)
{
   TotalDeregistrations++;

   ASAPDeregistrationResponse* response = new ASAPDeregistrationResponse("ASAP_DEREGISTRATION_RESPONSE", ASAP);
   response->setProtocol(ASAP);
   response->setDstAddress(msg->getSrcAddress());
   response->setSrcPort(RegistrarPort);
   response->setDstPort(msg->getSrcPort());
   response->setPoolHandle(msg->getPoolHandle());
   response->setIdentifier(msg->getIdentifier());

   cPoolElement* poolElement = Handlespace->findPoolElement(msg->getPoolHandle(),
                                                            msg->getIdentifier());
   if(poolElement) {
      EV << Description << "Removing pool element "
         << getPoolElementDescription(*poolElement) << " ..." << endl;
      // ====== Take ownership, if necessary ================================
      if(poolElement->getHomeRegistrarIdentifier() != MyIdentifier) {
         Handlespace->updatePoolElementOwnership(poolElement, MyIdentifier);
      }

      // ====== Send updates ================================================
      sendENRPHandleUpdates(poolElement, DEL_PE);

      // ====== Deregister pool element =====================================
      if(poolElement->EndpointKeepAliveTransmissionTimer) {
         stopEndpointKeepAliveTransmissionTimer(poolElement);
      }
      if(poolElement->EndpointKeepAliveTimeoutTimer) {
         stopEndpointKeepAliveTimeoutTimer(poolElement);
      }
      if(poolElement->LifetimeExpiryTimer) {
         stopLifetimeExpiryTimer(poolElement);
      }
      Handlespace->deregisterPoolElement(poolElement);
      response->setError(RSPERR_OKAY);
   }
   else {
      EV << Description << "Pool element "
         << msg->getIdentifier() << " of pool " << msg->getPoolHandle()
         << " is found, it seems to be already removed" << endl;
      response->setError(RSPERR_OKAY);
   }

   PoolElementCountVector->record(Handlespace->getPoolElements());
   OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());
   printHandlespace();

   response->setTimestamp(simTime());
   send(response, "toTransport");
}


// ###### Randomize MaxHandleResolutionItems ################################
unsigned int RegistrarProcess::randomizeMaxHandleResolutionItems(const unsigned int maxHandleResolutionItems,
                                                                 const char*        poolHandle)
{
   unsigned int maxItems = std::min((unsigned int)Handlespace->getPoolElementsOfPool(poolHandle),
                                    maxHandleResolutionItems + 1);
   if(maxItems > 1) {
      maxItems--;
   }
   else {
      maxItems = 1;
   }
   const unsigned int items = intuniform(1, maxItems);
   return(items);
}


// ###### Handle ASAP_HANDLE_RESOLUTION message #############################
void RegistrarProcess::handleASAPHandleResolution(ASAPHandleResolution* msg)
{
   TotalHandleResolutions++;

   if((double)par("registrarMaxHandleResolutionRate") > 0.0) {
      const double handleResolutionRate =
         UserList.noteHandleResolutionOfPoolUser(msg->getPoolHandle(),
                                                 msg->getSrcAddress(),
                                                 msg->getSrcPort(),
                                                 par("registrarHandleResolutionRateBuckets"),
                                                 par("registrarHandleResolutionRateMaxEntries"));
      if(handleResolutionRate > (double)par("registrarMaxHandleResolutionRate")) {
         TotalRefusedHandleResolutions++;
         EV << Description << "Refusing handle resolution for pool user at "
                           << msg->getSrcAddress() << ":" << msg->getSrcPort() << endl;
         return;
      }
   }

   ASAPHandleResolutionResponse* response = new ASAPHandleResolutionResponse("ASAP_HANDLE_RESOLUTION_RESPONSE", ASAP);
   response->setProtocol(ASAP);
   response->setDstAddress(msg->getSrcAddress());
   response->setSrcPort(RegistrarPort);
   response->setDstPort(msg->getSrcPort());
   response->setPoolHandle(msg->getPoolHandle());

   if( (!inStartupPhase()) || ((bool)par("asapNoServiceDuringStartup") == false) ) {
      size_t items;
      if((bool)par("registrarRandomizeMaxHandleResolutionItems") == true) {
         items = randomizeMaxHandleResolutionItems(
                  (unsigned int)par("registrarMaxHandleResolutionItems"),
                  msg->getPoolHandle());
      }
      else {
         items = (unsigned int)par("registrarMaxHandleResolutionItems");
      }

      EV << Description << "Selecting up to " << items << " of pool "
         << msg->getPoolHandle() << " ..." << endl;

      cPoolElement* selectionArray[items];
      unsigned int policyType = Handlespace->selectPoolElementsByPolicy(
                                 msg->getPoolHandle(),
                                 (cPoolElement**)&selectionArray, items,
                                 items,
                                 (unsigned int)par("registrarMaxIncrement"));

      EV << Description << "Selected pool elements:" << endl;
      for(unsigned int i = 0;i < items;i++) {
         EV << "   ";
         selectionArray[i]->print(true);
         EV << endl;
      }

      response->setPoolHandle(msg->getPoolHandle());
      cPoolPolicyParameter overallPoolElementSelectionPolicy;
      overallPoolElementSelectionPolicy.setPolicyType(policyType);
      response->setOverallPoolElementSelectionPolicy(overallPoolElementSelectionPolicy);
      response->setPoolElementParameterArraySize(items);
      for(unsigned int i = 0;i < items;i++) {
         response->setPoolElementParameter(i, selectionArray[i]->toPoolElementParameter());
      }
   }
   else {
      EV << Description << "Rejecting handle resolution request in startup phase" << endl;
      response->setRejectFlag(true);
   }

   response->setTimestamp(simTime());
   send(response, "toTransport");
}


// ###### Handle ASAP_ENDPOINT_UNREACHABLE message ##########################
void RegistrarProcess::handleASAPEndpointUnreachable(ASAPEndpointUnreachable* msg)
{
   TotalEndpointUnreachables++;

   if((double)par("registrarMaxEndpointUnreachableRate") > 0.0) {
      const double endpointUnreachableRate =
         UserList.noteEndpointUnreachableOfPoolUser(msg->getPoolHandle(),
                                                    msg->getSrcAddress(),
                                                    msg->getSrcPort(),
                                                    0, /* msg->getIdentifier(), --- only for full pool! --- */
                                                    par("registrarEndpointUnreachableRateBuckets"),
                                                    par("registrarEndpointUnreachableRateMaxEntries"));

      if(endpointUnreachableRate > (double)par("registrarMaxEndpointUnreachableRate")) {
         TotalRefusedEndpointUnreachables++;
         EV << Description << "Refusing handle resolution for pool user at "
                           << msg->getSrcAddress() << ":" << msg->getSrcPort() << endl;
         return;
      }
   }

   cPoolElement* poolElement = Handlespace->findPoolElement(msg->getPoolHandle(),
                                                            msg->getIdentifier());
   if(poolElement) {
      poolElement->setUnreachabilityReports(poolElement->getUnreachabilityReports() + 1);
      EV << Description << "Failure reported for pool element ";
      poolElement->print(true);
      EV << endl;

      if(poolElement->getUnreachabilityReports() >= (unsigned int)par("registrarMaxBadPEReports")) {
         EV << Description << "Too many unreachability reports -> removing it ..." << endl;
         if(poolElement->EndpointKeepAliveTransmissionTimer) {
            stopEndpointKeepAliveTransmissionTimer(poolElement);
         }
         if(poolElement->EndpointKeepAliveTimeoutTimer) {
            stopEndpointKeepAliveTimeoutTimer(poolElement);
         }
         if(poolElement->LifetimeExpiryTimer) {
            stopLifetimeExpiryTimer(poolElement);
         }
         if(poolElement->getHomeRegistrarIdentifier() == MyIdentifier) {
            sendENRPHandleUpdates(poolElement, DEL_PE);
         }
         Handlespace->deregisterPoolElement(poolElement);

         PoolElementCountVector->record(Handlespace->getPoolElements());
         OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());
      }
   }
}


// ###### Send ASAP_ENDPOINT_KEEP_ALIVE #####################################
void RegistrarProcess::sendASAPEndpointKeepAlive(const cPoolElement* poolElement,
                                                 const bool          homeFlag)
{
   ASAPEndpointKeepAlive* endpointKeepAlive = new ASAPEndpointKeepAlive("ASAP_ENDPOINT_KEEP_ALIVE", ASAP);
   endpointKeepAlive->setProtocol(ASAP);
   endpointKeepAlive->setDstAddress(poolElement->getRegistratorAddress());
   endpointKeepAlive->setDstPort(poolElement->getRegistratorPort());
   endpointKeepAlive->setSrcPort(RegistrarPort);
   endpointKeepAlive->setPoolHandle(poolElement->getOwnerPoolHandle());
   endpointKeepAlive->setIdentifier(poolElement->getIdentifier());
   endpointKeepAlive->setHomeFlag(homeFlag);

   endpointKeepAlive->setTimestamp(simTime());
   send(endpointKeepAlive, "toTransport");
   TotalEndpointKeepAlivesSent++;
}


// ###### Handle ASAP_ENDPOINT_KEEP_ALIVE_ACK message #######################
void RegistrarProcess::handleASAPEndpointKeepAliveAck(ASAPEndpointKeepAliveAck* msg)
{
   cPoolElement* poolElement = Handlespace->findPoolElement(msg->getPoolHandle(),
                                                            msg->getIdentifier());
   if((poolElement) && (poolElement->EndpointKeepAliveTimeoutTimer)) {
      stopEndpointKeepAliveTimeoutTimer(poolElement);
      startEndpointKeepAliveTransmissionTimer(poolElement);
      EV << Description << "Endpoint is alive" << endl;
      TotalEndpointKeepAliveAcksReceived++;
   }
   else {
      EV << Description << "Unexpected ASAP_ENDPOINT_KEEP_ALIVE_ACK!" << endl;
   }
}


// ###### Send ENRP_LIST_REQUEST message ####################################
void RegistrarProcess::sendENRPListRequest(cPeerListNode* node)
{
   OPP_CHECK(!(node->getStatus() & PLNS_LISTSYNC));
   node->setStatus(node->getStatus() | PLNS_LISTSYNC);

   ENRPListRequest* peerListRequest = new ENRPListRequest("ENRP_LIST_REQUEST", ENRP);
   peerListRequest->setProtocol(ENRP);
   peerListRequest->setDstAddress(node->getAddress());
   peerListRequest->setDstPort(node->getPort());
   peerListRequest->setSrcPort(RegistrarPort);
   peerListRequest->setSenderServerID(MyIdentifier);
   peerListRequest->setReceiverServerID(node->getIdentifier());

   peerListRequest->setTimestamp(simTime());
   send(peerListRequest, "toTransport");
}


// ###### Handle ENRP_LIST_REQUEST message ##################################
void RegistrarProcess::handleENRPListRequest(ENRPListRequest* msg)
{
   OPP_CHECK(msg->getSenderServerID() != MyIdentifier);

   ENRPListResponse* peerListResponse = new ENRPListResponse("ENRP_LIST_RESPONSE", ENRP);
   peerListResponse->setProtocol(ENRP);
   peerListResponse->setDstAddress(msg->getSrcAddress());
   peerListResponse->setSrcPort(RegistrarPort);
   peerListResponse->setDstPort(msg->getSrcPort());
   peerListResponse->setSenderServerID(MyIdentifier);
   peerListResponse->setReceiverServerID(msg->getSenderServerID());

   ServerInformationParameter serverInfoArray[PeerList->getPeers() + 1];
   serverInfoArray[0] = OwnServerInfo;
   size_t peers = 1;
   cPeerListNode* node = PeerList->getFirstPeerListNode();
   while(node != NULL) {
      if(node->getIdentifier() != UNDEFINED_REGISTRAR_IDENTIFIER) {
         serverInfoArray[peers] = node->toServerInformationParameter();
         peers++;
      }
      node = PeerList->getNextPeerListNode(node);
   }

   peerListResponse->setServerInformationArraySize(peers);
   for(size_t i = 0;i < peers;i++) {
      peerListResponse->setServerInformation(i, serverInfoArray[i]);
   }

   peerListResponse->setTimestamp(simTime());
   send(peerListResponse, "toTransport");
}


// ###### Send ENRP_LIST_RESPONSE message ###################################
bool RegistrarProcess::handleENRPListResponse(ENRPListResponse* msg)
{
   OPP_CHECK(msg->getSenderServerID() != MyIdentifier);

   cPeerListNode* node = PeerList->findPeerListNode(msg->getSenderServerID());
   if(msg->getRejectFlag() == 0) {
      EV << Description << "Adding peers to Peer List ..." << endl;
      for(unsigned int i = 0;i < msg->getServerInformationArraySize();i++) {
         ServerInformationParameter serverInfo = msg->getServerInformation(i);
         if(serverInfo.getServerID() != MyIdentifier) {
            OPP_CHECK(PeerList->registerPeerListNode(serverInfo, node) == RSPERR_OKAY);
            if(node->getNewFlag())  {
               EV << Description << "Sending Presence to new peer "
                  << node->getIdentifier() << endl;
               sendENRPPresence(node, true);
            }
         }
      }
      PeerList->print();
      return(true);
   }
   else {
      EV << Description << "Peer rejected peer list request!" << endl;
      return(false);
   }
}


// ###### Send ENRP_HANDLE_TABLE_REQUEST message ############################
void RegistrarProcess::sendENRPHandleTableRequest(cPeerListNode* node,
                                                  const bool     ownChildrenOnly)
{
   OPP_CHECK(!(node->getStatus() & PLNS_HTSYNC));
   node->setStatus(node->getStatus() | PLNS_HTSYNC);

   ENRPHandleTableRequest* handleTableRequest = new ENRPHandleTableRequest("ENRP_HANDLE_TABLE_REQUEST", ENRP);
   handleTableRequest->setProtocol(ENRP);
   handleTableRequest->setDstAddress(node->getAddress());
   handleTableRequest->setDstPort(node->getPort());
   handleTableRequest->setSrcPort(RegistrarPort);
   handleTableRequest->setSenderServerID(MyIdentifier);
   handleTableRequest->setReceiverServerID(node->getIdentifier());
   handleTableRequest->setOwnChildrenOnlyFlag(ownChildrenOnly);

   handleTableRequest->setTimestamp(simTime());
   send(handleTableRequest, "toTransport");
}


// ###### Handle ENRP_HANDLE_TABLE_REQUEST message ##########################
void RegistrarProcess::handleENRPHandleTableRequest(ENRPHandleTableRequest* msg)
{
   OPP_CHECK(msg->getSenderServerID() != MyIdentifier);

   ENRPHandleTableResponse* handleTableResponse = new ENRPHandleTableResponse("ENRP_HANDLE_TABLE_RESPONSE", ENRP);
   handleTableResponse->setProtocol(ENRP);
   handleTableResponse->setDstAddress(msg->getSrcAddress());
   handleTableResponse->setSrcPort(RegistrarPort);
   handleTableResponse->setDstPort(msg->getSrcPort());
   handleTableResponse->setSenderServerID(MyIdentifier);
   handleTableResponse->setReceiverServerID(msg->getSenderServerID());
   handleTableResponse->setRejectFlag(false);
   handleTableResponse->setMoreToSendFlag(false);

   cArray* poolEntryArray = Handlespace->exportToPoolEntries(
                               msg->getOwnChildrenOnlyFlag() ? MyIdentifier : 0);
   if(poolEntryArray) {
      handleTableResponse->setPoolEntryArraySize(poolEntryArray->size());
      for(int i = 0;i < poolEntryArray->size();i++) {
         cPoolEntry* poolEntry = (cPoolEntry*)(*poolEntryArray)[i];
         handleTableResponse->setPoolEntry(i, *poolEntry);
      }
      delete poolEntryArray;
   }

   handleTableResponse->setTimestamp(simTime());
   send(handleTableResponse, "toTransport");
}


// ###### Handle ENRP_HANDLE_TABLE_RESPONSE message #########################
bool RegistrarProcess::handleENRPHandleTableResponse(ENRPHandleTableResponse* msg)
{
   OPP_CHECK(msg->getSenderServerID() != MyIdentifier);

   cPeerListNode* node = PeerList->findPeerListNode(msg->getSenderServerID());
   if(node) {
      node->setStatus(node->getStatus() & ~PLNS_HTSYNC);
   }

   if(msg->getRejectFlag() == 0) {
      // ====== Update handlespace ==========================================
      EV << Description << "Adding handle table content to handlespace ..." << endl;
      for(unsigned int i = 0;i < msg->getPoolEntryArraySize();i++) {
         EV << "Adding pool element "
            << msg->getPoolEntry(i).getPoolElementParameter().getIdentifier()
            << " of pool " << msg->getPoolEntry(i).getPoolHandle() << " ..." << endl;

         OPP_CHECK(msg->getPoolEntry(i).getPoolElementParameter().getHomeRegistrarIdentifier() != MyIdentifier);
         cPoolElement* poolElement;
         bool          updated;
         if(Handlespace->registerPoolElement(msg->getPoolEntry(i).getPoolHandle(),
                                             msg->getPoolEntry(i).getPoolElementParameter(),
                                             0, 0,
                                             poolElement, updated) == RSPERR_OKAY) {
            if(poolElement->EndpointKeepAliveTransmissionTimer) {
               stopEndpointKeepAliveTransmissionTimer(poolElement);
            }
            if(poolElement->EndpointKeepAliveTimeoutTimer) {
               stopEndpointKeepAliveTimeoutTimer(poolElement);
            }
            if(poolElement->LifetimeExpiryTimer) {
               stopLifetimeExpiryTimer(poolElement);
            }
            if(poolElement->getHomeRegistrarIdentifier() == MyIdentifier) {
               EV << Description << "Handle Table from " << msg->getSenderServerID()
                  << " made me home registrar of pool element " << poolElement->getIdentifier() << endl;
               startEndpointKeepAliveTransmissionTimer(poolElement);
            }
            else {
               startLifetimeExpiryTimer(poolElement);
            }
         }
      }
      PoolElementCountVector->record(Handlespace->getPoolElements());
      OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());

      // ====== Handlespace learned from mentor =============================
      if( (inStartupPhase()) && (node->getStatus() & PLNS_MENTOR) ) {
         node->setStatus(node->getStatus() & ~PLNS_MENTOR);
         beginNormalOperation(true);
      }

      return(true);
   }
   else {
      EV << Description << "Peer rejected peer list request!" << endl;
      if( (inStartupPhase()) && (node->getStatus() & PLNS_MENTOR) ) {
         node->setStatus(node->getStatus() & ~PLNS_MENTOR);
         beginNormalOperation(false);
      }
      return(false);
   }
}


// ###### Send ENRP_PRESENCE ################################################
void RegistrarProcess::sendENRPPresence(const cPeerListNode* node,
                                        const bool           replyRequired)
{
   ENRPPresence* peerPresence = new ENRPPresence("ENRP_PRESENCE", ENRP);
   peerPresence->setProtocol(ENRP);
   peerPresence->setSrcAddress(LocalAddress);
   peerPresence->setSrcPort(RegistrarPort);
   peerPresence->setDstAddress(node->getAddress());
   peerPresence->setDstPort(node->getPort());
   peerPresence->setSenderServerID(MyIdentifier);
   peerPresence->setReceiverServerID(node->getIdentifier());
   peerPresence->setReplyRequiredFlag(replyRequired);
   peerPresence->setChecksum(Handlespace->getOwnershipChecksum());
   peerPresence->setServerInformation(OwnServerInfo);

   peerPresence->setTimestamp(simTime());
   send(peerPresence, "toTransport");
}


// ###### Handle ENRP_PRESENCE message ######################################
void RegistrarProcess::handleENRPPresence(ENRPPresence* msg)
{
   // ====== Add/update peer node ===========================================
   cPeerListNode* node;
   OPP_CHECK(PeerList->registerPeerListNode(msg->getServerInformation(), node) == RSPERR_OKAY);
   EV << Description << "New peer list is: " << endl;
   PeerList->print();

   // ====== Timer handling =================================================
   if(node->LastHeardTimeoutTimer) {
      stopLastHeardTimeoutTimer(node);
   }
   if(node->ResponseTimeoutTimer) {
      stopResponseTimeoutTimer(node);
   }
   if(node->TakeoverExpiryTimer) {
      EV << "Peer " << node->getIdentifier()
         << " is still alive -> stopping takeover" << endl;
      stopTakeoverExpiryTimer(node);
      delete node->Takeover;
      node->Takeover = NULL;
   }
   startLastHeardTimeoutTimer(node);

   // ====== Reply required? ================================================
   if(msg->getReplyRequiredFlag()) {
      TotalRequestedPresences++;
      sendENRPPresence(node, false);
   }
   // ====== Presence to new peer ===========================================
   else {
      if(node->getNewFlag())  {
         EV << Description << "Sending Presence to new peer "
            << node->getIdentifier() << endl;
         sendENRPPresence(node, false);
      }
   }

   // ====== Mentor query ===================================================
   if( (inStartupPhase()) &&
       (MentorServerID == UNDEFINED_REGISTRAR_IDENTIFIER) &&
       (!(node->getStatus() & PLNS_HTSYNC)) ) {

      EV << Description << "Using peer " << node->getIdentifier()
         << " as mentor" << endl;
      node->setStatus(node->getStatus() | PLNS_MENTOR);
      MentorServerID = node->getIdentifier();

      if(hasGUI()) {
         char str[256];
         snprintf((char*)&str, sizeof(str), "Using peer at %u (%u:%u) as mentor",
                  node->getIdentifier(), node->getAddress(), node->getPort());
         getParentModule()->bubble(str);
      }

      TotalHandleTableRequests++;
      TotalPeerListRequests++;
      sendENRPListRequest(node);
      sendENRPHandleTableRequest(node, true);
   }

   // ====== Resolve handlespace inconsistencies ============================
   if(!(node->getStatus() & PLNS_HTSYNC)) {
      if(node->getOwnershipChecksum() != msg->getChecksum()) {
         if(hasGUI()) {
            char str[256];
            snprintf((char*)&str, sizeof(str), "Synchronizing with %u (%u:%u)",
                     node->getIdentifier(), node->getAddress(), node->getPort());
            getParentModule()->bubble(str);
         }
         TotalHandleTableRequests++;
         sendENRPHandleTableRequest(node, true);
      }
   }

   // ====== Get peer's server list =========================================
   if(!(node->getStatus() & PLNS_LISTSYNC)) {
      if(hasGUI()) {
         char str[256];
         snprintf((char*)&str, sizeof(str), "Requesting list from %u (%u:%u)",
                  node->getIdentifier(), node->getAddress(), node->getPort());
         getParentModule()->bubble(str);
      }
      TotalPeerListRequests++;
      sendENRPListRequest(node);
   }
}


// ###### Send ENRP_UPDATE message ##########################################
void RegistrarProcess::sendENRPHandleUpdates(const cPoolElement* poolElement,
                                             const unsigned int  updateAction)
{
   EV << Description << "Sending Handle Update: "
      << ((updateAction == ADD_PE) ? "ADD_PE" : "DEL_PE")
      << " for pool element " << poolElement->getIdentifier()
      << " of pool " << poolElement->getOwnerPoolHandle() << endl;


   // ====== Get more useful PR, if possible ================================
  cPeerListNode* betterPeerForPE = NULL;
   if( (updateAction == ADD_PE) &&
       (par("asapUseTakeoverSuggestion")) ) {
      betterPeerForPE = PeerList->getUsefulPeerForPE(poolElement->getIdentifier());
      if(betterPeerForPE) {
         printf("Peer $%08x is more useful than me ($%08x) for PE $%08x\n",
                MyIdentifier, betterPeerForPE->getIdentifier(),
                poolElement->getIdentifier());
      }
   }


   cPeerListNode* node = PeerList->getFirstPeerListNode();
   while(node != NULL) {
      if(node->getIdentifier() != UNDEFINED_REGISTRAR_IDENTIFIER) {
         ENRPHandleUpdate* handleUpdate = new ENRPHandleUpdate("ENRP_HANDLE_UPDATE", ENRP);
         handleUpdate->setProtocol(ENRP);
         handleUpdate->setSrcAddress(LocalAddress);
         handleUpdate->setSrcPort(RegistrarPort);
         handleUpdate->setDstAddress(node->getAddress());
         handleUpdate->setDstPort(node->getPort());
         handleUpdate->setSenderServerID(MyIdentifier);
         handleUpdate->setReceiverServerID(node->getIdentifier());
         handleUpdate->setUpdateAction(updateAction);
         handleUpdate->setPoolHandle(poolElement->getOwnerPoolHandle());
         handleUpdate->setPoolElementParameter(poolElement->toPoolElementParameter());

         handleUpdate->setTakeoverSuggested( (node == betterPeerForPE) );

         handleUpdate->setTimestamp(simTime());
         send(handleUpdate, "toTransport");
      }
      node = PeerList->getNextPeerListNode(node);
   }
}


// ###### Handle ENRP_UPDATE message ########################################
void RegistrarProcess::handleENRPHandleUpdate(ENRPHandleUpdate* msg)
{
   OPP_CHECK(msg->getReceiverServerID() == MyIdentifier);

   if(uniform(0.0, 1.0) <= (double)par("registrarUpdateLossProbability")) {
      EV << "Dropping incoming ENRP handle update due to registrarUpdateLoss setting" << endl;
      return;
   }

   TotalHandleUpdates++;

   EV << Description << "Received Handle Update: "
      << ((msg->getUpdateAction() == ADD_PE) ? "ADD_PE" : "DEL_PE")
      << " for pool element " << msg->getPoolElementParameter().getIdentifier()
      << " of pool " << msg->getPoolHandle() << endl;
   OPP_CHECK(msg->getPoolElementParameter().getHomeRegistrarIdentifier() != MyIdentifier);

   if(msg->getUpdateAction() == ADD_PE) {
      // ====== Set distance ================================================
      const simtime_t delay = simTime() - msg->getTimestamp();
      msg->getPoolElementParameter().getPoolPolicyParameter().setDistance(
         msg->getPoolElementParameter().getPoolPolicyParameter().getDistance() +
         (unsigned int)ceil(1000.0 * delay.dbl()));

      // ====== Register pool element =======================================
      cPoolElement* poolElement;
      bool          updated;
      if(Handlespace->registerPoolElement(msg->getPoolHandle(), msg->getPoolElementParameter(),
                                          0, 0, poolElement, updated) == RSPERR_OKAY) {
         if(poolElement->EndpointKeepAliveTransmissionTimer) {
            stopEndpointKeepAliveTransmissionTimer(poolElement);
         }
         if(poolElement->EndpointKeepAliveTimeoutTimer) {
            stopEndpointKeepAliveTimeoutTimer(poolElement);
         }
         if(poolElement->LifetimeExpiryTimer) {
            stopLifetimeExpiryTimer(poolElement);
         }
         startLifetimeExpiryTimer(poolElement);
      }

      // ====== Handle takeover suggestion ==================================
      if(msg->getTakeoverSuggested()) {
         sendASAPEndpointKeepAlive(poolElement, true);
      }
   }
   else {
      cPoolElement* poolElement = Handlespace->findPoolElement(
                                     msg->getPoolHandle(),
                                     msg->getPoolElementParameter().getIdentifier());
      if(poolElement) {
         if(poolElement->EndpointKeepAliveTransmissionTimer) {
            stopEndpointKeepAliveTransmissionTimer(poolElement);
         }
         if(poolElement->EndpointKeepAliveTimeoutTimer) {
            stopEndpointKeepAliveTimeoutTimer(poolElement);
         }
         if(poolElement->LifetimeExpiryTimer) {
            stopLifetimeExpiryTimer(poolElement);
         }
         Handlespace->deregisterPoolElement(poolElement);
      }
   }

   PoolElementCountVector->record(Handlespace->getPoolElements());
   OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());
   printHandlespace();
}


// ###### Send ENRP_INIT_TAKEOVER messages ##################################
void RegistrarProcess::sendENRPInitTakeovers(const unsigned int targetServerID)
{
   EV << Description << "Sending Init Takeover for peer "
      << targetServerID << endl;

   cPeerListNode* node = PeerList->getFirstPeerListNode();
   while(node != NULL) {
      if(node->getIdentifier() != UNDEFINED_REGISTRAR_IDENTIFIER) {
         ENRPInitTakeover* initTakeover = new ENRPInitTakeover("ENRP_INIT_TAKEOVER", ENRP);
         initTakeover->setProtocol(ENRP);
         initTakeover->setSrcAddress(LocalAddress);
         initTakeover->setSrcPort(RegistrarPort);
         initTakeover->setDstAddress(node->getAddress());
         initTakeover->setDstPort(node->getPort());
         initTakeover->setSenderServerID(MyIdentifier);
         initTakeover->setReceiverServerID(node->getIdentifier());
         initTakeover->setTargetServerID(targetServerID);

         initTakeover->setTimestamp(simTime());
         send(initTakeover, "toTransport");
      }
      node = PeerList->getNextPeerListNode(node);
   }
}


// ###### Send ENRP_INIT_TAKEOVER_ACK messages ##############################
void RegistrarProcess::sendENRPInitTakeoverAck(const unsigned int targetServerID,
                                               const unsigned int receiverServerID,
                                               const unsigned int receiverServerAddress,
                                               const unsigned int receiverServerPort)
{
   EV << Description << "Sending Init Takeover Ack for peer "
      << targetServerID << " to peer " << receiverServerID << endl;

   ENRPInitTakeoverAck* initTakeoverAck = new ENRPInitTakeoverAck("ENRP_INIT_TAKEOVER_ACK", ENRP);
   initTakeoverAck->setProtocol(ENRP);
   initTakeoverAck->setSrcAddress(LocalAddress);
   initTakeoverAck->setSrcPort(RegistrarPort);
   initTakeoverAck->setDstAddress(receiverServerAddress);
   initTakeoverAck->setDstPort(receiverServerPort);
   initTakeoverAck->setSenderServerID(MyIdentifier);
   initTakeoverAck->setReceiverServerID(receiverServerID);
   initTakeoverAck->setTargetServerID(targetServerID);

   initTakeoverAck->setTimestamp(simTime());
   send(initTakeoverAck, "toTransport");
}


// ###### Send ENRP_TAKEOVER_SERVER messages ################################
void RegistrarProcess::sendENRPTakeoverServers(const unsigned int targetServerID)
{
   EV << Description << "Sending Takeover Server for peer "
      << targetServerID << endl;

   cPeerListNode* node = PeerList->getFirstPeerListNode();
   while(node != NULL) {
      if(node->getIdentifier() != UNDEFINED_REGISTRAR_IDENTIFIER) {
         ENRPTakeoverServer* initTakeover = new ENRPTakeoverServer("ENRP_TAKEOVER_SERVER", ENRP);
         initTakeover->setProtocol(ENRP);
         initTakeover->setSrcAddress(LocalAddress);
         initTakeover->setSrcPort(RegistrarPort);
         initTakeover->setDstAddress(node->getAddress());
         initTakeover->setDstPort(node->getPort());
         initTakeover->setSenderServerID(MyIdentifier);
         initTakeover->setReceiverServerID(node->getIdentifier());
         initTakeover->setTargetServerID(targetServerID);

         initTakeover->setTimestamp(simTime());
         send(initTakeover, "toTransport");
      }
      node = PeerList->getNextPeerListNode(node);
   }
}


// ###### Handle ENRP_INIT_TAKEOVER message #################################
void RegistrarProcess::handleENRPInitTakeover(ENRPInitTakeover* msg)
{
   OPP_CHECK(msg->getSenderServerID() != MyIdentifier);

   EV << Description << "Init Takeover for registrar " << msg->getTargetServerID()
      << " from registrar " << msg->getSenderServerID() << endl;

   // ====== We are the takeover target -> try to stop it by Presence =======
   if(msg->getTargetServerID() == MyIdentifier) {
      EV << Description << "We are takeover target -> trying to stop this" << endl;
      if(hasGUI()) {
         char str[256];
         snprintf((char*)&str, sizeof(str), "%u (%u:%u) is trying to take me over!!!",
                  msg->getSenderServerID(), msg->getSrcAddress(), msg->getSrcPort());
         getParentModule()->bubble(str);
      }

      cPeerListNode* node = PeerList->getFirstPeerListNode();
      while(node != NULL) {
         sendENRPPresence(node, false);
         node = PeerList->getNextPeerListNode(node);
      }
   }

   //====== Another peer is the takeover target =============================
   else {
      cPeerListNode* node = PeerList->findPeerListNode(msg->getTargetServerID());
      if(node) {
         // ====== We have also started a takeover -> negotiation required ==
         if(node->Takeover != NULL) {
            if(msg->getSenderServerID() < MyIdentifier) {
               EV << Description << "Peer " << msg->getSenderServerID()
                  << ", also trying takeover of " << msg->getTargetServerID()
                  << ", has lower ID -> we (" << MyIdentifier << ") abort our takeover" << endl;
               if(hasGUI()) {
                  char str[256];
                  snprintf((char*)&str, sizeof(str), "%u has higher ID than me (%u) -> aborting takeover of %u (%u:%u)",
                           msg->getSenderServerID(), MyIdentifier,
                           node->getIdentifier(), node->getAddress(), node->getPort());
                  getParentModule()->bubble(str);
               }

               // Acknowledge peer's takeover
               node->setTakeoverRegistrarID(msg->getSenderServerID());
               sendENRPInitTakeoverAck(msg->getTargetServerID(),
                                       msg->getSenderServerID(),
                                       msg->getSrcAddress(), msg->getSrcPort());

               // Cancel our takeover process
               stopTakeoverExpiryTimer(node);
               delete node->Takeover;
               node->Takeover = NULL;

               // No more monitoring of this peer!
               // We do *not* schedule a max-time-no-response timer; if this
               // peer comes up again, its ENRP Presence will do!
            }
            else {
               EV << Description << "Peer " << msg->getSenderServerID()
                  << ", also trying takeover of " << msg->getTargetServerID()
                  << ", has higher ID -> we (" << MyIdentifier << ") continue our takeover" << endl;
            }
         }

         // ====== Acknowledge takeover =====================================
         else {
            EV << Description << "Acknowledging peer " << msg->getSenderServerID()
               << "'s takeover of peer " << msg->getTargetServerID() << endl;
            node->setTakeoverRegistrarID(msg->getSenderServerID());
            sendENRPInitTakeoverAck(msg->getTargetServerID(),
                                    msg->getSenderServerID(),
                                    msg->getSrcAddress(), msg->getSrcPort());
            // Stop monitoring
            if(node->LastHeardTimeoutTimer) {
               stopLastHeardTimeoutTimer(node);
            }
            if(node->ResponseTimeoutTimer) {
               stopResponseTimeoutTimer(node);
            }
         }
      }

      // ====== The target is unknown. Agree to takeover. ===================
      else {
         EV << Description << "Acknowledging peer " << msg->getSenderServerID()
            << "'s takeover of peer " << msg->getTargetServerID()
            << " (which is unknown for us!)" << endl;
         sendENRPInitTakeoverAck(msg->getTargetServerID(),
                                 msg->getSenderServerID(),
                                 msg->getSrcAddress(), msg->getSrcPort());
      }
   }
}


// ###### Handle ENRP_INIT_TAKEOVER_ACK message #############################
void RegistrarProcess::handleENRPInitTakeoverAck(ENRPInitTakeoverAck* msg)
{
   OPP_CHECK(msg->getSenderServerID() != MyIdentifier);

   cPeerListNode* node = PeerList->findPeerListNode(msg->getTargetServerID());
   if((node) && (node->Takeover)) {
      EV << "Got InitTakeoverAck from peer " << msg->getSenderServerID()
         << " for target " << msg->getTargetServerID() << endl;

      if(node->Takeover->acknowledge(msg->getTargetServerID(),
                                     msg->getSenderServerID())) {
         EV << "Peer " << msg->getSenderServerID()
            << " acknowledges takeover of target "
            << msg->getTargetServerID() << ". "
            << node->Takeover->getOutstandingAcks() << " to go." << endl;
         if(hasGUI()) {
            char str[256];
            snprintf((char*)&str, sizeof(str), "Still waiting for %u acknowledgements for takeover of %u (%u:%u)",
                     (unsigned int)node->Takeover->getOutstandingAcks(),
                     node->getIdentifier(), node->getAddress(), node->getPort());
            getParentModule()->bubble(str);
         }
      }
      else {
         EV << Description << "Ready for takeover of target " << msg->getTargetServerID() << endl;
         if(hasGUI()) {
            char str[256];
            snprintf((char*)&str, sizeof(str), "Taking over %u (%u:%u)",
                     node->getIdentifier(), node->getAddress(), node->getPort());
            getParentModule()->bubble(str);
         }
         TotalTakeoversByConsent++;
         finishTakeover(node);
      }
   }
   else {
      EV << "Ignoring InitTakeoverAck from peer " << msg->getSenderServerID()
         << " for target " << msg->getTargetServerID() << endl;
   }
}


// ###### Handle ENRP_TAKEOVER_SERVER message ###############################
void RegistrarProcess::handleENRPTakeoverServer(ENRPTakeoverServer* msg)
{
   OPP_CHECK(msg->getSenderServerID() != MyIdentifier);

   EV << Description << "Got TakeoverServer from peer "
      << msg->getSenderServerID() << " for target "
      << msg->getTargetServerID() << endl;

   // ====== Remove the peer ================================================
   cPeerListNode* node = PeerList->findPeerListNode(msg->getTargetServerID());
   if(node) {
      // Stop monitoring (if again(!) in progress)
      if(node->LastHeardTimeoutTimer) {
         stopLastHeardTimeoutTimer(node);
      }
      if(node->ResponseTimeoutTimer) {
         stopResponseTimeoutTimer(node);
      }
      if(node->Takeover) {
         stopTakeoverExpiryTimer(node);
         delete node->Takeover;
         node->Takeover = NULL;
      }
      PeerList->deregisterPeerListNode(node);
   }

   // ====== Update PEs' home PR identifier =================================
   cPoolElement* poolElement = Handlespace->getFirstPoolElementOwnedBy(msg->getTargetServerID());
   while(poolElement) {
      cPoolElement* nextPoolElement = Handlespace->getNextPoolElementOfSameOwner(poolElement);

      EV << Description << "Changing ownership of pool element "
         << poolElement->getIdentifier() << " in pool "
         << poolElement->getOwnerPoolHandle() << endl;

      // Stop timer
      if(poolElement->EndpointKeepAliveTransmissionTimer) {
         stopEndpointKeepAliveTransmissionTimer(poolElement);
      }
      if(poolElement->EndpointKeepAliveTimeoutTimer) {
         stopEndpointKeepAliveTimeoutTimer(poolElement);
      }
      if(poolElement->LifetimeExpiryTimer) {
         stopLifetimeExpiryTimer(poolElement);
      }

      // Update handlespace
      Handlespace->updatePoolElementOwnership(poolElement, msg->getSenderServerID());

      // Start LifetimeExpiry timer
      startLifetimeExpiryTimer(poolElement);

      poolElement = nextPoolElement;
   }

   PoolElementCountVector->record(Handlespace->getPoolElements());
   OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());
}


// ###### Finish a takeover by actually taking the PEs' ownership ###########
void RegistrarProcess::finishTakeover(cPeerListNode* node)
{
   EV << "Takeover of target " << node->getIdentifier()
      << " confirmed. Taking it over now." << endl;

   // ====== Take over PEs ==================================================
   cPoolElement* poolElement = Handlespace->getFirstPoolElementOwnedBy(node->getIdentifier());
   while(poolElement) {
      cPoolElement* nextPoolElement = Handlespace->getNextPoolElementOfSameOwner(poolElement);

      EV << Description << "Taking ownership of pool element "
         << poolElement->getIdentifier() << " in pool "
         << poolElement->getOwnerPoolHandle() << endl;

      // Deactivate expiry timer
      stopLifetimeExpiryTimer(poolElement);

      // Update handlespace
      Handlespace->updatePoolElementOwnership(poolElement, MyIdentifier);

      // Tell node about new home PR and schedule endpoint keep-alive timeout
      sendASAPEndpointKeepAlive(poolElement, true);
      startEndpointKeepAliveTimeoutTimer(poolElement);
      poolElement = nextPoolElement;
   }

   // ====== Inform other registrars of takeover ============================
   sendENRPTakeoverServers(node->getIdentifier());

   PoolElementCountVector->record(Handlespace->getPoolElements());
   OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());

   // ====== Remove the peer ================================================
   if(node->TakeoverExpiryTimer) {
      stopTakeoverExpiryTimer(node);
   }
   delete node->Takeover;
   node->Takeover = NULL;
   PeerList->deregisterPeerListNode(node);
}


// ###### Begin normal operation mode #######################################
void RegistrarProcess::beginNormalOperation(const bool initializedFromMentor)
{
   EV << Description << "Going from initialization phase to normal operation, after "
      << (initializedFromMentor ? "initialization from mentor" : "mentor discovery timeout") << endl;
   colorizeModule(getParentModule(), "#00ff00");

   // ====== Stop mentor discovery ==========================================
   MentorServerID = UNDEFINED_REGISTRAR_IDENTIFIER;
   if(MentorDiscoveryTimeoutTimer) {
      stopMentorDiscoveryTimeoutTimer();
   }

   if(initializedFromMentor) {
      TotalStartupsWithMentor++;
   }
   else {
      TotalStartupsAlone++;
   }
}


// ###### Registrar service startup #########################################
void RegistrarProcess::startupService()
{
   EV << Description << "Starting ENRP service ..." << endl;
   colorizeModule(getParentModule(), "#ffff55");
   MentorServerID = UNDEFINED_REGISTRAR_IDENTIFIER;
   PoolElementCountVector->record(0);
   OwnedPoolElementCountVector->record(0);
   startMentorDiscoveryTimeoutTimer();
   startShutdownTimer();

   // ====== Tell all peers that we are there ===============================
   cPeerListNode* node = PeerList->getFirstPeerListNode();
   while(node != NULL) {
      /* Required reply: ensure as soon as possible that we have a consistent
         handlespace. */
      sendENRPPresence(node, true);
      node = PeerList->getNextPeerListNode(node);
   }

   // ====== Start regular heartbeats =======================================
   startPeerHeartbeatCycleTimer();
}


// ###### Registrar service startup #########################################
void RegistrarProcess::shutdownService()
{
   EV << Description << "Service shutdown ..." << endl;

   // ------ Clear handlespace ----------------------------------------------
   PoolElementCountVector->record(Handlespace->getPoolElements());
   OwnedPoolElementCountVector->record(Handlespace->getOwnedPoolElements());
   cPoolElement* poolElement = Handlespace->getFirstPoolElementNode();
   while(poolElement) {
      if(poolElement->EndpointKeepAliveTransmissionTimer) {
         stopEndpointKeepAliveTransmissionTimer(poolElement);
      }
      if(poolElement->EndpointKeepAliveTimeoutTimer) {
         stopEndpointKeepAliveTimeoutTimer(poolElement);
      }
      if(poolElement->LifetimeExpiryTimer) {
         stopLifetimeExpiryTimer(poolElement);
      }
      poolElement = Handlespace->getNextPoolElementNode(poolElement);
   }
   Handlespace->clear();
   PoolElementCountVector->record(0);
   OwnedPoolElementCountVector->record(0);

   // ------ Clear peer list ------------------------------------------------
   cPeerListNode* node = PeerList->getFirstPeerListNode();
   while(node != NULL) {
      node->setStatus(node->getStatus() & ~(PLNS_LISTSYNC|PLNS_HTSYNC|PLNS_MENTOR));
      if(node->LastHeardTimeoutTimer) {
         stopLastHeardTimeoutTimer(node);
      }
      if(node->ResponseTimeoutTimer) {
         stopResponseTimeoutTimer(node);
      }
      if(node->Takeover) {
         stopTakeoverExpiryTimer(node);
         delete node->Takeover;
         node->Takeover = NULL;
      }
      cPeerListNode* current = node;
      node = PeerList->getNextPeerListNode(node);
      if(current->getIdentifier() != UNDEFINED_REGISTRAR_IDENTIFIER) {
         // Do not delete static registrar entries (ID 0)!
         PeerList->deregisterPeerListNode(current);
      }
   }

   // ------ Stop MentorDiscoveryTimer --------------------------------------
   if(MentorDiscoveryTimeoutTimer) {
      stopMentorDiscoveryTimeoutTimer();
   }
}


// ###### Handle message ####################################################
void RegistrarProcess::handleMessage(cMessage* msg)
{
   EV << Description << "Received message \"" << msg->getName()
      << "\" in state " << State.getStateName() << endl;

   FSM_Switch(State) {

      // ====================================================================
      // ==== Startup Phase                                              ====
      // ====================================================================

      case FSM_Exit(INIT):
         if(msg == StartupTimer) {
            StartupTimer = NULL;
            FSM_Goto(State, STARTUP_SERVICE);
         }
         else if((dynamic_cast<ASAPPacket*>(msg)) ||
                 (dynamic_cast<ENRPPacket*>(msg))) {
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(STARTUP_SERVICE):
         startupService();
         FSM_Goto(State, RUN_SERVICE);
       break;

      // ====================================================================
      // ==== Service Phase                                              ====
      // ====================================================================

      case FSM_Exit(RUN_SERVICE):
         // ====== ASAP part ================================================
         if(dynamic_cast<ASAPPacket*>(msg)) {
            if(dynamic_cast<ASAPRegistration*>(msg)) {
               handleASAPRegistration((ASAPRegistration*)msg);
            }
            else if(dynamic_cast<ASAPDeregistration*>(msg)) {
               handleASAPDeregistration((ASAPDeregistration*)msg);
            }
            else if(dynamic_cast<ASAPHandleResolution*>(msg)) {
               handleASAPHandleResolution((ASAPHandleResolution*)msg);
            }
            else if(dynamic_cast<ASAPEndpointUnreachable*>(msg)) {
               handleASAPEndpointUnreachable((ASAPEndpointUnreachable*)msg);
            }
            else if(dynamic_cast<ASAPEndpointKeepAliveAck*>(msg)) {
               handleASAPEndpointKeepAliveAck((ASAPEndpointKeepAliveAck*)msg);
            }
            else {
               handleUnexpectedMsgState(msg, State);
            }
         }
         else if(dynamic_cast<EndpointKeepAliveTransmissionMessage*>(msg)) {
            handleEndpointKeepAliveTransmissionTimer((cPoolElement*)msg->getContextPointer());
         }
         else if(dynamic_cast<EndpointKeepAliveTimeoutMessage*>(msg)) {
            handleEndpointKeepAliveTimeoutTimer((cPoolElement*)msg->getContextPointer());
         }
         else if(dynamic_cast<LifetimeExpiryMessage*>(msg)) {
            handleLifetimeExpiryTimer((cPoolElement*)msg->getContextPointer());
         }
         else if(msg == MentorDiscoveryTimeoutTimer) {
            MentorDiscoveryTimeoutTimer = NULL;
            EV << Description << "Mentor discovery phase is over, no mentor found" << endl;
            beginNormalOperation(false);
         }

         // ====== ENRP part ================================================
         else if(dynamic_cast<ENRPPacket*>(msg)) {
            if(dynamic_cast<ENRPListRequest*>(msg)) {
               handleENRPListRequest((ENRPListRequest*)msg);
            }
            else if(dynamic_cast<ENRPHandleTableRequest*>(msg)) {
               handleENRPHandleTableRequest((ENRPHandleTableRequest*)msg);
            }
            else if(dynamic_cast<ENRPPresence*>(msg)) {
               handleENRPPresence((ENRPPresence*)msg);
            }
            else if(dynamic_cast<ENRPHandleUpdate*>(msg)) {
               handleENRPHandleUpdate((ENRPHandleUpdate*)msg);
            }
            else if(dynamic_cast<ENRPListResponse*>(msg)) {
               handleENRPListResponse((ENRPListResponse*)msg);
            }
            else if(dynamic_cast<ENRPHandleTableResponse*>(msg)) {
               handleENRPHandleTableResponse((ENRPHandleTableResponse*)msg);
            }
            else if(dynamic_cast<ENRPInitTakeover*>(msg)) {
               handleENRPInitTakeover((ENRPInitTakeover*)msg);
            }
            else if(dynamic_cast<ENRPInitTakeoverAck*>(msg)) {
               handleENRPInitTakeoverAck((ENRPInitTakeoverAck*)msg);
            }
            else if(dynamic_cast<ENRPTakeoverServer*>(msg)) {
               handleENRPTakeoverServer((ENRPTakeoverServer*)msg);
            }
            else {
               handleUnexpectedMsgState(msg, State);
            }
         }
         else if(dynamic_cast<LastHeardTimeoutMessage*>(msg)) {
            handleLastHeardTimeoutTimer((cPeerListNode*)msg->getContextPointer());
         }
         else if(dynamic_cast<ResponseTimeoutMessage*>(msg)) {
            handleResponseTimeoutTimer((cPeerListNode*)msg->getContextPointer());
         }
         else if(dynamic_cast<TakeoverExpiryMessage*>(msg)) {
            handleTakeoverExpiryTimer((cPeerListNode*)msg->getContextPointer());
         }
         else if(msg == PeerHeartbeatCycleTimer) {
            PeerHeartbeatCycleTimer = NULL;
            handlePeerHeartbeatCycleTimer();
         }

         // ====== Shutdown =================================================
         else if(msg == ShutdownTimer) {
            ShutdownTimer = NULL;
            if(PeerHeartbeatCycleTimer) {
               stopPeerHeartbeatCycleTimer();
            }
            FSM_Goto(State, SHUTDOWN_SERVICE);
         }

         // ====== Unexpected message =======================================
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;


      // ====================================================================
      // ==== Shutdown Phase                                             ====
      // ====================================================================

      case FSM_Exit(SHUTDOWN_SERVICE):
         colorizeModule(getParentModule(), "#ff0000");
         shutdownService();
         if(Run < (unsigned int)par("componentRuns")) {
            Run++;
            startRestartDelayTimer();
            FSM_Goto(State, WAIT_FOR_RESTART);
         }
         else {
            colorizeModule(getParentModule(), "");
            EV << Description << "--- FINISHED ---" << endl;
            colorizeModule(getParentModule());
            FSM_Goto(State, FINISHED);
         }
       break;

      case FSM_Exit(WAIT_FOR_RESTART):
         if(msg == RestartDelayTimer) {
            RestartDelayTimer = NULL;
            EV << Description << "Restarting service ..." << endl;
            FSM_Goto(State, STARTUP_SERVICE);
         }
         else if((dynamic_cast<ASAPPacket*>(msg)) ||
                 (dynamic_cast<ENRPPacket*>(msg)) ||
                 (dynamic_cast<EndpointKeepAliveTransmissionMessage*>(msg)) ||
                 (dynamic_cast<EndpointKeepAliveTimeoutMessage*>(msg))) {
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

   updateNumberStatistics();
   delete msg;
}
