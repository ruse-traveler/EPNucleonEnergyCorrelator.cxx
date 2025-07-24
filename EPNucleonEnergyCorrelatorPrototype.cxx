// ============================================================================
//! \file   EPNucleonEnergyCorrelatorPrototype.cxx
//! \author Derek Anderson
//! \date   07.12.2025
// ----------------------------------------------------------------------------
//! Prototype macro to calculate Nucleon Energy Correlators
//! using EIC software
// ============================================================================

#define EPNucleonEnergyCorrelatorPrototype_cxx

// edm types
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
// root libraries
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RDF/HistoModels.hxx>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
// c++ utilities
#include <cmath>
#include <iostream>
#include <map>
#include <vector>

// alias for convenience
using RDF    = ROOT::RDataFrame;
using TH1Def = ROOT::RDF::TH1DModel;
using TH2Def = ROOT::RDF::TH2DModel;



// ============================================================================
//! Struct to consolidate user options
// ============================================================================
struct Options {
  std::string inFile;   //!< input file
  std::string outFile;  //!< output file
  double      minQ2;    //!< min Q2 to analyze
  double      maxQ2;    //!< max Q2 to analyze
  double      nPow;     //!< power to raise xb to
} DefaultOptions {
  "root://dtn-eic.jlab.org//volatile/eic/EPIC/RECO/25.06.1/epic_craterlake/DIS/NC/10x100/minQ2=10/pythia8NCDIS_10x100_minQ2=10_beamEffects_xAngle=-0.025_hiDiv_5.1287.eicrecon.edm4eic.root",
  "test.root",
  0.0,
  100.0,
  1.0
};



// ============================================================================
//! Helper struct for histogram axes/binning
// ============================================================================
struct Axis {
  std::string title;  //!< title of axis
  std::size_t num;  //!< no. of bins
  double      start;  //!< low edge of bin 1
  double      stop;   //!< low edge of bin num+1
};



// ============================================================================
//! Prototype Nucleon Energy Correlator Calculation
// ============================================================================
void EPNucleonEnergyCorrelatorPrototype(const Options& opt = DefaultOptions) {

  // turn on histogram errors
  TH1::SetDefaultSumw2(true);
  TH2::SetDefaultSumw2(true);

  // announce start
  std::cout << "\n  Starting prototype NEC calculation!" << std::endl;

  // open input/output --------------------------------------------------------

  // create output file
  TFile* output = new TFile(opt.outFile.data(), "recreate");
  if (!output) {
    std::cerr << "PANIC: couldn't open output file!\n" << std::endl;
    return;
  }
  std::cout << "    Opened output file" << std::endl;

  // initialize dataframe
  ROOT::RDataFrame frame("events", opt.inFile);
  if (frame.Count() == 0) {
    std::cerr << "PANIC: no events found!" << std::endl;
    return;
  }
  std::cout << "    Opened RDataFrame" << std::endl;

  // define histograms --------------------------------------------------------

  // binning definitions
  std::map<std::string, Axis> axes = {
    {"ene", {"E [GeV]", 200, 0., 200.}},
    {"rap", {"y = ln tan(#theta/2)", 200, -15, 5}},
    {"weight", {"E/E_{p}", 30, -1., 2.}},
    {"x", {"x_{B}", 60, -1., 2.}},
    {"lnx", {"ln x_{B}", 100, -50., 50.}}
  };

  // lambda to create histogram title
  auto makeTitle = [](
    const std::string& x,
    const std::string& y = "",
    const std::string& t = ""
  ) {
    return t + ";" + x + ";" + y;
  };

  // lambda to create a 1d histogram
  auto makeHist1D = [&axes, &makeTitle](
    const std::string& axis,
    const std::string& name,
    const std::string& ytitle = "",
    const std::string& title = ""
  ) {
    return TH1Def(
      name.data(),
      makeTitle(axes[axis].title, title, ytitle).data(),
      axes[axis].num,
      axes[axis].start,
      axes[axis].stop
    );
  };

  // define 1D histograms
  std::map<std::string, TH1Def> hists = {
    {"nec", makeHist1D("rap", "hNEC", "#GTNEC#LT")},
    {"ypar", makeHist1D("rap", "hRapPar")},
    {"epar", makeHist1D("ene", "hEnePar")},
    {"enuc", makeHist1D("ene", "hEneNuc")},
    {"weight", makeHist1D("weight", "hEneFrac")},
    {"xrec", makeHist1D("x", "hXBRec")},
    {"xgen", makeHist1D("x", "hXBGen")},
    {"lnxrec", makeHist1D("lnx", "hLogXBRec")},
    {"lnxgen", makeHist1D("lnx", "hLogXBGen")}
  };
  std::cout << "    Defined histograms" << std::endl;

  // lambdas for analysis -----------------------------------------------------

  // check if inclusive kinematic collection is present
  auto hasKine = [](std::vector<edm4eic::InclusiveKinematicsData> kines) {
    return !kines.empty();
  };

  // grab Q2 from an inclusive kinematics
  auto cutQ2 = [&opt](std::vector<edm4eic::InclusiveKinematicsData> kines) {
    return ((kines.front().Q2 > opt.minQ2) && (kines.front().Q2 < opt.maxQ2));
  };

  // grab xb from an inclusive kinematics
  auto getXB = [](std::vector<edm4eic::InclusiveKinematicsData> kines) {
    return kines.front().x;
  };

  // take log of a number
  auto logXB = [](float xb) {
    return std::log(xb);
  };

  // run analysis -------------------------------------------------------------

  auto analysis = frame.Filter(hasKine, {"InclusiveKinematicsElectron"})
                       .Filter(cutQ2, {"InclusiveKinematicsElectron"})
                       .Define("xbRec", getXB, {"InclusiveKinematicsElectron"})
                       .Define("lnxbRec", logXB, {"xbRec"})
                       .Define("xbGen", getXB, {"InclusiveKinematicsTruth"})
                       .Define("lnxbGen", logXB, {"xbGen"});

  // get histograms
  auto hXBRec    = analysis.Histo1D(hists["xrec"], "xbRec");
  auto hXBGen    = analysis.Histo1D(hists["xgen"], "xbGen");
  auto hLogXBRec = analysis.Histo1D(hists["lnxrec"], "lnxbRec");
  auto hLogXBGen = analysis.Histo1D(hists["lnxgen"], "lnxbGen");

  // save & close -------------------------------------------------------------

  // save histograms
  output    -> cd();
  hXBRec    -> Write();
  hXBGen    -> Write();
  hLogXBRec -> Write();
  hLogXBGen -> Write();

  // close files
  output -> cd();
  output -> Close();
  std::cout << "    Closed output file\n"
            << "  NEC calculation finished!\n"
            << std::endl;

  // exit macro
  return;

}

// end ========================================================================
