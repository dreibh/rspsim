# ###########################################################################
#             Thomas Dreibholz's R Simulation Scripts Collection
#                  Copyright (C) 2005-2012 by Thomas Dreibholz
#
#               Author: Thomas Dreibholz, dreibh@iem.uni-due.de
# ###########################################################################


source("simulation.R")
source("loaddistributions-version3.R")
source("numberdistributions-version1.R")
source("delaydistributions-version2.R")
source("timedistributions-version1.R")


# ###########################################################################
# #### rspsim5-specific Part                                             ####
# ###########################################################################

# ====== Write OMNeT++ INI file header ======================================
rspsim5WriteHeader <- function(iniFile, simulationRun, scalarName, vectorName, duration)
{
   cat(sep="", "[General]\n", file=iniFile)
   cat(sep="", "network = ", simCreatorSimulationNetwork, "\n", file=iniFile)
   cat(sep="", "rng-class = cMersenneTwister\n", file=iniFile)
   cat(sep="", "seed-0-mt = ", simulationRun, "\n", file=iniFile)
   cat(sep="", "output-scalar-file = ", "run", simulationRun, "-scalars.sca\n", file=iniFile)
   cat(sep="", "# output-vector-file = ", "run", simulationRun, "-vectors.vec\n", file=iniFile)
   cat(sep="", "# --- Note: Set sim-time-limit larger than Controller::statisticsWriteTimeStamp! ---\n", file=iniFile)
   cat(sep="", "sim-time-limit = ", simCreatorSimulationStartup, " ", duration, "s 1ms\n", file=iniFile)
   cat(sep="", "simtime-resolution = 10ns   # 10ns time scale\n", file=iniFile)
   cat(sep="", "\n\n", file=iniFile)

   cat(sep="", "cmdenv-express-mode = true\n", file=iniFile)
   cat(sep="", "\n", file=iniFile)

   if(simulationStoreVectors) {
      cat(sep="", "**.vector-recording = true\n", file=iniFile)
   }
   else {
      cat(sep="", "**.vector-recording = false\n", file=iniFile)
   }
   cat(sep="", "**.vector-recording-intervals = ", simCreatorSimulationStartup, "..", simCreatorSimulationStartup, " ", duration, "s\n", file=iniFile)
   cat(sep="", "\n", file=iniFile)
}


# ====== rspsim5 automatic parameter setup ==================================
rspsim5AutoParameters <- function(simulationConfigurations)
{
   # ------Compute JobInterval from PU:PE ratio and JobSize -----------------
   if(checkVariableType(simulationConfigurations, "calcAppPoolUserServiceJobIntervalVariable", CVT_Auto)) {
      if(simulationScriptOutputVerbosity > 6) {
         cat(sep="", "\n   AUTO:JobInterval")
         cat(sep="", " puToPERatio=", getGlobalVariable("puToPERatio"), " ")
         cat(sep="", " jobSize=", getGlobalVariable("calcAppPoolUserServiceJobSizeVariable"), " ")
         cat(sep="", " targetSystemUtilization=", getGlobalVariable("targetSystemUtilization"), " ")
         cat(sep="", " CalcAppPEs=", getGlobalVariable("scenarioNumberOfCalcAppPoolElementsVariable"), " => ")
      }

      setGlobalVariable("scenarioNumberOfCalcAppPoolUsersVariable",
         round(as.numeric(getGlobalVariable("puToPERatio")) * as.numeric(getGlobalVariable("scenarioNumberOfCalcAppPoolElementsVariable"))))
      gamma <- max(1.0, as.numeric(getGlobalVariable("puToPERatio")))
      setGlobalVariable("calcAppPoolUserServiceJobIntervalVariable",
          gamma * as.numeric(getGlobalVariable("calcAppPoolUserServiceJobSizeVariable")) /
          (as.numeric(getGlobalVariable("targetSystemUtilization")) *
           as.numeric(getGlobalVariable("calcAppPoolElementServiceCapacityVariable"))))

      if(simulationScriptOutputVerbosity > 6) {
         cat(sep="", " CalcAppPUs=", getGlobalVariable("scenarioNumberOfCalcAppPoolUsersVariable"), " ")
         cat(sep="", " jobInterval=", getGlobalVariable("calcAppPoolUserServiceJobIntervalVariable"), "\n   ")
      }
   }

   # ------Compute PU:PE ratio from JobInterval and  JobSize ----------------
   else if(checkVariableType(simulationConfigurations, "puToPERatio", CVT_Auto)) {
      if(simulationScriptOutputVerbosity > 6) {
         cat(sep="", "\n   AUTO:puToPERatio")
         cat(sep="", " jobInterval=", getGlobalVariable("calcAppPoolUserServiceJobIntervalVariable"), "\n   ")
         cat(sep="", " jobSize=", getGlobalVariable("calcAppPoolUserServiceJobSizeVariable"), " ")
         cat(sep="", " targetSystemUtilization=", getGlobalVariable("targetSystemUtilization"), " ")
         cat(sep="", " CalcAppPEs=", getGlobalVariable("scenarioNumberOfCalcAppPoolElementsVariable"), " => ")
      }

      gamma <- (as.numeric(getGlobalVariable("calcAppPoolUserServiceJobIntervalVariable")) *
                as.numeric(getGlobalVariable("targetSystemUtilization")) *
                as.numeric(getGlobalVariable("calcAppPoolElementServiceCapacityVariable"))) /
                  as.numeric(getGlobalVariable("calcAppPoolUserServiceJobSizeVariable"))
      if(gamma < 1.0) {
         if(simulationScriptOutputVerbosity > 5) {
            cat(sep="", "jobSize=", calcAppPoolUserServiceJobSizeVariable, "   ",
                        "jobInterval=", calcAppPoolUserServiceJobIntervalVariable,  "   ",
                        "CalcAppPEs=", scenarioNumberOfCalcAppPoolElementsVariable, "    =>   ",
                        "gamma=", gamma,
                        "   -> SKIPPING!\n")
         }
         return(FALSE)
      }
      else {
         setGlobalVariable("puToPERatio", gamma)
         setGlobalVariable("scenarioNumberOfCalcAppPoolUsersVariable",
                           round(gamma * as.numeric(getGlobalVariable("scenarioNumberOfCalcAppPoolElementsVariable"))))
      }

      if(simulationScriptOutputVerbosity > 6) {
         cat(sep="", " CalcAppPUs=", getGlobalVariable("scenarioNumberOfCalcAppPoolUsersVariable"), " ")
         cat(sep="", " puToPERatio=", getGlobalVariable("puToPERatio"), " ")
      }
   }

   # ------Compute JobSize from JobInterval and PU:PE ratio -----------------
   else if(checkVariableType(simulationConfigurations, "calcAppPoolUserServiceJobSizeVariable", CVT_Auto)) {
      if(simulationScriptOutputVerbosity > 6) {
         cat(sep="", "\n   AUTO:JobSize")
         cat(sep="", " puToPERatio=", getGlobalVariable("puToPERatio"), " ")
         cat(sep="", " jobInterval=", getGlobalVariable("calcAppPoolUserServiceJobIntervalVariable"), "\n   ")
         cat(sep="", " targetSystemUtilization=", getGlobalVariable("targetSystemUtilization"), " ")
         cat(sep="", " CalcAppPEs=", getGlobalVariable("scenarioNumberOfCalcAppPoolElementsVariable"), " => ")
      }

      setGlobalVariable("scenarioNumberOfCalcAppPoolUsersVariable",
         round(as.numeric(getGlobalVariable("puToPERatio")) * as.numeric(getGlobalVariable("scenarioNumberOfCalcAppPoolElementsVariable"))))
      gamma <- max(1.0, as.numeric(getGlobalVariable("puToPERatio")))
      setGlobalVariable("calcAppPoolUserServiceJobSizeVariable",
                        (as.numeric(getGlobalVariable("calcAppPoolUserServiceJobIntervalVariable")) *
                         as.numeric(getGlobalVariable("targetSystemUtilization")) *
                         as.numeric(getGlobalVariable("calcAppPoolElementServiceCapacityVariable")))
                        / gamma)

      if(simulationScriptOutputVerbosity > 6) {
         cat(sep="", " CalcAppPUs=", getGlobalVariable("scenarioNumberOfCalcAppPoolUsersVariable"), " ")
         cat(sep="", " jobSize=", getGlobalVariable("calcAppPoolUserServiceJobSizeVariable"), " ")
      }
   }

#    # ------Set number of PUs by number of PEs -------------------------------
#    else if(checkVariableType(simulationConfigurations, "scenarioNumberOfCalcAppPoolUsersVariable", CVT_Auto)) {
#       if(simulationScriptOutputVerbosity > 6) {
#          cat(sep="", "\n   AUTO:NumberOfPUs")
#          cat(sep="", " puToPERatio=", getGlobalVariable("puToPERatio"), " ")
#          cat(sep="", " CalcAppPEs=", getGlobalVariable("scenarioNumberOfCalcAppPoolElementsVariable"), " => ")
#       }
#       setGlobalVariable("scenarioNumberOfCalcAppPoolUsersVariable",
#          round(as.numeric(getGlobalVariable("puToPERatio")) * as.numeric(getGlobalVariable("scenarioNumberOfCalcAppPoolElementsVariable"))))
#       if(simulationScriptOutputVerbosity > 6) {
#          cat(sep="", " CalcAppPUs=", getGlobalVariable("scenarioNumberOfCalcAppPoolUsersVariable"), "\n")
#       }
#    }

   # ------ Job Request Timeout from Job Keep-Alive Timeout -----------------
   if(checkVariableType(simulationConfigurations, "calcAppProtocolServiceJobRequestTimeout", CVT_Auto)) {
      setGlobalVariable("calcAppProtocolServiceJobRequestTimeout",
         getGlobalVariable("calcAppProtocolServiceJobKeepAliveTimeout"))
      if(simulationScriptOutputVerbosity > 6) {
         cat(sep="", " jReqTO=", getGlobalVariable("calcAppProtocolServiceJobKeepAliveTimeout"), " ")
      }
   }

   if(as.numeric(getGlobalVariable("enrpMaxTimeLastHeared")) <= 0)  {
      setGlobalVariable("enrpMaxTimeLastHeared",
         as.numeric(getGlobalVariable("enrpPeerHeartbeatCycle")) +
         as.numeric(getGlobalVariable("enrpMaxTimeNoResponse")))
      if(simulationScriptOutputVerbosity > 6) {
         cat(sep="", " enrpMTLH=", getGlobalVariable("enrpMaxTimeLastHeared"), " ")
      }
   }

   return(TRUE)
}


# ====== Write rspsim5 INI file parameter section ===========================
rspsim5WriteParameterSection <- function(filePrefix, iniFile, simulationRun, duration)
{
   # NewParam: Add appropriate lines to the .ini file in this function.

   # ------ Basic settings --------------------------------------------------
   # cat(sep="", "[Parameters]\n", file=iniFile)
   cat(sep="", "###### Basic Scenario Settings #############################################\n", file=iniFile)
   cat(sep="", "gammaScenario.controller.statisticsResetTime = ", simCreatorSimulationStartup, "\n", file=iniFile)
   cat(sep="", "gammaScenario.controller.statisticsWriteTime = ", simCreatorSimulationStartup, " ", duration, "s\n\n", file=iniFile)


   # ------ Special settings ------------------------------------------------
   cat(sep="", "###### Special Settings #####################################################\n", file=iniFile)
   if(SPECIAL0 != "") { cat(SPECIAL0, "\n", file=iniFile) }
   if(SPECIAL1 != "") { cat(SPECIAL1, "\n", file=iniFile) }
   if(SPECIAL2 != "") { cat(SPECIAL2, "\n", file=iniFile) }
   if(SPECIAL3 != "") { cat(SPECIAL3, "\n", file=iniFile) }
   if(SPECIAL4 != "") { cat(SPECIAL4, "\n", file=iniFile) }
   if(SPECIAL5 != "") { cat(SPECIAL5, "\n", file=iniFile) }
   if(SPECIAL6 != "") { cat(SPECIAL6, "\n", file=iniFile) }
   if(SPECIAL7 != "") { cat(SPECIAL7, "\n", file=iniFile) }
   if(SPECIAL8 != "") { cat(SPECIAL8, "\n", file=iniFile) }
   if(SPECIAL9 != "") { cat(SPECIAL9, "\n", file=iniFile) }
   cat("\n\n", file=iniFile)


   # ------ LAN scenario settings -------------------------------------------
   cat(sep="", "###### LAN Settings #########################################################\n", file=iniFile)

   cat(sep="", "gammaScenario.numberOfLANs = ", scenarioNumberOfLANs, "\n", file=iniFile)
   PRs <- c()
   CalcAppPEs <- c()
   CalcAppPUs <- c()
   Attackers <- c()
   nextComponentAddress <- 1
   globalPRAddressList <- c()
   originalCalcAppPoolElementStaticRegistrarList <- calcAppPoolElementStaticRegistrarList
   originalCalcAppPoolUserStaticRegistrarList <- calcAppPoolUserStaticRegistrarList
   originalAttackerStaticRegistrarList <- attackerStaticRegistrarList
   for(i in seq(1, as.numeric(scenarioNumberOfLANs))) {
      localPRAddressList <- c()
      calcAppPoolElementStaticRegistrarList <- originalCalcAppPoolElementStaticRegistrarList
      calcAppPoolUserStaticRegistrarList    <- originalCalcAppPoolUserStaticRegistrarList
      attackerStaticRegistrarList           <- originalAttackerStaticRegistrarList

      # ------ Numbers of components ----------------------------------------
      cat(sep="", "# ----- LAN #", i, " -----\n", file=iniFile)
      result <- eval(call(scenarioNumberOfRegistrarsDistribution, i, scenarioNumberOfLANs, scenarioNumberOfRegistrarsVariable, scenarioNumberOfRegistrarsGamma, scenarioNumberOfRegistrarsVariable))
      PRs <- append(PRs, c(result[2]))
      result <- eval(call(scenarioNumberOfCalcAppPoolElementsDistribution, i, scenarioNumberOfLANs, scenarioNumberOfCalcAppPoolElementsVariable, scenarioNumberOfCalcAppPoolElementsGamma, scenarioNumberOfCalcAppPoolElementsLambda))
      CalcAppPEs <- append(CalcAppPEs, c(result[2]))
      result <- eval(call(scenarioNumberOfCalcAppPoolUsersDistribution, i, scenarioNumberOfLANs, scenarioNumberOfCalcAppPoolUsersVariable, scenarioNumberOfCalcAppPoolUsersGamma, scenarioNumberOfCalcAppPoolUsersLambda))
      CalcAppPUs <- append(CalcAppPUs, c(result[2]))
      result <- eval(call(scenarioNumberOfCalcAppPoolUsersDistribution, i, scenarioNumberOfLANs, scenarioNumberOfAttackersVariable, scenarioNumberOfAttackersGamma, scenarioNumberOfAttackersLambda))
      Attackers <- append(Attackers, c(result[2]))

      cat(sep="", "gammaScenario.lan[", i - 1, "].numberOfRegistrars = ",   PRs[i], "\n", file=iniFile)
      cat(sep="", "gammaScenario.lan[", i - 1, "].numberOfCalcAppPoolElements = ", CalcAppPEs[i], "\n", file=iniFile)
      cat(sep="", "gammaScenario.lan[", i - 1, "].numberOfCalcAppPoolUsers = ",    CalcAppPUs[i], "\n", file=iniFile)
      cat(sep="", "gammaScenario.lan[", i - 1, "].numberOfAttackers = ",    Attackers[i], "\n", file=iniFile)


      # ------ Component configurations -------------------------------------
      if(simulationScriptOutputVerbosity > 8) {
         cat(paste(sep="", "   => LAN #", i, ":   " , PRs[i], " PRs, ", CalcAppPEs[i], " CalcAppPEs, ", CalcAppPUs[i], " CalcAppPUs\n"))
      }

      # ------ Registrars ---------------------------------------------------
      if(as.numeric(PRs[i]) > 0) {
         for(j in seq(1, as.numeric(PRs[i]))) {
            result <- eval(call(scenarioNetworkLANDelayDistribution, i, scenarioNumberOfLANs, "PR", j, as.numeric(PRs[i]), scenarioNetworkLANDelayVariable, scenarioNetworkLANDelayGamma, scenarioNetworkLANDelayLambda))
            lanDelay <- as.numeric(result[2])
            # ------ Component address -----------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].registrarArray[", j - 1, "].transportNode.interfaceAddress = ", nextComponentAddress, "\n", file=iniFile)
            localPRAddressList <- append(localPRAddressList, c(nextComponentAddress))
            globalPRAddressList <- append(globalPRAddressList, c(nextComponentAddress))
            nextComponentAddress <- nextComponentAddress + 1
            # ------ Component link delay --------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].switch.toRegistrar[", j - 1, "].channel.delay = ", lanDelay / 2, "ms\n", file=iniFile)
            cat(sep="", "gammaScenario.lan[", i - 1, "].registrarArray[", j - 1, "].toNetwork.channel.delay = ", lanDelay / 2, "ms\n", file=iniFile)
         }
      }

      staticRegistrars <- ""
      for(prAddress in localPRAddressList) {
         staticRegistrars <- paste(sep="", prAddress, " ", staticRegistrars)
      }
      # cat(paste(sep="", "   => LAN #", i, ": staticRegistrars =", staticRegistrars, "\n"))

      if( (calcAppPoolElementStaticRegistrarList == "") | (calcAppPoolElementStaticRegistrarList == "-")) {
         calcAppPoolElementStaticRegistrarList <- staticRegistrars
      }
      if( (calcAppPoolUserStaticRegistrarList == "") | (calcAppPoolUserStaticRegistrarList == "-")) {
         calcAppPoolUserStaticRegistrarList <- staticRegistrars
      }
      if( (attackerStaticRegistrarList == "") | (attackerStaticRegistrarList == "-")) {
         attackerStaticRegistrarList <- staticRegistrars
      }

      # ------ CalcAppPEs ---------------------------------------------------
      if(as.numeric(CalcAppPEs[i]) > 0) {
         for(j in seq(1, as.numeric(CalcAppPEs[i]))) {
            result <- eval(call(scenarioNetworkLANDelayDistribution, i, scenarioNumberOfLANs, "PE", j, as.numeric(CalcAppPEs[i]), scenarioNetworkLANDelayVariable, scenarioNetworkLANDelayGamma, scenarioNetworkLANDelayLambda))
            lanDelay <- as.numeric(result[2])
            # ------ Component address -----------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolElementArray[", j - 1, "].transportNode.interfaceAddress = ", nextComponentAddress, "\n", file=iniFile)
            nextComponentAddress <- nextComponentAddress + 1
            # ------ Registrar list --------------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolElementArray[", j - 1, "].registrarTable.staticRegistrarsList = \"", calcAppPoolElementStaticRegistrarList, "\"\n", file=iniFile)
            # ------ Component link delay --------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].switch.toCalcAppPoolElement[", j - 1, "].channel.delay = ", lanDelay / 2, "ms\n", file=iniFile)
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolElementArray[", j - 1, "].toNetwork.channel.delay = ", lanDelay / 2, "ms\n", file=iniFile)
         }
      }

      # ------ CalcAppPUs ---------------------------------------------------
      if(as.numeric(CalcAppPUs[i]) > 0) {
         for(j in seq(1, as.numeric(CalcAppPUs[i]))) {
            result <- eval(call(scenarioNetworkLANDelayDistribution, i, scenarioNumberOfLANs, "PU", j, as.numeric(CalcAppPUs[i]), scenarioNetworkLANDelayVariable, scenarioNetworkLANDelayGamma, scenarioNetworkLANDelayLambda))
            lanDelay <- as.numeric(result[2])
            # ------ Component address -----------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolUserArray[", j - 1, "].transportNode.interfaceAddress = ", nextComponentAddress, "\n", file=iniFile)
            nextComponentAddress <- nextComponentAddress + 1
            # ------ Registrar list --------------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolUserArray[", j - 1, "].registrarTable.staticRegistrarsList = \"", calcAppPoolUserStaticRegistrarList, "\"\n", file=iniFile)
            # ------ Component link delay --------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].switch.toCalcAppPoolUser[", j - 1, "].channel.delay = ", lanDelay / 2, "ms\n", file=iniFile)
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolUserArray[", j - 1, "].toNetwork.channel.delay = ", lanDelay / 2, "ms\n", file=iniFile)
         }
      }

      # ------ Attackers ---------------------------------------------------
      if(as.numeric(Attackers[i]) > 0) {
         for(j in seq(1, as.numeric(Attackers[i]))) {
            result <- eval(call(scenarioNetworkLANDelayDistribution, i, scenarioNumberOfLANs, "PU", j, as.numeric(Attackers[i]), scenarioNetworkLANDelayVariable, scenarioNetworkLANDelayGamma, scenarioNetworkLANDelayLambda))
            lanDelay <- as.numeric(result[2])
            # ------ Component address -----------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].attackerArray[", j - 1, "].transportNode.interfaceAddress = ", nextComponentAddress, "\n", file=iniFile)
            nextComponentAddress <- nextComponentAddress + 1
            # ------ Registrar list --------------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].attackerArray[", j - 1, "].attackerProcess.attackTargetRegistrarsList = \"", attackerStaticRegistrarList, "\"\n", file=iniFile)
            # ------ Component link delay --------------------------------------
            cat(sep="", "gammaScenario.lan[", i - 1, "].switch.toAttacker[", j - 1, "].channel.delay = ", lanDelay / 2, "ms\n", file=iniFile)
            cat(sep="", "gammaScenario.lan[", i - 1, "].attackerArray[", j - 1, "].toNetwork.channel.delay = ", lanDelay / 2, "ms\n", file=iniFile)
         }
      }

      # ------ Switch address -----------------------------------------------
      cat(sep="", "gammaScenario.lan[", i - 1, "].switch.transportNode.interfaceAddress = ", nextComponentAddress, "\n", file=iniFile)
      cat(sep="", "gammaScenario.lan[", i - 1, "].switch.transportNode.interfaceStatusChanges = \"\"\n", file=iniFile)
      nextComponentAddress <- nextComponentAddress + 1
   }

   # ------ Peer list -------------------------------------------------------
   staticPeers <- ""
   for(prAddress in globalPRAddressList) {
      staticPeers <- paste(sep="", staticPeers, prAddress, " ")
   }
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.enrpStaticPeersList = \"", staticPeers, "\"\n", file=iniFile)


   # ------ WAN settings ----------------------------------------------------
   if(as.numeric(scenarioNumberOfLANs) > 0) {
      for(i in seq(1, as.numeric(scenarioNumberOfLANs) - 1)) {
         result <- eval(call(scenarioNetworkWANDelayDistribution, i, scenarioNumberOfLANs, "WAN", 1, 1, scenarioNetworkWANDelayVariable, scenarioNetworkWANDelayGamma, scenarioNetworkWANDelayLambda))
         wanDelay <- as.numeric(result[2])
         if(i > 0) {
            cat(sep="", "# ----- WAN link of LAN #", i, " <-> LAN #", i + 1, " -----\n", file=iniFile)
            cat(sep="", "gammaScenario.lan[", i - 1, "].toRightNetwork.channel.delay = ", wanDelay, "ms\n", file=iniFile)
         }
         if(i < as.numeric(scenarioNumberOfLANs) ) {
            cat(sep="", "gammaScenario.lan[", i, "].toLeftNetwork.channel.delay  = ", wanDelay, "ms\n", file=iniFile)
         }
      }
   }
   cat(sep="", "\n\n", file=iniFile)


   # ------ PR settings -----------------------------------------------------
   cat(sep="", "###### Registrars ###########################################################\n", file=iniFile)
   result <- eval(call(registrarTransportInterfaceUptimeDistribution, registrarTransportInterfaceUptimeVariable, registrarTransportInterfaceUptimeGamma, registrarTransportInterfaceUptimeLambda))
   transportInterfaceUptime <- result[2]
   result <- eval(call(registrarTransportInterfaceDowntimeDistribution, registrarTransportInterfaceDowntimeVariable, registrarTransportInterfaceDowntimeGamma, registrarTransportInterfaceDowntimeLambda))
   transportInterfaceDowntime <- result[2]
   result <- eval(call(registrarComponentUptimeDistribution, registrarComponentUptimeVariable, registrarComponentUptimeGamma, registrarComponentUptimeLambda))
   componentUptime <- result[2]
   result <- eval(call(registrarComponentDowntimeDistribution, registrarComponentDowntimeVariable, registrarComponentDowntimeGamma, registrarComponentDowntimeLambda))
   componentDowntime <- result[2]
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].transportNode.interfaceUptime = ", transportInterfaceUptime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].transportNode.interfaceDowntime = ", transportInterfaceDowntime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].transportNode.interfaceStatusChanges = \"\"\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.componentRuns = 1000000000\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.componentUptime = ", componentUptime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.componentDowntime = ", componentDowntime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.componentStatusChanges = \"\"\n", file=iniFile)
   cat(sep="", "\n", file=iniFile)


#    prID <- 1000000
#    for(i in seq(1, as.numeric(scenarioNumberOfLANs))) {
#       for(j in seq(1, as.numeric(PRs[i]))) {
#          cat(sep="", "gammaScenario.lan[", i - 1, "].registrarArray[", j - 1, "].registrarProcess.registrarIdentifier = ", prID, "\n", file=iniFile)
#          prID <- prID + 1
#       }
#    }
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarIdentifier = intuniform(0x00000001,0x7FFFFFFF)\n", file=iniFile)


   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarMentorDiscoveryTimeout = ", registrarMentorDiscoveryTimeout, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarMaxHandleResolutionItems = ", registrarMaxHandleResolutionItems, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarRandomizeMaxHandleResolutionItems = ", registrarRandomizeMaxHandleResolutionItems, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarMaxIncrement = ", as.integer(registrarMaxIncrement), "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarUpdateLossProbability = ", registrarUpdateLossProbability, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarMaxBadPEReports = ", as.integer(registrarMaxBadPEReports), "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarMaxEndpointUnreachableRate = ", registrarMaxEndpointUnreachableRate, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarEndpointUnreachableRateBuckets = ", registrarEndpointUnreachableRateBuckets, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarEndpointUnreachableRateMaxEntries = ", registrarEndpointUnreachableRateMaxEntries, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarMaxHandleResolutionRate = ", registrarMaxHandleResolutionRate, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarHandleResolutionRateBuckets = ", registrarHandleResolutionRateBuckets, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.registrarHandleResolutionRateMaxEntries = ", registrarHandleResolutionRateMaxEntries, "\n", file=iniFile)

   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.asapEndpointKeepAliveInterval = ", asapEndpointKeepAliveInterval, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.asapEndpointKeepAliveTimeout = ", asapEndpointKeepAliveTimeout, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.asapNoServiceDuringStartup = ", asapNoServiceDuringStartup, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.asapUseTakeoverSuggestion = ", asapUseTakeoverSuggestion, "\n", file=iniFile)

   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.enrpPeerHeartbeatCycle = ", enrpPeerHeartbeatCycle, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.enrpMaxTimeLastHeared = ", enrpMaxTimeLastHeared, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.enrpMaxTimeNoResponse = ", enrpMaxTimeNoResponse, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].registrarArray[*].registrarProcess.enrpTakeoverExpiry = ", enrpTakeoverExpiry, "s\n", file=iniFile)
   cat(sep="", "\n\n", file=iniFile)


   # ------ PE settings -----------------------------------------------------
   cat(sep="", "###### Pool Elements ####################################\n", file=iniFile)
   result <- eval(call(calcAppPoolElementTransportInterfaceUptimeDistribution, calcAppPoolElementTransportInterfaceUptimeVariable, calcAppPoolElementTransportInterfaceUptimeGamma, calcAppPoolElementTransportInterfaceUptimeLambda))
   transportInterfaceUptime <- result[2]
   result <- eval(call(calcAppPoolElementTransportInterfaceDowntimeDistribution, calcAppPoolElementTransportInterfaceDowntimeVariable, calcAppPoolElementTransportInterfaceDowntimeGamma, calcAppPoolElementTransportInterfaceDowntimeLambda))
   transportInterfaceDowntime <- result[2]
   result <- eval(call(calcAppPoolElementComponentUptimeDistribution, calcAppPoolElementComponentUptimeVariable, calcAppPoolElementComponentUptimeGamma, calcAppPoolElementComponentUptimeLambda))
   componentUptime <- result[2]
   result <- eval(call(calcAppPoolElementComponentDowntimeDistribution, calcAppPoolElementComponentDowntimeVariable, calcAppPoolElementComponentDowntimeGamma, calcAppPoolElementComponentDowntimeLambda))
   componentDowntime <- result[2]
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].transportNode.interfaceUptime = ", transportInterfaceUptime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].transportNode.interfaceDowntime = ", transportInterfaceDowntime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].transportNode.interfaceStatusChanges = \"\"\n", file=iniFile)

   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.componentRuns = 1000000000\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.componentUptime = ", componentUptime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.componentDowntime = ", componentDowntime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.componentStatusChanges = \"\"\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.componentCleanShutdownProbability = ", calcAppPoolElementComponentCleanShutdownProbability, "\n", file=iniFile)
   cat(sep="", "\n", file=iniFile)

   for(i in seq(1, as.numeric(scenarioNumberOfLANs))) {
      if(as.numeric(CalcAppPEs[i]) > 0) {
         for(j in seq(1, as.numeric(CalcAppPEs[i]))) {
            result <- eval(call(calcAppPoolElementServiceCapacityDistribution, i, scenarioNumberOfLANs, j, as.numeric(CalcAppPEs[i]), calcAppPoolElementServiceCapacityVariable, calcAppPoolElementServiceCapacityGamma, calcAppPoolElementServiceCapacityLambda))
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolElementArray[", j - 1, "].calcAppServer.serviceCapacity = ",
               result[2],
               "\n", file=iniFile)
         }
      }
   }
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.serviceMaxJobs = ", calcAppPoolElementServiceMaxJobs, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.serviceMinCapacityPerJob = ", calcAppPoolElementServiceMinCapacityPerJob, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.serviceRejectProbability = ", calcAppPoolElementServiceRejectProbability, "\n", file=iniFile)

   cat(sep="", "\n", file=iniFile)

   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.selectionPolicy = \"", calcAppPoolElementSelectionPolicy, "\"\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.selectionPolicyWeight = ", calcAppPoolElementSelectionPolicyWeight, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = ", calcAppPoolElementSelectionPolicyLoadDegradation, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDPF = ", calcAppPoolElementSelectionPolicyLoadDPF, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.selectionPolicyWeightDPF = ", calcAppPoolElementSelectionPolicyWeightDPF, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.selectionPolicyUpdateThreshold = ", calcAppPoolElementSelectionPolicyUpdateThreshold, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.selectionReregisterImmediatelyOnUpdate = ", calcAppPoolElementReregisterImmediatelyOnUpdate, "\n", file=iniFile)
   cat(sep="", "\n", file=iniFile)

   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.servicePoolElementIdentifier = intuniform(0x00000001,0x7FFFFFFF)\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.servicePoolHandle = \"CalcAppPool\"\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.serviceRegistrationLife = ", calcAppPoolElementServerRegistrationLife, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.serviceJobKeepAliveInterval = ", calcAppProtocolServiceJobKeepAliveInterval, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.serviceJobKeepAliveTimeout = ", calcAppProtocolServiceJobKeepAliveTimeout, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.serviceCookieMaxCalculations = ", calcAppPoolElementServerCookieMaxCalculations, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].calcAppServer.serviceCookieMaxTime = ", calcAppPoolElementServerCookieMaxTime, "\n", file=iniFile)
   cat(sep="", "\n", file=iniFile)

   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].poolElementASAP.asapRegistrationTimeout = ", asapRequestTimeout, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].poolElementASAP.asapDeregistrationTimeout = ", asapRequestTimeout, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].poolElementASAP.asapMaxRegistrationAttempts = ", asapMaxRequestRetransmit, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolElementArray[*].poolElementASAP.asapServerHuntRetryDelay = ", asapServerHuntRetryDelay, "\n", file=iniFile)
   cat(sep="", "\n\n", file=iniFile)


   # ------ PU settings -----------------------------------------------------
   cat(sep="", "###### Pool Users #######################################\n", file=iniFile)
   result <- eval(call(calcAppPoolUserTransportInterfaceUptimeDistribution, calcAppPoolUserTransportInterfaceUptimeVariable, calcAppPoolUserTransportInterfaceUptimeGamma, calcAppPoolUserTransportInterfaceUptimeLambda))
   transportInterfaceUptime <- result[2]
   result <- eval(call(calcAppPoolUserTransportInterfaceDowntimeDistribution, calcAppPoolUserTransportInterfaceDowntimeVariable, calcAppPoolUserTransportInterfaceDowntimeGamma, calcAppPoolUserTransportInterfaceDowntimeLambda))
   transportInterfaceDowntime <- result[2]
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].transportNode.interfaceUptime = ", transportInterfaceUptime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].transportNode.interfaceDowntime = ", transportInterfaceDowntime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].transportNode.interfaceStatusChanges = \"\"\n", file=iniFile)

   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].calcAppQueuingClient.servicePoolHandle = \"CalcAppPool\"\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].calcAppQueuingClient.serviceJobRetryDelay = ", calcAppPoolUserServiceJobRetryDelay, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].calcAppQueuingClient.serviceHandleResolutionRetryDelay = ", calcAppPoolUserServiceHandleResolutionRetryDelay, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].calcAppQueuingClient.serviceJobKeepAliveInterval = ", calcAppProtocolServiceJobKeepAliveInterval, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].calcAppQueuingClient.serviceJobKeepAliveTimeout = ", calcAppProtocolServiceJobKeepAliveTimeout, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].calcAppQueuingClient.serviceJobRequestTimeout = ", calcAppProtocolServiceJobRequestTimeout, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].calcAppQueuingClient.serviceJobCount = 1000000000\n", file=iniFile)
   for(i in seq(1, as.numeric(scenarioNumberOfLANs))) {
      if(as.numeric(CalcAppPUs[i]) > 0) {
         for(j in seq(1, as.numeric(CalcAppPUs[i]))) {
            result <- eval(call(calcAppPoolUserServiceJobSizeDistribution, i, as.numeric(scenarioNumberOfLANs), j, as.numeric(CalcAppPUs[i]), as.numeric(calcAppPoolUserServiceJobSizeVariable), calcAppPoolUserServiceJobSizeGamma, calcAppPoolUserServiceJobSizeLambda))
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolUserArray[", j - 1, "].calcAppQueuingClient.serviceJobSize = ",
               result[2],
               "\n", file=iniFile)
            result <- eval(call(calcAppPoolUserServiceJobIntervalDistribution, i, as.numeric(scenarioNumberOfLANs), j, as.numeric(CalcAppPUs[i]), as.numeric(calcAppPoolUserServiceJobIntervalVariable), calcAppPoolUserServiceJobIntervalGamma, calcAppPoolUserServiceJobIntervalLambda))
            cat(sep="", "gammaScenario.lan[", i - 1, "].calcAppPoolUserArray[", j - 1, "].calcAppQueuingClient.serviceJobInterval = ",
               result[2],
               "\n", file=iniFile)
         }
      }
   }

   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].poolUserASAP.asapStaleCacheValue = ", asapStaleCacheValue, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].poolUserASAP.asapRequestTimeout = ", asapRequestTimeout, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].poolUserASAP.asapMaxRequestRetransmit = ", asapMaxRequestRetransmit, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].calcAppPoolUserArray[*].poolUserASAP.asapServerHuntRetryDelay = ", asapServerHuntRetryDelay, "\n", file=iniFile)
   cat(sep="", "\n\n", file=iniFile)


   # ------ Attacker settings -----------------------------------------------
   cat(sep="", "###### Attackers ########################################\n", file=iniFile)
   result <- eval(call(attackerTransportInterfaceUptimeDistribution, attackerTransportInterfaceUptimeVariable, attackerTransportInterfaceUptimeGamma, attackerTransportInterfaceUptimeLambda))
   transportInterfaceUptime <- result[2]
   result <- eval(call(attackerTransportInterfaceDowntimeDistribution, attackerTransportInterfaceDowntimeVariable, attackerTransportInterfaceDowntimeGamma, attackerTransportInterfaceDowntimeLambda))
   transportInterfaceDowntime <- result[2]
   result <- eval(call(attackerComponentUptimeDistribution, attackerComponentUptimeVariable, attackerComponentUptimeGamma, attackerComponentUptimeLambda))
   componentUptime <- result[2]
   result <- eval(call(attackerComponentDowntimeDistribution, attackerComponentDowntimeVariable, attackerComponentDowntimeGamma, attackerComponentDowntimeLambda))
   componentDowntime <- result[2]
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].transportNode.interfaceUptime = ", transportInterfaceUptime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].transportNode.interfaceDowntime = ", transportInterfaceDowntime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].transportNode.interfaceStatusChanges = \"\"\n", file=iniFile)

   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.componentRuns = 1000000000\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.componentUptime = ", componentUptime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.componentDowntime = ", componentDowntime, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.componentStatusChanges = \"\"\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackType = \"", attackerAttackType, "\"\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackInterval = ", attackerAttackInterval, "s\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackTargetPoolHandle = \"CalcAppPool\"\n", file=iniFile)
   cat(sep="", "# NOTE: attackTargetPolicy set to calcAppPoolElementSelectionPolicy!\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackTargetPolicy = \"", calcAppPoolElementSelectionPolicy, "\"\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackTargetPolicyLoadDegradation = ", attackTargetPolicyLoadDegradation, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackTargetPolicyWeight = ", sprintf("%1.1f", as.numeric(attackTargetPolicyWeight)), "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackRandomizeIdentifier = ", attackRandomizeIdentifier, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackAnswerKeepAlive = ", attackAnswerKeepAlive, "\n", file=iniFile)
   cat(sep="", "gammaScenario.lan[*].attackerArray[*].attackerProcess.attackReportUnreachableProbability = ", attackReportUnreachableProbability, "\n", file=iniFile)
   cat(sep="", "\n", file=iniFile)
}


# The sources directory of the simulation.
simCreatorSourcesDirectory <- paste(sep="", getwd(), "/../model")

# The simulation binary.
# NOTE: The path here is relative to the directory set in sourcesDirectory!
simCreatorSimulationBinary <- "model"

# The directory where the binary should be executed.
# NOTE: The path here is relative to the directory set in sourcesDirectory!
simCreatorSimulationBaseDir <- "."

# A list of directories to be recursively searched for NED files. These NED
# files will be copied into the environment directory.
# NOTE: The paths here are relative to the directory set in sourcesDirectory!
# Example: list("src", "examples/sctp")
simCreatorNEDFiles <- list("model")
# NOTE: Before examples/sctp can be used, examples/package.ned must be read.
#       It contains the package name. Without this, there will be an error
#       about wrong package name when the simulation is run!

# A list of directories to be recursively searched for misc files. These misc
# files will be copied into the environment directory.
# NOTE: The paths here are relative to the directory set in sourcesDirectory!
# Example: list(c("*.mrt", "examples/sctp/cmttest1"))
simCreatorMiscFiles <- list()

# The simulation network to be loaded.
simCreatorSimulationNetwork <- "gammaScenario"

simCreatorSimulationStartup     <- "15min"
simCreatorWriteHeader           <- rspsim5WriteHeader
simCreatorWriteParameterSection <- rspsim5WriteParameterSection
simCreatorAutoParameters        <- rspsim5AutoParameters
simCreatorAggregationLevel      <- 1
# Add variables here which should be handled as active (i.e. their value
# is recorded) even it is constant for all runs. The value of this variable
# is required for plotting.
# NewParam: If new parameters should always be recorded, add them here.
simCreatorAdditionalActiveVariables <- c(
   "targetSystemUtilization",
   "puToPERatio",
   "scenarioNumberOfRegistrarsVariable",
   "scenarioNumberOfCalcAppPoolElementsVariable",
   "scenarioNumberOfCalcAppPoolUsersVariable",
   "calcAppPoolElementSelectionPolicy",
   "calcAppPoolElementServiceCapacityVariable",
   "calcAppPoolUserServiceJobSizeVariable",
   "calcAppPoolUserServiceJobIntervalVariable"
)


# Here, you can provide .ned files or directories containing .ned files. All
# these files will be copied into a new directory and transfered to the remote
# processing PEs. A corresponding nedfiles.lst will be written and also
# transferred to the PE.
# Examples: simCreatorNEDFiles  <- "."
#           simCreatorNEDFiles  <- "../alpha/nedfiles ../beta/nedfiles"
simCreatorNEDFiles  <- "."

# Here, you can provide additional files for packaging, e.g. "my-nedfile.ned".
# You may even use a pattern, e.g. "Test*.ned".
simulationMiscFiles <- ""


# NewParam: Finally, add the new parameter to your simulation configuration.
rspsim5PlotVariables <- list(
   # ------ Format example --------------------------------------------------
   # list("Variable",
   #         "Unit[x]{v]"
   #          "100.0 * data1$x / data1$y", <- Manipulator expression:
   #                                           "data" is the data table
   #                                        NA here means: use data1$Variable.
   #          "myColor",
   #          list("InputFile1", "InputFile2", ...))
   #             (simulationDirectory/Results/....data.tar.bz2 is added!)
   # ------------------------------------------------------------------------

   list("controller.SystemAverageUtilization",
           "Average Utilization[%]",
           "100.0 * data1$controller.SystemUtilization",
           "blue4",
           list("controller-SystemUtilization")),
   # IMPORTANT: Do *not* use controller-SystemAverageUtilization here!
   #            This would be the *unweighted* average over all PE utilizations!
   #            Heterogeneous scenarios would cause strange results!

   list("controller.SystemAverageHandlingSpeed",
           "Average Request Handling Speed[%]",
           "100.0 * data1$controller.SystemAverageHandlingSpeed / data1$calcAppPoolElementServiceCapacityVariable",
           "brown4",
           list("controller-SystemAverageHandlingSpeed")),
   list("controller.SystemAverageProcessingSpeed",
           "Average Request Processing Speed[%]",
           "100.0 * data1$controller.SystemAverageProcessingSpeed / data1$calcAppPoolElementServiceCapacityVariable",
           "green4",
           list("controller-SystemAverageProcessingSpeed")),
   list("controller.SystemAverageQueuingDelay",
           "Average Queuing Delay[s]",
           "data1$controller.SystemAverageQueuingDelay",
           "red4",
           list("controller-SystemAverageQueuingDelay")),
   list("controller.SystemAverageStartupDelay",
           "Average Startup Delay[s]",
           "data1$controller.SystemAverageStartupDelay",
           "yellow4",
           list("controller-SystemAverageStartupDelay")),
   list("controller.CalcAppPUGlobalCalcAppAccepts",
           "Accepted Requests[1]",
           NA, "green2",
          list("controller-CalcAppPUGlobalCalcAppAccepts")),
   list("controller.RegistrarGlobalTakeoversStarted",
           "Total Takeovers Started[1]",
           NA, "yellow3",
          list("controller-RegistrarGlobalTakeoversStarted")),

   list("controller.RegistrarGlobalTakeoversByTimeout",
           "Takeovers by Timeout[%]",
          "safeDiv(100.0 * data2$controller.RegistrarGlobalTakeoversByTimeout, (data1$controller.RegistrarGlobalTakeoversByConsent + data2$controller.RegistrarGlobalTakeoversByTimeout))",
          "gray4",
          list("controller-RegistrarGlobalTakeoversByConsent",
               "controller-RegistrarGlobalTakeoversByTimeout")),
   list("controller.RegistrarGlobalTakeoversByConsent",
           "Takeovers by Consent[%]",
          "safeDiv(100.0 * data1$controller.RegistrarGlobalTakeoversByConsent, (data1$controller.RegistrarGlobalTakeoversByConsent + data2$controller.RegistrarGlobalTakeoversByTimeout))",
          "brown4",
          list("controller-RegistrarGlobalTakeoversByConsent",
               "controller-RegistrarGlobalTakeoversByTimeout")),
   list("controller.RegistrarGlobalRefusedHandleResolutions",
           "Refused Handle Resolutions[1]",
           NA, "red2",
          list("controller-RegistrarGlobalRefusedHandleResolutions")),

   list("lan.registrarArray.registrarProcess.RegistrarTotalTakeoversByConsent",
          "Total Takeovers by Consent[1]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalTakeoversByConsent", "grey4",
          list("lan.registrarArray.registrarProcess-RegistrarTotalTakeoversByConsent")),
   list("lan.registrarArray.registrarProcess.RegistrarTotalTakeoversByTimeout",
          "Total Takeovers by Timeout[1]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalTakeoversByTimeout", "grey4",
          list("lan.registrarArray.registrarProcess-RegistrarTotalTakeoversByTimeout")),
   list("lan.registrarArray.registrarProcess.RegistrarTotalTakeoversStarted",
          "Total Takeovers Started[1]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalTakeoversStarted", "grey4",
          list("lan.registrarArray.registrarProcess-RegistrarTotalTakeoversStarted")),

   list("lan.registrarArray.registrarProcess.RegistrarTotalEndpointKeepAlivesSent",
          "Number of Endpoint Keep-Alives[1]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalEndpointKeepAlivesSent", "grey4",
          list("lan.registrarArray.registrarProcess-RegistrarTotalEndpointKeepAlivesSent")),
   list("lan.registrarArray.registrarProcess.RegistrarTotalHandleUpdates",
          "Number of Handle Updates[1]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalHandleUpdates", "green3",
          list("lan.registrarArray.registrarProcess-RegistrarTotalHandleUpdates")),
   list("lan.registrarArray.registrarProcess.RegistrarTotalRegistrations",
          "Number of Registrations[1]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalRegistrations", "green4",
          list("lan.registrarArray.registrarProcess-RegistrarTotalRegistrations")),

   list("lan.registrarArray.registrarProcess.RegistrarTotalEndpointKeepAlivesSent.Rate",
          "Endpoint Keep-Alive Rate[1/s]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalEndpointKeepAlivesSent / data1$simulationDuration", "grey4",
          list("lan.registrarArray.registrarProcess-RegistrarTotalEndpointKeepAlivesSent")),
   list("lan.registrarArray.registrarProcess.RegistrarTotalHandleUpdates.Rate",
          "Handle Update Rate[1/s]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalHandleUpdates / data1$simulationDuration", "green3",
          list("lan.registrarArray.registrarProcess-RegistrarTotalHandleUpdates")),
   list("lan.registrarArray.registrarProcess.RegistrarTotalRegistrations.Rate",
          "Registrations Rate[1/s]",
          "data1$lan.registrarArray.registrarProcess.RegistrarTotalRegistrations / data1$simulationDuration", "green4",
          list("lan.registrarArray.registrarProcess-RegistrarTotalRegistrations")),

   list("lan.registrarArray.registrarProcess.RegistrarAverageNumberOfPoolElements",
          "Pool Elements[1]",
          "data1$lan.registrarArray.registrarProcess.RegistrarAverageNumberOfPoolElements", "blue2",
          list("lan.registrarArray.registrarProcess-RegistrarAverageNumberOfPoolElements")),
   list("lan.registrarArray.registrarProcess.RegistrarAverageNumberOfOwnedPoolElements",
          "Owned Pool Elements[1]",
          "data1$lan.registrarArray.registrarProcess.RegistrarAverageNumberOfOwnedPoolElements", "blue2",
          list("lan.registrarArray.registrarProcess-RegistrarAverageNumberOfOwnedPoolElements")),
   list("lan.registrarArray",
           "Registrar Number {:rho:}[#]",
           "1 + data1$lan.registrarArray", "black"),

   list("controller.RegistrarGlobalRefusedEndpointUnreachables",
           "Refused Endpoint Unreachables[1]",
           NA, "orange2",
          list("controller-RegistrarGlobalRefusedEndpointUnreachables")),
   list("controller.CalcAppPUGlobalCalcAppRejects",
           "Rejected Requests[1]",
           NA, "red2",
          list("controller-CalcAppPUGlobalCalcAppRejects")),
   list("controller.CalcAppAcceptance",
           "Request Acceptance[%]",
          "100.0 * data1$controller.CalcAppPUGlobalCalcAppAccepts / (data1$controller.CalcAppPUGlobalCalcAppAccepts + data2$controller.CalcAppPUGlobalCalcAppRejects)",
          "green4",
          list("controller-CalcAppPUGlobalCalcAppAccepts", "controller-CalcAppPUGlobalCalcAppRejects")),
   list("controller.CalcAppRejection",
           "Request Rejection[%]",
          "100.0 * data2$controller.CalcAppPUGlobalCalcAppRejects / (data1$controller.CalcAppPUGlobalCalcAppAccepts + data2$controller.CalcAppPUGlobalCalcAppRejects)",
          "red4",
          list("controller-CalcAppPUGlobalCalcAppAccepts", "controller-CalcAppPUGlobalCalcAppRejects")),
   list("controller.HandleResolutionsPerRequest",
           "Handle Resolutions per Request[1]",
          "data1$controller.RegistrarGlobalHandleResolutions / data2$controller.CalcAppPUGlobalJobsCompleted",
          "orange3",
          list("controller-RegistrarGlobalHandleResolutions", "controller-CalcAppPUGlobalJobsCompleted")),
   list("controller.TransportNodeGlobalASAPPackets",
          "Outgoing ASAP Packets[1/s]",
          "data1$controller.TransportNodeGlobalASAPPackets / data1$simulationDuration",
          "blue2",
          list("controller-TransportNodeGlobalASAPPackets")),
   list("controller.TransportNodeGlobalENRPPackets",
          "Outgoing ENRP Packets[1/s]",
          "data1$controller.TransportNodeGlobalENRPPackets / data1$simulationDuration",
          "orange2",
          list("controller-TransportNodeGlobalENRPPackets")),
   list("controller.GlobalAttackerIgnoredApplicationMessages",
          "Ignored Application Packets{i}[1]",
          NA,
          "brown3",
          list("controller-GlobalAttackerIgnoredApplicationMessages")),

   list("scenarioNumberOfRegistrarsVariable",
           "Number of Registrars{NumPRs}[1]",
           NA, "black"),
   list("scenarioNumberOfCalcAppPoolElementsVariable",
           "Number of Pool Elements{NumPEs}[1]",
           NA, "black"),
   list("scenarioNumberOfCalcAppPoolUsersVariable",
           "Number of Pool Users{NumPUs}[1]",
           NA, "black"),
   list("scenarioNumberOfAttackersVariable",
           "Number of Attackers{:alpha:}[1]",
           NA, "black"),
   list("scenarioNetworkLANDelayVariable",
           "Network Delay{d}[ms]",
           NA, "black"),
   list("scenarioNetworkWANDelayVariable",
           "WAN Delay{D}[ms]",
           NA, "black"),

   list("targetSystemUtilization",
           "Target System Utilization{:theta:}[%]",
           "100.0 * data1$targetSystemUtilization", "black"),
   list("puToPERatio",
           "PU:PE Ratio{r}[1]",
           NA, "black"),
   list("calcAppPoolElementSelectionPolicyLoadDegradation",
           "Load Degradation{L}[%]",
           "100.0 * data1$calcAppPoolElementSelectionPolicyLoadDegradation", "black"),
   list("jsToSC",
           "Request Size:PE Capacity Ratio{s}[1]",
           "data1$calcAppPoolUserServiceJobSizeVariable / data1$calcAppPoolElementServiceCapacityVariable",
           "black"),
   list("cmcToJS",
           "Cookie Max Calculations to Request Size Ratio [1]",
           "data1$calcAppPoolElementServerCookieMaxCalculations / data1$calcAppPoolUserServiceJobSizeVariable",
           "black"),
   list("RTT/jsToSC",
           "RTT / Request Size:PE Capacity Ratio{s}[1]",
           "(0.002 * data1$scenarioNetworkLANDelayVariable) * (data1$calcAppPoolUserServiceJobSizeVariable / data1$calcAppPoolElementServiceCapacityVariable)",
           "black"),
   list("SCV/RI",
           "Stale Cache Value:Request Interval Ratio{c}[1]",
           "data1$asapStaleCacheValue / data1$calcAppPoolUserServiceJobIntervalVariable",
           "black"),
   list("JKA/jsToSC",
           "JKA/(Req.Size/PE Capacity){j}[1]",
           "data1$calcAppProtocolServiceJobKeepAliveInterval / (data1$calcAppPoolUserServiceJobSizeVariable / data1$calcAppPoolElementServiceCapacityVariable)",
           "black"),

   list("asapStaleCacheValue",
           "Stale Cache Value{c}[s]",
           NA, "black"),
   list("asapEndpointKeepAliveInterval",
           "Endpoint Keep-Alive Interval{e}[s]",
           NA, "black"),
   list("asapEndpointKeepAliveTimeout",
           "Endpoint Keep-Alive Timeout{t}[s]",
           NA, "black"),
   list("asapUseTakeoverSuggestion",
           "Use Takeover Suggestion {:tau:}",
           NA, "black"),

   list("enrpPeerHeartbeatCycle",
           "Peer Heartbeat Cycle{h}[s]",
          NA, "brown4"),
   list("enrpTakeoverExpiry",
           "Takeover Expiry{x}[s]",
          NA, "brown4"),
   list("enrpMaxTimeLastHeared",
           "Max Time Last Heared{l}[s]",
          NA, "brown4"),
   list("enrpMaxTimeNoResponse",
           "Max Time No Response{o}[s]",
          NA, "brown4"),

   list("registrarMentorDiscoveryTimeout",
           "Mentor Discovery Timeout{M}[s]",
          NA, "brown4"),
   list("registrarMaxIncrement",
           "Max Increment[1]",
          NA, "brown4"),

   list("calcAppPoolElementServerRegistrationLife",
           "Registration Life{L}[s]",
           NA, "black"),
   list("calcAppPoolUserServiceJobIntervalVariable",
           "Request Interval{i}[1]",
           NA, "black"),
   list("calcAppPoolElementSelectionPolicy",
           "Pool Policy{p}",
           NA, "black"),
   list("calcAppPoolElementServiceMaxJobs",
           "Max Requests{x}[1]",
           NA, "black"),
   list("calcAppPoolElementServiceRejectProbability",
           "Request Rejection Probability{a}[%]",
           "100.0 * data1$calcAppPoolElementServiceRejectProbability", "brown4"),
   list("calcAppPoolElementComponentCleanShutdownProbability",
           "Probability of a Clean Shutdown [%]",
           "100.0 * data1$calcAppPoolElementComponentCleanShutdownProbability", "brown4"),
   list("calcAppPoolElementComponentUptime",
           "Pool Element MTBF{M}[s]",
          NA, "brown4"),
   list("calcAppPoolElementComponentUptimeVariable-MTBF",
           "MTBF{M}[MTBF/(Req.Size/PE Capacity)]",
           "data1$calcAppPoolElementComponentUptimeVariable", "brown4"),
   list("calcAppPoolElementComponentUptimeVariable-MTBF-10",
           "MTBF{M}[MTBF/(Req.Size/PE Capacity=10)]",
           "data1$calcAppPoolElementComponentUptimeVariable", "brown4"),
   list("calcAppPoolUserServiceJobSizeDistribution",
           "Request Size Distribution{:delta:}",
           "data1$calcAppPoolUserServiceJobSizeDistribution", "brown4"),

   list("calcAppPoolUserASAPStaleCacheValue",
           "Stale Cache Value{c}[s]",
          NA, "brown4"),
   list("calcAppPoolUserServiceHandleResolutionRetryDelay",
           "HR Retry Delay{H}[s]",
          NA, "brown4"),

   list("staleCacheValueTojsToSC",
           "Stale Cache Value/(Request Size:PE Capacity){c}[1]",
           "data1$calcAppPoolUserASAPStaleCacheValue / (data1$calcAppPoolUserServiceJobSizeVariable / data1$calcAppPoolElementServiceCapacityVariable)",
           "black"),

   list("registrarMaxBadPEReports",
           "MaxBadPEReports{m}[1]",
          NA, "brown4"),
   list("registrarComponentUptimeVariable",
           "Registrar MTBF{M}[s]",
          NA, "brown4"),
   list("registrarMaxHandleResolutionRate",
           "Max Handle Resolution Rate{H}[1/s]",
          NA, "brown4"),
   list("registrarMaxEndpointUnreachableRate",
           "Max Endpoint Unreachable Rate{U}[1/s]",
          NA, "brown4"),

   list("scenarioNumberOfPoolElementsGammaDisasterScenario",
          "Disaster Area LAN{A}",
          "data1$scenarioNumberOfCalcAppPoolElementsGamma", "brown4"),
   list("scenarioNumberOfPoolElementsLambdaDisasterScenario",
          "Remaining Capacity in Disaster Area [%]",
          "100.0 * data1$scenarioNumberOfCalcAppPoolElementsLambda", "brown4"),
   list("calcAppPoolElementSelectionPolicyUpdateThreshold",
          "Update Threshold{T}[%]",
          "100.0 * data1$calcAppPoolElementSelectionPolicyUpdateThreshold", "brown4"),
   list("calcAppPoolElementSelectionPolicyLoadDPF",
           "Load DPF{l}[1/ms]",
           NA, "black"),
   list("calcAppPoolElementSelectionPolicyWeightDPF",
           "Weight DPF{w}[1/ms]",
           NA, "black"),

   list("calcAppPoolElementServiceCapacityGamma.FastServers",
           ":kappa: [1]",
           "data1$calcAppPoolElementServiceCapacityGamma", "black"),
   list("calcAppPoolElementServiceCapacityGamma.Linear",
           ":gamma: [1]",
           "data1$calcAppPoolElementServiceCapacityGamma", "black"),
   list("calcAppPoolElementServiceCapacityGamma.Uniform",
           ":gamma: [1]",
           "data1$calcAppPoolElementServiceCapacityGamma", "black"),
   list("calcAppPoolElementServiceCapacityGamma.TruncNormal",
           ":vartheta: [1]",
           "data1$calcAppPoolElementServiceCapacityGamma", "black"),
   list("calcAppProtocolServiceJobKeepAliveInterval",
           "Session Keep-Alive Interval [s]",
           NA, "black"),
  list("calcAppProtocolServiceJobKeepAliveTimeout",
           "Session Keep-Alive Timeout [s]",
           NA, "black"),

   list("attackerAttackInterval",
           "Attack Interval{A}[s]",
           NA, "black"),
   list("attackerAttackFrequency",
           "Attack Frequency{F}[1/s]",
           "1.0 / data1$attackerAttackInterval", "black"),
   list("attackReportUnreachableProbability",
           "Unreachable Probability{u}[%]",
           "100.0 * data1$attackReportUnreachableProbability", "black"),
   list("attackRandomizeIdentifier",
           "Randomize Identifier{RI}",
           NA, "black"),
   list("attackAnswerKeepAlive",
           "Answer Keep-Alive{AK}",
           NA, "black")
)


rspsim5DefaultConfiguration <- list(
   # ====== Special Settings ================================================
   list("SPECIAL0", ""),
   list("SPECIAL1", ""),
   list("SPECIAL2", ""),
   list("SPECIAL3", ""),
   list("SPECIAL4", ""),
   list("SPECIAL5", ""),
   list("SPECIAL6", ""),
   list("SPECIAL7", ""),
   list("SPECIAL8", ""),
   list("SPECIAL9", ""),

   # ====== Variables Settings ==============================================
   list("targetSystemUtilization", 0.80),
   list("puToPERatio", 10),

   # ====== Scenario Settings ===============================================
   list("scenarioNumberOfLANs", 1),
   list("scenarioNetworkWANDelayDistribution", "identityDelayDistribution"),
   list("scenarioNetworkWANDelayVariable", 0),
   list("scenarioNetworkWANDelayGamma", 0),
   list("scenarioNetworkWANDelayLambda", 0),
   list("scenarioNetworkLANDelayDistribution", "identityDelayDistribution"),
   list("scenarioNetworkLANDelayVariable", 0),
   list("scenarioNetworkLANDelayGamma", 0),
   list("scenarioNetworkLANDelayLambda", 0),
   list("scenarioNumberOfRegistrarsDistribution", "identityNumberDistribution"),
   list("scenarioNumberOfRegistrarsVariable", 1),
   list("scenarioNumberOfRegistrarsGamma", 0),
   list("scenarioNumberOfRegistrarsLambda", 0),
   list("scenarioNumberOfCalcAppPoolElementsDistribution", "identityNumberDistribution"),
   list("scenarioNumberOfCalcAppPoolElementsVariable", 10),
   list("scenarioNumberOfCalcAppPoolElementsGamma", 0),
   list("scenarioNumberOfCalcAppPoolElementsLambda", 0),
   list("scenarioNumberOfCalcAppPoolUsersDistribution", "identityNumberDistribution"),
   list("scenarioNumberOfCalcAppPoolUsersVariable", -1),   # --- To be computed! ---
   list("scenarioNumberOfCalcAppPoolUsersGamma", 0),
   list("scenarioNumberOfCalcAppPoolUsersLambda", 0),
   list("scenarioNumberOfAttackersDistribution", "identityNumberDistribution"),
   list("scenarioNumberOfAttackersVariable", 0),
   list("scenarioNumberOfAttackersGamma", 0),
   list("scenarioNumberOfAttackersLambda", 0),

   # ====== General Settings ================================================
   # ------ ASAP ------------------------------------------
   list("asapRequestTimeout", 5),
   list("asapMaxRequestRetransmit", 3),
   list("asapStaleCacheValue", 0),
   list("asapEndpointKeepAliveInterval", 50),
   list("asapEndpointKeepAliveTimeout", 50),
   list("asapServerHuntRetryDelay", "uniform(0ms, 200ms)"),
   list("asapNoServiceDuringStartup", "true"),
   list("asapUseTakeoverSuggestion", "false"),
   # ------ ENRP ------------------------------------------
   list("enrpPeerHeartbeatCycle", 30),
   list("enrpMaxTimeLastHeared", 61),
   list("enrpMaxTimeNoResponse", 5),
   list("enrpTakeoverExpiry", 30),
   # ------ CalcAppProtocol -------------------------------
   list("calcAppProtocolServiceJobKeepAliveInterval", 50),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 50),
   list("calcAppProtocolServiceJobRequestTimeout", 50),

   # ====== Registrar Settings ==============================================
   list("registrarTransportInterfaceUptimeDistribution", "timeIdentityDistribution"),
   list("registrarTransportInterfaceDowntimeDistribution", "timeIdentityDistribution"),
   list("registrarTransportInterfaceUptimeVariable", "100d"),
   list("registrarTransportInterfaceUptimeGamma", 0),
   list("registrarTransportInterfaceUptimeLambda", 0),
   list("registrarTransportInterfaceDowntimeVariable", "0s"),
   list("registrarTransportInterfaceDowntimeGamma", 0),
   list("registrarTransportInterfaceDowntimeLambda", 0),
   list("registrarComponentUptimeDistribution", "timeIdentityDistribution"),
   list("registrarComponentDowntimeDistribution", "timeIdentityDistribution"),
   list("registrarComponentUptimeVariable", "100d"),
   list("registrarComponentUptimeGamma", 0),
   list("registrarComponentUptimeLambda", 0),
   list("registrarComponentDowntimeVariable", "0s"),
   list("registrarComponentDowntimeGamma", 0),
   list("registrarComponentDowntimeLambda", 0),
   list("registrarMentorDiscoveryTimeout", 30),
   list("registrarMaxHandleResolutionItems", 1),
   list("registrarRandomizeMaxHandleResolutionItems", "false"),
   list("registrarMaxIncrement", 0),
   list("registrarMaxBadPEReports", 1000000000),
   list("registrarUpdateLossProbability", 0.0),
   list("registrarMaxEndpointUnreachableRate", -1.0),
   list("registrarEndpointUnreachableRateBuckets", 64),
   list("registrarEndpointUnreachableRateMaxEntries", 16),
   list("registrarMaxHandleResolutionRate", -1.0),
   list("registrarHandleResolutionRateBuckets", 64),
   list("registrarHandleResolutionRateMaxEntries", 16),

   # ====== Pool Element Settings ===========================================
   list("calcAppPoolElementTransportInterfaceUptimeDistribution", "timeIdentityDistribution"),
   list("calcAppPoolElementTransportInterfaceDowntimeDistribution", "timeIdentityDistribution"),
   list("calcAppPoolElementTransportInterfaceUptimeVariable", "100d"),
   list("calcAppPoolElementTransportInterfaceUptimeGamma", 0),
   list("calcAppPoolElementTransportInterfaceUptimeLambda", 0),
   list("calcAppPoolElementTransportInterfaceDowntimeVariable", "0s"),
   list("calcAppPoolElementTransportInterfaceDowntimeGamma", 0),
   list("calcAppPoolElementTransportInterfaceDowntimeLambda", 0),
   list("calcAppPoolElementComponentUptimeDistribution", "timeIdentityDistribution"),
   list("calcAppPoolElementComponentDowntimeDistribution", "timeIdentityDistribution"),
   list("calcAppPoolElementComponentUptimeVariable", "100d"),
   list("calcAppPoolElementComponentUptimeGamma", 0),
   list("calcAppPoolElementComponentUptimeLambda", 0),
   list("calcAppPoolElementComponentDowntimeVariable", "0s"),
   list("calcAppPoolElementComponentDowntimeGamma", 0),
   list("calcAppPoolElementComponentDowntimeLambda", 0),
   list("calcAppPoolElementComponentCleanShutdownProbability", 1.0),
   list("calcAppPoolElementStaticRegistrarList", ""),
   list("calcAppPoolElementServiceCapacityDistribution", "workloadIdentityDistribution"),
   list("calcAppPoolElementServiceCapacityVariable", 1000000),
   list("calcAppPoolElementServiceCapacityGamma", 0.0),
   list("calcAppPoolElementServiceCapacityLambda", 0.0),
   list("calcAppPoolElementServiceMaxJobs", 0),
   list("calcAppPoolElementServiceMinCapacityPerJob", 10000),
   list("calcAppPoolElementServiceRejectProbability", 0.0),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolElementSelectionPolicyWeight", -1),
   list("calcAppPoolElementSelectionPolicyLoadDegradation", -1),
   list("calcAppPoolElementSelectionPolicyLoadDPF", 0.0),
   list("calcAppPoolElementSelectionPolicyWeightDPF", 0.0),
   list("calcAppPoolElementSelectionPolicyUpdateThreshold", 0.0),
   list("calcAppPoolElementReregisterImmediatelyOnUpdate", "true"),
   list("calcAppPoolElementServerRegistrationLife", 300),
   list("calcAppPoolElementServerCookieMaxCalculations", 10000000),
   list("calcAppPoolElementServerCookieMaxTime", "100d"),

   # ====== Pool User Settings ==============================================
   list("calcAppPoolUserTransportInterfaceUptimeDistribution", "timeIdentityDistribution"),
   list("calcAppPoolUserTransportInterfaceDowntimeDistribution", "timeIdentityDistribution"),
   list("calcAppPoolUserTransportInterfaceUptimeVariable", "100d"),
   list("calcAppPoolUserTransportInterfaceUptimeGamma", 0),
   list("calcAppPoolUserTransportInterfaceUptimeLambda", 0),
   list("calcAppPoolUserTransportInterfaceDowntimeVariable", "0s"),
   list("calcAppPoolUserTransportInterfaceDowntimeGamma", 0),
   list("calcAppPoolUserTransportInterfaceDowntimeLambda", 0),
   list("calcAppPoolUserStaticRegistrarList", ""),
   list("calcAppPoolUserServiceJobSizeDistribution", "workloadExponentialRandomizedDistribution"),
   list("calcAppPoolUserServiceJobSizeVariable", 1e7),
   list("calcAppPoolUserServiceJobSizeGamma", 0.0),
   list("calcAppPoolUserServiceJobSizeLambda", 0),
   list("calcAppPoolUserServiceJobIntervalDistribution", "workloadExponentialRandomizedDistribution"),
   list("calcAppPoolUserServiceJobIntervalVariable"),
   list("calcAppPoolUserServiceJobIntervalGamma", 0),
   list("calcAppPoolUserServiceJobIntervalLambda", 0),
   list("calcAppPoolUserServiceHandleResolutionRetryDelay", 30),
   list("calcAppPoolUserServiceJobRetryDelay", "uniform(0ms,200ms)"),

   # ====== Attacker Settings ===============================================
   list("attackerTransportInterfaceUptimeDistribution", "timeIdentityDistribution"),
   list("attackerTransportInterfaceDowntimeDistribution", "timeIdentityDistribution"),
   list("attackerComponentUptimeDistribution", "timeIdentityDistribution"),
   list("attackerComponentDowntimeDistribution", "timeIdentityDistribution"),
   list("attackerTransportInterfaceUptimeVariable", "100d"),
   list("attackerTransportInterfaceUptimeGamma", 0),
   list("attackerTransportInterfaceUptimeLambda", 0),
   list("attackerTransportInterfaceDowntimeVariable", "0s"),
   list("attackerTransportInterfaceDowntimeGamma", 0),
   list("attackerTransportInterfaceDowntimeLambda", 0),
   list("attackerComponentUptimeVariable", "100d"),
   list("attackerComponentUptimeGamma", 0),
   list("attackerComponentUptimeLambda", 0),
   list("attackerComponentDowntimeVariable", "0s"),
   list("attackerComponentDowntimeGamma", 0),
   list("attackerComponentDowntimeLambda", 0),
   list("attackerStaticRegistrarList", ""),
   list("attackerAttackType", "None"),
   list("attackerAttackInterval", 1.0),
   list("attackTargetPolicyLoadDegradation", 0),
   list("attackTargetPolicyWeight", 4294967295.0),
   list("attackRandomizeIdentifier", "false"),
   list("attackAnswerKeepAlive", "false"),
   list("attackReportUnreachableProbability", 0.0)
)
