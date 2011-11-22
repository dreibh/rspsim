# ###########################################################################
# Name:        attack-handleResolutions-countermeasureII
# Description: Endpoint Unreachable attack countermeasure
#              - Handle Resolution Rate not limited!
#              - Only RegLife of 30s (300s would be much more worse!)
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "attack-handleResolutions-countermeasureII"
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
        "Provider's Perspective", list(seq(0,50,5)), NA, list(0,0),
        "attackReportUnreachableProbability", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicy", "registrarMaxEndpointUnreachableRate", "calcAppPoolElementServerRegistrationLife"),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", list(seq(0,50,5)), list(seq(0,70,10)), list(1,0.95),
        "attackReportUnreachableProbability", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicy", "registrarMaxEndpointUnreachableRate", "calcAppPoolElementServerRegistrationLife"),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-RefusedHandleResolutions.pdf"),
        "User's Perspective", list(seq(0,50,5)), NA, list(1,1),
        "attackReportUnreachableProbability", "controller.RegistrarGlobalRefusedHandleResolutions",
        "calcAppPoolElementSelectionPolicy", "registrarMaxEndpointUnreachableRate", "calcAppPoolElementServerRegistrationLife"),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-RefusedEndpointUnreachables.pdf"),
        "User's Perspective", list(seq(0,50,5)), NA, list(1,1),
        "attackReportUnreachableProbability", "controller.RegistrarGlobalRefusedEndpointUnreachables",
        "calcAppPoolElementSelectionPolicy", "registrarMaxEndpointUnreachableRate", "calcAppPoolElementServerRegistrationLife")
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
