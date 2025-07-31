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
  std::string outFile;  //!< output file
  std::string inFile;   //!< input file
  std::string recPars;  //!< input reconstructed particles
  std::string genPars;  //!< input generated particles
  double      minQ2;    //!< min Q2 to analyze
  double      maxQ2;    //!< max Q2 to analyze
  double      nPow;     //!< power to raise xb to
} DefaultOptions {
  "testRunWithNEC.epic25061ncdis10x100minq10.d29m7y2025.root",
  "root://dtn-eic.jlab.org//volatile/eic/EPIC/RECO/25.06.1/epic_craterlake/DIS/NC/10x100/minQ2=10/pythia8NCDIS_10x100_minQ2=10_beamEffects_xAngle=-0.025_hiDiv_5.1287.eicrecon.edm4eic.root",
  "ReconstructedBreitFrameParticles",
  "GeneratedBreitFrameParticles",
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
    {"ene", {"E [GeV]", 201, -1., 200.}},
    {"ang", {"#theta_{breit} [rad]", 90, -3.15, 3.15}},
    {"rap", {"y = ln tan(#theta/2)", 200, -15., 5.}},
    {"weight", {"E/E_{p}", 21, -0.1, 2.}},
    {"x", {"x_{B}", 21, -0.1, 2.}},
    {"lnx", {"ln x_{B}", 300, -20., 10.}},
    {"q", {"Q^{2} [GeV/c]^{2}", 101, -10., 1000}},
    {"lnq", {"ln Q^{2}", 51, -1., 50.}}
  };

  // lambda to create histogram title
  auto makeTitle = [](
    const std::string& x,
    const std::string& y = "",
    const std::string& z = "",
    const std::string& t = ""
  ) {
    return t + ";" + x + ";" + y + ";" + z;
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
      makeTitle(axes[axis].title, ytitle, "", title).data(),
      axes[axis].num,
      axes[axis].start,
      axes[axis].stop
    );
  };

  // lambda to create a 2d histogram
  auto makeHist2D = [&axes, &makeTitle](
    const std::string& xaxis,
    const std::string& yaxis,
    const std::string& name,
    const std::string& ztitle = "",
    const std::string& title = ""
  ) {
    return TH2Def(
      name.data(),
      makeTitle(axes[xaxis].title, axes[yaxis].title, ztitle, title).data(),
      axes[xaxis].num,
      axes[xaxis].start,
      axes[xaxis].stop,
      axes[yaxis].num,
      axes[yaxis].start,
      axes[yaxis].stop
    );
  };

  // define 1d histograms/profiles
  //   - TODO add
  //       - lab rapidity (rec, gen)
  std::map<std::string, TH1Def> hist1D = {
    {"necXyrec", makeHist1D("rap", "hNECVsRapRec", "#LTNEC#GT")},
    {"necXygen", makeHist1D("rap", "hNECVsRapGen", "#LTNEC#GT")},
    {"necXthrec", makeHist1D("ang", "hNECVsThetaRec", "#LTNEC#GT")},
    {"necXthgen", makeHist1D("ang", "hNECVsThetaGen", "#LTNEC#GT")},
    {"thparrec", makeHist1D("ang", "hThetaParRec")},
    {"thpargen", makeHist1D("ang", "hThetaParGen")},
    {"yparrec", makeHist1D("rap", "hRapParRec")},
    {"ypargen", makeHist1D("rap", "hRapParGen")},
    {"eparrec", makeHist1D("ene", "hEneParRec")},
    {"epargen", makeHist1D("ene", "hEneParGen")},
    {"enucrec", makeHist1D("ene", "hEneNucRec")},
    {"enucgen", makeHist1D("ene", "hEneNucGen")},
    {"weight", makeHist1D("weight", "hEneFrac")},
    {"xrec", makeHist1D("x", "hXBRec")},
    {"xgen", makeHist1D("x", "hXBGen")},
    {"lnxrec", makeHist1D("lnx", "hLogXBRec")},
    {"lnxgen", makeHist1D("lnx", "hLogXBGen")},
    {"qrec", makeHist1D("q", "hQ2Rec")},
    {"qgen", makeHist1D("q", "hQ2Gen")},
    {"lnqrec", makeHist1D("lnq", "hLogQ2Rec")},
    {"lnqgen", makeHist1D("lnq", "hLogQ2Gen")}
  };

  // define 2d histograms/profiles
  //   - TODO add
  //       - nec vs. q2 (rec, gen)
  //       - lab vs. breit rapidity (rec, gen)
  //       - breit angle vs. rapidity (rec, gen)
  std::map<std::string, TH2Def> hist2D = {
    {"xrecXgen", makeHist2D("x", "x", "hXBRecVsGen")},
    {"lnxrecXgen", makeHist2D("lnx", "lnx", "hLogXBRecVsGen")},
    {"qrecXgen", makeHist2D("q", "q", "hQ2RecVsGen")},
    {"lnqrecXgen", makeHist2D("lnq", "lnq", "hLogQ2RecVsGen")}
  };
  std::cout << "    Defined histograms" << std::endl;

  // lambdas for analysis -----------------------------------------------------

  // TODO define
  //   - exclude DIS electron
  //   - calculate lab rapidity
  //   - calculate nec

  // check if inclusive kinematic collection is present
  auto hasKine = [](const std::vector<edm4eic::InclusiveKinematicsData>& kines) {
    return !kines.empty();
  };

  // check if particle collection is present
  auto hasPars = [](const std::vector<edm4eic::ReconstructedParticleData>& pars) {
    return !pars.empty();
  };

  // check if Q2 is in specified cuts 
  auto cutQ2 = [&opt](const float q2) {
    return ((q2 > opt.minQ2) && (q2 < opt.maxQ2));
  };

  // grab Q2 from an inclusive kinematics
  auto getQ2 = [](const std::vector<edm4eic::InclusiveKinematicsData>& kines) {
    return kines.front().Q2;
  }; 

  // grab xb from an inclusive kinematics
  auto getXB = [](const std::vector<edm4eic::InclusiveKinematicsData>& kines) {
    return kines.front().x;
  };

  // take log of a number
  auto doLog = [](const float num) {
    return std::log(num);
  };

  // extract particle energies
  //   - TODO merge with get breit/rapidity lambdas
  //     below
  auto getEnergies = [](const std::vector<edm4eic::ReconstructedParticleData>& pars) {
    std::vector<float> energies;
    for (const auto& par : pars) {
      energies.push_back(par.energy);
    }
    return energies;
  };

  // extract particle breit angle
  //   - n.b. by definition, the beam is at z = 0 in the breit frame
  //   - TODO expand this to extract additional information
  auto getBreitAngles = [](const std::vector<edm4eic::ReconstructedParticleData>& pars) {
    std::vector<float> angles;
    for (const auto& par : pars) {
      angles.push_back(
        edm4hep::utils::anglePolar(
          edm4hep::Vector3f(par.momentum.x, par.momentum.y, par.momentum.z)
        )
      );
    }
    return angles;
  };

  // calculate particle rapidity
  //   - TODO collect this into a struct and merge with
  //     getBreitAngle lambda
  auto getRapidity = [](const std::vector<float>& angles) {
    std::vector<float> raps;
    for (const float angle : angles) {
      raps.push_back(
        std::log(std::tan(angle/2))
      );
    }
    return raps;
  };

  // calculate weights (energy fractions)
  //   - FIXME beam 4-momentum should be extracted from
  //     kinematics
  //   - TODO allow for raising weight to power
  auto getWeights = [](const std::vector<float>& energies, const float xb) {
    std::vector<float> weights;
    for (const float energy : energies) {
      weights.push_back(xb * (energy / 100.));  // FIXME this is hacky!!
    }
    return weights;
  };

  // run analysis -------------------------------------------------------------

  auto analysis = frame.Filter(hasKine, {"InclusiveKinematicsElectron"})
                       .Filter(hasPars, {opt.recPars})
                       .Filter(hasPars, {opt.genPars})
                       .Define("q2Rec", getQ2, {"InclusiveKinematicsElectron"})
                       .Define("q2Gen", getQ2, {"InclusiveKinematicsTruth"})
                       .Define("lnQ2Rec", doLog, {"q2Rec"})
                       .Define("lnQ2Gen", doLog, {"q2Gen"})
                       .Filter(cutQ2, {"q2Rec"})
                       .Define("xbRec", getXB, {"InclusiveKinematicsElectron"})
                       .Define("xbGen", getXB, {"InclusiveKinematicsTruth"})
                       .Define("lnXBRec", doLog, {"xbRec"})
                       .Define("lnXBGen", doLog, {"xbGen"})
                       .Define("eRec", getEnergies, {opt.recPars})
                       .Define("eGen", getEnergies, {opt.genPars})
                       .Define("wRec", getWeights, {"eRec", "xbRec"})
                       .Define("wGen", getWeights, {"eGen", "xbGen"})
                       .Define("thRec", getBreitAngles, {opt.recPars})
                       .Define("thGen", getBreitAngles, {opt.genPars})
                       .Define("yRec", getRapidity, {"thRec"})
                       .Define("yGen", getRapidity, {"thGen"});

  // get 1d histograms
  auto hXBRec         = analysis.Histo1D(hist1D["xrec"], "xbRec");
  auto hXBGen         = analysis.Histo1D(hist1D["xgen"], "xbGen");
  auto hLogXBRec      = analysis.Histo1D(hist1D["lnxrec"], "lnXBRec");
  auto hLogXBGen      = analysis.Histo1D(hist1D["lnxgen"], "lnXBGen");
  auto hQ2Rec         = analysis.Histo1D(hist1D["qrec"], "q2Rec");
  auto hQ2Gen         = analysis.Histo1D(hist1D["qgen"], "q2Gen");
  auto hLogQ2Rec      = analysis.Histo1D(hist1D["lnqrec"], "lnQ2Rec");
  auto hLogQ2Gen      = analysis.Histo1D(hist1D["lnqgen"], "lnQ2Gen");
  auto hThetaParRec   = analysis.Histo1D(hist1D["thparrec"], "thRec");
  auto hThetaParGen   = analysis.Histo1D(hist1D["thpargen"], "thGen");
  auto hRapParRec     = analysis.Histo1D(hist1D["yparrec"], "yRec");
  auto hRapParGen     = analysis.Histo1D(hist1D["ypargen"], "yGen");
  auto hNECVsRapRec   = analysis.Histo1D(hist1D["necXyrec"], "yRec", "wRec");
  auto hNECVsRapGen   = analysis.Histo1D(hist1D["necXygen"], "yGen", "wGen");
  auto hNECVsThetaRec = analysis.Histo1D(hist1D["necXthrec"], "thRec", "wRec");
  auto hNECVsThetaGen = analysis.Histo1D(hist1D["necXthgen"], "thGen", "wGen");

  // get 2d histograms
  auto hXBRecVsGen    = analysis.Histo2D(hist2D["xrecXgen"], "xbGen", "xbRec");
  auto hLogXBRecVsGen = analysis.Histo2D(hist2D["lnxrecXgen"], "lnXBGen", "lnXBRec");
  auto hQ2RecVsGen    = analysis.Histo2D(hist2D["qrecXgen"], "q2Gen", "q2Rec");
  auto hLogQ2RecVsGen = analysis.Histo2D(hist2D["lnqrecXgen"], "lnQ2Gen", "lnQ2Rec");

  // save & close -------------------------------------------------------------

  // save histograms
  output         -> cd();
  hXBRec         -> Write();
  hXBGen         -> Write();
  hLogXBRec      -> Write();
  hLogXBGen      -> Write();
  hQ2Rec         -> Write();
  hQ2Gen         -> Write();
  hLogQ2Rec      -> Write();
  hLogQ2Gen      -> Write();
  hXBRecVsGen    -> Write();
  hLogXBRecVsGen -> Write();
  hQ2RecVsGen    -> Write();
  hLogQ2RecVsGen -> Write();
  hThetaParRec   -> Write();
  hThetaParGen   -> Write();
  hRapParRec     -> Write();
  hRapParGen     -> Write();
  hNECVsRapRec   -> Write();
  hNECVsRapGen   -> Write();
  hNECVsThetaRec -> Write();
  hNECVsThetaGen -> Write();

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
