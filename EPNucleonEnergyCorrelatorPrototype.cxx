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
#include <iostream>
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
  19.5,
  20.5,
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
  std::vector<Axis> axes = {
    {"E [GeV]", 200, 0., 200.},
    {"y = ln tan(#theta/2)", 200, -15, 5},
    {"E/E_{p}", 30, -1., 2.},
    {"x_{B}", 60, -1., 2.}
  };

  // lambda to create histogram title
  auto title = [](const std::string& x, const std::string& y = "", const std::string& t = "") {
    return t + ";" + x + ";" + y;
  };

  // define histograms
  std::vector<TH1Def> hists = {
    TH1Def("hNEC", title(axes[1].title, "#GTNEC#LT").data(), axes[1].num, axes[1].start, axes[2].stop),
    TH1Def("hRapPar", title(axes[1].title).data(), axes[1].num, axes[1].start, axes[1].stop),
    TH1Def("hEnePar", title(axes[0].title).data(), axes[0].num, axes[0].start, axes[0].stop),
    TH1Def("hEneNuc", title(axes[0].title).data(), axes[0].num, axes[0].start, axes[0].stop),
    TH1Def("hEneFrac", title(axes[2].title).data(), axes[2].num, axes[2].start, axes[2].stop),
    TH1Def("hXBjork", title(axes[3].title).data(), axes[3].num, axes[3].start, axes[3].stop)
  };
  std::cout << "    Defined histograms" << std::endl;

  // lambdas for analysis -----------------------------------------------------

  auto hasKine = [](std::vector<edm4eic::InclusiveKinematicsData> kines) {
    return !kines.empty();
  };

  auto cutQ2 = [&opt](std::vector<edm4eic::InclusiveKinematicsData> kines) {
    return ((kines.front().Q2 > opt.minQ2) && (kines.front().Q2 < opt.maxQ2));
  };

  auto getXB = [](std::vector<edm4eic::InclusiveKinematicsData> kines) {
    return kines.front().Q2;
  };

  // run analysis -------------------------------------------------------------

  auto analysis = frame.Filter(hasKine, {"InclusiveKinematicsElectron"})
                       .Filter(cutQ2, {"InclusiveKinematicsElectron"})
                       .Define("xBjork", getXB, {"InclusiveKinematicsElectron"});

  // get histograms
  auto hXBjork = analysis.Histo1D(hists[5], "xBjork");

  // save & close -------------------------------------------------------------

  // save histograms
  output  -> cd();
  hXBjork -> Write();

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
