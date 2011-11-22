# ###########################################################################
# Name:        wp1-reject-delay-cache-puToPERatio-jobIntervalI
# Description: Improving performance using the PU-side cache
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-reject-delay-cache-puToPERatio-jobIntervalI"
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
   list("targetSystemUtilization", 0.60),
   list("scenarioNetworkLANDelayVariable", 100),
   list("puToPERatio", 3),
   list("calcAppPoolElementServiceRejectProbability", 0.00, 0.05, 0.10, 0.20),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random"),
   list("calcAppPoolUserServiceJobSizeVariable"),   # --- To be computed ---
   list("calcAppPoolUserServiceJobIntervalVariable", 1),
   list("asapStaleCacheValue", 0.00, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.75, 1.00) 
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
