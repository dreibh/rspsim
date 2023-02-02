#!/usr/bin/Rscript --vanilla
#  =================================================================
#           #     #                 #     #
#           ##    #   ####   #####  ##    #  ######   #####
#           # #   #  #    #  #    # # #   #  #          #
#           #  #  #  #    #  #    # #  #  #  #####      #
#           #   # #  #    #  #####  #   # #  #          #
#           #    ##  #    #  #   #  #    ##  #          #
#           #     #   ####   #    # #     #  ######     #
#
#        ---   The NorNet Testbed for Multi-Homed Systems  ---
#                        https://www.nntb.no
#  =================================================================
#
#  High-Performance Connectivity Tracer (HiPerConTracer)
#  Copyright (C) 2020-2023 by Thomas Dreibholz
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#  Contact: dreibh@simula.no

dir.create(path = Sys.getenv("R_LIBS_USER"), showWarnings = FALSE, recursive = TRUE)
options(repos = "https://cran.uib.no/", Ncpus = parallel::detectCores(), lib = Sys.getenv("R_LIBS_USER"))

update.packages(ask=FALSE, lib = Sys.getenv("R_LIBS_USER"))

install.packages("OpenStreetMap", lib = Sys.getenv("R_LIBS_USER"))
install.packages("RMySQL", lib = Sys.getenv("R_LIBS_USER"))
install.packages("Rcpp", lib = Sys.getenv("R_LIBS_USER"))
install.packages("R.utils", lib = Sys.getenv("R_LIBS_USER"))
install.packages("anytime", lib = Sys.getenv("R_LIBS_USER"))
install.packages("data.table", lib = Sys.getenv("R_LIBS_USER"))
install.packages("doParallel", lib = Sys.getenv("R_LIBS_USER"))
install.packages("dplyr", lib = Sys.getenv("R_LIBS_USER"))
install.packages("fields", lib = Sys.getenv("R_LIBS_USER"))
install.packages("foreach", lib = Sys.getenv("R_LIBS_USER"))
install.packages("geosphere", lib = Sys.getenv("R_LIBS_USER"))
install.packages("ggflags", lib = Sys.getenv("R_LIBS_USER"))
install.packages("ggforce", lib = Sys.getenv("R_LIBS_USER"))
install.packages("ggmap", lib = Sys.getenv("R_LIBS_USER"))
install.packages("ggplot2", lib = Sys.getenv("R_LIBS_USER"))
install.packages("ggpubr", lib = Sys.getenv("R_LIBS_USER"))
install.packages("ggrepel", lib = Sys.getenv("R_LIBS_USER"))
install.packages("gridExtra", lib = Sys.getenv("R_LIBS_USER"))
install.packages("hexbin", lib = Sys.getenv("R_LIBS_USER"))
install.packages("kknn", lib = Sys.getenv("R_LIBS_USER"))
install.packages("plot3D", lib = Sys.getenv("R_LIBS_USER"))
install.packages("princurve", lib = Sys.getenv("R_LIBS_USER"))
install.packages("pryr", lib = Sys.getenv("R_LIBS_USER"))
install.packages("readr", lib = Sys.getenv("R_LIBS_USER"))
install.packages("rgeolocate", lib = Sys.getenv("R_LIBS_USER"))
install.packages("rgl", lib = Sys.getenv("R_LIBS_USER"))
install.packages("rlang", lib = Sys.getenv("R_LIBS_USER"))
install.packages("scales", lib = Sys.getenv("R_LIBS_USER"))
install.packages("sp", lib = Sys.getenv("R_LIBS_USER"))

devtools::install_github('rensa/ggflags')
