#!/usr/bin/Rscript
#
# SELF — Self-contained User Data Preserving Framework
# Copyright (C) 2021 by Thomas Dreibholz
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Contact: dreibh@simula.no

library("anytime")
library("data.table")
library("plyr",  warn.conflicts = FALSE)
library("dplyr", warn.conflicts = FALSE)
library("ggplot2")
library("xtable")


plotColours <- c("steelblue4", "steelblue1",
                 "salmon4",    "salmon1",
                 "seagreen4",  "seagreen1",
                 "goldenrod3", "goldenrod1",
                 "gray40",     "gray10",
                 "cyan4",      "cyan1",
                 "yellow4",    "yellow1")


# ###### Get policy types ###################################################
getPolicyType <- function(policies)
{
   return(as.factor(mapvalues(as.vector(policies),
          from = c("Random", "RoundRobin",
                   "LeastUsed", "LeastUsedDegradation", "PriorityLeastUsed", "PriorityLeastUsedDegradation", "PriorityLeastUsedDPF", "PriorityLeastUsedDegradationDPF"),
          to   = c("Non-Adaptive", "Non-Adaptive",
                   "Adaptive", "Adaptive",              "Adaptive",          "Adaptive",                     "Adaptive with DPF",    "Adaptive with DPF"))))
}


# ###### Provide alignment setting for xtable() #############################
getAlignment <- function(data)
{
   alignment <- "l|"
   for(i in seq(1, ncol(data))) {
      alignment <- paste(sep="", alignment, "c|")
   }
   return(alignment)
}


# ###### Text sanitiser for xtable(): some LaTeX markup #####################
markup.latex <- function(text)
{
   return(gsub(" -> ", " $\\rightarrow$ ",
             gsub(">=", "$\\ge$",
                gsub("%", "\\%",
                   gsub("Q([0-9]+)", "Q$_{\\\\mathrm{\\1%}}$", text, fixed = FALSE),
                   fixed = TRUE),
                fixed = TRUE),
             fixed = TRUE))
}


# ###### Text sanitiser for xtable(): some HTML markup ######################
markup.html <- function(text)
{
   return(gsub(" -> ", " → ",
             gsub(">=", "&ge;",
                gsub("Q([0-9]+)", "Q<sub>\\1%</sub>", text, fixed = FALSE),
             fixed = TRUE),
          fixed = TRUE))
}


# ###### Text sanitiser for xtable(): Use LaTeX bold text ###################
bold.latex <- function(text)
{
   return(paste(sep="", "{\\textbf{", markup.latex(text), "}}"))
}


# ###### Text sanitiser for xtable(): Use HTML bold text ####################
bold.html <- function(text) {
   return(paste(sep="", "<strong>", markup.html(text), "</strong>"))
}


# ###### Read output file ###################################################
readResults <- function(directory)
{
   dataName <- paste(sep="/", directory, "controller-CalcAppPEGlobalUsedCapacity.data.bz2")
   print(paste(sep="", "Trying to read ", dataName, " ..."))
   data <- read.table(pipe(paste(sep="", "bzcat ", dataName)))
   return(data)
}


# ###### Plot results #######################################################
plotPEUtilisation <- function(name, prefix)
{
   # ====== Plot as PDF file ================================================
   calcAppPETotalUsedCapacity   <- read.table(bzfile(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalUsedCapacity.data.bz2"), "rt"))
   calcAppPETotalWastedCapacity <- read.table(bzfile(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalWastedCapacity.data.bz2"), "rt"))
   # print(sort(colnames(calcAppPETotalUsedCapacity)))

   cairo_pdf(paste(sep="", name, "-", prefix, "-Utilisation.pdf"),
             width=12, height=20, family="Helvetica", pointsize=32)

   title <- ""

   # ====== Calculate utilisation ===========================================
   utilisation <- data.table(utilisation = 100.0 * calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity /
                               (calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity + calcAppPETotalWastedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalWastedCapacity))
   calcAppPETotalUsedCapacity <- cbind(calcAppPETotalUsedCapacity, utilisation)

   # ====== Use factors for LAN and LAN.CalcAppPoolElementArray =============
   calcAppPETotalUsedCapacity$lan <- recode_factor(as.factor(calcAppPETotalUsedCapacity$lan),
                                                   "0" = "UE",
                                                   "1" = "MEC",
                                                   "2" = "PMC")
   calcAppPETotalUsedCapacity$calcAppPoolElementSelectionPolicyType <- getPolicyType(calcAppPETotalUsedCapacity$calcAppPoolElementSelectionPolicy)
   calcAppPETotalUsedCapacity$calcAppPoolElementSelectionPolicy <-
      recode_factor(as.factor(calcAppPETotalUsedCapacity$calcAppPoolElementSelectionPolicy),
                    "Random"                          = "Random",
                    "RoundRobin"                      = "RoundRobin",
                    "LeastUsed"                       = "LeastUsed",
                    "LeastUsedDegradation"            = "LeastUsedDeg.",
                    "PriorityLeastUsed"               = "Prio.LeastUsed",
                    "PriorityLeastUsedDegradation"    = "Prio.LeastUsedDeg.",
                    "PriorityLeastUsedDPF"            = "Prio.LeastUsedDPF",
                    "PriorityLeastUsedDegradationDPF" = "Prio.LeastUsedDeg.DPF"
                   )
   calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray <- factor(calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray)

   plotColours <- c(
      "orange",  "green4", "green1",  "blue1", "blue4", "purple1", "purple4", "goldenrod3"
   )


   # ====== Use dplyr to summarise results ==================================
   summarised <- calcAppPETotalUsedCapacity %>%
                    group_by(calcAppPoolElementSelectionPolicyType, calcAppPoolElementSelectionPolicy, scenarioNumberOfCalcAppPoolUsersVariable, lan, lan.calcAppPoolElementArray) %>%
                    summarise(#.groups = "keep",   # Keep the grouping as is. Otherwise, it would drop the last one!
                              MeanCalcAppPEUtilisation = mean(utilisation),
                              MinCalcAppPEUtilisation  = min(utilisation),
                              MaxCalcAppPEUtilisation  = max(utilisation),
                              Q10CalcAppPEUtilisation  = quantile(utilisation, 0.10),
                              Q90CalcAppPEUtilisation  = quantile(utilisation, 0.90))
   # print(summarised)


   # ====== Create plots ====================================================
   p <- ggplot(summarised,
               aes(x = scenarioNumberOfCalcAppPoolUsersVariable,
                   y = MeanCalcAppPEUtilisation,
                   color = lan.calcAppPoolElementArray)
            ) +
         theme(title             = element_text(size=32),
               plot.title        = element_text(size=20, hjust = 0.5, face="bold"),
               axis.title        = element_text(size=20, face="bold"),
               strip.text        = element_text(size=18, face="bold"),
               axis.text.x       = element_text(size=16, angle=90, face="bold", colour="black"),
               axis.text.y       = element_text(size=16, angle=90, hjust=0.5, colour="black"),
               legend.title      = element_blank(),
               legend.text       = element_text(size=18, face="bold"),
               legend.position   = "none",   # NOTE: Legned turned OFF!
#                legend.background = element_rect(colour = bgColor,  fill = lgColor, size=1),
#                panel.background  = element_rect(fill = paste(sep="", "#", colorCU), color=bgColor, size=2),
               # panel.grid.major  = element_line(size=0.5,  linetype="solid", color="lightgray"),
               # panel.grid.minor  = element_line(size=0.25, linetype="solid", color="lightgray")
               # strip.background = element_blank(),
               panel.grid.major = element_line(size=0.4, colour = "black"),
               panel.grid.minor = element_line(size=0.2, colour = "gray"),
               panel.background = element_blank(),
               ) +
         labs(title = title,
               x     = "Number of Pool User Instances [1]",
               y     = "Average Utilisation [%]") +
         facet_grid(calcAppPoolElementSelectionPolicyType + calcAppPoolElementSelectionPolicy ~ lan) +
         geom_line(aes(color = lan.calcAppPoolElementArray), size = 2) +
         geom_errorbar(aes(ymin = MinCalcAppPEUtilisation, ymax = MaxCalcAppPEUtilisation, color=lan.calcAppPoolElementArray),
                        size=0.5, width=.5) +
         geom_errorbar(aes(ymin = Q10CalcAppPEUtilisation, ymax = Q90CalcAppPEUtilisation, color=lan.calcAppPoolElementArray),
                        size=1.5, width=.25) +
         geom_ribbon(aes(ymin = Q10CalcAppPEUtilisation, ymax = Q90CalcAppPEUtilisation, color=lan.calcAppPoolElementArray),
                     size=0.01, linetype=2, alpha=0.1) +
         scale_color_manual(values = rep("goldenrod3", 1024))

   print(p)

   dev.off()
   return(summarised)
}


# ###### Plot results #######################################################
plotPUHandlingSpeed <- function(name, prefix, createPDF = TRUE)
{

   # ====== Plot as PDF file ================================================
   systemAverageHandlingSpeed <- read.table(bzfile(paste(sep="/", name, "controller-SystemAverageHandlingSpeed.data.bz2"), "rt"))
   # print(sort(colnames(systemAverageHandlingSpeed)))

   if(createPDF) {
      cairo_pdf(paste(sep="", name, "-", prefix, "-HandlingSpeed.pdf"),
               width=12, height=12, family="Helvetica", pointsize=32)
      title <- ""
   }

   systemAverageHandlingSpeed <- systemAverageHandlingSpeed %>%
      mutate(controller.SystemAverageHandlingSpeed = controller.SystemAverageHandlingSpeed / 1000)

   systemAverageHandlingSpeed$calcAppPoolElementSelectionPolicyType <- getPolicyType(systemAverageHandlingSpeed$calcAppPoolElementSelectionPolicy)
   systemAverageHandlingSpeed$calcAppPoolElementSelectionPolicy <-
      recode_factor(as.factor(systemAverageHandlingSpeed$calcAppPoolElementSelectionPolicy),
                    "Random"                          = "Random",
                    "RoundRobin"                      = "RoundRobin",
                    "LeastUsed"                       = "LeastUsed",
                    "LeastUsedDegradation"            = "LeastUsedDeg.",
                    "PriorityLeastUsed"               = "Prio.LeastUsed",
                    "PriorityLeastUsedDegradation"    = "Prio.LeastUsedDeg.",
                    "PriorityLeastUsedDPF"            = "Prio.LeastUsedDPF",
                    "PriorityLeastUsedDegradationDPF" = "Prio.LeastUsedDeg.DPF"
                   )

   plotColours <- c(
      "orange",  "green4", "green1",
      "goldenrod1",
      "cyan4",      "cyan1",
      "goldenrod3", "red1"
   )


   # ====== Use dplyr to summarise results ==================================
   summarised <- systemAverageHandlingSpeed %>%
                    group_by(calcAppPoolElementSelectionPolicyType, calcAppPoolElementSelectionPolicy, scenarioNumberOfCalcAppPoolUsersVariable) %>%
                    summarise(#.groups = "keep",   # Keep the grouping as is. Otherwise, it would drop the last one!
                              MeanCalcAppPUHandlingSpeed = mean(controller.SystemAverageHandlingSpeed),
                              MinCalcAppPUHandlingSpeed  = min(controller.SystemAverageHandlingSpeed),
                              MaxCalcAppPUHandlingSpeed  = max(controller.SystemAverageHandlingSpeed),
                              Q10CalcAppPUHandlingSpeed  = quantile(controller.SystemAverageHandlingSpeed, 0.10),
                              Q90CalcAppPUHandlingSpeed  = quantile(controller.SystemAverageHandlingSpeed, 0.90))
   # print(summarised)


   # ====== Create plots ====================================================
   p <- ggplot(summarised,
               aes(x = scenarioNumberOfCalcAppPoolUsersVariable,
                   y = MeanCalcAppPUHandlingSpeed)
            ) +
         theme(title             = element_text(size=32),
               plot.title        = element_text(size=20, hjust = 0.5, face="bold"),
               axis.title        = element_text(size=20, face="bold"),
               strip.text        = element_text(size=18, face="bold"),
               axis.text.x       = element_text(size=16, angle=90, face="bold", colour="black"),
               axis.text.y       = element_text(size=16, angle=90, hjust=0.5, colour="black"),
               legend.title      = element_blank(),
               legend.text       = element_text(size=18, face="bold"),
#                legend.background = element_rect(colour = bgColor,  fill = lgColor, size=1),
#                panel.background  = element_rect(fill = paste(sep="", "#", colorCU), color=bgColor, size=2),
               # panel.grid.major  = element_line(size=0.5,  linetype="solid", color="lightgray"),
               # panel.grid.minor  = element_line(size=0.25, linetype="solid", color="lightgray")
               # strip.background = element_blank(),
               panel.grid.major = element_line(size=0.4, colour = "black"),
               panel.grid.minor = element_line(size=0.2, colour = "gray"),
               panel.background = element_blank(),
               ) +
         labs(title = title,
               x     = "Number of Pool User Instances [1]",
               y     = "Handling Speed [1000 Calculations/s]") +
         facet_wrap( ~ calcAppPoolElementSelectionPolicyType, nrow = 3) +
         geom_line(aes(color = calcAppPoolElementSelectionPolicy), size = 2) +
         geom_errorbar(aes(ymin = MinCalcAppPUHandlingSpeed, ymax = MaxCalcAppPUHandlingSpeed, color = calcAppPoolElementSelectionPolicy),
                        size=0.5, width=.5) +
         geom_errorbar(aes(ymin = Q10CalcAppPUHandlingSpeed, ymax = Q90CalcAppPUHandlingSpeed, color = calcAppPoolElementSelectionPolicy),
                        size=1.5, width=.25) +
         geom_ribbon(aes(ymin = Q10CalcAppPUHandlingSpeed, ymax = Q90CalcAppPUHandlingSpeed, color = calcAppPoolElementSelectionPolicy),
                     size=0.01, linetype=2, alpha=0.1) +
         scale_color_manual(values = plotColours)

   print(p)

   if(createPDF) {
      dev.off()
   }
   return(summarised)
}


# ###### Plot results #######################################################
computeDelays <- function(name, prefix, createPDF = TRUE)
{

   # ====== Queuing =========================================================
   queuing <- read.table(bzfile(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageQueuingDelay.data.bz2"), "rt")) %>%
                 select(c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable", "lan.calcAppPoolUserArray", "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay"))
   # print(colnames(queuing))

   queuingSummary <- queuing %>%
      group_by(calcAppPoolElementSelectionPolicy, scenarioNumberOfCalcAppPoolUsersVariable) %>%
      filter(scenarioNumberOfCalcAppPoolUsersVariable %in% c(10, 60, 120)) %>%
      summarise(QueuingMean = mean(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay),
                QueuingMin  = min(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay),
                QueuingMax  = max(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay),
                QueuingQ10  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay, 0.10),
                QueuingQ90  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay, 0.90)
               )

   print(queuingSummary)


   # ====== Startup =========================================================
   startup <- read.table(bzfile(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageStartupDelay.data.bz2"), "rt")) %>%
                 select(c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable", "lan.calcAppPoolUserArray", "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay"))
   # print(colnames(startup))

   startupSummary <- startup %>%
      group_by(calcAppPoolElementSelectionPolicy, scenarioNumberOfCalcAppPoolUsersVariable) %>%
      filter(scenarioNumberOfCalcAppPoolUsersVariable %in% c(10, 60, 120)) %>%
      summarise(StartupMean = mean(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay),
                StartupMin  = min(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay),
                StartupMax  = max(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay),
                StartupQ10  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay, 0.10),
                StartupQ90  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay, 0.90)
               )

   print(startupSummary)


   # ====== Processing =========================================================
   processing <- read.table(bzfile(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageProcessingTime.data.bz2"), "rt")) %>%
                 select(c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable", "lan.calcAppPoolUserArray", "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime"))
   # print(colnames(processing))

   processingSummary <- processing %>%
      group_by(calcAppPoolElementSelectionPolicy, scenarioNumberOfCalcAppPoolUsersVariable) %>%
      filter(scenarioNumberOfCalcAppPoolUsersVariable %in% c(10, 60, 120)) %>%
      summarise(ProcessingMean = mean(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime),
                ProcessingMin  = min(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime),
                ProcessingMax  = max(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime),
                ProcessingQ10  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime, 0.10),
                ProcessingQ90  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime, 0.90)
               )

   print(processingSummary)


   # ====== Join summaries ==================================================
   summary <- queuingSummary %>%
      left_join(startupSummary, by = c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable")) %>%
      left_join(processingSummary, by = c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable")) %>%
      rename(Policy = calcAppPoolElementSelectionPolicy,
             PUs    = scenarioNumberOfCalcAppPoolUsersVariable) %>%
      mutate(PolicyType = getPolicyType(as.vector(Policy))) %>%
      select(PolicyType, everything())   # Move PolicyType to front


   # ====== Write tables ====================================================
   caption <- "Queuing, Startup and Processing Times for each Policy"
   latexOutputTable <- xtable(summary,
                              align   = getAlignment(summary),
                              label   = paste(sep="", "tab:", prefix, "QSP-Times"),
                              caption = caption)
   htmlOutputTable  <- xtable(summary,
                              align   = getAlignment(summary),
                              label   = paste(sep="", "tab:", prefix, "QSP-Times"),
                              caption = caption)

   latexName <- paste(sep="", name, "-", prefix, "-QSP-Times.tex")
   print(latexOutputTable,
         size                       = "footnotesize",
         include.rownames           = FALSE,
         sanitize.colnames.function = bold.latex,
         table.placement            = NULL,
         caption.placement          = "top",
         hline.after                = c(-1, 0, seq(0, nrow(latexOutputTable))),
         floating.environment       = "table*",
         NA.string                  = "--",
         file                       = latexName,
         type                       = "latex")
   cat(sep="", "Wrote ", latexName, "\n")

   htmlName <- paste(sep="", name, "-", prefix, "-QSP-Times.html")
   print(htmlOutputTable,
         include.rownames           = FALSE,
         table.placement            = NULL,
         caption.placement          = "top",
         sanitize.colnames.function = bold.html,
         hline.after                = c(-1, 0, seq(0, nrow(htmlOutputTable))),
         NA.string                  = "--",
         file                       = htmlName,
         type                       = "html")
   cat(sep="", "Wrote ", htmlName, "\n")

   return(summary)
}


# ###### Main program #######################################################

# dataUtilisation   <- plotPEUtilisation("mec1-test1/Results", "MEC1")
# dataHandlingSpeed <- plotPUHandlingSpeed("mec1-test1/Results", "MEC1")
summary <- computeDelays("mec1-test1/Results", "MEC1")
