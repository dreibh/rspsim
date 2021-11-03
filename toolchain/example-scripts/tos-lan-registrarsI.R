# ###########################################################################
# Name:        tos-lan-registrarsI
# Description: Varying the number of registrars (LAN setup)
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "tos-lan-registrarsI"
simulationRuns <- 12
simulationDuration <- 10
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
   list("scenarioNumberOfRegistrarsVariable", 1, 2, 3, 4, 5),
   list("scenarioNumberOfCalcAppPoolElementsVariable", 100),   # !!!
   list("scenarioNetworkLANDelayVariable", 0, 10, 25),
   list("scenarioNetworkLANDelayDistribution", "uniformDelayDistribution"),   # !!!

   list("asapUseTakeoverSuggestion", "false"),

   list("registrarRandomizeMaxHandleResolutionItems", "false"),
   list("registrarMaxHandleResolutionItems", 1),
   list("registrarMaxIncrement", 1),

   # --- list("calcAppPoolElementStaticRegistrarList", "-", "1"),

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "RoundRobin", "Random"),
   list("calcAppPoolUserServiceJobSizeVariable", 1e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
