# ###########################################################################
# Name:        wp1-cache-hom-jobInterval-puToPERatioI
# Description: The impact of the PU-side cache
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-cache-hom-jobInterval-puToPERatioI"
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
   list("puToPERatio", 1, 3),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolUserServiceJobSizeVariable"),   # --- To be computed ---
   list("calcAppPoolUserServiceJobIntervalVariable", 1),
   list("asapStaleCacheValue", 0, 0.5, 1.0, 1.5, 2.0, 3.0, 5.0, 7.5, 10.0)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
