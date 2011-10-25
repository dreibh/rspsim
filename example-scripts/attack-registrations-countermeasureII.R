# ###########################################################################
# Name:        attack-registrations-countermeasureII
# Description: Countermeasure for registration attacks:
#              only a single PE per user (by fixed user-specific PE ID)
#              Parameters corresponding to lab setup (50%, delay, PU:PE ratio 3)
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "attack-registrations-countermeasureII"
simulationRuns <- 24
simulationDuration <- 120
simulationStoreVectors <- FALSE
simulationExecuteMake <- TRUE
simulationScriptOutputVerbosity <- 3
simulationSummaryCompressionLevel <- 9
simulationSummarySkipList <- c("lan.calcApp", "lan,registrar", "lan.switch")
# -------------------------------------
source("computation-pool.R")
# -------------------------------------

# ###########################################################################

simulationConfigurations <- list(
   list("targetSystemUtilization", 0.50),   # !!!
   list("puToPERatio", 3),

   list("scenarioNetworkLANDelayVariable", 25),   # !!!
   list("scenarioNetworkLANDelayDistribution", "uniformDelayDistribution"),  # !!!

   list("scenarioNumberOfAttackersVariable", 1),

   list("calcAppProtocolServiceJobKeepAliveInterval", 10),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 10),
   list("calcAppProtocolServiceJobRequestTimeout", 10),

   list("registrarMaxBadPEReports", 3),   # !!!
   list("registrarMaxEndpointUnreachableRate", -1),   # !!! Alte Werte: 1, 100 !!!
   list("registrarEndpointUnreachableRateBuckets", 64),
   list("registrarEndpointUnreachableRateMaxEntries", 16),
   list("registrarMaxHandleResolutionRate", -1),
   list("registrarHandleResolutionRateBuckets", 64),
   list("registrarHandleResolutionRateMaxEntries", 16),

   list("calcAppPoolElementServiceMinCapacityPerJob", 250000),   # !!! Für LoadDeg wichtig!
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "LeastUsedDegradation", "Random", "RoundRobin"),
   list("calcAppPoolElementServerRegistrationLife", 30),

   list("calcAppPoolUserServiceJobSizeVariable", 1e7),

   list("attackerAttackType", "Registration"),
   list("attackerAttackInterval", 0.001, 0.01, 0.5, 1, 5, 10, 15, 20, 25, 30, 37.5, 43.75, 50),
   list("attackTargetPolicyLoadDegradation", 0.25),
   list("attackTargetPolicyWeight", 4294967295),
   list("attackRandomizeIdentifier", "false"),   # !!!
   list("attackAnswerKeepAlive", "true"),
   list("attackReportUnreachableProbability", 0.0)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
