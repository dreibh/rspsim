# ###########################################################################
# Name:        attack-handleResolutionsI
# Description: Handle resolution attack
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "attack-handleResolutionsI"
simulationRuns <- 48
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

   list("calcAppProtocolServiceJobKeepAliveInterval", 10),   # !!!
   list("calcAppProtocolServiceJobKeepAliveTimeout", 10),   # !!!
   list("calcAppProtocolServiceJobRequestTimeout", 10),   # !!!

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
   list("attackerAttackInterval", 10.0, 8.75, 7.5, 5.0, 2.5, 1.0, 0.5, 0.25, 0.1),
   list("attackTargetPolicyLoadDegradation", 0),
   list("attackTargetPolicyWeight", 4294967295),
   list("attackRandomizeIdentifier", "false"),
   list("attackAnswerKeepAlive", "false"),
   list("attackReportUnreachableProbability", 0.0, 1.0)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
