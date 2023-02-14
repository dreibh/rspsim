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
 * Copyright (C) 2003-2023 by Thomas Dreibholz
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

#ifdef VERIFY
#warning Use VERIFY only for debugging purposes - it is very slow!
#endif

#include "handlespacemanagementwrapper.h"
#include "utilities.h"



// ##########################################################################
// #### Pool Element                                                     ####
// ##########################################################################

// ###### Get PoolElementParameter description ##############################
opp_string getPoolElementParameterDescription(cPoolElementParameter& poolElementParameter)
{
   return(format("$%08x at %u:%u",
                 poolElementParameter.getIdentifier(),
                 poolElementParameter.getUserTransportParameter().getAddress(),
                 poolElementParameter.getUserTransportParameter().getPort()));
}


// ###### Get PoolElement description #######################################
opp_string getPoolElementDescription(cPoolElement& poolElement)
{
   return(format("$%08x at %u:%u",
                 poolElement.getIdentifier(),
                 poolElement.getAddress(),
                 poolElement.getPort()));
}


// ###### Get PoolPolicyParameter description ################################
opp_string getPoolPolicyParameterDescription(cPoolPolicyParameter& poolPolicyParameter)
{
   PoolPolicySettings poolPolicySettings;
   char               poolPolicySettingsDescription[256];

   poolPolicySettingsNew(&poolPolicySettings);
   poolPolicySettings.PolicyType      = poolPolicyParameter.getPolicyType();
   poolPolicySettings.Load            = poolPolicyParameter.getLoad();
   poolPolicySettings.LoadDegradation = poolPolicyParameter.getLoadDegradation();
   poolPolicySettings.Weight          = poolPolicyParameter.getWeight();
   poolPolicySettings.LoadDPF         = poolPolicyParameter.getLoadDPF();
   poolPolicySettings.WeightDPF       = poolPolicyParameter.getWeightDPF();
   poolPolicySettings.Distance        = poolPolicyParameter.getDistance();

   poolPolicySettingsGetDescription(&poolPolicySettings,
                                    (char*)&poolPolicySettingsDescription,
                                    sizeof(poolPolicySettingsDescription));
   return(opp_string(poolPolicySettingsDescription));
}


// ###### Get PoolElementParameter from PoolElement #########################
cPoolElementParameter cPoolElement::toPoolElementParameter() const
{
   cPoolElementParameter destination;
   destination.setIdentifier(getIdentifier());
   destination.setHomeRegistrarIdentifier(getHomeRegistrarIdentifier());
   destination.setRegistrationLife(getRegistrationLife());

   cTransportParameter userTransportParameter;
   userTransportParameter.setAddress(getAddress());
   userTransportParameter.setPort(getPort());
   destination.setUserTransportParameter(userTransportParameter);

   cTransportParameter registratorTransportParameter;
   registratorTransportParameter.setAddress(getRegistratorAddress());
   registratorTransportParameter.setPort(getRegistratorPort());
   destination.setRegistratorTransportParameter(registratorTransportParameter);

   cPoolPolicyParameter poolPolicyParameter;
   poolPolicyParameter.setPolicyType(getPolicyType());
   poolPolicyParameter.setWeight(getWeight());
   poolPolicyParameter.setLoad(getLoad());
   poolPolicyParameter.setLoadDegradation(getLoadDegradation());
   poolPolicyParameter.setLoadDPF(getLoadDPF());
   poolPolicyParameter.setWeightDPF(getWeightDPF());
   poolPolicyParameter.setDistance(getDistance());
   destination.setPoolPolicyParameter(poolPolicyParameter);
   return(destination);
}


// ###### Print PoolElement #################################################
void cPoolElement::print(const bool full)
{
   char poolElementNodeDescription[1024];

   TMPL_CLASS(poolElementNodeGetDescription, SimpleRedBlackTree)(
      Node,
      (char*)&poolElementNodeDescription, sizeof(poolElementNodeDescription),
      ((full == true) ? PENPO_FULL : PENPO_ONLY_ID));
   EV << poolElementNodeDescription;
}


// ###### PoolElement disposal callback for handlespace cleanup ##############
void cPoolHandlespace::killPoolElement(TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode,
                                       void*                                            userData)
{
   cPoolElement* poolElement = (cPoolElement*)poolElementNode->UserData;
   if(getSimulation()->getActivityModule()) {
      // The endpoint keepalive timers may still be scheduled. If this is the
      // case, it is now time to cancel them.
      // NOTE: This may not be executed by getSimulation()->deleteNetwork(), since
      //       the FES entries are already deleted when this part is executed!
      if(poolElement->EndpointKeepAliveTransmissionTimer) {
         delete getSimulation()->getActivityModule()->cancelEvent(poolElement->EndpointKeepAliveTransmissionTimer);
         poolElement->EndpointKeepAliveTransmissionTimer = NULL;
      }
      if(poolElement->EndpointKeepAliveTimeoutTimer) {
         delete getSimulation()->getActivityModule()->cancelEvent(poolElement->EndpointKeepAliveTimeoutTimer);
         poolElement->EndpointKeepAliveTimeoutTimer = NULL;
      }
   }
   delete poolElement;
}



// ##########################################################################
// #### Handlespace                                                      ####
// ##########################################################################

// ###### Constructor #######################################################
cPoolHandlespace::cPoolHandlespace(const unsigned int homeRegistrarIdentifier)
{
   TMPL_CLASS(poolHandlespaceManagementNew, SimpleRedBlackTree)(
      &Handlespace, homeRegistrarIdentifier, NULL, killPoolElement, this);
}


// ###### Destructor ########################################################
cPoolHandlespace::~cPoolHandlespace()
{
   clear();
   TMPL_CLASS(poolHandlespaceManagementDelete, SimpleRedBlackTree)(&Handlespace);
}


// ###### Clear #############################################################
void cPoolHandlespace::clear()
{
   TMPL_CLASS(poolHandlespaceManagementClear, SimpleRedBlackTree)(&Handlespace);
}


// ###### Print #############################################################
void cPoolHandlespace::print(const unsigned int homeRegistrarIdentifier)
{
   TMPL_CLASS(PoolNode, SimpleRedBlackTree)*        poolNode;
   TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNodeS;
   TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNodeI;
   cPoolElement*                                    poolElementS;
   cPoolElement*                                    poolElementI;
   char                                             description[256];

   TMPL_CLASS(poolHandlespaceManagementGetDescription, SimpleRedBlackTree)(
      &Handlespace, (char*)&description, sizeof(description));

   EV << description << endl;

   poolNode = TMPL_CLASS(poolHandlespaceManagementGetFirstPoolNode, SimpleRedBlackTree)(&Handlespace);
   while(poolNode != NULL) {
      EV << "+--- ";
      TMPL_CLASS(poolNodeGetDescription, SimpleRedBlackTree)(
         poolNode,
         (char*)&description, sizeof(description));
      EV << description << endl;

      poolElementNodeS = TMPL_CLASS(poolNodeGetFirstPoolElementNodeFromSelection, SimpleRedBlackTree)(poolNode);
      poolElementNodeI = TMPL_CLASS(poolNodeGetFirstPoolElementNodeFromIndex, SimpleRedBlackTree)(poolNode);
      while(poolElementNodeS != NULL) {
         poolElementS = (cPoolElement*)poolElementNodeS->UserData;
         poolElementI = (cPoolElement*)poolElementNodeI->UserData;
         EV << "   - ";
         poolElementI->print(false);
         EV << "   ";
         poolElementS->print(true);
         poolElementNodeS = TMPL_CLASS(poolNodeGetNextPoolElementNodeFromSelection, SimpleRedBlackTree)(poolNode, poolElementNodeS);
         poolElementNodeI = TMPL_CLASS(poolNodeGetNextPoolElementNodeFromIndex, SimpleRedBlackTree)(poolNode, poolElementNodeI);
         EV << endl;
      }

      poolNode = TMPL_CLASS(poolHandlespaceManagementGetNextPoolNode, SimpleRedBlackTree)(&Handlespace, poolNode);
   }
}


// ###### Register pool element #############################################
unsigned int cPoolHandlespace::registerPoolElement(
                                const char*                  poolHandle,
                                const cPoolElementParameter& poolElementParameter,
                                const unsigned int           registratorAddress,
                                const unsigned int           registratorPort,
                                cPoolElement*&               poolElement,
                                bool&                        updated)
{
   struct sockaddr_testaddr address1;
   address1.ta_family = AF_TEST;
   address1.ta_addr   = poolElementParameter.getUserTransportParameter().getAddress();
   address1.ta_port   = poolElementParameter.getUserTransportParameter().getPort();

   char userTransportBuffer[transportAddressBlockGetSize(1)];
   struct TransportAddressBlock* userTransport = (struct TransportAddressBlock*)&userTransportBuffer;
   transportAddressBlockNew(userTransport,
                            IPPROTO_SCTP,
                            poolElementParameter.getUserTransportParameter().getPort(),
                            TABF_CONTROLCHANNEL,
                            (sockaddr_union*)&address1, 1, 1);

   address1.ta_family = AF_TEST;
   address1.ta_addr   = poolElementParameter.getRegistratorTransportParameter().getAddress();
   address1.ta_port   = poolElementParameter.getRegistratorTransportParameter().getPort();

   char registratorTransportBuffer[transportAddressBlockGetSize(1)];
   struct TransportAddressBlock* registratorTransport = (struct TransportAddressBlock*)&registratorTransportBuffer;
   transportAddressBlockNew(registratorTransport,
                            IPPROTO_SCTP,
                            poolElementParameter.getRegistratorTransportParameter().getPort(),
                            0,
                            (sockaddr_union*)&address1, 1, 1);

   struct PoolPolicySettings poolPolicySettings;
   poolPolicySettingsNew(&poolPolicySettings);
   poolPolicySettings.PolicyType      = poolElementParameter.getPoolPolicyParameter().getPolicyType();
   poolPolicySettings.Weight          = poolElementParameter.getPoolPolicyParameter().getWeight();
   poolPolicySettings.Load            = poolElementParameter.getPoolPolicyParameter().getLoad();
   poolPolicySettings.LoadDegradation = poolElementParameter.getPoolPolicyParameter().getLoadDegradation();
   poolPolicySettings.LoadDPF         = poolElementParameter.getPoolPolicyParameter().getLoadDPF();
   poolPolicySettings.WeightDPF       = poolElementParameter.getPoolPolicyParameter().getWeightDPF();
   poolPolicySettings.Distance        = poolElementParameter.getPoolPolicyParameter().getDistance();

   struct PoolHandle myPoolHandle;
   poolHandleNew(&myPoolHandle,
                 (const unsigned char*)poolHandle,
                 getPoolHandleSize(poolHandle));

   TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode;
   unsigned int errorCode = TMPL_CLASS(poolHandlespaceManagementRegisterPoolElement, SimpleRedBlackTree)(
                               &Handlespace,
                               &myPoolHandle,
                               poolElementParameter.getHomeRegistrarIdentifier(),
                               poolElementParameter.getIdentifier(),
                               poolElementParameter.getRegistrationLife(),
                               &poolPolicySettings,
                               userTransport,
                               registratorTransport,
                               -1, 0,
                               (unsigned long long)(1000000.0 * getSimulation()->getSimTime().dbl()),
                               &poolElementNode);
   if(errorCode == RSPERR_OKAY) {
      updated = (poolElementNode->Flags &= PENF_UPDATED);
      if(poolElementNode->UserData != NULL) {
         poolElement = (cPoolElement*)poolElementNode->UserData;
      }
      else {
         poolElement = new cPoolElement;
         OPP_CHECK(poolElement);
         poolElement->Node                               = poolElementNode;
         poolElement->EndpointKeepAliveTransmissionTimer = NULL;
         poolElement->EndpointKeepAliveTimeoutTimer      = NULL;
         poolElement->LifetimeExpiryTimer                = NULL;

         poolElementNode->UserData = (void*)poolElement;
      }
   }
   else {
      updated     = false;
      poolElement = NULL;
   }

#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(&Handlespace);
#endif
   return(errorCode);
}


// ###### Find pool element #################################################
cPoolElement* cPoolHandlespace::findPoolElement(const char*        poolHandle,
                                                const unsigned int peIdentifier)
{
   struct PoolHandle myPoolHandle;
   poolHandleNew(&myPoolHandle,
                 (const unsigned char*)poolHandle,
                 getPoolHandleSize(poolHandle));

   struct TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode =
       TMPL_CLASS(poolHandlespaceManagementFindPoolElement, SimpleRedBlackTree)(
          &Handlespace,
          &myPoolHandle,
          peIdentifier);

   if(poolElementNode) {
      return((cPoolElement*)poolElementNode->UserData);
   }
   return(NULL);
}


// ###### Deregister pool element ###########################################
unsigned int cPoolHandlespace::deregisterPoolElement(cPoolElement* poolElement)
{
   unsigned int errorCode = TMPL_CLASS(poolHandlespaceManagementDeregisterPoolElementByPtr, SimpleRedBlackTree)(
                               &Handlespace,
                               poolElement->Node);
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(&Handlespace);
#endif
   return(errorCode);
}


// ###### Deregister pool element ###########################################
unsigned int cPoolHandlespace::deregisterPoolElement(const char*        poolHandle,
                                                     const unsigned int peIdentifier)
{
   struct PoolHandle myPoolHandle;
   poolHandleNew(&myPoolHandle,
                 (const unsigned char*)poolHandle,
                 getPoolHandleSize(poolHandle));
   unsigned int errorCode = TMPL_CLASS(poolHandlespaceManagementDeregisterPoolElement, SimpleRedBlackTree)(
                               &Handlespace,
                               &myPoolHandle,
                               peIdentifier);
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(&Handlespace);
#endif
   return(errorCode);
}


// ###### Update pool element ownership #####################################
void cPoolHandlespace::updatePoolElementOwnership(cPoolElement*      poolElement,
                                                  const unsigned int registrarIdentifier)
{
   ST_CLASS(poolHandlespaceManagementUpdateOwnershipOfPoolElementNode)(
      &Handlespace, poolElement->Node,
      (RegistrarIdentifierType)registrarIdentifier);
}


// ###### Get number of pool element of certain pool ########################
size_t cPoolHandlespace::getPoolElementsOfPool(const char* poolHandle)
{
   struct PoolHandle myPoolHandle;
   poolHandleNew(&myPoolHandle,
                 (const unsigned char*)poolHandle,
                 getPoolHandleSize(poolHandle));

   return(TMPL_CLASS(poolHandlespaceManagementGetPoolElementsOfPool, SimpleRedBlackTree)(
             &Handlespace,
             &myPoolHandle));
}


// ###### Select pool elements by policy ####################################
size_t cPoolHandlespace::selectPoolElementsByPolicy(const char*    poolHandle,
                                                    cPoolElement** selectionArray,
                                                    size_t&        items,
                                                    const size_t   maxHandleResolutionItems,
                                                    const size_t   maxIncrement)
{
   struct TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* array[maxHandleResolutionItems];

   struct PoolHandle myPoolHandle;
   poolHandleNew(&myPoolHandle,
                 (const unsigned char*)poolHandle,
                 getPoolHandleSize(poolHandle));

   TMPL_CLASS(poolHandlespaceManagementHandleResolution, SimpleRedBlackTree)(
      &Handlespace,
      &myPoolHandle,
      (struct TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)**)&array,
      &items,
      maxHandleResolutionItems, maxIncrement);
   for(size_t i = 0;i < items;i++) {
      selectionArray[i] = (cPoolElement*)array[i]->UserData;
   }

#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(&Handlespace);
#endif
   return(items);
}


// ###### Restart pool element expiry timer #################################
void cPoolHandlespace::restartPoolElementExpiryTimer(
        cPoolElement*            poolElement,
        const unsigned long long expiryTimeout)
{
   /* poolHandlespaceManagementRestartPoolElementExpiryTimer() only takes expiry
      timeout as parameter. Absolute expiry time is time stamp of last update +
      given expiry timeout! */
   TMPL_CLASS(poolHandlespaceManagementRestartPoolElementExpiryTimer, SimpleRedBlackTree)(
      &Handlespace, poolElement->Node, expiryTimeout);
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(&Handlespace);
#endif
}


// ###### Purge expired pool elements from handlespace ######################
size_t cPoolHandlespace::purgeExpiredPoolElements()
{
   const size_t purged = TMPL_CLASS(poolHandlespaceManagementPurgeExpiredPoolElements, SimpleRedBlackTree)(
                            &Handlespace,
                            (unsigned long long)(1000000.0 * getSimulation()->getSimTime().dbl()));
   // TMPL_CLASS(poolHandlespaceManagementPrint, SimpleRedBlackTree)(&Handlespace,stdout,~0);
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(&Handlespace);
#endif
   return(purged);
}


// ###### Export PE list ####################################################
cArray* cPoolHandlespace::exportToPoolEntries(const unsigned int homeRegistrarIdentifier)
{
   ST_CLASS(HandleTableExtract)                     hte;
   TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode;
   cPoolElement*                                    poolElement;
   cArray*                                          poolEntryArray;
   int                                              hasData;
   size_t                                           total;

   poolEntryArray = new cArray("PoolEntryArray");
   OPP_CHECK(poolEntryArray);
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(&Handlespace);
#endif

   total   = 0;
   hasData = ST_CLASS(poolHandlespaceManagementGetHandleTable)(&Handlespace,
                                                               homeRegistrarIdentifier,
                                                               &hte,
                                                               HTEF_START | ((homeRegistrarIdentifier != UNDEFINED_REGISTRAR_IDENTIFIER) ? HTEF_OWNCHILDSONLY : 0),
                                                               NTE_MAX_POOL_ELEMENT_NODES);
   while(hasData) {
      for(size_t i = 0;i < hte.PoolElementNodes;i++) {
         cPoolEntry* poolEntry = new cPoolEntry;
         OPP_CHECK(poolEntry);
         poolEntry->setPoolHandle((const char*)&hte.PoolElementNodeArray[i]->OwnerPoolNode->Handle.Handle);
         poolElementNode = hte.PoolElementNodeArray[i];
         poolElement = (cPoolElement*)poolElementNode->UserData;
         poolEntry->setPoolElementParameter(poolElement->toPoolElementParameter());
         poolEntryArray->add(poolEntry);
      }
      total += hte.PoolElementNodes;

      if(hte.PoolElementNodes < NTE_MAX_POOL_ELEMENT_NODES) {
         break;
      }
      hasData = ST_CLASS(poolHandlespaceManagementGetHandleTable)(&Handlespace,
                                                                  homeRegistrarIdentifier,
                                                                  &hte,
                                                                  (homeRegistrarIdentifier != UNDEFINED_REGISTRAR_IDENTIFIER) ? HTEF_OWNCHILDSONLY : 0,
                                                                  NTE_MAX_POOL_ELEMENT_NODES);
   }

   if(total == 0) {
      delete poolEntryArray;
      return(NULL);
   }
   return(poolEntryArray);
}


// ###### Get first pool element of given owner #############################
cPoolElement* cPoolHandlespace::getFirstPoolElementOwnedBy(const unsigned int homeRegistrarIdentifier)
{
   TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode =
      ST_CLASS(poolHandlespaceManagementGetFirstPoolElementOwnershipNodeForIdentifier)(&Handlespace, (RegistrarIdentifierType)homeRegistrarIdentifier);
   if(poolElementNode) {
      return((cPoolElement*)poolElementNode->UserData);
   }
   return(NULL);
}


// ###### Get next pool element of same owner ###############################
cPoolElement* cPoolHandlespace::getNextPoolElementOfSameOwner(struct cPoolElement* poolElement)
{
   TMPL_CLASS(PoolElementNode, SimpleRedBlackTree)* poolElementNode =
      ST_CLASS(poolHandlespaceManagementGetNextPoolElementOwnershipNodeForSameIdentifier)(&Handlespace, poolElement->Node);
   if(poolElementNode) {
      return((cPoolElement*)poolElementNode->UserData);
   }
   return(NULL);
}



// ##########################################################################
// #### PoolUser List                                                    ####
// ##########################################################################


// ###### Constructor #######################################################
cPoolUserList::cPoolUserList()
{
   TMPL_CLASS(poolUserListNew, SimpleRedBlackTree)(&List);
   NewPoolUserNode = NULL;
}


// ###### Destructor ########################################################
cPoolUserList::~cPoolUserList()
{
   clear();
   TMPL_CLASS(poolUserListDelete, SimpleRedBlackTree)(&List);
   if(NewPoolUserNode) {
      free(NewPoolUserNode);
      NewPoolUserNode = NULL;
   }
}


// ###### Clear #############################################################
void cPoolUserList::clear()
{
   TMPL_CLASS(poolUserListClear, SimpleRedBlackTree)(&List);
}


// ###### Print #############################################################
void cPoolUserList::print()
{
   char                                          description[256];
   TMPL_CLASS(PoolUserNode, SimpleRedBlackTree)* poolUserNode;

   TMPL_CLASS(poolUserListGetDescription, SimpleRedBlackTree)(
      &List, (char*)&description, sizeof(description));
   EV << description << endl;

   poolUserNode = TMPL_CLASS(poolUserListGetFirstPoolUserNode, SimpleRedBlackTree)(&List);
   while(poolUserNode != NULL) {
      EV << "+--- ";
      TMPL_CLASS(poolUserNodeGetDescription, SimpleRedBlackTree)(
         poolUserNode,
         (char*)&description, sizeof(description), ~0);
      EV << description << endl;
      poolUserNode = TMPL_CLASS(poolUserListGetNextPoolUserNode, SimpleRedBlackTree)(&List, poolUserNode);
   }
}


// ###### Register pool user ################################################
TMPL_CLASS(PoolUserNode, SimpleRedBlackTree)* cPoolUserList::registerPoolUser(
   const unsigned int address,
   const unsigned int port)
{
   if(NewPoolUserNode == NULL) {
      NewPoolUserNode = (struct ST_CLASS(PoolUserNode)*)malloc(sizeof(struct ST_CLASS(PoolUserNode)));
   }
   CHECK(NewPoolUserNode != NULL);
   TMPL_CLASS(poolUserNodeNew, SimpleRedBlackTree)(NewPoolUserNode,
      (int)address, (sctp_assoc_t)port);
   struct ST_CLASS(PoolUserNode)* addedPoolUserNode =
      TMPL_CLASS(poolUserListAddPoolUserNode, SimpleRedBlackTree)(
         &List, NewPoolUserNode);
   if(addedPoolUserNode == NewPoolUserNode) {
      NewPoolUserNode = NULL;
   }
   addedPoolUserNode->LastUpdateTimeStamp = (unsigned long long)(1000000.0 * getSimulation()->getSimTime().dbl());
   return(addedPoolUserNode);
}


// ###### Purge pool users ##################################################
void cPoolUserList::purge(const simtime_t minTime)
{
   const unsigned long long minTimeStamp = (unsigned long long)(1000000.0 * minTime.dbl());

   TMPL_CLASS(PoolUserNode, SimpleRedBlackTree)* poolUserNode =
      TMPL_CLASS(poolUserListGetFirstPoolUserNode, SimpleRedBlackTree)(&List);
   while(poolUserNode != NULL) {
      TMPL_CLASS(PoolUserNode, SimpleRedBlackTree)* nextPoolUserNode = TMPL_CLASS(poolUserListGetNextPoolUserNode, SimpleRedBlackTree)(&List, poolUserNode);
      if(poolUserNode->LastUpdateTimeStamp < minTimeStamp) {
         char description[256];
         TMPL_CLASS(poolUserNodeGetDescription, SimpleRedBlackTree)(
            poolUserNode,
            (char*)&description, sizeof(description), ~0);
            EV << "Purging " << description << endl;

         TMPL_CLASS(poolUserListRemovePoolUserNode, SimpleRedBlackTree)(
            &List, poolUserNode);
         free(poolUserNode);
      }
      poolUserNode = nextPoolUserNode;
   }
}


// ###### Note a handle resolution ##########################################
double cPoolUserList::noteHandleResolutionOfPoolUser(const char*        poolHandle,
                                                     const unsigned int address,
                                                     const unsigned int port,
                                                     const size_t       buckets,
                                                     const size_t       maxEntries)
{
   struct PoolHandle                                    poolHandleStruct;
   struct TMPL_CLASS(PoolUserNode, SimpleRedBlackTree)* poolUserNode =
      registerPoolUser(address, port);

   CHECK(poolUserNode != NULL);
   poolHandleNew(&poolHandleStruct, (const unsigned char*)poolHandle, strlen(poolHandle));
   const double rate = TMPL_CLASS(poolUserNodeNoteHandleResolution, SimpleRedBlackTree)(
      poolUserNode,
      &poolHandleStruct,
      (unsigned long long)(1000000.0 * getSimulation()->getSimTime().dbl()),
      buckets, maxEntries);
   return(rate);
}


// ###### Note a handle resolution ##########################################
double cPoolUserList::noteEndpointUnreachableOfPoolUser(const char*        poolHandle,
                                                        const unsigned int address,
                                                        const unsigned int port,
                                                        const unsigned int peIdentifier,
                                                        const size_t       buckets,
                                                        const size_t       maxEntries)
{
   struct PoolHandle                                    poolHandleStruct;
   struct TMPL_CLASS(PoolUserNode, SimpleRedBlackTree)* poolUserNode =
      registerPoolUser(address, port);

   CHECK(poolUserNode != NULL);
   poolHandleNew(&poolHandleStruct, (const unsigned char*)poolHandle, strlen(poolHandle));
   const double rate = TMPL_CLASS(poolUserNodeNoteEndpointUnreachable, SimpleRedBlackTree)(
      poolUserNode,
      &poolHandleStruct,
      peIdentifier,
      (unsigned long long)(1000000.0 * getSimulation()->getSimTime().dbl()),
      buckets, maxEntries);
   return(rate);
}



// ##########################################################################
// #### Peer List Node                                                   ####
// ##########################################################################

// ###### Print #############################################################
void cPeerListNode::print(const bool full)
{
   char peerListNodeDescription[1024];

   TMPL_CLASS(peerListNodeGetDescription, SimpleRedBlackTree)(
      Node,
      (char*)&peerListNodeDescription, sizeof(peerListNodeDescription),
      ((full == true) ? PLPO_FULL : PLPO_ONLY_INDEX));
   EV << peerListNodeDescription;
}


// ###### Export to ServerInformationParameter ##############################
ServerInformationParameter cPeerListNode::toServerInformationParameter() const
{
   ServerInformationParameter serverInformationParameter;

   serverInformationParameter.setServerID(getIdentifier());
   serverInformationParameter.setAddress(getAddress());
   serverInformationParameter.setPort(getPort());

   return(serverInformationParameter);
}



// ##########################################################################
// #### Peer List                                                        ####
// ##########################################################################


// ###### Constructor #######################################################
cPeerList::cPeerList(cPoolHandlespace*  handlespace,
                     const unsigned int registrarIdentifier)
{
   Handlespace = handlespace;
   TMPL_CLASS(peerListManagementNew, SimpleRedBlackTree)(
      &List, &Handlespace->Handlespace, registrarIdentifier,
      killPeerListNode, NULL);
}


// ###### Destructor ########################################################
cPeerList::~cPeerList()
{
   clear();
   TMPL_CLASS(peerListManagementDelete, SimpleRedBlackTree)(&List);
}


// ###### PoolElement disposal callback for handlespace cleanup ##############
void cPeerList::killPeerListNode(TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode,
                                 void*                                         userData)
{
   cPeerListNode* node = (cPeerListNode*)peerListNode->UserData;
   delete node;
}


// ###### Clear #############################################################
void cPeerList::clear()
{
   TMPL_CLASS(peerListManagementClear, SimpleRedBlackTree)(&List);
}


// ###### Print #############################################################
void cPeerList::print()
{
   char                                          description[1024];
   TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode;

   TMPL_CLASS(peerListManagementGetDescription, SimpleRedBlackTree)(
      &List, (char*)&description, sizeof(description));
   EV << description << endl;

   peerListNode = TMPL_CLASS(peerListManagementGetFirstPeerListNodeFromIndexStorage, SimpleRedBlackTree)(&List);
   while(peerListNode != NULL) {
      EV << "+--- ";
      TMPL_CLASS(peerListNodeGetDescription, SimpleRedBlackTree)(
         peerListNode,
         (char*)&description, sizeof(description),
         PLPO_FULL);
      EV << description << endl;
      peerListNode = TMPL_CLASS(peerListManagementGetNextPeerListNodeFromIndexStorage, SimpleRedBlackTree)(&List, peerListNode);
   }
}


// ###### Register peer #####################################################
unsigned int cPeerList::registerPeerListNode(const ServerInformationParameter& serverInformationParameter,
                                             cPeerListNode*&                   node)
{
   struct sockaddr_testaddr address1;
   address1.ta_family = AF_TEST;
   address1.ta_addr   = serverInformationParameter.getAddress();
   address1.ta_port   = serverInformationParameter.getPort();

   char registrarTransportBuffer[transportAddressBlockGetSize(1)];
   struct TransportAddressBlock* registrarTransport = (struct TransportAddressBlock*)&registrarTransportBuffer;
   transportAddressBlockNew(registrarTransport,
                            IPPROTO_SCTP,
                            serverInformationParameter.getPort(),
                            0,
                            (sockaddr_union*)&address1, 1, 1);

   TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode;
   unsigned int errorCode = TMPL_CLASS(peerListManagementRegisterPeerListNode, SimpleRedBlackTree)(
                               &List,
                               serverInformationParameter.getServerID(),
                               (serverInformationParameter.getServerID() == UNDEFINED_REGISTRAR_IDENTIFIER) ? 0 : PLNF_DYNAMIC,
                               registrarTransport,
                               (unsigned long long)(1000000.0 * getSimulation()->getSimTime().dbl()),
                               &peerListNode);
   if(errorCode == RSPERR_OKAY) {
      if(peerListNode->UserData != NULL) {
         node = (cPeerListNode*)peerListNode->UserData;
      }
      else {
         node = new cPeerListNode;
         OPP_CHECK(node);
         node->Node                  = peerListNode;
         node->Takeover              = NULL;
         node->MentorTrials          = 0;
         node->LastHeardTimeoutTimer = NULL;
         node->ResponseTimeoutTimer  = NULL;
         node->TakeoverExpiryTimer   = NULL;
         peerListNode->UserData = (void*)node;
      }
   }
   else {
      node = NULL;
   }
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(List.Handlespace);
   TMPL_CLASS(peerListManagementVerify, SimpleRedBlackTree)(&List);
#endif
   return(errorCode);
}


// ###### Deregister peer by pointer ########################################
unsigned int cPeerList::deregisterPeerListNode(cPeerListNode* peerListNode)
{
   unsigned int errorCode = TMPL_CLASS(peerListManagementDeregisterPeerListNodeByPtr, SimpleRedBlackTree)(
                               &List,
                               peerListNode->Node);
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(List.Handlespace);
   TMPL_CLASS(peerListManagementVerify, SimpleRedBlackTree)(&List);
#endif
   return(errorCode);
}


// ###### Deregister peer by ID #############################################
unsigned int cPeerList::deregisterPeerListNode(unsigned int identifier)
{
   unsigned int errorCode = TMPL_CLASS(peerListManagementDeregisterPeerListNode, SimpleRedBlackTree)(
                               &List,
                               identifier, NULL);
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(List.Handlespace);
   TMPL_CLASS(peerListManagementVerify, SimpleRedBlackTree)(&List);
#endif
   return(errorCode);
}


// ###### Find entry by ID ##################################################
cPeerListNode* cPeerList::findPeerListNode(const unsigned int identifier)
{
   OPP_CHECK(identifier != UNDEFINED_REGISTRAR_IDENTIFIER);
   TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode =
      TMPL_CLASS(peerListManagementFindPeerListNode, SimpleRedBlackTree)(
         &List, identifier, NULL);
   if(peerListNode) {
      return((cPeerListNode*)peerListNode->UserData);
   }
   return(NULL);
}


// ###### Find entry by address/port ########################################
cPeerListNode* cPeerList::findPeerListNode(const unsigned int address,
                                           const unsigned int port)
{
   struct sockaddr_testaddr address1;
   address1.ta_family = AF_TEST;
   address1.ta_addr   = address;
   address1.ta_port   = port;
   char registrarTransportBuffer[transportAddressBlockGetSize(1)];
   struct TransportAddressBlock* registrarTransport = (struct TransportAddressBlock*)&registrarTransportBuffer;
   transportAddressBlockNew(registrarTransport,
                            IPPROTO_SCTP,
                            port,
                            0,
                            (sockaddr_union*)&address1, 1, 1);

   TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode =
      TMPL_CLASS(peerListManagementFindPeerListNode, SimpleRedBlackTree)(
         &List, 0, registrarTransport);
   if(peerListNode) {
      return((cPeerListNode*)peerListNode->UserData);
   }
   return(NULL);
}


// ###### Get random peer list node #########################################
cPeerListNode* cPeerList::getRandomPeerListNode()
{
   TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode =
      TMPL_CLASS(peerListManagementGetRandomPeerListNode, SimpleRedBlackTree)(&List);
   if(peerListNode) {
      return((cPeerListNode*)peerListNode->UserData);
   }
   return(NULL);
}


// ###### Purge #############################################################
void cPeerList::purge()
{
   const unsigned long long now = (unsigned long long)(1000000.0 * getSimulation()->getSimTime().dbl());

   TMPL_CLASS(peerListManagementPurgeExpiredPeerListNodes, SimpleRedBlackTree)(
      &List, now);
#ifdef VERIFY
   TMPL_CLASS(poolHandlespaceManagementVerify, SimpleRedBlackTree)(List.Handlespace);
   TMPL_CLASS(peerListManagementVerify, SimpleRedBlackTree)(&List);
#endif
}


// ###### Reset mentor selection information ################################
void cPeerList::resetMentorSelection()
{
   TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode;

   peerListNode = TMPL_CLASS(peerListManagementGetFirstPeerListNodeFromIndexStorage, SimpleRedBlackTree)(&List);
   while(peerListNode != NULL) {
      cPeerListNode* node = (cPeerListNode*)peerListNode->UserData;
      node->MentorTrials = 0;
      peerListNode = TMPL_CLASS(peerListManagementGetNextPeerListNodeFromIndexStorage, SimpleRedBlackTree)(&List, peerListNode);
   }
}


// ###### Get mentor server #################################################
cPeerListNode* cPeerList::findMentorServer(const unsigned int localAddress,
                                           const unsigned int localPort,
                                           const unsigned int maxTrials)
{
   TMPL_CLASS(PeerListNode, SimpleRedBlackTree)* peerListNode;

   peerListNode = TMPL_CLASS(peerListManagementGetFirstPeerListNodeFromIndexStorage, SimpleRedBlackTree)(&List);
   while(peerListNode != NULL) {
      cPeerListNode* node = (cPeerListNode*)peerListNode->UserData;

      if(((node->getAddress() != localAddress) || (node->getPort() != localPort)) &&
         (node->MentorTrials < maxTrials)) {
         node->MentorTrials++;
         return(node);
      }

      peerListNode = TMPL_CLASS(peerListManagementGetNextPeerListNodeFromIndexStorage, SimpleRedBlackTree)(&List, peerListNode);
   }
   return(NULL);
}


/* ###### Get better peer for PE ######################################### */
cPeerListNode* cPeerList::getUsefulPeerForPE(const unsigned int identifier)
{
   ST_CLASS(PeerListNode)* peerListNode = ST_CLASS(peerListManagementGetUsefulPeerForPE)(&List, identifier);
   if(peerListNode) {
      cPeerListNode* node = (cPeerListNode*)peerListNode->UserData;
      return(node);
   }
   return(NULL);
}



// ##########################################################################
// #### Takeover Process                                                 ####
// ##########################################################################

// ###### Constructor #######################################################
cTakeoverProcess::cTakeoverProcess(const unsigned int targetID,
                                   cPeerList*         peerList)
{
   Takeover = takeoverProcessNew((RegistrarIdentifierType)targetID, &peerList->List);
   OPP_CHECK(Takeover);
}


// ###### Destructor ########################################################
cTakeoverProcess::~cTakeoverProcess()
{
   takeoverProcessDelete(Takeover);
   Takeover = NULL;
}


// ###### Acknowledge takeover ##############################################
size_t cTakeoverProcess::acknowledge(const unsigned int targetID,
                                     const unsigned int acknowledgerID)
{
   return(takeoverProcessAcknowledge(Takeover,
                                     (RegistrarIdentifierType)targetID,
                                     (RegistrarIdentifierType)acknowledgerID));
}
