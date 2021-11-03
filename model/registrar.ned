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
// Copyright (C) 2003-2022 by Thomas Dreibholz
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
// Contact: dreibh@iem.uni-due.de


import transportnode.ned;


simple RegistrarProcess
{
    parameters:
        // ------ Component Parameters --------------------------------------
        double          componentStartupDelay @unit(s);
        int             componentRuns;
        string          componentStatusChanges;
        volatile double componentUptime @unit(s);
        volatile double componentDowntime @unit(s);
        // ------ Registrar Parameters --------------------------------------
        int             registrarIdentifier;
        double          registrarMentorDiscoveryTimeout @unit(s);
        int             registrarMaxBadPEReports;
        volatile int    registrarMaxHandleResolutionItems;
        bool            registrarRandomizeMaxHandleResolutionItems;
        volatile int    registrarMaxIncrement;
        volatile double registrarUpdateLossProbability;
        double          registrarMaxEndpointUnreachableRate;
        double          registrarEndpointUnreachableRateBuckets;
        double          registrarEndpointUnreachableRateMaxEntries;
        double          registrarMaxHandleResolutionRate;
        double          registrarHandleResolutionRateBuckets;
        double          registrarHandleResolutionRateMaxEntries;
        // ------ ENRP Parameters -------------------------------------------
        volatile double enrpPeerHeartbeatCycle @unit(s);
        double          enrpMaxTimeLastHeared @unit(s);
        double          enrpMaxTimeNoResponse @unit(s);
        double          enrpTakeoverExpiry @unit(s);
        string          enrpStaticPeersList;
        // ------ ASAP Parameters -------------------------------------------
        volatile double asapEndpointKeepAliveInterval @unit(s);
        volatile double asapEndpointKeepAliveTimeout @unit(s);
        bool            asapNoServiceDuringStartup;
        bool            asapUseTakeoverSuggestion;
    gates:
        output toTransport;
        input fromTransport;
}


module Registrar
{
    parameters:
        // ------ Component Parameters --------------------------------------
        double componentStartupDelay @unit(s);

    gates:
        output toNetwork;
        input fromNetwork;
    submodules:
        transportNode: TransportNode {
            parameters:
                interfaceAddress;
                interfaceUptime = default(1000001d);
                interfaceDowntime = default(0s);
                @display("p=162,184;i=block/plug");
            gates:
                fromNetwork[1];
                toNetwork[1];
                toApplication[1];
                fromApplication[1];
        }
        registrarProcess: RegistrarProcess {
            parameters:
                componentStartupDelay = parent.componentStartupDelay;
                componentRuns = default(1);
                componentStatusChanges = default("");
                componentUptime = default(1000001d);
                componentDowntime = default(0s);

                registrarIdentifier = default(uniform(1, 4294967295.0));
                registrarMaxBadPEReports = default(3);
                registrarMaxHandleResolutionItems = default(5);
                registrarRandomizeMaxHandleResolutionItems = default(false);
                registrarMaxIncrement = default(1000000000);
                registrarUpdateLossProbability = default(0.0);

                registrarMaxEndpointUnreachableRate = default(-1.0);
                registrarEndpointUnreachableRateBuckets = default(64);
                registrarEndpointUnreachableRateMaxEntries = default(16);
                registrarMaxHandleResolutionRate = default(-1.0);
                registrarHandleResolutionRateBuckets = default(64);
                registrarHandleResolutionRateMaxEntries = default(16);

                enrpStaticPeersList = default("");
                enrpPeerHeartbeatCycle = default(5s);
                enrpMaxTimeLastHeared = default(61s);
                enrpMaxTimeNoResponse = default(5s);
                enrpTakeoverExpiry = default(30s);

                asapEndpointKeepAliveInterval = default(5s);
                asapEndpointKeepAliveTimeout = default(5s);
                asapNoServiceDuringStartup = default(false);
                asapUseTakeoverSuggestion = default(false);
                @display("p=162,81;i=misc/globe");
        }
    connections:
        fromNetwork --> transportNode.fromNetwork[0];
        transportNode.toNetwork[0] --> toNetwork;

        transportNode.toApplication[0] --> registrarProcess.fromTransport;
        registrarProcess.toTransport --> transportNode.fromApplication[0];
}
