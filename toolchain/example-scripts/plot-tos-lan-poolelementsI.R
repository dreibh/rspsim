# ###########################################################################
# Name:        tos-lan-poolelementsI
# Description: Varying the number of PEs (LAN setup)
# Revision:    $Id$
# ###########################################################################


source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "tos-lan-poolelementsI"
plotColorMode        <- cmColor
plotHideLegend       <- FALSE
plotLegendSizeFactor <- 0.8
plotOwnOutput        <- FALSE
plotFontFamily       <- "Helvetica"
plotFontPointsize    <- 22
plotWidth            <- 40
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
        "Provider's Perspective", NA, NA, list(1,0),
        "scenarioNumberOfRegistrarsVariable", "controller.SystemAverageUtilization",
        "scenarioNumberOfCalcAppPoolElementsVariable", "scenarioNetworkLANDelayVariable", "",
        "calcAppPoolElementSelectionPolicy", ""),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", NA, NA, list(0.5,0.5),
        "scenarioNumberOfRegistrarsVariable", "controller.SystemAverageHandlingSpeed",
        "scenarioNumberOfCalcAppPoolElementsVariable", "scenarioNetworkLANDelayVariable", "",
        "calcAppPoolElementSelectionPolicy", "")
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
