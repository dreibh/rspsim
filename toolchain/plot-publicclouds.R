library("data.table")
library("xtable")

mediaTypes     <- c("SD", "HD", "4K")
mediaTypeSizes <- 1024**3 * c(1, 3, 7)
handshake      <- 4096
mtu            <- 1500
mss            <- mtu - 40   # TCP/IPv4


# % %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# % Please add the following required packages to your document preamble:
# % \usepackage{multirow}
# \begin{table}
# \caption{Throughput of public cloud service providers}
# \begin{adjustbox}{max width=14.75cm}
# \begin{tabular}{|c|ccc|ccc|ccc|}
# \hline
# \multirow{2}{*}{\textbf{Region}}                                         & \multicolumn{3}{c|}{\textbf{\begin{tabular}[c]{@{}c@{}}Compute Throughput \\ (Up) Mb/s\end{tabular}}} & \multicolumn{3}{c|}{\textbf{\begin{tabular}[c]{@{}c@{}}Compute Throughput \\ (Downlink) Mb/s\end{tabular}}} & \multicolumn{3}{c|}{\textbf{\begin{tabular}[c]{@{}c@{}}Storage Throughput \\ (Downlink)  Mb/s\end{tabular}}} \\ \cline{2-10}
#                                                                          & \multicolumn{1}{c|}{Mean}         & \multicolumn{1}{c|}{10th Percentile}       & 90th Percentile      & \multicolumn{1}{c|}{Mean}           & \multicolumn{1}{c|}{10th Percentile}         & 90th Percentile        & \multicolumn{1}{c|}{Mean}           & \multicolumn{1}{c|}{10th Percentile}         & 90th Percentile         \\ \hline
# \begin{tabular}[c]{@{}c@{}}Amazon Cloud\\ (Stockholm)\end{tabular}       & \multicolumn{1}{c|}{100.95}       & \multicolumn{1}{c|}{96.48}                 & 107.81               & \multicolumn{1}{c|}{85.66}          & \multicolumn{1}{c|}{76.69}                   & 95.33                  & \multicolumn{1}{c|}{99.57}          & \multicolumn{1}{c|}{82.04}                   & 118.95                  \\ \hline
# \begin{tabular}[c]{@{}c@{}}Microsoft Azure \\ (Norway East)\end{tabular} & \multicolumn{1}{c|}{113.55}       & \multicolumn{1}{c|}{107.85}                & 118.66               & \multicolumn{1}{c|}{105.45}         & \multicolumn{1}{c|}{99.53}                   & 112.39                 & \multicolumn{1}{c|}{42.48}          & \multicolumn{1}{c|}{35.89}                   & 47.44                   \\ \hline
# \begin{tabular}[c]{@{}c@{}}Google Cloud\\ (Finland)\end{tabular}         & \multicolumn{1}{c|}{102.62}       & \multicolumn{1}{c|}{100.03}                & 105.31               & \multicolumn{1}{c|}{111.92}         & \multicolumn{1}{c|}{102.92}                  & 119.96                 & \multicolumn{1}{c|}{147.09}         & \multicolumn{1}{c|}{112.67}                  & 167                     \\ \hline
# \end{tabular}
# \end{adjustbox}
# \end{table}
# % %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


# ###### Text sanitiser for xtable(): Use LaTeX bold text ###################
bold.latex <- function(text)
{
   return(paste(sep="", "{\\textbf{", text, "}}"))
}


# ###### Compute download time ##############################################
computeDownloadTime <- function(downloadSpeed, mediaType)
{
   if(!identical(as.character(downloadSpeed), character(0))) {
      payload <- mediaTypeSizes[which(mediaTypes == mediaType)]
      packets <- ceiling(payload / mss)

      bytes <- handshake + (packets * mtu)
      bps   <- (1000000 / 8) * downloadSpeed
      time  <- bytes / bps

      m <- floor(time / 60)
      s <- floor(time - (60 * m))
      return(sprintf("%1.0f:%02.0f", m, s))
   }
   return("--")
}


data <- data.table(
   Cloud            = c("Amazon Cloud", "Microsoft Azure", "Google Cloud"),
   Region           = c("Stockholm",    "Norway East",     "Finland"),
   AvgDownloadSpeed = c(100.95, 113.55, 102.62)
)

data$SD   <- lapply(data$AvgDownloadSpeed, function(x) computeDownloadTime(x, "SD"))
data$HD   <- lapply(data$AvgDownloadSpeed, function(x) computeDownloadTime(x, "HD"))
data$"4K" <- lapply(data$AvgDownloadSpeed, function(x) computeDownloadTime(x, "4K"))


latexName <- "PublicClouds.tex"
latexOutputTable <- xtable(data,
                           align   = "l|c|c|c|c|c|c|",
                           label   = "tab:Public-Clouds",
                           caption = "Content downloading time (minutes:seconds) from public Cloud providers")
print(latexOutputTable,
      size                       = "footnotesize",
      include.rownames           = FALSE,
      sanitize.colnames.function = bold.latex,
      # sanitize.text.function     = flag.latex,
      table.placement            = NULL,
      caption.placement          = "top",
      hline.after                = c(-1, 0, seq(0, nrow(latexOutputTable))),
      floating.environment       = "table*",
      NA.string                  = "--",
      file                       = latexName,
      type                       = "latex")
cat(sep="", "Wrote ", latexName, "\n")
