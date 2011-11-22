# ###########################################################################
# Name:        wp2-endpointkeepaliveI
# Description: Varying the ASAP Endpoint Keep-Alive interval
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "wp2-endpointkeepaliveI"
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
        "Provider's Perspective", NA, seq(20, 80, 10), list(1,1),
        "asapEndpointKeepAliveInterval", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicy", "calcAppPoolElementComponentUptimeVariable-MTBF", "calcAppProtocolServiceJobKeepAliveTimeout"),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", NA, seq(0, 70, 10), list(1,1),
        "asapEndpointKeepAliveInterval", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicy", "calcAppPoolElementComponentUptimeVariable-MTBF", "calcAppProtocolServiceJobKeepAliveTimeout"),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-ProcessingSpeed.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "asapEndpointKeepAliveInterval", "controller.SystemAverageProcessingSpeed",
        "calcAppPoolElementSelectionPolicy", "calcAppPoolElementComponentUptimeVariable-MTBF", "calcAppProtocolServiceJobKeepAliveTimeout"),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-StartupDelay.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "asapEndpointKeepAliveInterval", "controller.SystemAverageStartupDelay",
        "calcAppPoolElementSelectionPolicy", "calcAppPoolElementComponentUptimeVariable-MTBF", "calcAppProtocolServiceJobKeepAliveTimeout"),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-QueuingDelay.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "asapEndpointKeepAliveInterval", "controller.SystemAverageQueuingDelay",
        "calcAppPoolElementSelectionPolicy", "calcAppPoolElementComponentUptimeVariable-MTBF", "calcAppProtocolServiceJobKeepAliveTimeout"),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-ASAPPackets.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "asapEndpointKeepAliveInterval", "controller.TransportNodeGlobalASAPPackets",
        "calcAppPoolElementSelectionPolicy", "calcAppPoolElementComponentUptimeVariable-MTBF", "calcAppProtocolServiceJobKeepAliveTimeout"),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-ENRPPackets.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "asapEndpointKeepAliveInterval", "controller.TransportNodeGlobalENRPPackets",
        "calcAppPoolElementSelectionPolicy", "calcAppPoolElementComponentUptimeVariable-MTBF", "calcAppProtocolServiceJobKeepAliveTimeout")
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
