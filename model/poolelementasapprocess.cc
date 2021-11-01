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

#include "utilities.h"


class PoolElementASAPProcess : public cSimpleModule
{
   virtual void initialize();
   virtual void handleMessage(cMessage* msg);


   // ====== States ==========================================================
   private:
   enum {
      INIT                                         = 0,

      WAIT_FOR_APPLICATION                         = FSM_Steady(1),

      BIND                                         = FSM_Transient(2),

      SEND_REGISTRATION                            = FSM_Transient(3),
      WAIT_FOR_REGISTRATION_RESPONSE               = FSM_Steady(4),
      REGISTRATION_SEND_SERVER_HUNT_REQUEST        = FSM_Transient(5),
      REGISTRATION_WAIT_FOR_SERVER_HUNT_RESPONSE   = FSM_Steady(6),

      REGISTERED                                   = FSM_Steady(7),

      SEND_DEREGISTRATION                          = FSM_Transient(8),
      WAIT_FOR_DEREGISTRATION_RESPONSE             = FSM_Steady(9),
      DEREGISTRATION_SEND_SERVER_HUNT_REQUEST      = FSM_Transient(10),
      DEREGISTRATION_WAIT_FOR_SERVER_HUNT_RESPONSE = FSM_Steady(11),

      UNBIND                                       = FSM_Transient(12)
   };
   cFSM State;


   // ====== Methods =========================================================
   void startT2RegistrationTimer();
   void stopT2RegistrationTimer();
   void startT3DeregistrationTimer();
   void stopT3DeregistrationTimer();
   void startT4ReregistrationTimer(double nextReregistration = -1.0);
   void stopT4ReregistrationTimer();
   void sendASAPRegistration();
   void sendASAPDeregistration();
   bool handleASAPRegistrationResponse(ASAPRegistrationResponse* msg);
   bool handleASAPDeregistrationResponse(ASAPDeregistrationResponse* msg);
   void handleASAPEndpointKeepAlive(ASAPEndpointKeepAlive* msg);
   void startRegistrarHunt();
   void handleRegistrarHuntResponse(RegistrarHuntResponse* msg);
   void updatePoolPolicy(PolicyUpdate* msg);
   void updatePoolElementParameter(RegisterPoolElement* msg);
   void bindService();
   void unbindService();


   // ====== Timers ==========================================================
   cMessage*             T2RegistrationTimer;
   cMessage*             T3DeregistrationTimer;
   cMessage*             T4ReregistrationTimer;


   // ====== Variables =======================================================
   unsigned int          LocalPort;
   opp_string            PoolHandle;
   cPoolElementParameter PoolElement;
   unsigned int          HomeRegistrarAddress;
   unsigned int          RegistrationAttempts;
   unsigned int          DeregAttempts;
   bool                  HasSentRegisterPoolElementAck;
   bool                  HasReceivedDeregistration;
   bool                  HasReceivedPolicyUpdate;

   opp_string            Description;
};

Define_Module(PoolElementASAPProcess);



// ###### Initialize ########################################################
void PoolElementASAPProcess::initialize()
{
   // ------ Initialize variables -------------------------------------------
   State.setName("State");

   LocalPort                     = PoolElementASAPPortStart;
   HomeRegistrarAddress          = 0;
   RegistrationAttempts          = 0;
   HasSentRegisterPoolElementAck = false;
   HasReceivedDeregistration     = false;
   HasReceivedPolicyUpdate       = false;
   T2RegistrationTimer           = NULL;
   T3DeregistrationTimer         = NULL;
   T4ReregistrationTimer         = NULL;

   Description = format("PoolElementASAPProcess at %u:%u> ",
                        getLocalAddress(this), LocalPort);

   // ------ Prepare startup ------------------------------------------------
   scheduleAt(simTime(), new cMessage("StartupTimer"));
}


// ###### Send ASAP_REGISTRATION ############################################
void PoolElementASAPProcess::sendASAPRegistration()
{
   ASAPRegistration* registration = new ASAPRegistration("ASAP_REGISTRATION", ASAP);
   registration->setProtocol(ASAP);
   registration->setDstAddress(HomeRegistrarAddress);
   registration->setSrcPort(LocalPort);
   registration->setDstPort(RegistrarPort);
   registration->setPoolHandle(PoolHandle.c_str());
   registration->setPoolElementParameter(PoolElement);

   registration->setTimestamp(simTime());
   send(registration, "toTransport");
}


// ###### Send ASAP_DEREGISTRATION ##########################################
void PoolElementASAPProcess::sendASAPDeregistration()
{
   ASAPDeregistration* deregistration = new ASAPDeregistration("ASAP_DEREGISTRATION", ASAP);
   deregistration->setProtocol(ASAP);
   deregistration->setDstAddress(HomeRegistrarAddress);
   deregistration->setSrcPort(LocalPort);
   deregistration->setDstPort(RegistrarPort);
   deregistration->setPoolHandle(PoolHandle.c_str());
   deregistration->setIdentifier(PoolElement.getIdentifier());

   deregistration->setTimestamp(simTime());
   send(deregistration, "toTransport");
}


// ###### Handle ASAP_REGISTRATION_RESPONSE #################################
bool PoolElementASAPProcess::handleASAPRegistrationResponse(ASAPRegistrationResponse* msg)
{
   const int cmpResult = opp_strcmp(msg->getPoolHandle(), PoolHandle.c_str());
   if(cmpResult != 0) {
      error("ASAP_REGISTRATION_RESPONSE for unexpected pool handle!");
      return(false);
   }

   if(msg->getIdentifier() != PoolElement.getIdentifier()) {
      error("ASAP_REGISTRATION_RESPONSE for unexpected PE identifier!");
      return(false);
   }
   return(msg->getError() == 0);
}


// ###### Handle ASAP_DEREGISTRATION_RESPONSE ###############################
bool PoolElementASAPProcess::handleASAPDeregistrationResponse(ASAPDeregistrationResponse* msg)
{
   const int cmpResult = opp_strcmp(msg->getPoolHandle(), PoolHandle.c_str());
   if(cmpResult != 0) {
      error("ASAP_DEREGISTRATION_RESPONSE for unexpected pool handle!");
      return(false);
   }

   if(msg->getIdentifier() != PoolElement.getIdentifier()) {
      error("ASAP_DEREGISTRATION_RESPONSE for unexpected PE identifier!");
      return(false);
   }
   return(msg->getError() == 0);
}


// ###### Handle ASAP_ENDPOINT_KEEP_ALIVE ###################################
void PoolElementASAPProcess::handleASAPEndpointKeepAlive(ASAPEndpointKeepAlive* msg)
{
   const int cmpResult = opp_strcmp(msg->getPoolHandle(), PoolHandle.c_str());
   if(cmpResult != 0) {
      error("ASAP_ENDPOINT_KEEP_ALIVE for unexpected pool handle!");
      return;
   }

   if(msg->getHomeFlag()) {
      EV << Description << "Changing home registrar: "
         <<  HomeRegistrarAddress << " -> " << msg->getSrcAddress() << endl;
      HomeRegistrarAddress = msg->getSrcAddress();
   }

   // Section 3.4 of ASAP draft
   ASAPEndpointKeepAliveAck* endpointKeepAliveAck = new ASAPEndpointKeepAliveAck("ASAP_ENDPOINT_KEEP_ALIVE_ACK", ASAP);
   endpointKeepAliveAck->setProtocol(ASAP);
   endpointKeepAliveAck->setDstAddress(msg->getSrcAddress());
   endpointKeepAliveAck->setSrcPort(LocalPort);
   endpointKeepAliveAck->setDstPort(msg->getSrcPort());
   endpointKeepAliveAck->setPoolHandle(PoolHandle.c_str());
   endpointKeepAliveAck->setIdentifier(PoolElement.getIdentifier());

   endpointKeepAliveAck->setTimestamp(simTime());
   send(endpointKeepAliveAck, "toTransport");
}


// ###### Start registration timer ##########################################
void PoolElementASAPProcess::startT2RegistrationTimer()
{
   OPP_CHECK(T2RegistrationTimer == NULL);
   T2RegistrationTimer = new cMessage("T2RegistrationTimer");
   scheduleAt(simTime() + (double)par("asapRegistrationTimeout"), T2RegistrationTimer);
}


// ###### Stop registration timer ###########################################
void PoolElementASAPProcess::stopT2RegistrationTimer()
{
   OPP_CHECK(T2RegistrationTimer != NULL);
   delete cancelEvent(T2RegistrationTimer);
   T2RegistrationTimer = NULL;
}


// ###### Start deregistration timer ########################################
void PoolElementASAPProcess::startT3DeregistrationTimer()
{
   OPP_CHECK(T3DeregistrationTimer == NULL);
   T3DeregistrationTimer = new cMessage("T3DeregistrationTimer");
   scheduleAt(simTime() + (double)par("asapDeregistrationTimeout"), T3DeregistrationTimer);
}


// ###### Stop deregistration timer #########################################
void PoolElementASAPProcess::stopT3DeregistrationTimer()
{
   OPP_CHECK(T3DeregistrationTimer != NULL);
   delete cancelEvent(T3DeregistrationTimer);
   T3DeregistrationTimer = NULL;
}


// ###### Start reregistration timer ########################################
void PoolElementASAPProcess::startT4ReregistrationTimer(double nextReregistration)
{
   OPP_CHECK(T4ReregistrationTimer == NULL);
   T4ReregistrationTimer = new cMessage("T4ReregistrationTimer");

   if(nextReregistration < 0.0) {
      // Setting as defined in section 5.1 of the ASAP draft
      nextReregistration = ((double)PoolElement.getRegistrationLife() / 1000.0) - 20.0;
      if(nextReregistration > 600.0) {
         nextReregistration = 600.0;
      }
      else if(nextReregistration < 1.0) {   // This should be a useful lower limit
         nextReregistration = 1.0;
      }
   }

   EV << Description << "Next reregistration in " << nextReregistration << "s" << endl;
   scheduleAt(simTime() + nextReregistration, T4ReregistrationTimer);
}


// ###### Stop reregistration timer #########################################
void PoolElementASAPProcess::stopT4ReregistrationTimer()
{
   OPP_CHECK(T4ReregistrationTimer != NULL);
   delete cancelEvent(T4ReregistrationTimer);
   T4ReregistrationTimer = NULL;
}


// ###### Update pool policy information ####################################
void PoolElementASAPProcess::updatePoolPolicy(PolicyUpdate* msg)
{
   PoolElement.setPoolPolicyParameter(msg->getPoolPolicyParameter());
}


// ###### Update pool element information ###################################
void PoolElementASAPProcess::updatePoolElementParameter(RegisterPoolElement* msg)
{
   PoolHandle  = opp_string(msg->getPoolHandle());
   PoolElement = msg->getPoolElementParameter();
}


// ###### Send RegistrarHuntRequest #########################################
void PoolElementASAPProcess::startRegistrarHunt()
{
   RegistrarHuntRequest* request = new RegistrarHuntRequest("RegistrarHuntRequest");
   EV << Description << "Starting registrar hunt" << endl;
   send(request, "toRegistrarTable");
}


// ###### Handle RegistrarHuntRequest #######################################
void PoolElementASAPProcess::handleRegistrarHuntResponse(RegistrarHuntResponse* msg)
{
   HomeRegistrarAddress = msg->getRegistrarAddress();
   EV << Description << "Selected name server at " << HomeRegistrarAddress << endl;
}


// ###### Bind ASAP service #################################################
void PoolElementASAPProcess::bindService()
{
   // ------ Bind to port ---------------------------------------------------
   LocalPort++;
   if(LocalPort > PoolElementASAPPortEnd) {
      LocalPort = PoolElementASAPPortStart;
   }
   Description = format("PoolElementASAPProcess at %u:%u> ",
                        getLocalAddress(this), LocalPort);
   EV << Description << "Binding port ..." << endl;

   BindMessage* msg = new BindMessage("Bind");
   msg->setPort(LocalPort);
   send(msg, "toTransport");

   // ------ Update registrator address/port --------------------------------
   cTransportParameter registratorTransportParameter;
   registratorTransportParameter.setAddress(PoolElement.getUserTransportParameter().getAddress());
   registratorTransportParameter.setPort(LocalPort);
   PoolElement.setRegistratorTransportParameter(registratorTransportParameter);

}


// ###### Unbind ASAP service ###############################################
void PoolElementASAPProcess::unbindService()
{
   EV << Description << "Unbinding port" << endl;

   // ------ Unbind port ----------------------------------------------------
   UnbindMessage* msg = new UnbindMessage("Bind");
   msg->setPort(LocalPort);
   send(msg, "toTransport");
}


// ###### Handle message ####################################################
void PoolElementASAPProcess::handleMessage(cMessage* msg)
{
   EV << Description << "Received message \"" << msg->getName()
      << "\" in state " << State.getStateName() << endl;

   FSM_Switch(State) {

      case FSM_Exit(INIT):
         FSM_Goto(State, WAIT_FOR_APPLICATION);
       break;

      case FSM_Exit(WAIT_FOR_APPLICATION):
         if(dynamic_cast<RegisterPoolElement*>(msg)) {
            EV << Description << "Got RegisterPoolElement -> Starting registration" << endl;
            RegistrationAttempts                = 0;
            DeregAttempts                       = 0;
            HasSentRegisterPoolElementAck       = false;
            HasReceivedDeregistration           = false;

            updatePoolElementParameter((RegisterPoolElement*)msg);
            FSM_Goto(State, BIND);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(BIND):
         bindService();
         FSM_Goto(State, SEND_REGISTRATION);
       break;

      case FSM_Exit(SEND_REGISTRATION):
         if( (HomeRegistrarAddress == 0) ||
             (RegistrationAttempts >= (unsigned int)par("asapMaxRegistrationAttempts")) ) {
            EV << Description << "Registration requires registrar hunt" << endl;
            RegistrationAttempts = 0;
            FSM_Goto(State, REGISTRATION_SEND_SERVER_HUNT_REQUEST);
         }
         else {
            RegistrationAttempts++;
            EV << Description << "Sending ASAP_REGISTRATION ... (attempt "
               << RegistrationAttempts << " of " << (unsigned int)par("asapMaxRegistrationAttempts") << ")" << endl;
            HasReceivedPolicyUpdate = false;
            sendASAPRegistration();
            startT2RegistrationTimer();
            FSM_Goto(State, WAIT_FOR_REGISTRATION_RESPONSE);
         }
       break;

      case FSM_Exit(WAIT_FOR_REGISTRATION_RESPONSE):
         if(dynamic_cast<ASAPRegistrationResponse*>(msg)) {
            EV << Description << "Got ASAP_REGISTRATION_RESPONSE" << endl;
            stopT2RegistrationTimer();
            RegistrationAttempts = 0;
            if(!(((ASAPRegistrationResponse*)msg)->getRejectFlag())) {
               if(handleASAPRegistrationResponse((ASAPRegistrationResponse*)msg)) {
                  EV << Description << "Registration successful" << endl;
                  startT4ReregistrationTimer();
                  if(!HasSentRegisterPoolElementAck) {
                     HasSentRegisterPoolElementAck = true;
                     send(new RegisterPoolElementAck("RegisterPoolElementAck"), "toApplication");
                  }
                  if(HasReceivedPolicyUpdate) {
                     EV << Description << "Delayed PolicyUpdate -> reregistering ..." << endl;
                     stopT4ReregistrationTimer();
                     FSM_Goto(State, SEND_REGISTRATION);
                  }
                  else if(HasReceivedDeregistration) {
                     EV << Description << "Delayed DeregisterPoolElement -> deregistering ..." << endl;
                     stopT4ReregistrationTimer();
                     DeregAttempts = 0;
                     FSM_Goto(State, SEND_DEREGISTRATION);
                  }
                  else {
                     FSM_Goto(State, REGISTERED);
                  }
               }
               else {
                  FSM_Goto(State, SEND_REGISTRATION);
               }
            }
            else {
               /* In order to avoid adding another state, we simply use the
                  T4ReregistrationTimer to wait the asapServerHuntRetryDelay.
                  After that, a new registrar will be chosen. */
               EV << Description << "Registration has been rejected! Trying to find other registrar!" << endl;
               HomeRegistrarAddress = 0;
               startT4ReregistrationTimer(par("asapServerHuntRetryDelay"));
               FSM_Goto(State, REGISTERED);
            }
         }
         else if(msg == T2RegistrationTimer) {
            T2RegistrationTimer = NULL;
            EV << Description << "Registration timed out" << endl;
            FSM_Goto(State, SEND_REGISTRATION);
         }
         else if(dynamic_cast<ASAPEndpointKeepAlive*>(msg)) {
            EV << Description << "Got ASAP_ENDPOINT_KEEP_ALIVE -> sending reply ..." << endl;
            handleASAPEndpointKeepAlive((ASAPEndpointKeepAlive*)msg);
         }
         else if(dynamic_cast<PolicyUpdate*>(msg)) {
            EV << Description << "Got PolicyUpdate in state " << State.getName()
               << " - will be processed later" << endl;
            HasReceivedPolicyUpdate = ((PolicyUpdate*)msg)->getReregisterImmediately();
            if(HasReceivedPolicyUpdate) {
               EV << "Immediately re-registering ..." << endl;
            }
            else {
               EV << "No immediate reregistration will be made" << endl;
            }
            updatePoolPolicy((PolicyUpdate*)msg);
         }
         else if(dynamic_cast<DeregisterPoolElement*>(msg)) {
            EV << Description << "Got DeregisterPoolElement in state " << State.getName()
               << " - will be processed later" << endl;
            HasReceivedDeregistration = true;
         }
         else if(dynamic_cast<ResetPoolElement*>(msg)) {
            EV << Description << "Received Reset - unclean shutdown ..." << endl;
            stopT2RegistrationTimer();
            send(new ResetPoolElementAck("ResetPoolElementAck"), "toApplication");
            FSM_Goto(State, UNBIND);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(REGISTRATION_SEND_SERVER_HUNT_REQUEST):
         startRegistrarHunt();
         FSM_Goto(State, REGISTRATION_WAIT_FOR_SERVER_HUNT_RESPONSE);
       break;

      case FSM_Exit(REGISTRATION_WAIT_FOR_SERVER_HUNT_RESPONSE):
         if(dynamic_cast<RegistrarHuntResponse*>(msg)) {
            handleRegistrarHuntResponse((RegistrarHuntResponse*)msg);
            FSM_Goto(State, SEND_REGISTRATION);
         }
         else if(dynamic_cast<PolicyUpdate*>(msg)) {
            EV << Description << "Got PolicyUpdate in state " << State.getName()
               << " - will be processed later" << endl;
            HasReceivedPolicyUpdate = ((PolicyUpdate*)msg)->getReregisterImmediately();
            if(HasReceivedPolicyUpdate) {
               EV << "Immediately re-registering ..." << endl;
            }
            else {
               EV << "No immediate reregistration will be made" << endl;
            }
            updatePoolPolicy((PolicyUpdate*)msg);
         }
         else if(dynamic_cast<DeregisterPoolElement*>(msg)) {
            EV << Description << "Got DeregisterPoolElement in state " << State.getName()
               << " - will be processed later" << endl;
            HasReceivedDeregistration = true;
         }
         else if(dynamic_cast<ASAPEndpointKeepAlive*>(msg)) {
            handleIgnore(Description, msg, State);
         }
         else if(dynamic_cast<ResetPoolElement*>(msg)) {
            EV << Description << "Received Reset - unclean shutdown ..." << endl;
            send(new ResetPoolElementAck("ResetPoolElementAck"), "toApplication");
            FSM_Goto(State, UNBIND);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(REGISTERED):
         if(msg == T4ReregistrationTimer) {
            T4ReregistrationTimer = NULL;
            EV << Description << "Reregistration timer -> reregistering" << endl;
            RegistrationAttempts = 0;
            FSM_Goto(State, SEND_REGISTRATION);
         }
         else if(dynamic_cast<ASAPEndpointKeepAlive*>(msg)) {
            EV << Description << "Got ASAP_ENDPOINT_KEEP_ALIVE -> sending reply ..." << endl;
            handleASAPEndpointKeepAlive((ASAPEndpointKeepAlive*)msg);
         }
         else if(dynamic_cast<PolicyUpdate*>(msg)) {
            EV << Description << "Got PolicyUpdate in state " << State.getName() << endl;
            updatePoolPolicy((PolicyUpdate*)msg);  // Always update information
            if(((PolicyUpdate*)msg)->getReregisterImmediately()) {
               EV << "Reregistering immediately ..." << endl;
               stopT4ReregistrationTimer();
               FSM_Goto(State, SEND_REGISTRATION);
            }
            else {
               EV << "Delaying reregistration until next interval" << endl;
            }
         }
         else if(dynamic_cast<DeregisterPoolElement*>(msg)) {
            EV << Description << "Got DeregisterPoolElement from application. Deregistering ..." << endl;
            stopT4ReregistrationTimer();
            DeregAttempts = 0;
            FSM_Goto(State, SEND_DEREGISTRATION);
         }
         else if(dynamic_cast<ResetPoolElement*>(msg)) {
            EV << Description << "Received Reset - unclean shutdown ..." << endl;
            stopT4ReregistrationTimer();
            send(new ResetPoolElementAck("ResetPoolElementAck"), "toApplication");
            FSM_Goto(State, UNBIND);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(SEND_DEREGISTRATION):
         if( (HomeRegistrarAddress == 0) ||
             (DeregAttempts >= (unsigned int)par("asapMaxRegistrationAttempts")) ) {
            EV << Description << "Deregistration requires registrar hunt" << endl;
            DeregAttempts = 0;
            FSM_Goto(State, DEREGISTRATION_SEND_SERVER_HUNT_REQUEST);
         }
         else {
            DeregAttempts++;
            EV << Description << "Sending ASAP_DEREGISTRATION ... (attempt "
               << DeregAttempts << " of " << (unsigned int)par("asapMaxRegistrationAttempts") << ")" << endl;
            sendASAPDeregistration();
            startT3DeregistrationTimer();
            FSM_Goto(State, WAIT_FOR_DEREGISTRATION_RESPONSE);
         }
       break;

      case FSM_Exit(WAIT_FOR_DEREGISTRATION_RESPONSE):
         if(dynamic_cast<ASAPDeregistrationResponse*>(msg)) {
            EV << Description << "Got ASAP_DEREGISTRATION_RESPONSE" << endl;
            stopT3DeregistrationTimer();
            DeregAttempts = 0;
            if(handleASAPDeregistrationResponse((ASAPDeregistrationResponse*)msg)) {
               EV << Description << "Deregistration successful" << endl;
               send(new DeregisterPoolElementAck("DeregisterPoolElementAck"), "toApplication");
               FSM_Goto(State, UNBIND);
            }
            else {
               EV << Description << "Deregistration failed -> retry" << endl;
               FSM_Goto(State, SEND_DEREGISTRATION);
            }
         }
         else if(msg == T3DeregistrationTimer) {
            T3DeregistrationTimer = NULL;
            EV << Description << "Deregistration timed out" << endl;
            FSM_Goto(State, SEND_DEREGISTRATION);
         }
         else if(dynamic_cast<ASAPEndpointKeepAlive*>(msg)) {
            handleIgnore(Description, msg, State);
         }
         else if(dynamic_cast<ResetPoolElement*>(msg)) {
            EV << Description << "Received Reset - unclean shutdown ..." << endl;
            stopT3DeregistrationTimer();
            send(new ResetPoolElementAck("ResetPoolElementAck"), "toApplication");
            FSM_Goto(State, UNBIND);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(DEREGISTRATION_SEND_SERVER_HUNT_REQUEST):
         startRegistrarHunt();
         FSM_Goto(State, DEREGISTRATION_WAIT_FOR_SERVER_HUNT_RESPONSE);
       break;

      case FSM_Exit(DEREGISTRATION_WAIT_FOR_SERVER_HUNT_RESPONSE):
         if(dynamic_cast<RegistrarHuntResponse*>(msg)) {
            handleRegistrarHuntResponse((RegistrarHuntResponse*)msg);
            FSM_Goto(State, SEND_DEREGISTRATION);
         }
         else if(dynamic_cast<ASAPEndpointKeepAlive*>(msg)) {
            handleIgnore(Description, msg, State);
         }
         else if(dynamic_cast<ResetPoolElement*>(msg)) {
            EV << Description << "Received Reset - unclean shutdown ..." << endl;
            send(new ResetPoolElementAck("ResetPoolElementAck"), "toApplication");
            FSM_Goto(State, UNBIND);
         }
         else {
            handleUnexpectedMsgState(msg, State);
         }
       break;

      case FSM_Exit(UNBIND):
         unbindService();
         FSM_Goto(State, WAIT_FOR_APPLICATION);
       break;

   }

   delete msg;
}
