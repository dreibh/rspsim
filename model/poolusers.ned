// $Id$
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
// Copyright (C) 2003-2012 by Thomas Dreibholz
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
import pooluserasapprocess.ned;


simple CalcAppQueuingClientProcess
{
    parameters:
        // ------ Component Parameters --------------------------------------
        double componentStartupDelay @unit(s);

        // ------ Service Parameters ----------------------------------------
        string servicePoolHandle;
        double serviceJobCount;
        volatile double serviceJobSize;
        volatile double serviceJobInterval;
        double serviceJobRequestTimeout @unit(s);
        volatile double serviceJobKeepAliveInterval @unit(s);
        double serviceJobKeepAliveTimeout @unit(s);
        volatile double serviceHandleResolutionRetryDelay @unit(s);
        volatile double serviceJobRetryDelay @unit(s);

    gates:
        output toASAP;
        output toTransport;
        input fromASAP;
        input fromTransport;
}


module CalcAppPoolUser
{
    parameters:
        // ------ Component Parameters --------------------------------------
        double componentStartupDelay @unit(s);

        @display("bgb=338,230");
    gates:
        output toNetwork;
        input fromNetwork;

    submodules:
        transportNode: TransportNode {
            parameters:
                interfaceAddress;
                @display("i=block/dispatch;p=194,177");
            gates:
                toApplication[3];
                fromApplication[3];
                fromNetwork[1];
                toNetwork[1];
        }
        poolUserASAP: PoolUserASAPProcess {
            parameters:
                asapRequestTimeout = default(5s);
                asapMaxRequestRetransmit = default(3);
                asapStaleCacheValue = default(0s);
                asapServerHuntRetryDelay = default(100ms);
                @display("i=block/cogwheel;p=84,53");
        }
        registrarTable: RegistrarTableProcess {
            parameters:
                staticRegistrarsList = default("");
                @display("p=84,131;i=block/table2");
        }
        calcAppQueuingClient: CalcAppQueuingClientProcess {
            parameters:
                componentStartupDelay = componentStartupDelay;

                serviceJobCount = default(1e9);
                serviceJobSize = default(10000000);
                serviceJobInterval = default(60.0);

                servicePoolHandle = default("CalcAppPool");
                serviceJobRequestTimeout = default(10s);
                serviceJobKeepAliveInterval = default(10s);
                serviceJobKeepAliveTimeout = default(5s);
                serviceJobRetryDelay = default(uniform(0ms, 200ms));
                serviceHandleResolutionRetryDelay = default(30s);
                @display("p=252,53;i=block/app2");
        }
    connections:
        fromNetwork --> transportNode.fromNetwork[0];
        transportNode.toNetwork[0] --> toNetwork;

        transportNode.toApplication[0] --> poolUserASAP.fromTransport;
        poolUserASAP.toTransport --> transportNode.fromApplication[0];

        transportNode.toApplication[1] --> calcAppQueuingClient.fromTransport;
        calcAppQueuingClient.toTransport --> transportNode.fromApplication[1];

        registrarTable.toUser --> poolUserASAP.fromRegistrarTable;
        poolUserASAP.toRegistrarTable --> registrarTable.fromUser;

        transportNode.toApplication[2] --> registrarTable.fromTransport;
        registrarTable.toTransport --> transportNode.fromApplication[2];

        poolUserASAP.toApplication --> calcAppQueuingClient.fromASAP;
        calcAppQueuingClient.toASAP --> poolUserASAP.fromApplication;
}
