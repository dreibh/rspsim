# ###########################################################################
# Name:        lud-puToPERatioI.R
# Description: LUD performance for variation of PU:PE ratio
# ###########################################################################

source("simulate-version11.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "lud-puToPERatioI"
simulationRuns <- 12
simulationDuration <- 120
# function() {   # Calculation run duration in *minutes*:
#    return(10 * (as.numeric(calcAppPoolUserServiceJobSizeVariable) / as.numeric(calcAppPoolElementServiceCapacityVariable)))
# }
simulationStoreVectors <- FALSE
simulationExecuteMake <- TRUE
simulationScriptOutputVerbosity <- 8
simulationSummaryCompressionLevel <- 3   # !!!
simulationSummarySkipList <- c("calcAppQueuingClient", "calcAppServer", "registrar", "transportNode", "switch")

# ###########################################################################

simulationConfigurations <- list(
   # ====== Variables Settings ==============================================
   list("targetSystemUtilization", 0.8),   # !!!
   list("puToPERatio", 1, 3, 10, 15, 25, 50, 100),   # !!!

   # ====== Scenario Settings ===============================================
   list("scenarioNumberOfLANs", 1),   # !!!!
   list("scenarioNetworkWANDelayDistribution", "identityDelayDistribution"),
   list("scenarioNetworkWANDelayVariable", 0),
   list("scenarioNetworkWANDelayGamma", 0),
   list("scenarioNetworkWANDelayLambda", 0),
   list("scenarioNetworkLANDelayDistribution", "identityDelayDistribution"),
   list("scenarioNetworkLANDelayVariable", 500),
   list("scenarioNetworkLANDelayGamma", 0),
   list("scenarioNetworkLANDelayLambda", 0),
   list("scenarioNumberOfRegistrarsDistribution", "identityAmountDistribution"),
   list("scenarioNumberOfRegistrarsVariable", 1),   # !!!!
   list("scenarioNumberOfRegistrarsGamma", 0),
   list("scenarioNumberOfRegistrarsLambda", 0),
   list("scenarioNumberOfCalcAppPoolElementsDistribution", "identityAmountDistribution"),
   list("scenarioNumberOfCalcAppPoolElementsVariable", 10),
   list("scenarioNumberOfCalcAppPoolElementsGamma", 0),
   list("scenarioNumberOfCalcAppPoolElementsLambda", 0),
   list("scenarioNumberOfCalcAppPoolUsersDistribution", "identityAmountDistribution"),
   list("scenarioNumberOfCalcAppPoolUsersVariable", -1),   # --- To be computed! ---
   list("scenarioNumberOfCalcAppPoolUsersGamma", 0),
   list("scenarioNumberOfCalcAppPoolUsersLambda", 0),
   list("scenarioNumberOfAttackersDistribution", "identityAmountDistribution"),
   list("scenarioNumberOfAttackersVariable", 0),
   list("scenarioNumberOfAttackersGamma", 0),
   list("scenarioNumberOfAttackersLambda", 0),

   # ====== General Settings ================================================
   # ------ ASAP ------------------------------------------
   list("asapRequestTimeout", 5),
   list("asapMaxRequestRetransmit", 1),   # !!!!
   list("asapStaleCacheValue", 0),
   list("asapEndpointKeepAliveInterval", 5),   # !!!!
   list("asapEndpointKeepAliveTimeout", 5),   # !!!!
   list("asapServerHuntRetryDelay", "uniform(0ms, 200ms)"),   # !!!!
   list("asapNoServiceDuringStartup", "true"),   # !!!!
   # ------ ENRP ------------------------------------------
   list("enrpPeerHeartbeatCycle", 30),
   list("enrpMaxTimeLastHeared", 61),
   list("enrpMaxTimeNoResponse", 5),
   list("enrpTakeoverExpiry", 30),
   # ------ CalcAppProtocol -------------------------------
   list("calcAppProtocolServiceJobKeepAliveInterval", 10),   # !!!!
   list("calcAppProtocolServiceJobKeepAliveTimeout", 10),   # !!!!
   list("calcAppProtocolServiceJobRequestTimeout", 10),   # !!!!

   # ====== Registrar Settings ==============================================
   list("registrarTransportInterfaceUptime", "999d"),
   list("registrarTransportInterfaceDowntime", "0s"),
   list("registrarComponentUptime", "999d"),
   list("registrarComponentDowntime", "0s"),
   list("registrarMentorDiscoveryTimeout", 30),
   list("registrarMaxHandleResolutionItems", 1),   # !!!
   list("registrarRandomizeMaxHandleResolutionItems", "false"),
   list("registrarMaxIncrement", 0),
   list("registrarMaxBadPEReports", 3),
   list("registrarUpdateLossProbability", 0.0),

   # ====== Pool Element Settings ===========================================
   list("calcAppPoolElementTransportInterfaceUptime", "999d"),
   list("calcAppPoolElementTransportInterfaceDowntime", "0s"),
   list("calcAppPoolElementComponentUptime", "999d"),
   list("calcAppPoolElementComponentDowntime", "0s"),
   list("calcAppPoolElementComponentCleanShutdownProbability", 1.0),
   list("calcAppPoolElementServiceCapacityDistribution", "poolElementServiceCapacityIdentityDistribution"),
   list("calcAppPoolElementServiceCapacityVariable", 1000000),
   list("calcAppPoolElementServiceCapacityGamma", 0.0),
   list("calcAppPoolElementServiceCapacityLambda", 0.0),
   list("calcAppPoolElementServiceMaxJobs", 0),
   list("calcAppPoolElementServiceMinCapacityPerJob", 10000),
   list("calcAppPoolElementServiceRejectProbability", 0.0),
   list("calcAppPoolElementSelectionPolicy", "LeastUsedDegradation", "LeastUsed"),
   list("calcAppPoolElementSelectionPolicyWeight", -1),   # --- Automatic ---
   list("calcAppPoolElementSelectionPolicyLoadDegradation", 0.2),   # !!!
   list("calcAppPoolElementSelectionPolicyLoadDPF", 0.0),
   list("calcAppPoolElementSelectionPolicyWeightDPF", 0.0),
   list("calcAppPoolElementSelectionPolicyUpdateThreshold", 0.0),
   list("calcAppPoolElementReregisterImmediatelyOnUpdate", "true"),
   list("calcAppPoolElementServerRegistrationLife", 30),   # !!!
   list("calcAppPoolElementServerCookieMaxCalculations", 10000000),
   list("calcAppPoolElementServerCookieMaxTime", "999d"),

   # ====== Pool User Settings ==============================================
   list("calcAppPoolUserTransportInterfaceUptime", "999d"),
   list("calcAppPoolUserTransportInterfaceDowntime", "0s"),
   list("calcAppPoolUserServiceJobSizeDistribution", "poolUserServiceJobSizeExponentialRandomizedDistribution"),
   list("calcAppPoolUserServiceJobSizeVariable", 2.5e5, 1e6, 5e6, 1e7),   # !!!
   list("calcAppPoolUserServiceJobSizeGamma", 0.0),
   list("calcAppPoolUserServiceJobSizeLambda", 0),
   list("calcAppPoolUserServiceJobIntervalDistribution", "poolUserServiceJobIntervalExponentialRandomizedDistribution"),
   list("calcAppPoolUserServiceJobIntervalVariable"),
   list("calcAppPoolUserServiceJobIntervalGamma", 0),
   list("calcAppPoolUserServiceJobIntervalLambda", 0),
   list("calcAppPoolUserServiceHandleResolutionRetryDelay", 30),
   list("calcAppPoolUserServiceJobRetryDelay", "uniform(0ms,200ms)"),

   # ====== Attacker Settings ===============================================
   list("attackerTransportInterfaceUptime", "999d"),
   list("attackerTransportInterfaceDowntime", "0s"),
   list("attackerComponentUptime", "999d"),
   list("attackerComponentDowntime", "0s"),
   list("attackerAttackType", "None"),
   list("attackerAttackInterval", 1.0),
   list("attackTargetPolicyLoadDegradation", 0),
   list("attackTargetPolicyWeight", 4294967295),
   list("attackRandomizeIdentifier", "false"),
   list("attackAnswerKeepAlive", "false", "true"),
   list("attackReportUnreachableProbability", 0.0)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations)
