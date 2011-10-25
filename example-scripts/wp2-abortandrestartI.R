# ###########################################################################
# Name:        wp2-abortandrestartI
# Description: Using "Abort and Restart" as failover mechanism
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp2-abortandrestartI"
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

   list("asapEndpointKeepAliveInterval", 1),
   list("asapEndpointKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobKeepAliveInterval", 1),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobRequestTimeout"),   # --- same as calcAppProtocolServiceJobKeepAliveTimeout

   list("registrarMaxBadPEReports", 1),

   list("calcAppPoolElementComponentUptimeDistribution", "timePEmtbfFromMTBFJobSizePECapacityRatioDistribution"),
   list("calcAppPoolElementComponentUptimeVariable", 2, 5, 100),
   list("calcAppPoolElementComponentUptimeGamma", 1e7),   # !!! Base job size = 1e7.

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolElementComponentCleanShutdownProbability", 0.0),

   list("calcAppPoolElementServerCookieMaxCalculations", 9.9e99),
   list("calcAppPoolElementServerCookieMaxTime", "100d"),

   list("calcAppPoolUserServiceJobSizeVariable", 1e6, 2.5e6, 5e6, 7.5e6, 1e7, 1.25e7, 1.5e7, 1.75e7, 2.5e7, 3.75e7, 5e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
