# ###########################################################################
# Name:        wp2-dynpool-mtbfI
# Description: The implications of a dynamic pool
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp2-dynpool-mtbfI"
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

   list("scenarioNetworkLANDelayVariable", 0, 10, 50),

   list("calcAppPoolElementComponentUptimeDistribution", "timePEmtbfFromMTBFJobSizePECapacityRatioDistribution"),
   list("calcAppPoolElementComponentUptimeVariable", 5, 2, 3.75, 2.5, 2, 1.5, 1, 0.5, 0.25, 0.10),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),

   list("calcAppPoolUserServiceJobSizeVariable", 1e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
