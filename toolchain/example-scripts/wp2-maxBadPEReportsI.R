# ###########################################################################
# Name:        wp2-maxBadPEReportsI
# Description: Reducing monitoring overhead with MaxBadPEReports
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp2-maxBadPEReportsI"
simulationRuns <- 24
simulationDuration <- 60
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
   list("targetSystemUtilization", 0.25),
   list("puToPERatio", 10),

   list("scenarioNetworkLANDelayVariable", 10),

   list("asapEndpointKeepAliveInterval", 1, 5, 30),
   list("asapEndpointKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobKeepAliveInterval", 1),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobRequestTimeout"),   # --- same as calcAppProtocolServiceJobKeepAliveTimeout

   list("registrarMaxBadPEReports", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10),

   list("calcAppPoolElementComponentUptimeDistribution", "timePEmtbfFromMTBFJobSizePECapacityRatioDistribution"),
   list("calcAppPoolElementComponentUptimeVariable", 5),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolElementComponentCleanShutdownProbability", 0.0),

   list("calcAppPoolUserServiceJobSizeVariable", 1e6)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
