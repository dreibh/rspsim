# ###########################################################################
# Name:        wp2-shutdownI
# Description: The impact of unclean shutdowns
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp2-shutdownI"
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
   list("calcAppProtocolServiceJobKeepAliveInterval", 1, 10, 100),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 1),
   list("calcAppProtocolServiceJobRequestTimeout"),   # --- same as calcAppProtocolServiceJobKeepAliveTimeout

   list("registrarMaxBadPEReports", 1),

   list("calcAppPoolElementComponentUptimeDistribution", "timePEmtbfFromMTBFJobSizePECapacityRatioDistribution"),
   list("calcAppPoolElementComponentUptimeVariable", 5),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolElementComponentCleanShutdownProbability", 0.0, 0.125, 0.25, 0.375, 0.50, 0.625, 0.75, 0.875, 1.00),

   list("calcAppPoolUserServiceJobSizeVariable", 1e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
