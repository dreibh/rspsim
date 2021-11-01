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

#ifndef HANDLESPACEMANAGEMENTWRAPPER_H
#define HANDLESPACEMANAGEMENTWRAPPER_H

#include "config.h"
#include "utilities.h"
#include "poolhandlespacemanagement.h"
#include "takeoverprocess.h"
#include "messages_m.h"
#include "registrarmessages_m.h"

#include <omnetpp.h>


// ##########################################################################
// #### Pool Element                                                     ####
// ##########################################################################

class cPoolElement
{
   public:
   void print(const bool full);

   // ====== Set/Get methods ================================================
   inline unsigned int getHomeRegistrarIdentifier() const {
      return(Node->HomeRegistrarIdentifier);
   }
   inline void setHomeRegistrarIdentifier(unsigned int identifier) {
      Node->HomeRegistrarIdentifier = identifier;
   }
   inline unsigned int getIdentifier() const {
      return(Node->Identifier);
   }
   inline void setIdentifier(unsigned int identifier) {
      Node->Identifier = identifier;
   }
   inline unsigned int getRegistratorAddress() const {
      return(Node->RegistratorTransport->AddressArray[0].ta.ta_addr);
   }
   inline void setRegistratorAddress(unsigned int address) {
      Node->RegistratorTransport->AddressArray[0].ta.ta_family = AF_TEST;
      Node->RegistratorTransport->AddressArray[0].ta.ta_addr   = address;
   }
   inline unsigned int getRegistratorPort() const {
      return(Node->RegistratorTransport->AddressArray[0].ta.ta_port);
   }
   inline void setRegistratorPort(unsigned int port) {
      Node->RegistratorTransport->AddressArray[0].ta.ta_family = AF_TEST;
      Node->RegistratorTransport->AddressArray[0].ta.ta_port   = port;
   }
   inline unsigned int getRegistrationLife() const {
      return(Node->RegistrationLife);
   }
   inline void setRegistrationLife(unsigned int registrationLife) {
      Node->RegistrationLife = registrationLife;
   }
   inline unsigned int getSelectionCounter() const {
      return(Node->SelectionCounter);
   }
   inline void setSelectionCounter(unsigned int selectionCounter) {
      Node->SelectionCounter = selectionCounter;
   }
   inline unsigned int getVirtualCounter() const {
      return(Node->VirtualCounter);
   }
   inline void setVirtualCounter(unsigned int selectionCounter) {
      Node->VirtualCounter = selectionCounter;
   }
   inline unsigned int getUnreachabilityReports() const {
      return(Node->UnreachabilityReports);
   }
   inline void setUnreachabilityReports(unsigned int reports) {
      Node->UnreachabilityReports = reports;
   }

   inline unsigned int getPolicyType() const {
      return(Node->PolicySettings.PolicyType);
   }
   inline void setPolicyType(unsigned int policyType) {
      Node->PolicySettings.PolicyType = policyType;
   }
   inline unsigned int getWeight() const {
      return(Node->PolicySettings.Weight);
   }
   inline void setWeight(unsigned int weight) {
      Node->PolicySettings.Weight = weight;
   }
   inline unsigned int getLoad() const {
      return(Node->PolicySettings.Load);
   }
   inline void setLoad(unsigned int load) {
      Node->PolicySettings.Load = load;
   }
   inline unsigned int getLoadDegradation() const {
      return(Node->PolicySettings.LoadDegradation);
   }
   inline void setLoadDegradation(unsigned int loadDegradation) {
      Node->PolicySettings.LoadDegradation = loadDegradation;
   }
   inline unsigned int getLoadDPF() const {
      return(Node->PolicySettings.LoadDPF);
   }
   inline void setLoadDPF(unsigned int loadDPF) {
      Node->PolicySettings.LoadDPF = loadDPF;
   }
   inline unsigned int getWeightDPF() const {
      return(Node->PolicySettings.WeightDPF);
   }
   inline void setWeightDPF(unsigned int loadDPF) {
      Node->PolicySettings.WeightDPF = loadDPF;
   }
   inline unsigned int getDistance() const {
      return(Node->PolicySettings.Distance);
   }
   inline void setDistance(unsigned int distance) {
      Node->PolicySettings.Distance = distance;
   }
   inline unsigned int getAddress() const {
      return(Node->UserTransport->AddressArray[0].ta.ta_addr);
   }
   inline void setAddress(unsigned int address) {
      Node->UserTransport->AddressArray[0].ta.ta_family = AF_TEST;
      Node->UserTransport->AddressArray[0].ta.ta_addr   = address;
   }
   inline unsigned int getPort() const {
      return(Node->UserTransport->AddressArray[0].ta.ta_port);
   }
   inline void setPort(unsigned int port) {
      Node->UserTransport->AddressArray[0].ta.ta_family = AF_TEST;
      Node->UserTransport->AddressArray[0].ta.ta_port   = port;
   }
   inline const char* getOwnerPoolHandle() const {
      return((const char*)Node->OwnerPoolNode->Handle.Handle);
   }

   cPoolElementParameter toPoolElementParameter() const;


   // ====== Public data ====================================================
   public:
   EndpointKeepAliveTransmissionMessage* EndpointKeepAliveTransmissionTimer;
   EndpointKeepAliveTimeoutMessage*      EndpointKeepAliveTimeoutTimer;
   LifetimeExpiryMessage*                LifetimeExpiryTimer;

   // ====== Private data ===================================================
   private:
   friend class cPoolHandlespace;

   struct TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* Node;
};


opp_string getPoolElementParameterDescription(cPoolElementParameter& poolElementParameter);
opp_string getPoolElementDescription(cPoolElement& poolElement);



// ##########################################################################
// #### Handlespace                                                      ####
// ##########################################################################

class cPoolHandlespace
{
   public:
   cPoolHandlespace(const unsigned int homeRegistrarIdentifier = 0);
   ~cPoolHandlespace();


   // ====== Handlespace management =========================================
   void clear();
   void print(const unsigned int homeRegistrarIdentifier = 0);

   unsigned int registerPoolElement(const char*            poolHandle,
                                    cPoolElementParameter& poolElementParameter,
                                    const unsigned int     registratorAddress,
                                    const unsigned int     registratorPort,
                                    cPoolElement*&         poolElement,
                                    bool&                  updated);
   cPoolElement* findPoolElement(const char*        poolHandle,
                                 const unsigned int peIdentifier);
   unsigned int deregisterPoolElement(cPoolElement* poolElement);
   unsigned int deregisterPoolElement(const char*        poolHandle,
                                      const unsigned int peIdentifier);
   void updatePoolElementOwnership(cPoolElement*      poolElement,
                                   const unsigned int registrarIdentifier);


   // ====== Set/Get methods ================================================
   unsigned int getHandlespaceChecksum() const {
      return(TMPL_CLASS(poolHandlespaceManagementGetHandlespaceChecksum, SimpleRedBlackTree)(
               &Handlespace));
   }
   unsigned int getOwnershipChecksum() const {
      return(TMPL_CLASS(poolHandlespaceManagementGetOwnershipChecksum, SimpleRedBlackTree)(
               &Handlespace));
   }
   size_t getPools() {
      return(TMPL_CLASS(poolHandlespaceManagementGetPools, SimpleRedBlackTree)(
               &Handlespace));
   }
   size_t getPoolElements() const {
      return(TMPL_CLASS(poolHandlespaceManagementGetPoolElements, SimpleRedBlackTree)(
               &Handlespace));
   }
   size_t getOwnedPoolElements() const {
      return(TMPL_CLASS(poolHandlespaceManagementGetOwnedPoolElements, SimpleRedBlackTree)(
               &Handlespace));
   }
   size_t getPoolElementsOfPool(const char* poolHandle);

   inline cPoolElement* getFirstPoolElementNode() {
      struct TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode =
         TMPL_CLASS(poolHandlespaceManagementGetFirstPoolElementOwnershipNode, SimpleRedBlackTree)(&Handlespace);
      if(poolElementNode) {
         return((cPoolElement*)poolElementNode->UserData);
      }
      return(NULL);
   }
   inline cPoolElement* getNextPoolElementNode(cPoolElement* node) {
      struct TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode =
         TMPL_CLASS(poolHandlespaceManagementGetNextPoolElementOwnershipNode, SimpleRedBlackTree)(
            &Handlespace,
            node->Node);
      if(poolElementNode) {
         return((cPoolElement*)poolElementNode->UserData);
      }
      return(NULL);
   }


   cPoolElement* getFirstPoolElementOwnedBy(const unsigned int homeRegistrarIdentifier);
   cPoolElement* getNextPoolElementOfSameOwner(struct cPoolElement* poolElement);

   void restartPoolElementExpiryTimer(cPoolElement*            poolElement,
                                      const unsigned long long expiryTimeout);
   size_t purgeExpiredPoolElements();


   size_t selectPoolElementsByPolicy(const char*    poolHandle,
                                     cPoolElement** selectionArray,
                                     size_t&        items,
                                     const size_t   maxHandleResolutionItems,
                                     const size_t   maxIncrement);
   cArray* exportToPoolEntries(const unsigned int homeRegistrarIdentifier);


   // ====== Private data ===================================================
   private:
   friend class cPeerList;

   static void killPoolElement(TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode,
                               void*                                            userData);

   struct TMPL_CLASS(PoolHandlespaceManagement, SimpleRedBlackTree) Handlespace;
};


// ###### Get length of pool handle #########################################
inline size_t getPoolHandleSize(const char* poolHandle)
{
   return(strlen(poolHandle) + 1);
}


// ###### Get policy type from name #########################################
inline unsigned int getPoolPolicyTypeByName(const char* name)
{
   return(poolPolicyGetPoolPolicyTypeByName(name));
}


// ###### Get policy name from type #########################################
inline const char* getPoolPolicyNameByType(const unsigned int policyType)
{
   return(poolPolicyGetPoolPolicyNameByType(policyType));
}



// ##########################################################################
// #### Peer List Node                                                   ####
// ##########################################################################

class cTakeoverProcess;

class cPeerListNode
{
   public:
   void print(const bool full);


   // ====== Set/Get methods ================================================
   unsigned int getIdentifier() const {
      return((unsigned int)Node->Identifier);
   }
   void setIdentifier(const unsigned int identifier) {
      Node->Identifier = (RegistrarIdentifierType)identifier;
   }
   inline unsigned int getStatus() const {
      return(Node->Status);
   }
   inline void setStatus(unsigned int status) {
      Node->Status = status;
   }
   inline unsigned int getAddress() const {
      return(Node->AddressBlock->AddressArray[0].ta.ta_addr);
   }
   inline void setAddress(unsigned int address) {
      Node->AddressBlock->AddressArray[0].ta.ta_family = AF_TEST;
      Node->AddressBlock->AddressArray[0].ta.ta_addr   = address;
   }
   inline unsigned int getPort() const {
      return(Node->AddressBlock->AddressArray[0].ta.ta_port);
   }
   inline void setPort(unsigned int port) {
      Node->AddressBlock->AddressArray[0].ta.ta_family = AF_TEST;
      Node->AddressBlock->AddressArray[0].ta.ta_port   = port;
   }
   inline bool getNewFlag() const {
      return((Node->Flags & PLNF_NEW) != 0);
   }
   inline simtime_t getLastHeared() const {
      return((simtime_t)(Node->LastUpdateTimeStamp / 1000000.0));
   }
   inline void setLastHeared(const simtime_t lastHeared) {
      Node->LastUpdateTimeStamp = (unsigned long long)(1000000.0 * lastHeared.dbl());
   }
   inline unsigned int getTakeoverRegistrarID() const {
      return(Node->TakeoverRegistrarID);
   }
   inline void setTakeoverRegistrarID(unsigned int registrarIdentifier) {
      Node->TakeoverRegistrarID = (RegistrarIdentifierType)registrarIdentifier;
   }
   unsigned int getOwnershipChecksum() const {
      return(TMPL_CLASS(peerListNodeGetOwnershipChecksum, SimpleRedBlackTree)(Node));
   }
   ServerInformationParameter toServerInformationParameter() const;


   // ====== Public data ====================================================
   public:
   LastHeardTimeoutMessage*  LastHeardTimeoutTimer;
   ResponseTimeoutMessage*   ResponseTimeoutTimer;
   TakeoverExpiryMessage*    TakeoverExpiryTimer;

   unsigned int                                         MentorTrials;
   struct TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* Node;
   cTakeoverProcess*                                    Takeover;
};



// ##########################################################################
// #### Peer List                                                        ####
// ##########################################################################

class cPeerList
{
   public:
   cPeerList(cPoolHandlespace*  handlespace,
             const unsigned int registrarIdentifier);
   ~cPeerList();


   // ====== PeerList management ============================================
   void clear();
   void print();
   unsigned int registerPeerListNode(ServerInformationParameter& serverInformationParameter,
                                     cPeerListNode*&             node);
   unsigned int deregisterPeerListNode(cPeerListNode* peerListEntry);
   unsigned int deregisterPeerListNode(unsigned int identifier);
   cPeerListNode* findPeerListNode(const unsigned int identifier);
   cPeerListNode* findPeerListNode(const unsigned int address,
                                   const unsigned int port);


   void purge();

   void resetMentorSelection();
   cPeerListNode* findMentorServer(const unsigned int localAddress,
                                   const unsigned int localPort,
                                   const unsigned int maxTrials);
   cPeerListNode* getUsefulPeerForPE(const unsigned int identifier);


   // ====== Set/Get methods ================================================
   cPeerListNode* getRandomPeerListNode();
   inline size_t getPeers() const {
      return(TMPL_CLASS(peerListManagementGetPeers, SimpleRedBlackTree)(&List));
   }
   inline cPeerListNode* getFirstPeerListNode() {
      struct TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode =
         TMPL_CLASS(peerListManagementGetFirstPeerListNodeFromIndexStorage, SimpleRedBlackTree)(&List);
      if(peerListNode) {
         return((cPeerListNode*)peerListNode->UserData);
      }
      return(NULL);
   }
   inline cPeerListNode* getNextPeerListNode(cPeerListNode* node) {
      struct TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode =
         TMPL_CLASS(peerListManagementGetNextPeerListNodeFromIndexStorage, SimpleRedBlackTree)(
            &List, node->Node);
      if(peerListNode) {
         return((cPeerListNode*)peerListNode->UserData);
      }
      return(NULL);
   }


   // ====== Private data ===================================================
   private:
   friend class cTakeoverProcess;
   static void killPeerListNode(TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode,
                                void*                                         userData);

   struct TMPL_CLASS(PeerListManagement, SimpleRedBlackTree) List;
   cPoolHandlespace*                                         Handlespace;
};



// ##########################################################################
// #### Pool User List                                                   ####
// ##########################################################################

class cPoolUserList
{
   public:
   cPoolUserList();
   ~cPoolUserList();


   // ====== PoolUserList management ========================================
   void clear();
   void print();

   TMPL_CLASS(PoolUserNode, SimpleRedBlackTree)* registerPoolUser(
      const unsigned int address,
      const unsigned int port);
   void purge(const simtime_t minTime);
   double noteHandleResolutionOfPoolUser(const char*        poolHandle,
                                         const unsigned int address,
                                         const unsigned int port,
                                         const size_t       buckets,
                                         const size_t       maxEntries);
   double noteEndpointUnreachableOfPoolUser(const char*        poolHandle,
                                            const unsigned int address,
                                            const unsigned int port,
                                            const unsigned int peIdentifier,
                                            const size_t       buckets,
                                            const size_t       maxEntries);


   // ====== Private data ===================================================
   private:
   struct TMPL_CLASS(PoolUserList, SimpleRedBlackTree) List;
   struct ST_CLASS(PoolUserNode)*                      NewPoolUserNode;
};



// ##########################################################################
// #### Takeover Process                                                 ####
// ##########################################################################

class cTakeoverProcess
{
   public:
   cTakeoverProcess(const unsigned int targetID,
                   cPeerList*          peerList);
   ~cTakeoverProcess();

   inline size_t getOutstandingAcks() const {
      return(takeoverProcessGetOutstandingAcks(Takeover));
   }
   size_t acknowledge(const unsigned int targetID,
                      const unsigned int acknowledgerID);

   // ====== Private data ===================================================
   private:
   TakeoverProcess* Takeover;
};

#endif
