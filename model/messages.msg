// --------------------------------------------------------------------------
//
//              //===//   //=====   //===//   //=====  //   //      //
//             //    //  //        //    //  //       //   //=/  /=//
//            //===//   //=====   //===//   //====   //   //  //  //
//           //   \\         //  //             //  //   //  //  //
//          //     \\  =====//  //        =====//  //   //      //  Version V
//
// ------------- An Open Source RSerPool Simulation for OMNeT++ -------------
//
// Copyright (C) 2003-2023 by Thomas Dreibholz
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Contact: thomas.dreibholz@gmail.com


// ###########################################################################
// #### Simple Transport                                                  ####
// ###########################################################################

message SimplePacket
{
    unsigned int DstAddress;
    unsigned int DstPort = 0;
    unsigned int SrcAddress = 0;
    unsigned int SrcPort = 0;
    unsigned int Protocol = 0;
    unsigned int HopCount = 32;
}


enum ProtocolTypes
{

    Ping = 1;
    Pong = 2;
    CalcAppProtocol = 3;
    ASAP = 4;
    ENRP = 5;
}


enum WellKnownPortNumbers
{
    RegistrarAnnouncePort = 50;
    RegistrarPort = 51;

    PoolElementASAPPortStart = 100;
    PoolElementASAPPortEnd = 199;

    PoolUserASAPPort = 1111;

    CalcAppClientPort = 1800;

    AttackerPort = 777;
}


message BindMessage
{
    unsigned int Port;
}


message UnbindMessage
{
    unsigned int Port;
}


class cPortMapping extends cObject
{
    unsigned int Port;
    int Gate;
}



// ###########################################################################
// #### ASAP - Aggregate Server Access Protocol                           ####
// ###########################################################################


class cPoolPolicyParameter extends cObject
{
    unsigned int PolicyType = 0;
    unsigned int MaxIncrement = 1000000000;
    unsigned int Weight = 0;
    unsigned int Load = 0;
    unsigned int LoadDegradation = 0;
    unsigned int LoadDPF = 0;
    unsigned int WeightDPF = 0;
    unsigned int Distance = 0;
}

class cTransportParameter extends cObject
{
    unsigned int Address = 0;
    unsigned int Port = 0;
}


class cPoolElementParameter extends cObject
{
    unsigned int Identifier = 0;
    unsigned int HomeRegistrarIdentifier = 0;
    unsigned int RegistrationLife = 300000;
    cTransportParameter UserTransportParameter;
    cTransportParameter RegistratorTransportParameter;
    cPoolPolicyParameter PoolPolicyParameter;
}


message ASAPPacket extends SimplePacket
{
}

message ASAPRegistration extends ASAPPacket
{
    string PoolHandle;
    cPoolElementParameter PoolElementParameter;
}

message ASAPRegistrationResponse extends ASAPPacket
{
    string PoolHandle;
    unsigned int Identifier;
    bool RejectFlag = false;
    unsigned int Error = 0;
}

message ASAPDeregistration extends ASAPPacket
{
    string PoolHandle;
    unsigned int Identifier;
}

message ASAPDeregistrationResponse extends ASAPPacket
{
    string PoolHandle;
    unsigned int Identifier;
    unsigned int Error = 0;
}

message ASAPEndpointKeepAlive extends ASAPPacket
{
    string PoolHandle;
    unsigned int Identifier;
    bool HomeFlag = false;
}

message ASAPEndpointKeepAliveAck extends ASAPPacket
{
    string PoolHandle;
    unsigned int Identifier;
}

message ASAPEndpointUnreachable extends ASAPPacket
{
    string PoolHandle;
    unsigned int Identifier;
}

message ASAPHandleResolution extends ASAPPacket
{
    string PoolHandle;
}

message ASAPHandleResolutionResponse extends ASAPPacket
{
    string PoolHandle;
    cPoolPolicyParameter OverallPoolElementSelectionPolicy;
    cPoolElementParameter PoolElementParameter[];
    bool RejectFlag = false;
}

message ASAPCookie extends ASAPPacket
{
}

message ASAPCookieEcho extends ASAPPacket
{
}



// ###########################################################################
// #### ENRP - Endpoint haNdlespace Redundancy Protocol                   ####
// ###########################################################################

class ServerInformationParameter extends cObject
{
    unsigned int ServerID;

    unsigned int Address;
    unsigned int Port;
}

class cPoolEntry extends cObject
{
    string PoolHandle;
    cPoolElementParameter PoolElementParameter;
}


message ENRPPacket extends SimplePacket
{
    unsigned int SenderServerID;
    unsigned int ReceiverServerID;
}

message ENRPPresence extends ENRPPacket
{
    bool ReplyRequiredFlag;
    unsigned int Checksum;
    ServerInformationParameter ServerInformation;
}

message ENRPHandleTableRequest extends ENRPPacket
{
    bool OwnChildrenOnlyFlag;
}

message ENRPHandleTableResponse extends ENRPPacket
{
    cPoolEntry PoolEntry[];
    bool RejectFlag;
    bool MoreToSendFlag;
}

enum UpdateActions
{

    ADD_PE = 0x00;
    DEL_PE = 0x01;
}

message ENRPHandleUpdate extends ENRPPacket
{
    unsigned int UpdateAction;
    string PoolHandle;
    cPoolElementParameter PoolElementParameter;
    bool TakeoverSuggested;
}

message ENRPListRequest extends ENRPPacket
{
}

message ENRPListResponse extends ENRPPacket
{
    bool RejectFlag;
    ServerInformationParameter ServerInformation[];
}

message ENRPInitTakeover extends ENRPPacket
{
    unsigned int TargetServerID;
}

message ENRPInitTakeoverAck extends ENRPPacket
{
    unsigned int TargetServerID;
}

message ENRPTakeoverServer extends ENRPPacket
{
    unsigned int TargetServerID;
}

message ENRPError extends ENRPPacket
{
    unsigned int Error;
}



// ###########################################################################
// #### CalcApp Protocol                                                  ####
// ###########################################################################

message CalcAppMessage extends SimplePacket
{
    unsigned int JobID;
}

message CalcAppRequest extends CalcAppMessage
{
    double JobCalculations;
}

message CalcAppAccept extends CalcAppMessage
{
    double JobCalculations;
}

message CalcAppReject extends CalcAppMessage
{
}

message CalcAppKeepAlive extends CalcAppMessage
{
}

message CalcAppKeepAliveAck extends CalcAppMessage
{
}

message CalcAppComplete extends CalcAppMessage
{
}

message CalcAppAbort extends CalcAppMessage
{
}

class CalcAppCookieParameter extends cObject
{
    unsigned int JobID;
    double JobCalculations;
    double JobCompletedCalculations;
}

message CalcAppCookie extends ASAPCookie
{
    CalcAppCookieParameter Cookie;
}

message CalcAppCookieEcho extends ASAPCookieEcho
{
    CalcAppCookieParameter Cookie;
}



// ###########################################################################
// #### Registrar Table Module                                            ####
// ###########################################################################

message RegistrarHuntRequest
{
}

message RegistrarHuntResponse
{
    unsigned int RegistrarAddress;
}



// ###########################################################################
// #### Pool Element ASAP Module                                          ####
// ###########################################################################

message RegisterPoolElement
{
    string PoolHandle;
    cPoolElementParameter PoolElementParameter;
}

message RegisterPoolElementAck
{
}

message ResetPoolElement
{
}

message ResetPoolElementAck
{
}

message DeregisterPoolElement
{
}

message DeregisterPoolElementAck
{
}

message PolicyUpdate
{
    cPoolPolicyParameter PoolPolicyParameter;
    bool ReregisterImmediately = true;
}



// ###########################################################################
// #### Pool User ASAP Module                                             ####
// ###########################################################################

message ServerSelectionRequest
{
    string PoolHandle;
}

message ServerSelectionSuccess
{
    string PoolHandle;
    cPoolElementParameter PoolElementParameter;
}

message ServerSelectionFailure
{
}

message EndpointUnreachable
{
    string PoolHandle;
    unsigned int Identifier;
}

message CachePurge
{
    string PoolHandle;
    unsigned int Identifier;
}



// ###########################################################################
// #### Peer List Classes                                                 ####
// ###########################################################################

class cPeerListEntry extends ServerInformationParameter
{
    @customize(true);
    double LastHeared = 0.0;
    unsigned int MentorTrials = 0;
}

class cPeerList extends cObject
{
    @customize(true);
    unsigned int Peers;
}
