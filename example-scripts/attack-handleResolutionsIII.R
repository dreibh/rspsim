# ###########################################################################
# Name:        attack-handleResolutionsIII
# Description: Handle resolution attack
#              Parameters corresponding to lab setup (50%, delay, PU:PE ratio 3)
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "attack-handleResolutionsIII"
simulationRuns <- 24
simulationDuration <- 120
simulationStoreVectors <- FALSE
simulationExecuteMake <- TRUE
simulationScriptOutputVerbosity <- 3
simulationSummaryCompressionLevel <- 9
simulationSummarySkipList <- c("lan.")
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

   list("calcAppPoolElementServerRegistrationLife", 30),   # !!!
   list("calcAppPoolElementServiceCapacityVariable", 1000000),   # !!!
   list("calcAppPoolElementServiceMinCapacityPerJob", 100000),   # !!!

   list("registrarMaxBadPEReports", 3),
   list("registrarMaxEndpointUnreachableRate", -1.0),
   list("registrarEndpointUnreachableRateBuckets", 64),
   list("registrarEndpointUnreachableRateMaxEntries", 16),
   list("registrarMaxHandleResolutionRate", -1.0),
   list("registrarHandleResolutionRateBuckets", 64),
   list("registrarHandleResolutionRateMaxEntries", 16),

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "LeastUsedDegradation", "Random", "RoundRobin"),
   list("calcAppPoolElementServerRegistrationLife", 30),   # !!!

   list("calcAppPoolUserServiceJobSizeVariable", 1e7),   # !!!

   list("attackerAttackType", "HandleResolution"),
   list("attackerAttackInterval", 1000, 5, 1, 0.75, 0.5, 0.375, 0.30, 0.25, 0.20, 0.15, 0.125, 0.1),
   list("attackTargetPolicyLoadDegradation", 0),
   list("attackTargetPolicyWeight", 4294967295),
   list("attackRandomizeIdentifier", "false"),
   list("attackAnswerKeepAlive", "false"),
   list("attackReportUnreachableProbability", 0.0, 1.0)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
