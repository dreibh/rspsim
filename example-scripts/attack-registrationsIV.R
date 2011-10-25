# ###########################################################################
# Name:        attack-registrationsIV
# Description: What happens in case of registration attacks?
#              (LoadDeg is 25%, Attacker 0%, MinCapPerReq=250000)
#              Parameters corresponding to lab setup (50%, delay, PU:PE ratio 3)
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "attack-registrationsIV"
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

   list("registrarMaxBadPEReports", 3),   # !!!
   list("registrarMaxEndpointUnreachableRate", -1),
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
   list("attackerAttackInterval", 1, 1.2, 1.42, 1.7, 2, 2.5, 3, 3.75, 5, 6.25, 7.5, 8.25, 10, 15, 20, 25, 30, 40, 50, 10000),

   list("attackTargetPolicyWeight", 4294967295),
   list("attackRandomizeIdentifier", "true"),    # !!!
   list("attackAnswerKeepAlive", "true"),   # !!!
   list("attackReportUnreachableProbability", 0.0)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
