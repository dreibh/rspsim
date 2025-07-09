#!/usr/bin/Rscript
#
# MEC/PMC performance analysis of DPF policies
# Copyright (C) 2022-2025 by Thomas Dreibholz
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


# ###### Read results file ###################################################
readResults <- function(name)
{
   cat(sep="", "Reading ", name, " ...\n")
   data <- fread(name, sep="\t", quote='"')
   colnames(data) <- gsub("-", ".", colnames(data))

   # print(sort(colnames(data)))
   return(data)
}


# ###### Write table ########################################################
writeTable <- function(table, name, prefix, label, caption)
{
   latexOutputTable <- xtable(table,
                              align   = getAlignment(table),
                              label   = paste(sep="", "tab:", prefix, label),
                              caption = caption)
   htmlOutputTable  <- xtable(table,
                              align   = getAlignment(table),
                              label   = paste(sep="", "tab:", prefix, label),
                              caption = caption)

   latexName <- paste(sep="", name, "-", prefix, "-", label, ".tex")
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

   htmlName <- paste(sep="", name, "-", prefix, "-", label, ".html")
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
}


# ###### Get policy types ###################################################
getPolicyType <- function(policies)
{
   return(as.factor(mapvalues(as.vector(policies),
          from = c("Random", "RoundRobin",
                   "LeastUsed", "LeastUsedDegradation", "PriorityLeastUsed", "PriorityLeastUsedDegradation", "PriorityLeastUsedDPF", "PriorityLeastUsedDegradationDPF"),
          to   = c("Non-Adaptive", "Non-Adaptive",
                   "Adaptive", "Adaptive",              "Adaptive",          "Adaptive",                     "Adaptive with DPF",    "Adaptive with DPF"),
                   warn_missing = FALSE)))
}


# ###### Get policy types ###################################################
getPolicyType2 <- function(policies)
{
   return(as.factor(mapvalues(as.vector(policies),
          from = c("Random", "RoundRobin",
                   "LeastUsed", "LeastUsedDegradation", "PriorityLeastUsed", "PriorityLeastUsedDegradation", "PriorityLeastUsedDPF", "PriorityLeastUsedDegradationDPF"),
          to   = c("Non-Adaptive", "Non-Adaptive",
                   "Adaptive", "Adaptive",              "Adaptive",          "Adaptive",                     "Adaptive/DPF",    "Adaptive/DPF"),
                   warn_missing = FALSE)))
}


# ###### Get policy abbreviation ############################################
getPolicyAbbreviations <- function(policies)
{
   policies <-
      recode_factor(as.factor(policies),
                    "Random"                          = "RAND",
                    "RoundRobin"                      = "RR",
                    "LeastUsed"                       = "LU",
                    "LeastUsedDegradation"            = "LUD",
                    "PriorityLeastUsed"               = "PLU",
                    "PriorityLeastUsedDegradation"    = "PLUD",
                    "PriorityLeastUsedDPF"            = "PLU-DPF",
                    "PriorityLeastUsedDegradationDPF" = "PLUD-DPF"

#                     "Random"                          = "Random",
#                     "RoundRobin"                      = "RoundRobin",
#                     "LeastUsed"                       = "LeastUsed",
#                     "LeastUsedDegradation"            = "LeastUsedDeg.",
#                     "PriorityLeastUsed"               = "Prio.LeastUsed",
#                     "PriorityLeastUsedDegradation"    = "Prio.LeastUsedDeg.",
#                     "PriorityLeastUsedDPF"            = "Prio.LeastUsedDPF",
#                     "PriorityLeastUsedDegradationDPF" = "Prio.LeastUsedDeg.DPF"
                   )
   return(policies)
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


# ###### Plot results #######################################################
systemSummaryTable <- function(name, prefix)
{
   calcAppPETotalUsedCapacity   <- readResults(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalUsedCapacity.data.bz2"))
   calcAppPETotalWastedCapacity <- readResults(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalWastedCapacity.data.bz2"))

   calcAppPUAverageHandlingTime <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageHandlingTime.data.bz2"))
   calcAppPUAverageQueuingDelay <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageQueuingDelay.data.bz2"))
   calcAppPUAverageStartupDelay <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageStartupDelay.data.bz2"))
   calcAppPUAverageProcessingTime <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageProcessingTime.data.bz2"))

   # ====== Calculate utilisation ===========================================
   utilisation <- data.table(utilisation = 100.0 * calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity /
                               (calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity + calcAppPETotalWastedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalWastedCapacity))
   calcAppPETotalUsedCapacity <- cbind(calcAppPETotalUsedCapacity, utilisation)


   # ====== Aggregate utilisation ===========================================
   PUs <- c(50)   # c(20, 50, 80)

   aggregation <- calcAppPETotalUsedCapacity %>%
                     filter(scenarioNumberOfCalcAppPoolUsersVariable %in% PUs) %>%
                     select(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy, lan, utilisation) %>%
                     group_by(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy, lan) %>%
                     summarise(.groups         = "keep",
                               utilisationMean = mean(utilisation))

   aggregationMEC <- aggregation %>% filter(lan == 1) %>% rename(utilisationMeanMEC = utilisationMean)
   aggregationPMC <- aggregation %>% filter(lan == 2) %>% rename(utilisationMeanPMC = utilisationMean)
   aggregation <- merge(aggregationMEC, aggregationPMC, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                     group_by(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy) %>%
                     select(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy, utilisationMeanMEC, utilisationMeanPMC)


   # ====== Aggregate times ===========================================
   prep <- function(data, key) {
      return(data %>%
                filter(scenarioNumberOfCalcAppPoolUsersVariable %in% PUs) %>%
                select(c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy", all_of(key))) %>%
                group_by(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy) %>%
                summarise(.groups = "keep",
                          time    =  mean(eval(parse(text=key)))
                         )
            )
   }
# #    a <- calcAppPUAverageHandlingTime %>% select("lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageHandlingTime")

   a <- prep(calcAppPUAverageQueuingDelay, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay") %>%
           rename("Queuing Time" = time)
   b <- prep(calcAppPUAverageStartupDelay, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay") %>%
           rename("Startup Time" = time)
   c <- prep(calcAppPUAverageProcessingTime, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime") %>%
           rename("Processing Time" = time)
   d <- prep(calcAppPUAverageHandlingTime, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageHandlingTime") %>%
           rename("Handling Time" = time)

   combined <- aggregation %>%
                  merge(a, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(b, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(c, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(d, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  mutate(calcAppPoolElementSelectionPolicy = getPolicyAbbreviations(calcAppPoolElementSelectionPolicy)) %>%
                  rename("Clients" = scenarioNumberOfCalcAppPoolUsersVariable,
                         "Policy"  = calcAppPoolElementSelectionPolicy,
                         "Util. Fog"     = utilisationMeanMEC,
                         "Util. Cloud"   = utilisationMeanPMC)  %>%
                  select(!Clients)   # !!!


   # ====== Aggregate times =================================================
   writeTable(combined,
               name, prefix, "Summary", "2-Hours Original: Average server utilisation (\\%) and average request times (s)")
}


# ###### Plot results #######################################################
plotPEUtilisation <- function(name, prefix)
{
   # ====== Plot as PDF file ================================================
   calcAppPETotalUsedCapacity   <- readResults(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalUsedCapacity.data.bz2"))
   calcAppPETotalWastedCapacity <- readResults(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalWastedCapacity.data.bz2"))

   cairo_pdf(paste(sep="", name, "-", prefix, "-Utilisation.pdf"),
             width=15, height=17.5, family="Helvetica", pointsize=32)

   title <- ""

   # ====== Calculate utilisation ===========================================
   utilisation <- data.table(utilisation = 100.0 * calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity /
                               (calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity + calcAppPETotalWastedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalWastedCapacity))
   calcAppPETotalUsedCapacity <- cbind(calcAppPETotalUsedCapacity, utilisation)

   # ====== Use factors for LAN and LAN.CalcAppPoolElementArray =============
   calcAppPETotalUsedCapacity$lan <- recode_factor(as.factor(calcAppPETotalUsedCapacity$lan),
                                                   "0" = "UE",
                                                   "1" = "EC",
                                                   "2" = "PMC")
   calcAppPETotalUsedCapacity$calcAppPoolElementSelectionPolicyType <- getPolicyType2(calcAppPETotalUsedCapacity$calcAppPoolElementSelectionPolicy)
   calcAppPETotalUsedCapacity$calcAppPoolElementSelectionPolicy <- getPolicyAbbreviations(calcAppPETotalUsedCapacity$calcAppPoolElementSelectionPolicy)
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
         theme(title             = element_text(size=30),
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
               panel.spacing    = unit(1, "cm"),
               panel.grid.major = element_line(size=0.4, colour = "black"),
               panel.grid.minor = element_line(size=0.2, colour = "gray"),
               panel.background = element_rect(fill="white", color="black", size=2)
               ) +
         labs(title = title,
               x     = "Number of Clients",
               y     = "Average Utilisation [%]") +
         facet_grid(calcAppPoolElementSelectionPolicyType + calcAppPoolElementSelectionPolicy ~ lan) +
         geom_line(aes(color = lan.calcAppPoolElementArray), size = 2) +
         geom_errorbar(aes(ymin = MinCalcAppPEUtilisation, ymax = MaxCalcAppPEUtilisation, color=lan.calcAppPoolElementArray),
                        size=0.5, width=.5) +
         geom_errorbar(aes(ymin = Q10CalcAppPEUtilisation, ymax = Q90CalcAppPEUtilisation, color=lan.calcAppPoolElementArray),
                        size=1.5, width=.25) +
         geom_ribbon(aes(ymin = Q10CalcAppPEUtilisation, ymax = Q90CalcAppPEUtilisation, color=lan.calcAppPoolElementArray),
                     size=0.01, linetype=2, alpha=0.05) +
         scale_color_manual(values = rep("blue", 1024)) +
         # Capacity planning baseline:
         geom_vline(xintercept=50, linetype="dotted", color="red", size=2.5) +
         # geom_hline(yintercept=70.92475, linetype="dotted", color="red", size=2.5) +
         # Axis ticks:
         scale_y_continuous(breaks=seq(floor(min(summarised$MeanCalcAppPEUtilisation) / 10) * 10,
                                       ceiling(max(summarised$MeanCalcAppPEUtilisation) / 10) * 10, 10)) +
         scale_x_continuous(breaks=seq(floor(min(summarised$scenarioNumberOfCalcAppPoolUsersVariable) / 10) * 10,
                                       ceiling(max(summarised$scenarioNumberOfCalcAppPoolUsersVariable) / 10) * 10, 10),
                            minor_breaks=seq(floor(min(summarised$scenarioNumberOfCalcAppPoolUsersVariable) / 5) * 5,
                                       ceiling(max(summarised$scenarioNumberOfCalcAppPoolUsersVariable) / 5) * 5, 5))

   print(p)

   dev.off()
   return(summarised)
}


# ###### Plot results #######################################################
plotPUHandlingSpeed <- function(name, prefix, createPDF = TRUE)
{

   # ====== Plot as PDF file ================================================
   systemAverageHandlingSpeed <- readResults(paste(sep="/", name, "controller-SystemAverageHandlingSpeed.data.bz2"))

   if(createPDF) {
      cairo_pdf(paste(sep="", name, "-", prefix, "-HandlingSpeed.pdf"),
               width=12, height=12, family="Helvetica", pointsize=32)
      title <- ""
   }

   systemAverageHandlingSpeed <- systemAverageHandlingSpeed %>%
      mutate(controller.SystemAverageHandlingSpeed = controller.SystemAverageHandlingSpeed / 60)

   systemAverageHandlingSpeed$calcAppPoolElementSelectionPolicyType <- getPolicyType(systemAverageHandlingSpeed$calcAppPoolElementSelectionPolicy)
   systemAverageHandlingSpeed$calcAppPoolElementSelectionPolicy <- getPolicyAbbreviations(systemAverageHandlingSpeed$calcAppPoolElementSelectionPolicy)

   plotColours <- c(
      "gray50", "lightblue", "green", "red",
      "blue", "black", "purple", "orange"

#       "orange",  "green4", "green1",
#       "goldenrod1",
#       "cyan4",      "cyan1",
#       "goldenrod3", "red1"
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
               legend.position   = "bottom",
#                legend.background = element_rect(colour = bgColor,  fill = lgColor, size=1),
#                panel.background  = element_rect(fill = paste(sep="", "#", colorCU), color=bgColor, size=2),
               # panel.grid.major  = element_line(size=0.5,  linetype="solid", color="lightgray"),
               # panel.grid.minor  = element_line(size=0.25, linetype="solid", color="lightgray")
               # strip.background = element_blank(),
               panel.spacing    = unit(1, "cm"),
               panel.grid.major = element_line(size=0.4, colour = "black"),
               panel.grid.minor = element_line(size=0.2, colour = "gray"),
               panel.background = element_rect(fill="white", color="black", size=2)
               ) +
         labs(title = title,
               x     = "Number of Clients",
               y     = "Handling Speed [Work Units/min]") +
         # facet_wrap( ~ calcAppPoolElementSelectionPolicyType, nrow = 3) +
         geom_line(aes(color = calcAppPoolElementSelectionPolicy), size = 2) +
         geom_errorbar(aes(ymin = MinCalcAppPUHandlingSpeed, ymax = MaxCalcAppPUHandlingSpeed, color = calcAppPoolElementSelectionPolicy),
                        size=0.5, width=.5) +
         geom_errorbar(aes(ymin = Q10CalcAppPUHandlingSpeed, ymax = Q90CalcAppPUHandlingSpeed, color = calcAppPoolElementSelectionPolicy),
                        size=1.5, width=.25) +
         geom_ribbon(aes(ymin = Q10CalcAppPUHandlingSpeed, ymax = Q90CalcAppPUHandlingSpeed, color = calcAppPoolElementSelectionPolicy),
                     size=0.01, linetype=2, alpha=0.1) +
         scale_color_manual(values = plotColours) +
         # Capacity planning baseline:
         geom_vline(xintercept=50, linetype="dotted", color="red", size=2.5) +
         geom_hline(yintercept=300, linetype="dashed", color="purple", size=2.5) +
         # Axis ticks:
         scale_y_continuous(breaks=seq(floor(min(summarised$MeanCalcAppPUHandlingSpeed) / 25) * 25,
                                       ceiling(max(summarised$MeanCalcAppPUHandlingSpeed) / 25) * 25, 25)) +
         scale_x_continuous(breaks=seq(floor(min(summarised$scenarioNumberOfCalcAppPoolUsersVariable) / 10) * 10,
                                       ceiling(max(summarised$scenarioNumberOfCalcAppPoolUsersVariable) / 10) * 10, 10),
                            minor_breaks=seq(floor(min(summarised$scenarioNumberOfCalcAppPoolUsersVariable) / 5) * 5,
                                       ceiling(max(summarised$scenarioNumberOfCalcAppPoolUsersVariable) / 5) * 5, 5))

   print(p)

   if(createPDF) {
      dev.off()
   }
   return(summarised)
}


# ###### Prepare results for table ##########################################
prepareDelayForTable <- function(data, shortenNames = TRUE)
{
   data <- data %>%
      filter(scenarioNumberOfCalcAppPoolUsersVariable %in% c(10, 60, 120)) %>%
      rename(Policy = calcAppPoolElementSelectionPolicy,
             PUs    = scenarioNumberOfCalcAppPoolUsersVariable) %>%
      mutate(PolicyType = getPolicyType(Policy)) %>%
      select(PUs, PolicyType, everything()) %>%  # Move PolicyType to front
      arrange(PUs)

   data$Policy <- getPolicyAbbreviations(data$Policy)

   if(shortenNames) {
      colnames(data) <- gsub("^(Queuing|Startup|Processing)", "", colnames(data))
   }

   return(data)
}


# ###### Plot results #######################################################
computeDelays <- function(name, prefix, createPDF = TRUE)
{
   # ====== Queuing =========================================================
   queuing <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageQueuingDelay.data.bz2")) %>%
                 select(c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable", "lan.calcAppPoolUserArray", "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay"))

   queuingSummary <- queuing %>%
      group_by(calcAppPoolElementSelectionPolicy, scenarioNumberOfCalcAppPoolUsersVariable) %>%
      summarise(QueuingMean = mean(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay),
                # QueuingMin  = min(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay),
                # QueuingMax  = max(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay),
                QueuingQ10  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay, 0.10, na.rm = TRUE),
                QueuingQ90  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay, 0.90, na.rm = TRUE)
               )
   # print(queuingSummary)

   writeTable(prepareDelayForTable(queuingSummary),
              name, prefix, "Queuing", "Queuing Time for each Policy")


   # ====== Startup =========================================================
   startup <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageStartupDelay.data.bz2")) %>%
                 select(c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable", "lan.calcAppPoolUserArray", "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay"))

   startupSummary <- startup %>%
      group_by(calcAppPoolElementSelectionPolicy, scenarioNumberOfCalcAppPoolUsersVariable) %>%
      summarise(StartupMean = mean(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay),
                # StartupMin  = min(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay),
                # StartupMax  = max(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay),
                StartupQ10  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay, 0.10, na.rm = TRUE),
                StartupQ90  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay, 0.90, na.rm = TRUE)
               )
   # print(startupSummary)

   writeTable(prepareDelayForTable(startupSummary),
              name, prefix, "Startup", "Startup Time for each Policy")


   # ====== Processing =========================================================
   processing <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageProcessingTime.data.bz2")) %>%
                    select(c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable", "lan.calcAppPoolUserArray", "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime"))

   processingSummary <- processing %>%
      group_by(calcAppPoolElementSelectionPolicy, scenarioNumberOfCalcAppPoolUsersVariable) %>%
      summarise(ProcessingMean = mean(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime),
                # ProcessingMin  = min(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime),
                # ProcessingMax  = max(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime),
                ProcessingQ10  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime, 0.10, na.rm = TRUE),
                ProcessingQ90  = quantile(lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime, 0.90, na.rm = TRUE)
               )
   # print(processingSummary)

   writeTable(prepareDelayForTable(processingSummary),
              name, prefix, "Processing", "Processing Time for each Policy")


   # ====== Join summaries ==================================================
   summary <- queuingSummary %>%
      left_join(startupSummary, by = c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable")) %>%
      left_join(processingSummary, by = c("calcAppPoolElementSelectionPolicy", "scenarioNumberOfCalcAppPoolUsersVariable"))

   writeTable(prepareDelayForTable(summary, FALSE),
              name, prefix, "QSP-Times", "Queuing, Startup and Processing Times for each Policy")


   # ====== Summary for plotting ============================================
   colnames(queuingSummary) <- gsub("^Queuing", "", colnames(queuingSummary))
   queuingSummary$Variable <- "Queuing Time"
   colnames(startupSummary) <- gsub("^Startup", "", colnames(startupSummary))
   startupSummary$Variable <- "Startup Time"
   colnames(processingSummary) <- gsub("^Processing", "", colnames(processingSummary))
   processingSummary$Variable <- "Processing Time"
   summary <- rbind(rbind(queuingSummary, startupSummary), processingSummary)
   summary$Variable <- factor(summary$Variable, levels=c("Queuing Time", "Startup Time", "Processing Time"))

   summary$PolicyType <- getPolicyType(summary$calcAppPoolElementSelectionPolicy)
   summary$calcAppPoolElementSelectionPolicy <- getPolicyAbbreviations(summary$calcAppPoolElementSelectionPolicy)

   # ====== Barplot =========================================================
   barplotData <- summary %>%
      filter(scenarioNumberOfCalcAppPoolUsersVariable %in% c(10, 60, 120)) %>%
      mutate(scenarioNumberOfCalcAppPoolUsersVariable = as.factor(scenarioNumberOfCalcAppPoolUsersVariable),
             calcAppPoolElementSelectionPolicy        = as.factor(calcAppPoolElementSelectionPolicy),
             #PolicyType                               = getPolicyType(calcAppPoolElementSelectionPolicy)
            ) %>%
      rename(Policy = calcAppPoolElementSelectionPolicy,
             PUs    = scenarioNumberOfCalcAppPoolUsersVariable)

   # Rename to "<n> PUs", but keep the sorting order:
   l <- paste(levels(factor(barplotData$PUs)), "PUs")
   barplotData$PUs <- factor(paste(as.vector(barplotData$PUs), "PUs"), levels=l)


   cairo_pdf(paste(sep="", name, "-", prefix, "-Barplot.pdf"),
             width=18, height=8, family="Helvetica", pointsize=32)

   plotColours <- c( "red", "blue", "green" )

   p <- ggplot(barplotData,
               aes(x    = Policy,
                   y    = Mean,
                   fill = Variable)
              ) +
        theme(title            = element_text(size=32),
              plot.title       = element_text(size=20, hjust = 0.5, face="bold"),
              axis.title       = element_text(size=20, face="bold"),
              strip.text       = element_text(size=18, face="bold"),
              axis.text.x      = element_text(size=16, angle=90, hjust=1.0, face="bold", colour="black"),
              axis.text.y      = element_text(size=16, angle=90, hjust=0.5, colour="black"),
              legend.title     = element_blank(),
              legend.text      = element_text(size=18, face="bold"),
              legend.position  = "bottom",
              # panel.grid.major = element_line(size=0.5,  linetype="solid", color="lightgray"),
              # panel.grid.minor = element_line(size=0.25, linetype="solid", color="lightgray")
              # strip.background = element_blank(),
              panel.spacing    = unit(1, "cm"),
              panel.grid.major = element_line(size=0.4, colour = "black"),
              panel.grid.minor = element_line(size=0.2, colour = "gray"),
              panel.background = element_rect(fill="white", color="black", size=2)
             ) +
           scale_fill_manual("", values=plotColours) +
           facet_grid( ~ PUs) +
           labs(y = "Mean Time [s]") +
           geom_bar(stat="identity")
   print(p)

   dev.off()


   # ====== Line plot =======================================================
   cairo_pdf(paste(sep="", name, "-", prefix, "-Line.pdf"),
             width=18, height=12, family="Helvetica", pointsize=32)

   lineplotData <- summary %>%
      # filter(scenarioNumberOfCalcAppPoolUsersVariable <= 160) %>%
      # mutate(PolicyType = getPolicyType(calcAppPoolElementSelectionPolicy)) %>%
      rename(Policy = calcAppPoolElementSelectionPolicy,
             PUs    = scenarioNumberOfCalcAppPoolUsersVariable)

   plotColours <- c(
      "gray50", "lightblue", "green", "red",
      "blue", "black", "purple", "orange"
   )

   p <- ggplot(lineplotData,
               aes(x = PUs,
                   y = Mean)
              ) +
        theme(title            = element_text(size=32),
              plot.title       = element_text(size=20, hjust = 0.5, face="bold"),
              axis.title       = element_text(size=20, face="bold"),
              strip.text       = element_text(size=18, face="bold"),
              axis.text.x      = element_text(size=16, angle=90, hjust=1.0, face="bold", colour="black"),
              axis.text.y      = element_text(size=16, angle=90, hjust=0.5, colour="black"),
              legend.title     = element_blank(),
              legend.text      = element_text(size=18, face="bold"),
              legend.position  = "bottom",
              # panel.grid.major = element_line(size=0.5,  linetype="solid", color="lightgray"),
              # panel.grid.minor = element_line(size=0.25, linetype="solid", color="lightgray")
              # strip.background = element_blank(),
              panel.spacing    = unit(1, "cm"),
              panel.grid.major = element_line(size=0.4, colour = "black"),
              panel.grid.minor = element_line(size=0.2, colour = "gray"),
              panel.background = element_rect(fill="white", color="black", size=2)
             ) +
           # coord_cartesian(ylim = c(0, 6)) +   # <<-- Sets y-axis limits without dropping values!
           facet_grid(PolicyType ~ Variable) +
           labs(y = "Mean Time [s]") +
           geom_line(aes(color = Policy), size=2) +
           scale_color_manual("", values=plotColours) +
           guides(colour = guide_legend(nrow = 1))
   print(p)

   dev.off()


   return(summary)
}


# ###### Main program #######################################################

systemSummary     <- systemSummaryTable("mec2-2hrO/Results",  "MEC2-2hrO")
dataUtilisation   <- plotPEUtilisation("mec2-2hrO/Results",   "MEC2-2hrO")
dataHandlingSpeed <- plotPUHandlingSpeed("mec2-2hrO/Results", "MEC2-2hrO")
summary           <- computeDelays("mec2-2hrO/Results",       "MEC2-2hrO")
