# ###########################################################################
# Name:        wp2-endpointkeepaliveI
# Description: Varying the ASAP Endpoint Keep-Alive interval
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp2-endpointkeepaliveI"
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

   list("asapEndpointKeepAliveInterval", 1, 2, 3.5, 5, 7.5, 10, 15, 22.5, 30),
   list("asapEndpointKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobKeepAliveInterval", 1),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobRequestTimeout"),   # --- same as calcAppProtocolServiceJobKeepAliveTimeout

   list("registrarMaxBadPEReports", 1e9),

   list("calcAppPoolElementComponentUptimeDistribution", "timePEmtbfFromMTBFJobSizePECapacityRatioDistribution"),
   list("calcAppPoolElementComponentUptimeVariable", 2, 5, 10),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolElementComponentCleanShutdownProbability", 0.0),

   list("calcAppPoolUserServiceJobSizeVariable", 1e6)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
