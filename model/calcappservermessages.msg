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
// Copyright (C) 2003-2025 by Thomas Dreibholz
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


class CalcAppServerJob extends cOwnedObject
{
    @customize(true);
    unsigned int JobID;
    simtime_t    JobStartedAt;
    simtime_t    JobCompletedAt;
    double       JobCalculations;
    double       JobCompletedCalculations;

    unsigned int ClientAddress;
    unsigned int ClientPort;

    simtime_t    LastAccountingAt;
    simtime_t    LastCookieAt;
    double       LastCookieCompletedCalculations;
}


message JobCompleteMessage
{
}

message JobKeepAliveTransmissionMessage
{
}

message JobKeepAliveTimeoutMessage
{
}

message CookieTransmissionMessage
{
}
