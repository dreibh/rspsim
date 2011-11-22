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


#include "handlespacemanagementwrapper.h"
#include "statisticswriterinterface.h"
#include "abstractcontroller.h"
#include "statuschangelist.h"
#include "utilities.h"


class AttackerProcess : public StatisticsWriterInterface,
                        public cSimpleModule
{
   // ====== Methods ========================================================
   virtual void initialize();
   virtual void finish();
   virtual void handleMessage(cMessage* msg);
   virtual void resetStatistics();
   virtual void writeStatistics();
   void addTargetRegistrar(const unsigned int address);
   unsigned int selectTargetRegistrar();


   // ====== Attacker Timers ================================================
   void startStartupTimer();
   void startShutdownTimer();
   void startRestartDelayTimer();
   void startNextAttackTimer();
   void stopNextAttackTimer();
   void handleNextAttackTimer();


   // ====== Attacked Protocols =============================================
   void handleASAPRegistrationResponse(ASAPRegistrationResponse* msg);
   void handleASAPEndpointKeepAlive(ASAPEndpointKeepAlive* msg);
   void handleApplicationMessage(CalcAppMessage* msg);
   void handleASAPHandleResolutionResponse(ASAPHandleResolutionResponse* msg);


   // ====== States =========================================================
   private:
   enum {
      INIT               = 0,
      STARTUP_SERVICE    = FSM_Transient(1),
      RUN_ATTACK         = FSM_Steady(2),
      SHUTDOWN_SERVICE   = FSM_Transient(3),
      WAIT_FOR_RESTART   = FSM_Steady(4),
      FINISHED           = FSM_Steady(5)
   };
   cFSM State;


   // ====== Private methods ================================================
   void sendASAPEndpointUnreachable(const unsigned int        registrarAddress,
                                    const unsigned int        registrarPort,
                                    const char*               poolHandle,
                                    PoolElementIdentifierType identifier);

   // ====== Private data ===================================================
   private:
   cMessage*                  StartupTimer;
   cMessage*                  ShutdownTimer;
   cMessage*                  RestartDelayTimer;
   cMessage*                  NextAttackTimer;

   unsigned int               Run;
   unsigned int               LocalAddress;
   opp_string                 Description;
   StatusChangeList           ComponentStatusChanges;
   cPeerList*                 TargetRegistrarTable;
   PoolElementIdentifierType  TargetIdentifier;

   opp_string                 VictimPoolHandle;
   size_t                     VictimPoolElements;
   PoolElementIdentifierType* VictimPoolElementList;
   bool                       GotHandleResolutionResponse;

   unsigned int               TotalRegistrations;
   unsigned int               TotalEndpointKeepAliveAcks;
   unsigned int               TotalHandleResolutions;
   unsigned int               TotalFailureReports;
   unsigned int               TotalIgnoredApplicationMessages;
};

Define_Module(AttackerProcess);


// ###### Initialize ########################################################
void AttackerProcess::initialize()
{
   // ------ Bind to port ---------------------------------------------------
   BindMessage* msg = new BindMessage("Bind");
   msg->setPort(AttackerPort);
   send(msg, "toTransport");

   // ------ Initialize variables -------------------------------------------
   State.setName("State");
   Description  = format("AttackerProcess at %u:%u> ",
                         getLocalAddress(this), AttackerPort);

   StartupTimer                = NULL;
   ShutdownTimer               = NULL;
   RestartDelayTimer           = NULL;
   NextAttackTimer             = NULL;

   VictimPoolElements          = 0;
   VictimPoolElementList       = NULL;
   GotHandleResolutionResponse = false;

   Run                         = 1;
   LocalAddress                = getLocalAddress(this);
   ComponentStatusChanges.setup(par("componentStatusChanges"));
   TargetIdentifier            = (PoolElementIdentifierType)rint(uniform(1, 0xffffffff));

   // ------ Create registrar table -----------------------------------------
   TargetRegistrarTable = new cPeerList(NULL, 0);
   OPP_CHECK(TargetRegistrarTable);

   const char*  staticRegistrarsList = par("attackTargetRegistrarsList");
   unsigned int registrarAddress;
   int          n;
   while(staticRegistrarsList[0] != 0x00) {
      if(sscanf(staticRegistrarsList, "%u%n", &registrarAddress, &n) == 1) {
         addTargetRegistrar(registrarAddress);
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
   ev << Description;
   TargetRegistrarTable->print();

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
void AttackerProcess::finish()
{
   delete TargetRegistrarTable;
   TargetRegistrarTable = NULL;
   if(VictimPoolElementList) {
      delete VictimPoolElementList;
      VictimPoolElementList = NULL;
   }
}


// ###### Add static registrar to registrar table ###########################
void AttackerProcess::addTargetRegistrar(const unsigned int address)
{
   ServerInformationParameter serverInformationParameter;
   serverInformationParameter.setServerID(0);
   serverInformationParameter.setAddress(address);
   serverInformationParameter.setPort(RegistrarPort);

   cPeerListNode* node;
   OPP_CHECK(TargetRegistrarTable->registerPeerListNode(serverInformationParameter, node) == RSPERR_OKAY);
}


// ###### Reset statistics ##################################################
void AttackerProcess::resetStatistics()
{
   TotalHandleResolutions          = 0;
   TotalFailureReports             = 0;
   TotalRegistrations              = 0;
   TotalEndpointKeepAliveAcks      = 0;
   TotalIgnoredApplicationMessages = 0;
}


// ###### Write statistics ##################################################
void AttackerProcess::writeStatistics()
{
   recordScalar("Total Attacker Handle Resolutions",           TotalHandleResolutions);
   recordScalar("Total Attacker Failure Reports",              TotalFailureReports);
   recordScalar("Total Attacker Registrations",                TotalRegistrations);
   recordScalar("Total Attacker Endpoint Keep-Alive Acks",     TotalEndpointKeepAliveAcks);
   recordScalar("Total Attacker Ignored Application Messages", TotalIgnoredApplicationMessages);

   AbstractController* controller = AbstractController::getController();
   if(controller) {
      controller->GlobalAttackerIgnoredApplicationMessages += TotalIgnoredApplicationMessages;
   }
}


// ###### Start Startup timer ###############################################
void AttackerProcess::startStartupTimer()
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
void AttackerProcess::startShutdownTimer()
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
void AttackerProcess::startRestartDelayTimer()
{
   OPP_CHECK(RestartDelayTimer == NULL);
   RestartDelayTimer = new cMessage("RestartDelayTimer");
   simtime_t delay = ComponentStatusChanges.expectedAction(simTime(), "up");
   if(delay < 0.0) {
      delay = (simtime_t)par("componentDowntime");
   }
   scheduleAt(simTime() + delay, RestartDelayTimer);
}


// ###### Start NextAttack timer ############################################
void AttackerProcess::startNextAttackTimer()
{
   OPP_CHECK(NextAttackTimer == NULL);
   NextAttackTimer = new cMessage("NextAttack");
   scheduleAt(simTime() + (double)par("attackInterval"), NextAttackTimer);
}


// ###### Stop NextAttack timer #############################################
void AttackerProcess::stopNextAttackTimer()
{
   OPP_CHECK(NextAttackTimer != NULL);
   delete cancelEvent(NextAttackTimer);
   NextAttackTimer = NULL;
}


// ###### Select a target registrar #########################################
unsigned int AttackerProcess::selectTargetRegistrar()
{
   cPeerListNode* node = TargetRegistrarTable->getRandomPeerListNode();
   if(node) {
      return(node->getAddress());
   }
   return(0);
}


// ###### Handle NextAttack timer ###########################################
void AttackerProcess::handleNextAttackTimer()
{
   ev << Description << "Attack!" << endl;

   const char*        attackType             = par("attackType");
   const unsigned int targetRegistrarAddress = selectTargetRegistrar();

   if(targetRegistrarAddress == 0) {
      error("No target registrar specified!");
   }

   // ====== Handle Resolution attack =======================================
   if(!(strcmp(attackType, "HandleResolution"))) {
      if( (!GotHandleResolutionResponse) &&
          (uniform(0.0, 1.0) <= (double)par("attackReportUnreachableProbability")) &&
          (VictimPoolElements > 0) ) {
         for(unsigned int i = 0;i < VictimPoolElements;i++) {
            ev << "Sending endpoint unreachable for PE #"
               << VictimPoolElementList[i] << "..." << endl;
            sendASAPEndpointUnreachable(targetRegistrarAddress, RegistrarPort,
                                        VictimPoolHandle.c_str(), VictimPoolElementList[i]);
         }
      }

      ASAPHandleResolution* handleResolution = new ASAPHandleResolution("ASAP_HANDLE_RESOLUTION", ASAP);
      handleResolution->setProtocol(ASAP);
      handleResolution->setDstAddress(targetRegistrarAddress);
      handleResolution->setSrcPort(AttackerPort);
      handleResolution->setDstPort(RegistrarPort);
      handleResolution->setPoolHandle(par("attackTargetPoolHandle"));

      ev << Description << "Sending ASAP Handle Resolution..." << endl;
      handleResolution->setTimestamp(simTime());
      send(handleResolution, "toTransport");
      GotHandleResolutionResponse = false;
   }

   // ====== Registration attack ============================================
   else if(!(strcmp(attackType, "Registration"))) {
      cTransportParameter userTransportParameter;
      userTransportParameter.setAddress(LocalAddress);
      userTransportParameter.setPort(AttackerPort);

      cTransportParameter registratorTransportParameter;
      registratorTransportParameter.setAddress(LocalAddress);
      registratorTransportParameter.setPort(AttackerPort);

      cPoolPolicyParameter poolPolicyParameter;
      const unsigned int targetPolicyType = getPoolPolicyTypeByName(par("attackTargetPolicy"));
      if(targetPolicyType == PPT_UNDEFINED) {
         throw new cRuntimeError("Bad pool policy given!");
      }
      poolPolicyParameter.setPolicyType(targetPolicyType);
      const double loadDegradation = par("attackTargetPolicyLoadDegradation");
      if((loadDegradation < 0.0) || (loadDegradation > 1.0)) {
         throw cRuntimeError("Bad setting for attackTargetPolicyLoadDegradation!");
      }
      poolPolicyParameter.setLoadDegradation((unsigned int)rint((double)PPV_MAX_LOAD_DEGRADATION * loadDegradation));
      const unsigned int weight = par("attackTargetPolicyWeight");
      if(weight == 0) {
         throw cRuntimeError("Bad setting for attackTargetPolicyWeight!");
      }
      poolPolicyParameter.setWeight(weight);

      cPoolElementParameter poolElementParameter;
      if((bool)par("attackRandomizeIdentifier")) {
         TargetIdentifier = (PoolElementIdentifierType)rint(uniform(1, 0xffffffff));
      }
      poolElementParameter.setIdentifier(TargetIdentifier);
      poolElementParameter.setHomeRegistrarIdentifier(0);
      poolElementParameter.setRegistrationLife(1000000000);
      poolElementParameter.setUserTransportParameter(userTransportParameter);
      poolElementParameter.setRegistratorTransportParameter(registratorTransportParameter);
      poolElementParameter.setPoolPolicyParameter(poolPolicyParameter);

      ASAPRegistration* registration = new ASAPRegistration("ASAP_REGISTRATION", ASAP);
      registration->setProtocol(ASAP);
      registration->setDstAddress(targetRegistrarAddress);
      registration->setSrcPort(AttackerPort);
      registration->setDstPort(RegistrarPort);
      registration->setPoolHandle(par("attackTargetPoolHandle"));
      registration->setPoolElementParameter(poolElementParameter);

      registration->setTimestamp(simTime());
      send(registration, "toTransport");
   }

   else if(!(strcmp(attackType, "None"))) {
   }
   else {
      throw new cRuntimeError("Invalid attack type: %s", attackType);
   }

   startNextAttackTimer();
}


// ###### Send ASAP_ENDPOINT_UNREACHABLE ####################################
void AttackerProcess::sendASAPEndpointUnreachable(const unsigned int        registrarAddress,
                                                  const unsigned int        registrarPort,
                                                  const char*               poolHandle,
                                                  PoolElementIdentifierType identifier)
{
   ASAPEndpointUnreachable* endpointUnreachable = new ASAPEndpointUnreachable("ASAP_ENDPOINT_UNREACHABLE", ASAP);
   endpointUnreachable->setProtocol(ASAP);
   endpointUnreachable->setDstAddress(registrarAddress);
   endpointUnreachable->setSrcPort(AttackerPort);
   endpointUnreachable->setDstPort(registrarPort);
   endpointUnreachable->setPoolHandle(poolHandle);
   endpointUnreachable->setIdentifier(identifier);

   endpointUnreachable->setTimestamp(simTime());
   send(endpointUnreachable, "toTransport");
}


// ###### Handle ASAP_HANDLE_RESOLUTION_RESPONSE message ####################
void AttackerProcess::handleASAPHandleResolutionResponse(ASAPHandleResolutionResponse* msg)
{
   if(!msg->getRejectFlag()) {
     VictimPoolHandle = msg->getPoolHandle();
     if(VictimPoolElementList) {
        delete VictimPoolElementList;
     }
     VictimPoolElements    = msg->getPoolElementParameterArraySize();
     VictimPoolElementList = new PoolElementIdentifierType[msg->getPoolElementParameterArraySize()];
     CHECK(VictimPoolElementList);
     for(unsigned int i = 0;i < VictimPoolElements;i++) {
        VictimPoolElementList[i] = msg->getPoolElementParameter(i).getIdentifier();
     }

     ev << Description << "Got handle resolution response" << endl;
     GotHandleResolutionResponse = true;
     TotalHandleResolutions++;
      if(uniform(0.0, 1.0) <= (double)par("attackReportUnreachableProbability")) {
         for(unsigned int i = 0;i < VictimPoolElements;i++) {
            ev << "Sending endpoint unreachable for PE #"
               << msg->getPoolElementParameter(i).getIdentifier() << "..." << endl;
            sendASAPEndpointUnreachable(msg->getSrcAddress(), AttackerPort,
                                        msg->getPoolHandle(),
                                        msg->getPoolElementParameter(i).getIdentifier());

            ASAPEndpointUnreachable* endpointUnreachable = new ASAPEndpointUnreachable("ASAP_ENDPOINT_UNREACHABLE", ASAP);
            endpointUnreachable->setProtocol(ASAP);
            endpointUnreachable->setDstAddress(msg->getSrcAddress());
            endpointUnreachable->setSrcPort(AttackerPort);
            endpointUnreachable->setDstPort(msg->getSrcPort());
            endpointUnreachable->setPoolHandle(msg->getPoolHandle());
            endpointUnreachable->setIdentifier(msg->getPoolElementParameter(i).getIdentifier());

            endpointUnreachable->setTimestamp(simTime());
            send(endpointUnreachable, "toTransport");

            TotalFailureReports++;
         }
      }
   }
}


// ###### Handle ASAP_REGISTRATION_RESPONSE message #########################
void AttackerProcess::handleASAPRegistrationResponse(ASAPRegistrationResponse* msg)
{
   if(!msg->getRejectFlag()) {
     ev << Description << "Got registration response" << endl;
     TotalRegistrations++;
   }
}


// ###### Handle ASAP_ENDPOINT_KEEP_ALIVE message ###########################
void AttackerProcess::handleASAPEndpointKeepAlive(ASAPEndpointKeepAlive* msg)
{
   if((bool)par("attackAnswerKeepAlive")) {
      ev << Description << "Replying endpoint keep-alive" << endl;
      ASAPEndpointKeepAliveAck* endpointKeepAliveAck = new ASAPEndpointKeepAliveAck("ASAP_ENDPOINT_KEEP_ALIVE_ACK");
      endpointKeepAliveAck->setProtocol(ASAP);
      endpointKeepAliveAck->setDstAddress(msg->getSrcAddress());
      endpointKeepAliveAck->setSrcPort(AttackerPort);
      endpointKeepAliveAck->setDstPort(msg->getSrcPort());
      endpointKeepAliveAck->setPoolHandle(msg->getPoolHandle());
      endpointKeepAliveAck->setIdentifier(msg->getIdentifier());

      endpointKeepAliveAck->setTimestamp(simTime());
      send(endpointKeepAliveAck, "toTransport");

      TotalEndpointKeepAliveAcks++;
   }
   else {
      ev << Description << "Ignoring endpoint keep-alive" << endl;
   }
}


// ###### Handle application message ########################################
void AttackerProcess::handleApplicationMessage(CalcAppMessage* msg)
{
   TotalIgnoredApplicationMessages++;
}


// ###### Handle message ####################################################
void AttackerProcess::handleMessage(cMessage* msg)
{
   ev << Description << "Received message \""   << msg->getName()
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
         startShutdownTimer();
         startNextAttackTimer();
         colorizeModule(getParentModule(), "#00ff00");
         FSM_Goto(State, RUN_ATTACK);
       break;

      case FSM_Exit(RUN_ATTACK):
         if(msg == NextAttackTimer) {
            NextAttackTimer = NULL;
            handleNextAttackTimer();
         }
         else if(dynamic_cast<ASAPHandleResolutionResponse*>(msg)) {
            handleASAPHandleResolutionResponse((ASAPHandleResolutionResponse*)msg);
         }
         else if(dynamic_cast<ASAPRegistrationResponse*>(msg)) {
            handleASAPRegistrationResponse((ASAPRegistrationResponse*)msg);
         }
         else if(dynamic_cast<ASAPEndpointKeepAlive*>(msg)) {
            handleASAPEndpointKeepAlive((ASAPEndpointKeepAlive*)msg);
         }
         else if(dynamic_cast<CalcAppMessage*>(msg)) {
            handleApplicationMessage((CalcAppMessage*)msg);
         }
         else if(msg == ShutdownTimer) {
            ShutdownTimer = NULL;
            FSM_Goto(State, SHUTDOWN_SERVICE);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(SHUTDOWN_SERVICE):
         colorizeModule(getParentModule(), "#ff0000");
         stopNextAttackTimer();
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
       break;

      case FSM_Exit(WAIT_FOR_RESTART):
         if(msg == RestartDelayTimer) {
            RestartDelayTimer = NULL;
            ev << Description << "Restarting service ..." << endl;
            FSM_Goto(State, STARTUP_SERVICE);
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
