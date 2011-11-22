# ###########################################################################
# Name:        tos-lan-jobSizeI
# Description: Varying the number of registrars (LAN setup)
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "tos-lan-jobSizeI"
simulationRuns <- 36
simulationDuration <- 120
simulationStoreVectors <- FALSE
simulationExecuteMake <- TRUE
simulationScriptOutputVerbosity <- 3
simulationSummaryCompressionLevel <- 9
simulationSummarySkipList <- c("lan.calcApp", "lan.switch", "lan.attacker", "lan.registrarArray.transport")
# -------------------------------------
source("computation-pool.R")
# -------------------------------------

# ###########################################################################

simulationConfigurations <- list(
   list("puToPERatio", 10),
   list("scenarioNumberOfRegistrarsVariable", 5),
   list("scenarioNumberOfCalcAppPoolElementsVariable", 25),
   list("scenarioNetworkLANDelayVariable", 0, 150),
   list("scenarioNetworkLANDelayDistribution", "uniformDelayDistribution"),   # !!!

   list("asapUseTakeoverSuggestion", "false", "true"),

   list("registrarRandomizeMaxHandleResolutionItems", "false"),
   list("registrarMaxHandleResolutionItems", 1),
   list("registrarMaxIncrement", 1),

   list("calcAppPoolElementStaticRegistrarList", "1"),   # !!!

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "RoundRobin", "Random"),
   list("calcAppPoolUserServiceJobSizeVariable", 1e6, 2.5e6, 5e6, 7.5e6, 1e7, 1.5e7, 2.0e7, 2.5e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
