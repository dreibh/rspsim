# ###########################################################################
# Name:        attack-handleResolutionsII
# Description: Handle Resolution attack with Endpoint Unreachable
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "attack-handleResolutionsII"
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
   list("puToPERatio", 10),

   list("scenarioNumberOfAttackersVariable", 1),

   list("calcAppProtocolServiceJobKeepAliveInterval", 10),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 10),
   list("calcAppProtocolServiceJobRequestTimeout", 10),

   list("registrarMaxBadPEReports", 3),
   list("registrarUpdateLossProbability", 0.0),
   list("registrarMaxEndpointUnreachableRate", -1),
   list("registrarEndpointUnreachableRateBuckets", 64),
   list("registrarEndpointUnreachableRateMaxEntries", 16),
   list("registrarMaxHandleResolutionRate", -1),
   list("registrarHandleResolutionRateBuckets", 64),
   list("registrarHandleResolutionRateMaxEntries", 16),

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "LeastUsedDegradation", "Random", "RoundRobin"),
   list("calcAppPoolElementServerRegistrationLife", 30),

   list("calcAppPoolUserServiceJobSizeVariable", 1e7),

   list("attackerAttackType", "HandleResolution"),
   list("attackerAttackInterval", 0.1),
   list("attackTargetPolicyLoadDegradation", 0),
   list("attackTargetPolicyWeight", 4294967295),
   list("attackRandomizeIdentifier", "false"),
   list("attackAnswerKeepAlive", "false"),
   list("attackReportUnreachableProbability", 0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.80)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
