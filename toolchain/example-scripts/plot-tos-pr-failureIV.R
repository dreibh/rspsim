# ###########################################################################
# Name:        tos-pr-failureIV
# Description: Unreliable PRs
# ###########################################################################

source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "tos-pr-failureIV"
plotColorMode        <- cmColor
plotHideLegend       <- FALSE
plotLegendSizeFactor <- 0.8
plotOwnOutput        <- FALSE
plotFontFamily       <- "Helvetica"
plotFontPointsize    <- 22
plotWidth            <- 10
plotHeight           <- 10
plotConfidence       <- 0.95

# ###########################################################################

# ------ Plots --------------------------------------------------------------
plotConfigurations <- list(
   # ------ Format example --------------------------------------------------
   # list(simulationDirectory, "output.pdf",
   #      "Plot Title",
   #      list(xAxisTicks) or NA, list(yAxisTicks) or NA, list(legendPos) or NA,
   #      "x-Axis Variable", "y-Axis Variable",
   #      "z-Axis Variable", "v-Axis Variable", "w-Axis Variable",
   #      "a-Axis Variable", "b-Axis Variable", "p-Axis Variable")
   # ------------------------------------------------------------------------

   list(simulationDirectory, paste(sep="", simulationDirectory, "-Utilization.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "registrarComponentUptimeVariable", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicy", "", "",
        "asapUseTakeoverSuggestion", "", ""),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", NA, NA, list(0.5,0.5),
        "registrarComponentUptimeVariable", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicy", "", "",
        "asapUseTakeoverSuggestion", "", ""),


   list(simulationDirectory, paste(sep="", simulationDirectory, "-OwnedPoolElements.pdf"),
        "Providers's Perspective", NA, NA, list(1,1),
        "registrarComponentUptimeVariable", "lan.registrarArray.registrarProcess.RegistrarAverageNumberOfOwnedPoolElements",
        "lan.registrarArray", "", "",
        "asapUseTakeoverSuggestion", "", "",
        "TRUE"),   # !!! Note: ID=Number-1 !!!


   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarTotalEndpointKeepAlivesSent.pdf"),
        "Providers's Perspective", NA, NA, list(1,1),
        "registrarComponentUptimeVariable", "lan.registrarArray.registrarProcess.RegistrarTotalEndpointKeepAlivesSent",
        "lan.registrarArray", "", "",
        "asapUseTakeoverSuggestion", "", "",
        "TRUE"),   # !!! Note: ID=Number-1 !!!
   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarTotalHandleUpdates.pdf"),
        "Providers's Perspective", NA, NA, list(1,1),
        "registrarComponentUptimeVariable", "lan.registrarArray.registrarProcess.RegistrarTotalHandleUpdates",
        "calcAppPoolElementSelectionPolicy", "lan.registrarArray", "",
        "asapUseTakeoverSuggestion", "", "",
        "(data1$lan.registrarArray == 0)|(data1$lan.registrarArray == 4)"),   # !!! Note: ID=Number-1 !!!
   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarTotalRegistrations.pdf"),
        "Providers's Perspective", NA, NA, list(1,1),
        "registrarComponentUptimeVariable", "lan.registrarArray.registrarProcess.RegistrarTotalRegistrations",
        "calcAppPoolElementSelectionPolicy", "lan.registrarArray", "",
        "asapUseTakeoverSuggestion", "", "",
        "(data1$lan.registrarArray == 0)|(data1$lan.registrarArray == 4)"),   # !!! Note: ID=Number-1 !!!


   # ------ Takeover (independent of policy!) ------
   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarGlobalTakeoversStarted.pdf"),
        "Providers's Perspective", NA, NA, list(0.5,0.5),
        "registrarComponentUptimeVariable", "controller.RegistrarGlobalTakeoversStarted",
        "", "", "",
        "asapUseTakeoverSuggestion", "", ""),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarGlobalTakeoversByTimeout.pdf"),
        "Providers's Perspective", NA, NA, list(0.5,0.5),
        "registrarComponentUptimeVariable", "controller.RegistrarGlobalTakeoversByTimeout",
        "", "", "",
        "asapUseTakeoverSuggestion", "", ""),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarGlobalTakeoversByConsent.pdf"),
        "Providers's Perspective", NA, NA, list(0.5,0.5),
        "registrarComponentUptimeVariable", "controller.RegistrarGlobalTakeoversByConsent",
        "", "", "",
        "asapUseTakeoverSuggestion", "", ""),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarTotalTakeoversByConsent.pdf"),
        "Providers's Perspective", NA, NA, list(1,1),
        "registrarComponentUptimeVariable", "lan.registrarArray.registrarProcess.RegistrarTotalTakeoversByConsent",
        "lan.registrarArray", "", "",
        "asapUseTakeoverSuggestion", "", "",
        "TRUE"),   # !!! Note: ID=Number-1 !!!
   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarTotalTakeoversByTimeout.pdf"),
        "Providers's Perspective", NA, NA, list(1,1),
        "registrarComponentUptimeVariable", "lan.registrarArray.registrarProcess.RegistrarTotalTakeoversByTimeout",
        "lan.registrarArray", "", "",
        "asapUseTakeoverSuggestion", "", "",
        "TRUE"),   # !!! Note: ID=Number-1 !!!
   list(simulationDirectory, paste(sep="", simulationDirectory, "-RegistrarTotalTakeoversStarted.pdf"),
        "Providers's Perspective", NA, NA, list(1,1),
        "registrarComponentUptimeVariable", "lan.registrarArray.registrarProcess.RegistrarTotalTakeoversStarted",
        "lan.registrarArray", "", "",
        "asapUseTakeoverSuggestion", "", "",
        "TRUE")   # !!! Note: ID=Number-1 !!!
)


# ------ Variable templates -------------------------------------------------
plotVariables <- append(list(
   # ------ Format example --------------------------------------------------
   # list("Variable",
   #         "Unit[x]{v]"
   #          "100.0 * data1$x / data1$y", <- Manipulator expression:
   #                                           "data" is the data table
   #                                        NA here means: use data1$Variable.
   #          "myColor",
   #          list("InputFile1", "InputFile2", ...))
   #             (simulationDirectory/Results/....data.tar.bz2 is added!)
   # ------------------------------------------------------------------------

   # list("controller.SystemAverageUtilization",
   #         "Average Utilization[%]",
   #         "100.0 * data1$controller.SystemAverageUtilization",
   #         "blue4",
   #         list("controller-SystemAverageUtilization"))

), rspsim5PlotVariables)

# ###########################################################################

createPlots(simulationDirectory, plotConfigurations)
