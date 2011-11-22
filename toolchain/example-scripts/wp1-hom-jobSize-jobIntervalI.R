# ###########################################################################
# Name:        wp1-hom-jobSize-jobIntervalI
# Description: Variation of request size and request interval
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-hom-jobSize-jobIntervalI"
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
   list("puToPERatio"),   # --- To be computed ---
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolUserServiceJobSizeVariable", 1.0e6, 5.0e6, 1.0e7, 2.5e7, 5.0e7, 7.5e7, 1.0e8),
   list("calcAppPoolUserServiceJobIntervalVariable", 50, 500)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
