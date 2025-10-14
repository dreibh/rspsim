<h1 align="center">
 RSPSIM<br />
 <span style="font-size: 30%">The Reliable Server Pooling Simulation Model</span><br />
 <a href="https://www.nntb.no/~dreibh/rserpool/">
  <img alt="RSPLIB Project Logo" src="logo/rsplib.svg" width="25%" /><br />
  <span style="font-size: 30%;">https://www.nntb.no/~dreibh/rserpool</span>
 </a>
</h1>


# 💡 What is RSPSIM?

[Reliable Server Pooling&nbsp;(RSerPool)](https://www.nntb.no/~dreibh/rserpool/#what-is-reliable-server-pooling-rserpool) is the new IETF framework for server pool management and session failover handling. In particular, it can be used for realising highly available services and load distribution. RSPSIM is a simulation model of RSerPool for the [OMNeT++](https://omnetpp.org/) discrete event simulator. It includes:

* The RSerPool model itself, including an application model for the [Calculation Application (CalcApp)](https://duepublico2.uni-due.de/servlets/MCRFileNodeServlet/duepublico_derivate_00016326/Dre2006_final.pdf#chapter.8) service.
* [SimProcTC](https://www.nntb.no/~dreibh/omnetpp/)-based tool-chain for running large-scale simulations, including support for using a real-world RSerPool setup with [RSPLIB](https://www.nntb.no/~dreibh/rserpool/) to distribute the simulation processing jobs.

Details about RSPSIM and the CalcApp service can be found in «[Reliable Server Pooling – Evaluation, Optimization and Extension of a Novel IETF Architecture](https://duepublico2.uni-due.de/servlets/MCRFileNodeServlet/duepublico_derivate_00016326/Dre2006_final.pdf#chapter.6)»!

Also take at look at [Thomas Dreibholz's Reliable Server Pooling Page](https://www.nntb.no/~dreibh/rserpool/) for details about the real-world RSerPool implementation RSPLIB.


# 💾 Build from Sources

RSPSIM is released under the [GNU General Public Licence&nbsp;(GPL)](https://www.gnu.org/licenses/gpl-3.0.en.html#license-text).

Please use the issue tracker at [https://github.com/dreibh/rspsim/issues](https://github.com/dreibh/rspsim/issues) to report bugs and issues!

## Development Version

The Git repository of the RSPSIM sources can be found at [https://github.com/dreibh/rspsim](https://github.com/dreibh/rspsim):

<pre><code><span class="fu">git</span> clone <a href="https://github.com/dreibh/rspsim">https://github.com/dreibh/rspsim</a>
<span class="bu">cd</span> rspsim
<span class="bu">cd</span> toolchain/tools && make <span class="fu">&&</span> <span class="bu">cd</span> ..
</code></pre>

Contributions:

* Issue tracker: [https://github.com/dreibh/rspsim/issues](https://github.com/dreibh/rspsim/issues).
  Please submit bug reports, issues, questions, etc. in the issue tracker!

* Pull Requests for RSPSIM: [https://github.com/dreibh/rspsim/pulls](https://github.com/dreibh/rspsim/pulls).
  Your contributions to RSPSIM are always welcome!

## Release Versions

See [https://www.nntb.no/~dreibh/rspsim/#current-stable-release](https://www.nntb.no/~dreibh/rspsim/#current-stable-release) for release packages!


# 🗃️ First Steps


## Directory Structure

* [`model`](https://github.com/dreibh/rspsim/blob/master/model): The model itself
* [`toolchain`](https://github.com/dreibh/rspsim/blob/master/toolchain): The SimProcTC files for parametrisation, run distribution and post-processing of results


## How to compile and run a simple model test

```bash
cd toolchain/tools && make && cd ../.. && \
cd model && \
opp_makemake -I . -f && \
make
```

Notes:

* Make sure to compile in the OMNeT++ Python environment (see the [OMNeT++ Installation Guide](https://doc.omnetpp.org/omnetpp/InstallGuide.pdf)), i.e.:

  ```bash
  source <PATH_TO_OMNET++_DIRECTORY>/setenv
  ```

  If `opp_makemake</tt> is not found, this step is likely missing!

* Make sure that everything compiles successfully. Otherwise, the tool-chain will not work properly!

After compilation, you can start the demo simulation by calling:

```bash
./model -f test1.ini
```

Take a look into [`test1.ini`](https://github.com/dreibh/rspsim/blob/master/model/test1.ini), the parameters should mostly be self-explaining. A detailed introduction to the model can be found in Chapter&nbsp;6 of «[Reliable Server Pooling – Evaluation, Optimization and Extension of a Novel IETF Architecture](https://duepublico2.uni-due.de/servlets/MCRFileNodeServlet/duepublico_derivate_00016326/Dre2006_final.pdf#chapter.6)»!


## How to run a SimProcTC-based example?

See [SimProcTC – A Simulation Processing Tool-Chain for OMNeT++ Simulations](https://www.nntb.no/~dreibh/omnetpp/) for the SimProcTC installation details. It particularly needs [GNU&nbsp;R](https://www.r-project.org/), [bzip2](https://sourceware.org/bzip2/) including headers, and `chrpath`. When it is installed, an R&nbsp;shell can be started in the [`toolchain`](https://github.com/dreibh/rspsim/blob/master/toolchain) directory:

```bash
cd toolchain
R --vanilla
```

In the R&nbsp;shell, [`example1.R`](https://github.com/dreibh/rspsim/blob/master/toolchain/example1.R) can be started:

```r
source("example1.R")
```

This will parametrise the simulation defined in [`example1.R`](https://github.com/dreibh/rspsim/blob/master/toolchain/example1.R), perform all runs (on the local machine) and collect the results.

The plot script [`plot-example1.R`](https://github.com/dreibh/rspsim/blob/master/toolchain/plot-example1.R) plots the results of the example1 run.

```r
source("plot-example1.R")
```

The resulting PDF file is `example1.pdf`.

Need more examples? Take a look into the directory [`example-scripts`](https://github.com/dreibh/rspsim/blob/master/toolchain/example-scripts). Each <tt><em>&lt;simulation&gt;</em>.R</tt> file is accompanied by a corresponding <tt>plot-<em>&lt;simulation&gt;</em>.R</tt> to create the corresponding PDF output. To use one of these scripts, copy it to the [`toolchain`](https://github.com/dreibh/rspsim/blob/master/toolchain) directory itself. They will **not** work in the [`example-scripts`](https://github.com/dreibh/rspsim/blob/master/toolchain/example-scripts) subdirectory!

To distribute simulation runs in a compute pool, and for more details about SimProcTC, see: [SimProcTC – A Simulation Processing Tool-Chain for OMNeT++ Simulations](https://www.nntb.no/~dreibh/omnetpp/).
All examples include the file [`computation-pool.R`](https://github.com/dreibh/rspsim/blob/master/toolchain/computation-pool.R), where the pool configuration has to be made!


# 🖋️ Citing RSPSIM in Publications

RSPSIM and related BibTeX entries can be found in [AllReferences.bib](https://www.nntb.no/~dreibh/rserpool/bibtex/AllReferences.bib)!

[Dreibholz, Thomas](https://www.nntb.no/~dreibh/): «[Reliable Server Pooling – Evaluation, Optimization and Extension of a Novel IETF Architecture](https://duepublico2.uni-due.de/servlets/MCRFileNodeServlet/duepublico_derivate_00016326/Dre2006_final.pdf)» ([PDF](https://duepublico2.uni-due.de/servlets/MCRFileNodeServlet/duepublico_derivate_00016326/Dre2006_final.pdf), 9080&nbsp;KiB, 267&nbsp;pages, 🇬🇧), University of Duisburg-Essen, Faculty of Economics, Institute for Computer Science and Business Information Systems, URN&nbsp;[urn:nbn:de:hbz:465-20070308-164527-0](https://nbn-resolving.org/urn:nbn:de:hbz:465-20070308-164527-0), March&nbsp;7, 2007.


# 🔗 Useful Links

## RSerPool Introductions on Wikipedia in Different Languages

* 🇧🇦 [Bosnian](https://bs.wikipedia.org/wiki/Reliable_Server_Pooling) (thanks to Nihad Cosić)
* 🇨🇳 [Chinese](https://zh.wikipedia.org/wiki/Reliable_Server_Pooling) (thanks to Xing Zhou)
* 🇭🇷 [Croatian](https://web.archive.org/web/20230709054321/https://hr.wikipedia.org/wiki/Reliable_server_Pooling) (thanks to Nihad Cosić)
* 🇬🇧 [English](https://en.wikipedia.org/wiki/Reliable_server_pooling)
* 🇫🇷 [French](https://fr.wikipedia.org/wiki/Reliable_Server_Pooling)
* 🇩🇪 [German](https://de.wikipedia.org/wiki/Reliable_Server_Pooling) (thanks to Jobin Pulinthanath)
* 🇮🇹 [Italian](https://it.wikipedia.org/wiki/Reliable_server_pooling)
* 🇳🇴 [Norwegian (bokmål)](https://no.wikipedia.org/wiki/Reliable_Server_Pooling)

What about helping Wikipedia by adding an article in your language?

## Networking and System Management Software

* [Thomas Dreibholz's Reliable Server Pooling (RSerPool) Page](https://www.nntb.no/~dreibh/rserpool/)
* [NetPerfMeter – A TCP/MPTCP/UDP/SCTP/DCCP Network Performance Meter Tool](https://www.nntb.no/~dreibh/netperfmeter/)
* [HiPerConTracer – High-Performance Connectivity Tracer](https://www.nntb.no/~dreibh/hipercontracer/)
* [TSCTP – An SCTP test tool](https://www.nntb.no/~dreibh/tsctp/)
* [_sctplib_ and _socketapi_ – The User-Space SCTP Library (_sctplib_) and Socket API Library (_socketapi_)](https://www.nntb.no/~dreibh/sctplib/)
* [SubNetCalc – An IPv4/IPv6 Subnet Calculator](https://www.nntb.no/~dreibh/subnetcalc/)
* [System-Tools – Tools for Basic System Management](https://www.nntb.no/~dreibh/system-tools/)
* [Wireshark](https://www.wireshark.org/)

## Simulation and Data Processing Software

* [OMNeT++ Discrete Event Simulator](https://omnetpp.org/)
* [The R Project for Statistical Computing](https://www.r-project.org)

## Other Resources

* [Thomas Dreibholz's SCTP Page](https://www.nntb.no/~dreibh/sctp/)
* [Thomas Dreibholz's Multi-Path TCP (MPTCP) Page](https://www.nntb.no/~dreibh/mptcp/)
* [Michael Tüxen's SCTP page](https://www.sctp.de/)
* [Michael Tüxen's RSerPool page](https://www.sctp.de/rserpool.html)
* [NorNet – A Real-World, Large-Scale Multi-Homing Testbed](https://www.nntb.no/)
* [GAIA – Cyber Sovereignty](https://gaia.nntb.no/)
