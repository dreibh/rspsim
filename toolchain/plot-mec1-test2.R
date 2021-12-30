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
library("dplyr", warn.conflicts = FALSE)
library("ggplot2")


plotColours <- c("steelblue4", "steelblue1",
                 "salmon4",    "salmon1",
                 "seagreen4",  "seagreen1",
                 "goldenrod3", "goldenrod1",
                 "gray40",     "gray10",
                 "cyan4",      "cyan1",
                 "yellow4",    "yellow1")


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
   calcAppPETotalUsedCapacity <- read.table(bzfile(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalUsedCapacity.data.bz2"), "rt"))
   calcAppPETotalWastedCapacity <- read.table(bzfile(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalWastedCapacity.data.bz2"), "rt"))
   print(sort(colnames(calcAppPETotalUsedCapacity)))

   cairo_pdf(paste(sep="", name, "-", prefix, "-Utilisation.pdf"),
             width=18, height=6, family="Helvetica", pointsize=22)

   title <- ""

   # ====== Calculate utilisation ===========================================
   utilisation <- data.table(utilisation = 100.0 * calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity /
                               (calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity + calcAppPETotalWastedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalWastedCapacity))
   calcAppPETotalUsedCapacity <- cbind(calcAppPETotalUsedCapacity, utilisation)

   # ====== Use factors =====================================================
   calcAppPETotalUsedCapacity$lan <- recode_factor(as.factor(calcAppPETotalUsedCapacity$lan),
                                                   "0" = "UE",
                                                   "1" = "MEC",
                                                   "2" = "PMC")
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
   calcAppPETotalUsedCapacity$calcAppPoolUserServiceJobIntervalVariable <- factor(calcAppPETotalUsedCapacity$calcAppPoolUserServiceJobIntervalVariable)
   # calcAppPETotalUsedCapacity$calcAppPoolUserServiceJobSizeVariable <- factor(calcAppPETotalUsedCapacity$calcAppPoolUserServiceJobSizeVariable)

   plotColours <- c(
      "orange",  "green4", "green1",
      "blue1", "blue4"
   )


   # ====== Use dplyr to summarise results ==================================
   summarised <- calcAppPETotalUsedCapacity %>%
                    filter(lan != "UE") %>%
                    filter(scenarioNumberOfCalcAppPoolUsersVariable != 50) %>%
                    group_by(calcAppPoolElementSelectionPolicy,scenarioNumberOfCalcAppPoolUsersVariable,lan,calcAppPoolUserServiceJobSizeVariable) %>%
                    summarise(#.groups = "keep",   # Keep the grouping as is. Otherwise, it would drop the last one!
                              MeanCalcAppPEUtilisation = mean(utilisation),
                              MinCalcAppPEUtilisation  = min(utilisation),
                              MaxCalcAppPEUtilisation  = max(utilisation),
                              Q10CalcAppPEUtilisation  = quantile(utilisation, 0.10),
                              Q90CalcAppPEUtilisation  = quantile(utilisation, 0.90))
   print(summarised)


   # ====== Create plots ====================================================
   p <- ggplot(summarised,
               aes(x = calcAppPoolUserServiceJobSizeVariable,
                   y = MeanCalcAppPEUtilisation)
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
               x     = "Average Request Size [Calculations]",
               y     = "Average Utilisation [%]") +
         facet_grid(lan ~ scenarioNumberOfCalcAppPoolUsersVariable) +
         geom_line(aes(color = calcAppPoolElementSelectionPolicy), size = 2) +
         geom_errorbar(aes(ymin = MinCalcAppPEUtilisation, ymax = MaxCalcAppPEUtilisation, color=calcAppPoolElementSelectionPolicy),
                        size=0.5, width=.5) +
         geom_errorbar(aes(ymin = Q10CalcAppPEUtilisation, ymax = Q90CalcAppPEUtilisation, color=calcAppPoolElementSelectionPolicy),
                        size=1.5, width=.25) +
         geom_ribbon(aes(ymin = Q10CalcAppPEUtilisation, ymax = Q90CalcAppPEUtilisation, color=calcAppPoolElementSelectionPolicy),
                     size=0.01, linetype=2, alpha=0.1) +
         scale_color_manual(values = plotColours)

   print(p)

   dev.off()
   return(summarised)
}


# ###### Plot results #######################################################
plotPUHandlingSpeed <- function(name, prefix, createPDF = TRUE)
{

   # ====== Plot as PDF file ================================================
   systemAverageHandlingSpeed <- read.table(bzfile(paste(sep="/", name, "controller-SystemAverageHandlingSpeed.data.bz2"), "rt"))
   print(sort(colnames(systemAverageHandlingSpeed)))

   if(createPDF) {
      cairo_pdf(paste(sep="", name, "-", prefix, "-HandlingSpeed.pdf"),
               width=18, height=5, family="Helvetica", pointsize=22)
      title <- ""
   }



systemAverageHandlingSpeed <- subset(systemAverageHandlingSpeed, systemAverageHandlingSpeed$calcAppPoolUserServiceJobSizeVariable <= 500000)



   systemAverageHandlingSpeed$calcAppPoolUserServiceJobIntervalVariable <- factor(systemAverageHandlingSpeed$calcAppPoolUserServiceJobIntervalVariable)
   # systemAverageHandlingSpeed$calcAppPoolUserServiceJobSizeVariable <- factor(systemAverageHandlingSpeed$calcAppPoolUserServiceJobSizeVariable)
   systemAverageHandlingSpeed$scenarioNumberOfCalcAppPoolUsersVariable <- factor(systemAverageHandlingSpeed$scenarioNumberOfCalcAppPoolUsersVariable)
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
      "blue1", "blue4"
   )


   # ====== Use dplyr to summarise results ==================================
   summarised <- systemAverageHandlingSpeed %>%
                    filter(scenarioNumberOfCalcAppPoolUsersVariable != 50) %>%
                    group_by(calcAppPoolElementSelectionPolicy,scenarioNumberOfCalcAppPoolUsersVariable,calcAppPoolUserServiceJobSizeVariable) %>%
                    summarise(#.groups = "keep",   # Keep the grouping as is. Otherwise, it would drop the last one!
                              MeanCalcAppPUHandlingSpeed = mean(controller.SystemAverageHandlingSpeed),
                              MinCalcAppPUHandlingSpeed  = min(controller.SystemAverageHandlingSpeed),
                              MaxCalcAppPUHandlingSpeed  = max(controller.SystemAverageHandlingSpeed),
                              Q10CalcAppPUHandlingSpeed  = quantile(controller.SystemAverageHandlingSpeed, 0.10),
                              Q90CalcAppPUHandlingSpeed  = quantile(controller.SystemAverageHandlingSpeed, 0.90))
   print(summarised)


   # ====== Create plots ====================================================
   p <- ggplot(summarised,
               aes(x = calcAppPoolUserServiceJobSizeVariable,
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
               x     = "Average Request Size [Calculations]",
               y     = "Handling Speed [Calculations/s]") +
         facet_grid( ~ scenarioNumberOfCalcAppPoolUsersVariable) +
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


# ###### Main program #######################################################

data <- plotPEUtilisation("mec1-test2/Results", "MEC2")
data <- plotPUHandlingSpeed("mec1-test2/Results", "MEC2")
