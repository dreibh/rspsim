# ###########################################################################
# Name:        wp1-hom-jobInterval-puToPERatioI
# Description: Variation of request interval and PU:PE ratio
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-hom-jobInterval-puToPERatioI"
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
   list("puToPERatio", 1, 3),
   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolUserServiceJobSizeVariable"),   # --- To be computed ---
   list("calcAppPoolUserServiceJobIntervalVariable", 1, 2, 3, 5, 10, 25, 50, 75, 100, 150, 200, 300, 400, 500)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
