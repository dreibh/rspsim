# ###########################################################################
# Name:        wp1-hom-lu-dpfI
# Description: LU-DPF policy proof of concept: WAN delay variation
# Revision:    $Id$
# ###########################################################################


source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "wp1-hom-lu-dpfI"
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
        "Provider's Perspective", NA, seq(45,65,5), list(0,0),
        "scenarioNetworkWANDelayVariable", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicyLoadDPF", "puToPERatio", ""),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", NA, seq(0,90,10), list(0.7,0.05),
        "scenarioNetworkWANDelayVariable", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicyLoadDPF", "puToPERatio", ""),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-ProcessingSpeed.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "scenarioNetworkWANDelayVariable", "controller.SystemAverageProcessingSpeed",
        "calcAppPoolElementSelectionPolicyLoadDPF", "puToPERatio", ""),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-StartupDelay.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "scenarioNetworkWANDelayVariable", "controller.SystemAverageStartupDelay",
        "calcAppPoolElementSelectionPolicyLoadDPF", "puToPERatio", ""),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-QueuingDelay.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "scenarioNetworkWANDelayVariable", "controller.SystemAverageQueuingDelay",
        "calcAppPoolElementSelectionPolicyLoadDPF", "puToPERatio", ""),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-ASAPPackets.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "scenarioNetworkWANDelayVariable", "controller.TransportNodeGlobalASAPPackets",
        "calcAppPoolElementSelectionPolicyLoadDPF", "puToPERatio", ""),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-ENRPPackets.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "scenarioNetworkWANDelayVariable", "controller.TransportNodeGlobalENRPPackets",
        "calcAppPoolElementSelectionPolicyLoadDPF", "puToPERatio", "")
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
