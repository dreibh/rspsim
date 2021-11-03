# ###########################################################################
# Name:        wp2-cookiesI
# Description: Using "Client-Based State Sharing" as failover mechanism
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp2-cookiesI"
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
   list("targetSystemUtilization", 0.60),
   list("puToPERatio", 10),

   list("scenarioNetworkLANDelayVariable", 10),

   list("asapEndpointKeepAliveInterval", 1),
   list("asapEndpointKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobKeepAliveInterval", 1),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobRequestTimeout"),   # --- same as calcAppProtocolServiceJobKeepAliveTimeout

   list("registrarMaxBadPEReports", 1),

   list("calcAppPoolElementComponentUptimeDistribution", "timePEmtbfFromMTBFJobSizePECapacityRatioDistribution"),
   list("calcAppPoolElementComponentUptimeVariable", 2, 5, 10),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolElementComponentCleanShutdownProbability", 0.0),

   list("calcAppPoolElementServerCookieMaxCalculations", 1.0e4, 1.0e5, 5.0e5, 1.0e6, 1.75e6, 2.5e6, 3.75e6, 5.0e6, 7.5e6, 1.0e7),
   list("calcAppPoolElementServerCookieMaxTime", "100d"),

   list("calcAppPoolUserServiceJobSizeVariable", 1e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
