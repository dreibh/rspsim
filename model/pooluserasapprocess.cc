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

#include <omnetpp.h>

#include "utilities.h"
#include "messages_m.h"
#include "handlespacemanagementwrapper.h"


class PoolUserASAPProcess : public cSimpleModule
{
   // ====== Methods ========================================================
   virtual void initialize();
   virtual void handleMessage(cMessage* msg);

   void selectPoolElement();
   bool selectPoolElementFromCache();
   void startRegistrarHunt();


   // ====== States =========================================================
   private:
   enum {
      INIT                                = 0,

      WAIT_FOR_APPLICATION                = FSM_Steady(1),

      SELECT_POOL_ELEMENT_FROM_CACHE      = FSM_Transient(2),
      SEND_HANDLE_RESOLUTION_REQUEST      = FSM_Transient(3),
      WAIT_FOR_HANDLE_RESOLUTION_RESPONSE = FSM_Steady(4),

      SERVER_HUNT_RETRY_DELAY             = FSM_Steady(5),
      SEND_SERVER_HUNT_REQUEST            = FSM_Transient(6),
      WAIT_FOR_SERVER_HUNT_RESPONSE       = FSM_Steady(7),

      SELECT_POOL_ELEMENT                 = FSM_Transient(8)
   };
   cFSM State;


   // ====== ASAP Timers ====================================================
   void startT1HandleResolutionRequestTimer();
   void stopT1HandleResolutionRequestTimer();
   void startServerHuntRetryTimer();
   void stopServerHuntRetryTimer();

   // ====== ASAP Protocol ==================================================
   void handleServerSelectionRequest(ServerSelectionRequest* msg);
   void handleEndpointUnreachable(EndpointUnreachable* msg);
   void handleCachePurge(CachePurge* msg);
   void sendASAPHandleResolution();
   bool handleASAPHandleResolutionResponse(ASAPHandleResolutionResponse* msg);
   void handleRegistrarHuntResponse(RegistrarHuntResponse* msg);


   // ====== Timers =========================================================
   cMessage*        T1HandleResolutionRequestTimer;
   cMessage*        ServerHuntRetryTimer;


   // ====== Variables ======================================================
   unsigned int     HandleResolutionRequestsSent;
   unsigned int     RegistrarAddress;
   opp_string       PoolHandle;
   cPoolHandlespace Cache;
   opp_string       Description;
};

Define_Module(PoolUserASAPProcess);


// ###### Initialize ########################################################
void PoolUserASAPProcess::initialize()
{
   // ------ Initialize variables -------------------------------------------
   State.setName("State");
   Description = format("PoolUserASAPProcess at %u:%u> ",
                        getLocalAddress(this), PoolUserASAPPort);

   RegistrarAddress               = 0;
   HandleResolutionRequestsSent   = 0;
   T1HandleResolutionRequestTimer = NULL;
   ServerHuntRetryTimer           = NULL;

   // ------ Bind to port ---------------------------------------------------
   BindMessage* msg = new BindMessage("Bind");
   msg->setPort(PoolUserASAPPort);
   send(msg, "toTransport");

   // ------ Prepare startup ------------------------------------------------
   scheduleAt(simTime(), new cMessage("StartupTimer"));
}


// ###### Start Handle Resolution Request timer #############################
void PoolUserASAPProcess::startT1HandleResolutionRequestTimer()
{
   OPP_CHECK(T1HandleResolutionRequestTimer == NULL);
   T1HandleResolutionRequestTimer = new cMessage("T1HandleResolutionRequestTimer");
   scheduleAt(simTime() + (double)par("asapRequestTimeout"), T1HandleResolutionRequestTimer);
}


// ###### Stop Handle Resolution Request timer ##############################
void PoolUserASAPProcess::stopT1HandleResolutionRequestTimer()
{
   OPP_CHECK(T1HandleResolutionRequestTimer != NULL);
   delete cancelEvent(T1HandleResolutionRequestTimer);
   T1HandleResolutionRequestTimer = NULL;
}


// ###### Start ENRP Request timer ##########################################
void PoolUserASAPProcess::startServerHuntRetryTimer()
{
   OPP_CHECK(ServerHuntRetryTimer == NULL);
   ServerHuntRetryTimer = new cMessage("ServerHuntRetryTimer");
   scheduleAt(simTime() + (double)par("asapServerHuntRetryDelay"), ServerHuntRetryTimer);
}


// ###### Stop ENRP Request timer ###########################################
void PoolUserASAPProcess::stopServerHuntRetryTimer()
{
   OPP_CHECK(ServerHuntRetryTimer != NULL);
   delete cancelEvent(ServerHuntRetryTimer);
   ServerHuntRetryTimer = NULL;
}


// ###### Handle ServerSelection request from application ###################
void PoolUserASAPProcess::handleServerSelectionRequest(ServerSelectionRequest* msg)
{
   PoolHandle = msg->getPoolHandle();
}


// ###### Send ASAP_HANDLE_RESOLUTION message ###############################
void PoolUserASAPProcess::sendASAPHandleResolution()
{
   ASAPHandleResolution* handleResolution = new ASAPHandleResolution("ASAP_HANDLE_RESOLUTION", ASAP);
   handleResolution->setProtocol(ASAP);
   handleResolution->setDstAddress(RegistrarAddress);
   handleResolution->setSrcPort(PoolUserASAPPort);
   handleResolution->setDstPort(RegistrarPort);
   handleResolution->setPoolHandle(PoolHandle.c_str());

   handleResolution->setTimestamp(simTime());
   send(handleResolution, "toTransport");
}


// ###### Handle ASAP_HANDLE_RESOLUTION_RESPONSE message ####################
bool PoolUserASAPProcess::handleASAPHandleResolutionResponse(ASAPHandleResolutionResponse* msg)
{
   if(!msg->getRejectFlag()) {
      /* Note the comments about purging in selectPoolElementFromCache()!
         Ensure, that no outdated elements remain in the cache. After adding
         the new elements below, the cache is ready for a
         selectPoolElements() call. */
      size_t purged = Cache.purgeExpiredPoolElements();
      if(purged > 0) {
         EV << Description << "Purged " << purged << " entries in cache" << endl;
      }
      const size_t oldElementCount = Cache.getPoolElementsOfPool(msg->getPoolHandle());

      const unsigned int items = msg->getPoolElementParameterArraySize();
      for(unsigned int i = 0;i < items;i++) {
         cPoolElement* poolElement;
         bool          updated;
         Cache.registerPoolElement(msg->getPoolHandle(),
                                   msg->getPoolElementParameter(i),
                                   0, 0,
                                   poolElement, updated);
         Cache.restartPoolElementExpiryTimer(poolElement,
                                             (unsigned long long)(1000000.0 * (double)par("asapStaleCacheValue")));
      }
      if(oldElementCount == 0) {
         OPP_CHECK(Cache.getPoolElementsOfPool(msg->getPoolHandle()) == items);
      }
      return(true);
   }
   return(false);
}


// ###### Select one pool element from cache ################################
bool PoolUserASAPProcess::selectPoolElementFromCache()
{
   cPoolElement* selectionArray[1];
   size_t        items  = 1;
   const size_t  purged = Cache.purgeExpiredPoolElements();
   if(purged > 0) {
      EV << Description << "Purged " << purged << " entries in cache" << endl;
   }
   Cache.selectPoolElementsByPolicy(PoolHandle.c_str(), (cPoolElement**)&selectionArray, items, 1, 1000000000);
   if(items > 0) {
      EV << Description << "Successfully selected pool element from cache: " << endl;
      selectionArray[0]->print(true);
      EV << "Cache content:" << endl;
      Cache.print();

      ServerSelectionSuccess* response = new ServerSelectionSuccess("ServerSelectionSuccess");
      response->setPoolHandle(PoolHandle.c_str());
      response->setPoolElementParameter(selectionArray[0]->toPoolElementParameter());
      send(response, "toApplication");
      return(true);
   }
   else {
      EV << Description << "No pool element available from cache" << endl;
   }
   return(false);
}


// ###### Select one pool element ###########################################
void PoolUserASAPProcess::selectPoolElement()
{
   cPoolElement* selectionArray[1];
   size_t        items  = 1;
   /* We do *not* purge the cache here. In the case of par("asapStaleCacheValue") == 0,
      this would clear the list just received from the NS.
      Instead, purging is done before the ServerSelectionResponse is handled.
      This ensures, that all cached elements are gone. */
   Cache.selectPoolElementsByPolicy(PoolHandle.c_str(), (cPoolElement**)&selectionArray, items, 1, 1000000000);
   if(items > 0) {
      EV << Description << "Successfully selected pool element after nameserver query: " << endl;
      selectionArray[0]->print(true);
      EV << "Cache contents:" << endl;
      Cache.print();

/*
      std::cerr << Description << ": Queried "
                << selectionArray[0]->getAddress() << ":"
                << selectionArray[0]->getPort() << std::endl;
*/

      ServerSelectionSuccess* response = new ServerSelectionSuccess("ServerSelectionSuccess");
      response->setPoolHandle(PoolHandle.c_str());
      response->setPoolElementParameter(selectionArray[0]->toPoolElementParameter());
      send(response, "toApplication");
   }
   else {
      EV << Description << "No pool element available" << endl;
      ServerSelectionFailure* response = new ServerSelectionFailure("ServerSelectionFailure");
      send(response, "toApplication");
   }
}


// ###### Handle EndpointUnreachable from application #######################
void PoolUserASAPProcess::handleEndpointUnreachable(EndpointUnreachable* msg)
{
   EV << Description << "Endpoint unreachable for " << msg->getIdentifier()
      << " in pool " << msg->getPoolHandle() << endl;
   Cache.deregisterPoolElement(msg->getPoolHandle(), msg->getIdentifier());

   ASAPEndpointUnreachable* endpointUnreachable = new ASAPEndpointUnreachable("ASAP_ENDPOINT_UNREACHABLE", ASAP);
   endpointUnreachable->setProtocol(ASAP);
   endpointUnreachable->setDstAddress(RegistrarAddress);
   endpointUnreachable->setSrcPort(PoolUserASAPPort);
   endpointUnreachable->setDstPort(RegistrarPort);
   endpointUnreachable->setPoolHandle(msg->getPoolHandle());
   endpointUnreachable->setIdentifier(msg->getIdentifier());

   endpointUnreachable->setTimestamp(simTime());
   send(endpointUnreachable, "toTransport");
}


// ###### Handle CachePurge from application ################################
void PoolUserASAPProcess::handleCachePurge(CachePurge* msg)
{
   EV << Description << "Cache purge for " << msg->getIdentifier()
      << " in pool " << msg->getPoolHandle() << endl;
   Cache.deregisterPoolElement(msg->getPoolHandle(), msg->getIdentifier());
}


// ###### Request RegistrarHunt from RegistrarTable #########################
void PoolUserASAPProcess::startRegistrarHunt()
{
   RegistrarHuntRequest* request = new RegistrarHuntRequest("RegistrarHuntRequest");
   EV << Description << "Starting registrar hunt" << endl;
   send(request, "toRegistrarTable");
}


// ###### Handle RegistrarHuntResponse from RegistrarTable ##################
void PoolUserASAPProcess::handleRegistrarHuntResponse(RegistrarHuntResponse* msg)
{
   RegistrarAddress = msg->getRegistrarAddress();
   EV << Description << "Selected name server at " << RegistrarAddress << endl;
}


// ###### Handle message from transport layer ###############################
void PoolUserASAPProcess::handleMessage(cMessage* msg)
{
   EV << Description << "Received message \"" << msg->getName()
      << "\" in state " << State.getStateName() << endl;

   FSM_Switch(State) {

      case FSM_Exit(INIT):
         FSM_Goto(State, WAIT_FOR_APPLICATION);
       break;

      case FSM_Exit(WAIT_FOR_APPLICATION):
         if(dynamic_cast<ServerSelectionRequest*>(msg)) {
            EV << Description << "Got ServerSelectionRequest ..." << endl;
            handleServerSelectionRequest((ServerSelectionRequest*)msg);
            FSM_Goto(State, SELECT_POOL_ELEMENT_FROM_CACHE);
         }
         else if(dynamic_cast<EndpointUnreachable*>(msg)) {
            EV << Description << "Got EndpointUnreachable ..." << endl;
            handleEndpointUnreachable((EndpointUnreachable*)msg);
         }
         else if(dynamic_cast<CachePurge*>(msg)) {
            EV << Description << "Got CachePurge ..." << endl;
            handleCachePurge((CachePurge*)msg);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(SELECT_POOL_ELEMENT_FROM_CACHE):
         EV << Description << "Select pool element from cache ..." << endl;
         if(selectPoolElementFromCache()) {
            FSM_Goto(State, WAIT_FOR_APPLICATION);
         }
         else {
            HandleResolutionRequestsSent = 0;
            FSM_Goto(State, SEND_HANDLE_RESOLUTION_REQUEST);
         }
       break;

      case FSM_Exit(SEND_HANDLE_RESOLUTION_REQUEST):
         if( (RegistrarAddress != UNDEFINED_REGISTRAR_IDENTIFIER) &&
             (HandleResolutionRequestsSent <= (unsigned int)par("asapMaxRequestRetransmit")) ) {
            HandleResolutionRequestsSent++;
            EV << Description << "Sending ASAP_HANDLE_RESOLUTION ... (attempt "
               << HandleResolutionRequestsSent << " of " << (unsigned int)par("asapMaxRequestRetransmit") << ")" << endl;
            sendASAPHandleResolution();
            startT1HandleResolutionRequestTimer();
            FSM_Goto(State, WAIT_FOR_HANDLE_RESOLUTION_RESPONSE);
         }
         else {
            FSM_Goto(State, SEND_SERVER_HUNT_REQUEST);
         }
       break;

      case FSM_Exit(WAIT_FOR_HANDLE_RESOLUTION_RESPONSE):
         if(dynamic_cast<ASAPHandleResolutionResponse*>(msg)) {
            EV << Description << "Got handle resolution response" << endl;
            stopT1HandleResolutionRequestTimer();
            if(handleASAPHandleResolutionResponse((ASAPHandleResolutionResponse*)msg)) {
               FSM_Goto(State, SELECT_POOL_ELEMENT);
            }
            else {
               EV << Description << "Handle resolution has been rejected! Trying to find other registrar!" << endl;
               startServerHuntRetryTimer();
               FSM_Goto(State, SERVER_HUNT_RETRY_DELAY);
            }
         }
         else if(msg == T1HandleResolutionRequestTimer) {
            EV << Description << "Handle resolution timed out" << endl;
            T1HandleResolutionRequestTimer = NULL;
            FSM_Goto(State, SEND_HANDLE_RESOLUTION_REQUEST);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(SERVER_HUNT_RETRY_DELAY):
         if(msg == ServerHuntRetryTimer) {
            ServerHuntRetryTimer = NULL;
            FSM_Goto(State, SEND_SERVER_HUNT_REQUEST);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(SEND_SERVER_HUNT_REQUEST):
         startRegistrarHunt();
         FSM_Goto(State, WAIT_FOR_SERVER_HUNT_RESPONSE);
       break;

      case FSM_Exit(WAIT_FOR_SERVER_HUNT_RESPONSE):
         if(dynamic_cast<RegistrarHuntResponse*>(msg)) {
            handleRegistrarHuntResponse((RegistrarHuntResponse*)msg);
            HandleResolutionRequestsSent = 0;
            FSM_Goto(State, SEND_HANDLE_RESOLUTION_REQUEST);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(SELECT_POOL_ELEMENT):
         EV << Description << "Select pool element ..." << endl;
         selectPoolElement();
         FSM_Goto(State, WAIT_FOR_APPLICATION);
       break;

   }

   delete msg;
}
