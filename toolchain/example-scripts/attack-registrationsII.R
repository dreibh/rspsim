# ###########################################################################
# Name:        attack-registrationsII
# Description: What happens in case of registration attacks?
#              (LoadDeg is 1%, Attacker: 0%, MinCapPerReq=10000)
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "attack-registrationsII"
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

   list("registrarMaxBadPEReports", 1, 3, 10),   # !!!
   list("registrarMaxEndpointUnreachableRate", -1),
   list("registrarEndpointUnreachableRateBuckets", 64),
   list("registrarEndpointUnreachableRateMaxEntries", 16),
   list("registrarMaxHandleResolutionRate", -1),
   list("registrarHandleResolutionRateBuckets", 64),
   list("registrarHandleResolutionRateMaxEntries", 16),

   list("calcAppPoolElementServiceMinCapacityPerJob", 10000),   # !!! Für LoadDeg wichtig!
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "LeastUsedDegradation", "Random", "RoundRobin"),
   list("calcAppPoolElementServerRegistrationLife", 30),

   list("calcAppPoolUserServiceJobSizeVariable", 1e7),

   list("attackerAttackType", "Registration"),
   list("attackerAttackInterval", 0.5, 1, 5, 10, 15, 25, 37.5, 50),
   list("attackTargetPolicyLoadDegradation", 0),
   list("attackTargetPolicyWeight", 4294967295),
   list("attackRandomizeIdentifier", "true"),    # !!!
   list("attackAnswerKeepAlive", "true"),   # !!!
   list("attackReportUnreachableProbability", 0.0)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
