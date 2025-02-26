/**
 * File              : AliAnalysisTaskAR.cxx
 * Author            : Anton Riedel <anton.riedel@tum.de>
 * Date              : 07.05.2021
 * Last Modified Date: 03.09.2021
 * Last Modified By  : Anton Riedel <anton.riedel@tum.de>
 */

/*************************************************************************
 * Copyright(c) 1998-2008, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include "AliAnalysisTaskAR.h"
#include "AliAODEvent.h"
#include "AliAODHeader.h"
#include "AliAODInputHandler.h"
#include "AliAODMCParticle.h"
#include "AliLog.h"
#include "AliMCEvent.h"
#include "AliMultSelection.h"
#include "AliVEvent.h"
#include "AliVParticle.h"
#include "AliVTrack.h"
#include <TColor.h>
#include <TFile.h>
#include <TH1.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TSystem.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

ClassImp(AliAnalysisTaskAR)

    AliAnalysisTaskAR::AliAnalysisTaskAR(const char *name,
                                         Bool_t useParticleWeights)
    : AliAnalysisTaskSE(name),
      // Constructor
      // Base list for all output objects
      fHistList(nullptr), fHistListName("outputStudentAnalysis"),
      // list holding all QA histograms
      fQAHistogramsList(nullptr), fQAHistogramsListName("QAHistograms"),
      fFillQAHistograms(kFALSE),
      // sublist holding centrality estimator correlation QA histograms
      fCenCorQAHistogramsList(nullptr),
      fCenCorQAHistogramsListName("CenCorQAHistograms"),
      // sublist holding multiplicity correlation QA histograms
      fMulCorQAHistogramsList(nullptr),
      fMulCorQAHistogramsListName("MulCorQAHistograms"),
      // sublist holding filterbit scan QA histograms
      fFBScanQAHistogramsList(nullptr),
      fFBScanQAHistogramsListName("FBScanQAHistograms"),
      // sublist holding self correlation QA histograms
      fSelfCorQAHistogramsList(nullptr),
      fSelfCorQAHistogramsListName("SelfCorQAHistograms"),
      // list holding all control histograms
      fControlHistogramsList(nullptr),
      fControlHistogramsListName("ControlHistograms"),
      // sublists holding all track control histograms
      fTrackControlHistogramsList(nullptr),
      fTrackControlHistogramsListName("TrackControlHistograms"),
      // sublists holding all event control histograms
      fEventControlHistogramsList(nullptr),
      fEventControlHistogramsListName("EventControlHistograms"),
      // cuts
      fFilterbit(128), fPrimaryOnly(kFALSE), fCentralityEstimator(kV0M),
      // final results
      fFinalResultsList(nullptr), fFinalResultsListName("FinalResults"),
      // flags for MC analysis
      fMCOnTheFly(kFALSE), fMCClosure(kFALSE), fSeed(0), fUseCustomSeed(kFALSE),
      fMCPdf(nullptr), fMCPdfName("pdf"), fMCFlowHarmonics({}),
      fMCNumberOfParticlesPerEventFluctuations(kFALSE),
      fMCNumberOfParticlesPerEvent(500), fLookUpTable(nullptr),
      // qvectors
      fWeightsAggregated({}), fUseWeightsAggregated(kFALSE),
      fResetWeightsAggregated(kFALSE), fCorrelators({}) {
  AliDebug(2, "AliAnalysisTaskAR::AliAnalysisTaskAR(const "
              "char *name, Bool_t useParticleWeights)");

  // create base list
  fHistList = new TList();
  fHistList->SetName(fHistListName);
  fHistList->SetOwner(kTRUE);

  // initialize all arrays
  this->InitializeArrays();

  DefineOutput(1, TList::Class());

  // legacy, pending removal?
  // Define input and output slots here
  // Input slot #0 works with an AliFlowEventSimple
  // DefineInput(0, AliFlowEventSimple::Class());
  // Input slot #1 is needed for the weights input file:
  // if(useParticleWeights)
  //{
  // DefineInput(1, TList::Class());
  //}
  // Output slot #0 is reserved
  // Output slot #1 writes into a TList container
  // if (useParticleWeights) {
  // TBI
  // }
}

AliAnalysisTaskAR::AliAnalysisTaskAR()
    : AliAnalysisTaskSE(),
      // Dummy constructor
      // Base list for all output objects
      fHistList(nullptr), fHistListName("outputStudentAnalysis"),
      // list holding all QA histograms
      fQAHistogramsList(nullptr), fQAHistogramsListName("QAHistograms"),
      fFillQAHistograms(kFALSE),
      // sublist holding centrality estimator correlation QA histograms
      fCenCorQAHistogramsList(nullptr),
      fCenCorQAHistogramsListName("CenCorQAHistograms"),
      // sublist holding multiplicity correlation QA histograms
      fMulCorQAHistogramsList(nullptr),
      fMulCorQAHistogramsListName("MulCorQAHistograms"),
      // sublist holding filterbit scan QA histograms
      fFBScanQAHistogramsList(nullptr),
      fFBScanQAHistogramsListName("FBScanQAHistograms"),
      // sublist holding self correlation QA histograms
      fSelfCorQAHistogramsList(nullptr),
      fSelfCorQAHistogramsListName("SelfCorQAHistograms"),
      // list holding all control histograms
      fControlHistogramsList(nullptr),
      fControlHistogramsListName("ControlHistograms"),
      // sublists holding all track control histograms
      fTrackControlHistogramsList(nullptr),
      fTrackControlHistogramsListName("TrackControlHistograms"),
      // sublists holding all event control histograms
      fEventControlHistogramsList(nullptr),
      fEventControlHistogramsListName("EventControlHistograms"),
      // cuts
      fFilterbit(128), fPrimaryOnly(kFALSE), fCentralityEstimator(kV0M),
      // final results
      fFinalResultsList(nullptr), fFinalResultsListName("FinalResults"),
      // flags for MC analysis
      fMCOnTheFly(kFALSE), fMCClosure(kFALSE), fSeed(0), fUseCustomSeed(kFALSE),
      fMCPdf(nullptr), fMCPdfName("pdf"), fMCFlowHarmonics({}),
      fMCNumberOfParticlesPerEventFluctuations(kFALSE),
      fMCNumberOfParticlesPerEvent(500), fLookUpTable(nullptr),
      // qvectors
      fWeightsAggregated({}), fUseWeightsAggregated(kFALSE),
      fResetWeightsAggregated(kFALSE), fCorrelators({}) {
  // initialize arrays
  this->InitializeArrays();
  AliDebug(2, "AliAnalysisTaskAR::AliAnalysisTaskAR()");
}

AliAnalysisTaskAR::~AliAnalysisTaskAR() {
  // Destructor

  // fHistlist owns (almost) all other data members, if we delete it, we will
  // recursively delete all other objects associative with this object
  if (fHistList) {
    delete fHistList;
  }

  // delete lookuptable
  if (fLookUpTable) {
    delete fLookUpTable;
  }

  // delete RNG, if neccessary
  if (fMCOnTheFly || fMCClosure) {
    delete gRandom;
  }
  // delete pdf, if necessary
  if (fMCOnTheFly) {
    delete fMCPdf;
  }
};

void AliAnalysisTaskAR::UserCreateOutputObjects() {
  // Called at every worker node to initialize objects

  // 1) Trick to avoid name clashes, part 1
  // 2) Book and nest all lists
  // 3) Book all objects
  // *) Trick to avoid name clashes, part 2

  // 1) Trick to avoid name clashes, part 1
  Bool_t oldHistAddStatus = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);

  // 2) Book and nest all lists
  this->BookAndNestAllLists();

  // 3) Book all objects
  if (fFillQAHistograms) {
    this->BookQAHistograms();
  }
  this->BookControlHistograms();
  this->BookFinalResultHistograms();
  this->BookFinalResultProfiles();
  this->fLookUpTable = new TExMap();

  if (fMCOnTheFly) {
    this->BookMCOnTheFlyObjects();
  }
  if (fMCOnTheFly || fMCClosure) {
    delete gRandom;
    fUseCustomSeed ? gRandom = new TRandom3(fSeed) : gRandom = new TRandom3(0);
  }

  // *) Trick to avoid name clashes, part
  TH1::AddDirectory(oldHistAddStatus);

  PostData(1, fHistList);
}

void AliAnalysisTaskAR::Terminate(Option_t *) {
  // Accessing the merged output list for final compution or for off-line
  // computations (i.e. after merging)

  // fHistList = (TList *)GetOutputData(1);
  if (!fHistList) {
    std::cout << __LINE__ << ": Did not get " << fHistListName << std::endl;
    Fatal("Terminate", "Invalid Pointer to fHistList");
  }

  /* get average value of phi and write it into its own histogram */
  fFinalResultHistograms[kPHIAVG]->SetBinContent(
      1, fTrackControlHistograms[kRECO][kPHI][kAFTER]->GetMean());

  // for local MC Analysis
  // compute analytical values of correlators and fill them into final result
  // profile
  if (fMCOnTheFly) {
    Double_t theoryValue = 1.;
    for (auto V : fCorrelators) {
      theoryValue = 1.;
      for (auto i : V) {
        theoryValue *= fMCFlowHarmonics.at(abs(i) - 1);
      }
      fFinalResultProfiles[kHARTHEO]->Fill(V.size() - 1.5, theoryValue);
    }
  }
}

void AliAnalysisTaskAR::InitializeArrays() {
  // Initialize all data members which are arrays in this method
  InitializeArraysForQAHistograms();
  InitializeArraysForTrackControlHistograms();
  InitializeArraysForEventControlHistograms();
  InitializeArraysForCuts();
  InitializeArraysForWeights();
  InitializeArraysForQvectors();
  InitializeArraysForFinalResultHistograms();
  InitializeArraysForFinalResultProfiles();
  InitializeArraysForMCAnalysis();
}

void AliAnalysisTaskAR::InitializeArraysForQAHistograms() {
  // initialize array of QA histograms for the correlation between centrality
  // estimators
  // there are N(N-1)/2 such correlators, i.e. the number of elemets above/below
  // the diagonal of a square matrix
  for (int cen = 0; cen < LAST_ECENESTIMATORS * (LAST_ECENESTIMATORS - 1) / 2;
       ++cen) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fCenCorQAHistograms[cen][ba] = nullptr;
    }
  }

  // initialize names in a loop
  for (int cen1 = 0; cen1 < LAST_ECENESTIMATORS; ++cen1) {
    for (int cen2 = cen1 + 1; cen2 < LAST_ECENESTIMATORS; ++cen2) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        fCenCorQAHistogramNames[IndexCorHistograms(
            cen1, cen2, LAST_ECENESTIMATORS)][ba][kNAME] =
            "fCorCenEstimatorQAHistograms[" + kCenEstimatorNames[cen1] + "+" +
            kCenEstimatorNames[cen2] + "]" + kBAName[ba];
        fCenCorQAHistogramNames[IndexCorHistograms(
            cen1, cen2, LAST_ECENESTIMATORS)][ba][kTITLE] =
            kCenEstimatorNames[cen1] + " vs " + kCenEstimatorNames[cen2] +
            kBAName[ba];
        fCenCorQAHistogramNames[IndexCorHistograms(
            cen1, cen2, LAST_ECENESTIMATORS)][ba][kXAXIS] =
            kCenEstimatorNames[cen1];
        fCenCorQAHistogramNames[IndexCorHistograms(
            cen1, cen2, LAST_ECENESTIMATORS)][ba][kYAXIS] =
            kCenEstimatorNames[cen2];
      }
    }
  }

  // set default bins
  Double_t CorCenEstimatorQAHistogramBins[2 * LAST_EBINS] = {
      // kBIN kLEDGE kUEDGE kBIN+LAST_EBINS kLEDGE+LAST_EBINS
      // kUEDGE+LAST_EBINS
      50., 0., 100., 50., 0., 100.};
  // initialize default bins
  for (int cen = 0; cen < LAST_ECENESTIMATORS * (LAST_ECENESTIMATORS - 1) / 2;
       ++cen) {
    for (int bin = 0; bin < 2 * LAST_EBINS; ++bin) {
      fCenCorQAHistogramBins[cen][bin] = CorCenEstimatorQAHistogramBins[bin];
    }
  }

  // initialize arrays for multiplicity correlation histograms
  // there are also N(N-1)/2 such correlators
  for (int mul = 0; mul < kMulEstimators * (kMulEstimators - 1) / 2; ++mul) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fMulCorQAHistograms[mul][ba] = nullptr;
    }
  }

  // initalize names in a loop
  for (int mul1 = 0; mul1 < kMulEstimators; ++mul1) {
    for (int mul2 = mul1 + 1; mul2 < kMulEstimators; ++mul2) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        fMulCorQAHistogramNames[IndexCorHistograms(mul1, mul2, kMulEstimators)]
                               [ba][kNAME] =
                                   "fMulCorQAHistograms[" +
                                   kMulEstimatorNames[mul1] + "+" +
                                   kMulEstimatorNames[mul2] + "]" + kBAName[ba];
        fMulCorQAHistogramNames[IndexCorHistograms(mul1, mul2, kMulEstimators)]
                               [ba][kTITLE] =
                                   kMulEstimatorNames[mul1] + " vs " +
                                   kMulEstimatorNames[mul2] + kBAName[ba];
        fMulCorQAHistogramNames[IndexCorHistograms(mul1, mul2, kMulEstimators)]
                               [ba][kXAXIS] = kMulEstimatorNames[mul1];
        fMulCorQAHistogramNames[IndexCorHistograms(mul1, mul2, kMulEstimators)]
                               [ba][kYAXIS] = kMulEstimatorNames[mul2];
      }
    }
  }

  // set default bins
  Double_t MulCorQAHistogramBins[2 * LAST_EBINS] = {
      // kBIN kLEDGE kUEDGE kBIN+LAST_EBINS kLEDGE+LAST_EBINS
      // kUEDGE+LAST_EBINS
      50., 0., 3000., 50., 0., 3000.};
  // initialize default bins
  for (int mul = 0; mul < kMulEstimators * (kMulEstimators - 1) / 2; ++mul) {
    for (int bin = 0; bin < 2 * LAST_EBINS; ++bin) {
      fMulCorQAHistogramBins[mul][bin] = MulCorQAHistogramBins[bin];
    }
  }

  // initialize array for filterbit scan QA histograms
  // i.e. count the filterbits associated with all tracks
  fFBScanQAHistogram = nullptr;
  // set name
  TString FBScanQAHistogramName[LAST_ENAME] = // NAME, TITLE, XAXIS, YAXIS
      {"fFBScanQAHistograms", "Filterbit Scan", "Filterbit", ""};
  // initialize names
  for (int name = 0; name < LAST_ENAME; ++name) {
    fFBScanQAHistogramName[name] = FBScanQAHistogramName[name];
  }

  // set bins
  Double_t FBScanQAHistogramBin[LAST_EBINS] = {kMaxFilterbit, 0.,
                                               kMaxFilterbit};
  // initialize bins
  for (int bin = 0; bin < LAST_EBINS; ++bin) {
    fFBScanQAHistogramBin[bin] = FBScanQAHistogramBin[bin];
  }

  // initialize array of track scan filterbit scan QA histograms
  // i.e. look at the spectra of kinematic varibles according to some
  // filterbit which filterbit are used are hardcode in header
  for (int track = 0; track < LAST_ETRACK; ++track) {
    for (int fb = 0; fb < kNumberofTestFilterBit; ++fb) {
      fFBTrackScanQAHistograms[track][fb] = nullptr;
    }
  }

  // set names
  TString FBTrackScanQAHistogramNames[LAST_ETRACK][LAST_ENAME] = {
      // NAME, TITLE, XAXIS, YAXIS
      {"fFBTrackScanQAHistogram[kPT]", "Filterbitscan p_{T}", "p_{t}", ""},
      {"fFBTrackScanQAHistogram[kPHI]", "Filterbitscan #varphi", "#varphi", ""},
      {"fFBTrackScanQAHistogram[kETA]", "Filterbitscan #eta", "#eta", ""},
      {"fFBTrackScanQAHistogram[kCHARGE]", "Filterbitscan Charge", "Q", ""},
      {"fFBTrackScanQAHistogram[kTPCNCLS]",
       "Filterbitscan number of TPC clusters", "", ""},
      {"fFBTrackScanQAHistogram[kITSNCLS]",
       "Filterbitscan number of ITS clusters", "", ""},
      {"fFBTrackScanQAHistogram[kCHI2PERNDF]", "Filterbitscan #chi^{2}/NDF", "",
       ""},
      {"fFBTrackScanQAHistogram[kDCAZ]", "Filterbitscan DCA", "", ""},
      {"fFBTrackScanQAHistogram[kDCAXY]", "Filterbitscan DCA in xy", "", ""},
  };

  // initialize names
  for (int track = 0; track < LAST_ETRACK; ++track) {
    for (int fb = 0; fb < kNumberofTestFilterBit; ++fb) {
      for (int name = 0; name < LAST_ENAME; ++name) {
        if (name == kNAME || name == kTITLE) {
          fFBTrackScanQAHistogramNames[track][fb][name] =
              FBTrackScanQAHistogramNames[track][name] +
              Form(" (%d) ", kTestFilterbit[fb]);
        } else
          fFBTrackScanQAHistogramNames[track][fb][name] =
              FBTrackScanQAHistogramNames[track][name];
      }
    }
  }
  // set default bins
  Double_t FBTrackScanHistogramBins[LAST_ETRACK][LAST_EBINS] = {
      // kBIN kLEDGE kUEDGE
      {100., 0., 10.},            // kPT
      {360., 0., TMath::TwoPi()}, // kPHI
      {200., -2., 2.},            // kETA
      {7., -3.5, 3.5},            // kCHARGE
      {160., 0., 160.},           // kTPCNCLS
      {10., 0., 10.},             // kITSNCLS
      {100., 0., 10.},            // kCHI2PERNDF
      {100., -10., 10.},          // kDCAZ
      {100, -10., 10.},           // kDCAXY
  };
  // initialize default bins
  for (int var = 0; var < LAST_ETRACK; ++var) {
    for (int bin = 0; bin < LAST_EBINS; ++bin) {
      fFBTrackScanQAHistogramBins[var][bin] =
          FBTrackScanHistogramBins[var][bin];
    }
  }

  // initialize arrays for self correlation QA histograms
  // i.e. compute k_i - k_j for i !=j for all kinematic variables
  // of all tracks in each event
  // if there are no self correlations, we expect a flat spectrum
  for (int var = 0; var < kKinematic; ++var) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fSelfCorQAHistograms[var][ba] = nullptr;
    }
  }

  // set names
  TString SelfCorQAHistogramNames[kKinematic][LAST_ENAME] = {
      // NAME, TITLE, XAXIS, YAXIS
      {"fSelfCorQAHistograms[kPT]", "p_{T}^{1}-p_{T}^{2}", "#Delta p_{T}", ""},
      {"fSelfCorQAHistograms[kPHI]", "#varphi_{1}-#varphi_{2}",
       "#Delta #varphi", ""},
      {"fSelfCorQAHistograms[kETA]", "#eta_{1}-#eta_{2}", "#Delta #eta", ""},
  };

  // initialize names
  for (int var = 0; var < kKinematic; ++var) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      for (int name = 0; name < LAST_ENAME; ++name) {
        if (name == kNAME || name == kTITLE) {
          fSelfCorQAHistogramNames[var][ba][name] =
              SelfCorQAHistogramNames[var][name] + kBAName[ba];
        } else {
          fSelfCorQAHistogramNames[var][ba][name] =
              SelfCorQAHistogramNames[var][name];
        }
      }
    }
  }

  // set default bins
  Double_t SelfCorQAHistogramBins[kKinematic][LAST_EBINS] = {
      // kBIN kLEDGE kUEDGE
      {100., -0.1, 0.1}, // kPT
      {100., -0.1, 0.1}, // kPHI
      {100., -0.1, 0.1}, // kETA
  };
  // initialize default bins
  for (int var = 0; var < kKinematic; ++var) {
    for (int bin = 0; bin < LAST_EBINS; ++bin) {
      fSelfCorQAHistogramBins[var][bin] = SelfCorQAHistogramBins[var][bin];
    }
  }
}

void AliAnalysisTaskAR::InitializeArraysForTrackControlHistograms() {
  // initialize array of track control histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    for (int var = 0; var < LAST_ETRACK; ++var) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        fTrackControlHistograms[mode][var][ba] = nullptr;
      }
    }
  }

  // set names
  TString TrackControlHistogramNames[LAST_ETRACK][LAST_ENAME] = {
      // NAME, TITLE, XAXIS, YAXIS
      {"fTrackControlHistograms[kPT]", "p_{T}", "p_{T}", ""},
      {"fTrackControlHistograms[kPHI]", "#varphi", "#varphi", ""},
      {"fTrackControlHistograms[kETA]", "#eta", "#eta", ""},
      {"fTrackControlHistograms[kCHARGE]", "Charge", "Q", ""},
      {"fTrackControlHistograms[kTPCNCLS]", "Number of clusters in TPC", ""},
      {"fTrackControlHistograms[kITSNCLS]", "Number of clusters in ITS", ""},
      {"fTrackControlHistograms[kCHI2PERNDF]", "CHI2PERNDF of track", "", ""},
      {"fTrackControlHistograms[kDCAZ]", "DCA in Z", ""},
      {"fTrackControlHistograms[kDCAXY]", "DCA in XY", ""}, // kBEFORE
  };
  // initialize names
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    for (int var = 0; var < LAST_ETRACK; ++var) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        for (int name = 0; name < LAST_ENAME; ++name) {
          if (name == kNAME || name == kTITLE) {
            fTrackControlHistogramNames[mode][var][ba][name] =
                kModeName[mode] + TrackControlHistogramNames[var][name] +
                kBAName[ba];
          } else {
            fTrackControlHistogramNames[mode][var][ba][name] =
                TrackControlHistogramNames[var][name];
          }
        }
      }
    }
  }

  // set default bins
  Double_t BinsTrackControlHistogramDefaults[LAST_ETRACK][LAST_EBINS] = {
      // kBIN kLEDGE kUEDGE
      {100., 0., 10.},            // kPT
      {360., 0., TMath::TwoPi()}, // kPHI
      {200., -2., 2.},            // kETA
      {7., -3.5, 3.5},            // kCHARGE
      {160., 0., 160.},           // kTPCNCLS
      {1000., 0., 1000.},         // kITSNCLS
      {100., 0., 10.},            // kCHI2PERNDF
      {100., -10., 10.},          // kDCAZ
      {100, -10., 10.},           // kDCAXY
  };
  // initialize default bins
  for (int var = 0; var < LAST_ETRACK; ++var) {
    for (int bin = 0; bin < LAST_EBINS; ++bin) {
      fTrackControlHistogramBins[var][bin] =
          BinsTrackControlHistogramDefaults[var][bin];
    }
  }
  // initialize track cuts counter histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    fTrackCutsCounter[mode] = nullptr;
  }
  // initialize name
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    fTrackCutsCounterNames[mode] = kModeName[mode] + "fTrackCutsCounter";
  }
  // set bin names of track cuts counter histogram
  TString TrackCutsCounterBinNames[LAST_ETRACK] = {
      "kPT",      "kPHI",        "kETA",  "kCHARGE", "kTPCNCLS",
      "kITSNCLS", "kCHI2PERNDF", "kDCAZ", "kDCAXY",
  };
  // initialize bin names of track cuts counter histogram
  for (int name = 0; name < LAST_ETRACK; name++) {
    for (int mm = 0; mm < LAST_EMINMAX; ++mm) {
      fTrackCutsCounterBinNames[name][mm] =
          TrackCutsCounterBinNames[name] + kMMName[mm];
    }
  }
}

void AliAnalysisTaskAR::InitializeArraysForEventControlHistograms() {
  // initialize array of event control histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    for (int var = 0; var < LAST_EEVENT; ++var) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        fEventControlHistograms[mode][var][ba] = nullptr;
      }
    }
  }

  // set name
  TString EventControlHistogramNames[LAST_EEVENT][LAST_ENAME] = {
      // NAME, TITLE, XAXIS, YAXIS
      {"fEventControlHistograms[kMUL]", "Multiplicity (without track cuts)",
       "M", ""},
      {"fEventControlHistograms[kMULQ]", "Multiplicity (with track cuts)", "M",
       ""},
      {"fEventControlHistograms[kMULW]", "Multiplicity (computed from weights)",
       "M", ""},
      {"fEventControlHistograms[kMULREF]", "Reference Multipliticy", "M", ""},
      {"fEventControlHistograms[kNCONTRIB]", "Number of Contributors",
       "#Contributors", ""},
      {"fEventControlHistograms[kCEN]", "Centrality", "Centrality Percentile",
       ""},
      {"fEventControlHistograms[kX]", "Primary Vertex X", "X", ""},
      {"fEventControlHistograms[kY]", "Primary Vertex Y", "Y", ""},
      {"fEventControlHistograms[kZ]", "Primary Vertex Z", "Z", ""},
      {"fEventControlHistograms[kVPOS]", "Vertex Position", "|r_{V}|", ""},
  };

  // initialize names
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    for (int var = 0; var < LAST_EEVENT; ++var) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        for (int name = 0; name < LAST_ENAME; ++name) {
          if (name == kNAME || name == kTITLE) {
            fEventControlHistogramNames[mode][var][ba][name] =
                kModeName[mode] + EventControlHistogramNames[var][name] +
                kBAName[ba];
          } else
            fEventControlHistogramNames[mode][var][ba][name] =
                EventControlHistogramNames[var][name];
        }
      }
    }
  }

  // set default bins
  Double_t BinsEventControlHistogramDefaults[LAST_EEVENT][LAST_EBINS] = {
      // kBIN kLEDGE kUEDGE
      {200., 0., 10000.}, // kMUL
      {200., 0., 10000.}, // kMULQ
      {200., 0., 10000.}, // kMULW
      {200., 0., 10000.}, // kMULREF
      {100., 0., 10000.}, // kNCONTRIB
      {10., 0., 100},     // kCEN
      {40., -20., 20.},   // kX
      {40., -20., 20.},   // kY
      {40., -20., 20.},   // kZ
      {100., 0., 100.},   // kVPOS
  };
  // initialize default bins
  for (int var = 0; var < LAST_EEVENT; ++var) {
    for (int bin = 0; bin < LAST_EBINS; ++bin) {
      fEventControlHistogramBins[var][bin] =
          BinsEventControlHistogramDefaults[var][bin];
    }
  }
  // initialize event cuts counter histogram
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    fEventCutsCounter[mode] = nullptr;
  }
  // initialize names
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    fEventCutsCounterNames[mode] = kModeName[mode] + "fEventCutsCounter";
  }
  // initialize bin names of event cuts counter histogram
  TString EventCutsCounterBinNames[LAST_EEVENT] = {
      "kMUL", "kMULQ", "kMULW", "kMULREF", "kNCONTRIB",
      "kCEN", "kX",    "kY",    "kZ",      "kVPOS",
  };
  for (int name = 0; name < LAST_EEVENT; name++) {
    for (int mm = 0; mm < LAST_EMINMAX; ++mm) {
      fEventCutsCounterBinNames[name][mm] =
          EventCutsCounterBinNames[name] + kMMName[mm];
    }
  }
}

void AliAnalysisTaskAR::InitializeArraysForCuts() {
  // initialize all arrays for cuts

  // default track cuts
  Double_t TrackCutDefaults[LAST_ETRACK][LAST_EMINMAX] = {
      // MIN MAX
      {0., 5.},             // kPT
      {0., TMath::TwoPi()}, // kPHI
      {-3., 3.},            // kETA
      {0.9, 3.1},           // kCHARGE
      {0., 160.},           // kTPCNCLS
      {0., 10.},            // kITSNCLS
      {0., 10.},            // kCHI2PERNDF
      {-10., 10},           // kDCAZ
      {-10., 10},           // kDCAXY
  };
  // initialize array for track cuts
  for (int var = 0; var < LAST_ETRACK; ++var) {
    for (int mm = 0; mm < LAST_EMINMAX; ++mm) {
      fTrackCuts[var][mm] = TrackCutDefaults[var][mm];
    }
  }

  // default event cuts
  Double_t EventCutDefaults[LAST_EEVENT][LAST_EMINMAX]{
      // MIN MAX
      {0., 1e6},   // kMUL
      {0., 1e6},   // kMULQ
      {0., 1e6},   // kMULW
      {0., 1e6},   // kMULREF
      {0., 1e6},   // kNCONTRIB
      {0., 100.},  // kCEN
      {-20., 20.}, // kX
      {-20., 20.}, // kY
      {-20., 20.}, // kZ
      {0., 100.},  // kVPOS
  };
  // initialize array for event cuts
  for (int var = 0; var < LAST_EEVENT; ++var) {
    for (int mm = 0; mm < LAST_EMINMAX; ++mm) {
      fEventCuts[var][mm] = EventCutDefaults[var][mm];
    }
  }

  // default parameters for cutting on centrality correlation
  Double_t DefaultCenCorCut[2] = {// m t
                                  1.3, 10.};
  for (int i = 0; i < 2; ++i) {
    fCenCorCut[i] = DefaultCenCorCut[i];
  }
  // initialize array for centrality estimators
  for (int cen = 0; cen < LAST_ECENESTIMATORS; ++cen) {
    fCentrality[cen] = 0;
  }

  // default parameters for cutting on multiplicity correlation
  Double_t DefaultMulCorCut[2] = {// m t
                                  1.3, 700.};
  for (int i = 0; i < 2; ++i) {
    fMulCorCut[i] = DefaultMulCorCut[i];
  }
  // initialize array for multiplicity estimators
  for (int mul = 0; mul < kMulEstimators; ++mul) {
    fMultiplicity[mul] = 0;
  }
}

void AliAnalysisTaskAR::InitializeArraysForFinalResultHistograms() {
  // initialize array for final result histograms
  for (int var = 0; var < LAST_EFINALHIST; ++var) {
    fFinalResultHistograms[var] = nullptr;
  }

  // set name
  TString FinalResultHistogramNames[LAST_EFINALHIST][LAST_ENAME] = {
      // NAME, TITLE, XAXIS
      {"fFinalResultHistograms[PHIAVG]", "Average #varphi", "#varphi",
       ""}, // PHIAVG
  };

  // initialize name
  for (int var = 0; var < LAST_EFINALHIST; ++var) {
    for (int name = 0; name < LAST_ENAME; ++name) {
      fFinalResultHistogramNames[var][name] =
          FinalResultHistogramNames[var][name];
    }
  }

  // default bins
  Double_t BinsFinalResultHistogramDefaults[LAST_EFINALHIST][LAST_EBINS] = {
      // BIN LEDGE UEDGE
      {1., 0., 1.}, // AVGPHI
  };
  // initialize default bins
  for (int var = 0; var < LAST_EFINALHIST; ++var) {
    for (int bin = 0; bin < LAST_EBINS; ++bin) {
      fFinalResultHistogramBins[var][bin] =
          BinsFinalResultHistogramDefaults[var][bin];
    }
  }
}

void AliAnalysisTaskAR::InitializeArraysForWeights() {
  // initialize all necessary components for weight and acceptance histograms
  for (int k = 0; k < kKinematic; ++k) {
    fAcceptanceHistogram[k] = nullptr;
    fWeightHistogram[k] = nullptr;
    fUseWeights[k] = kFALSE;
    fResetWeights[k] = kFALSE;
    fKinematics[k] = {};
    fKinematicWeights[k] = {};
  }
}

void AliAnalysisTaskAR::InitializeArraysForQvectors() {
  // initalize all arrays for Q-vectors
  for (Int_t h = 0; h < kMaxHarmonic; h++) {
    for (Int_t p = 0; p < kMaxPower; p++) {
      fQvector[h][p] = TComplex(0., 0.);
    }
  }
}

void AliAnalysisTaskAR::InitializeArraysForFinalResultProfiles() {
  // initialize array for final result profiles
  for (int var = 0; var < LAST_EFINALPROFILE; ++var) {
    fFinalResultProfiles[var] = nullptr;
  }

  // set names
  TString FinalResultProfileNames[LAST_EFINALPROFILE][LAST_ENAME] = {
      // kNAME, kTITLE, kXAXIS
      {"fFinalResultProfiles[kHARDATA]", "Flow Harmonics (Data)", "",
       ""}, // kHARDATA
      {"fFinalResultProfiles[kHARDATARESET]",
       "Flow Harmonics (Data, weights reset)", ""}, // kHARDATARESET
      {"fFinalResultProfiles[kHARTHEO]", "Flow Harmonics (Theory)", "",
       ""}, // kHARTHEO
  };

  // initialize names
  for (int var = 0; var < LAST_EFINALPROFILE; ++var) {
    for (int name = 0; name < LAST_ENAME; ++name) {
      fFinalResultProfileNames[var][name] = FinalResultProfileNames[var][name];
    }
  }

  // default bins
  Double_t BinsFinalResultProfileDefaults[LAST_EFINALPROFILE][LAST_EBINS] = {
      // kBIN kLEDGE kUEDGE
      {1., 0., 1.}, // kHARDATA
      {1., 0., 1.}, // kHARDATARESET
      {1., 0., 1.}, // kHARTHEO
  };
  // initialize default bins
  for (int var = 0; var < LAST_EFINALPROFILE; ++var) {
    for (int bin = 0; bin < LAST_EBINS; ++bin) {
      fFinalResultProfileBins[var][bin] =
          BinsFinalResultProfileDefaults[var][bin];
    }
  }
}

void AliAnalysisTaskAR::InitializeArraysForMCAnalysis() {
  // initialize arrays for MC analysis

  // range of pdf
  Double_t MCPdfRangeDefaults[LAST_EMINMAX] = {0.0, TMath::TwoPi()};
  for (int i = 0; i < LAST_EMINMAX; ++i) {
    fMCPdfRange[i] = MCPdfRangeDefaults[i];
  }

  // range of fluctuations of number of particles produced per event
  Int_t MCNumberOfParticlesPerEventRangeDefaults[LAST_EMINMAX] = {500, 1000};
  for (int i = 0; i < LAST_EMINMAX; ++i) {
    fMCNumberOfParticlesPerEventRange[i] =
        MCNumberOfParticlesPerEventRangeDefaults[i];
  }
}

void AliAnalysisTaskAR::BookAndNestAllLists() {
  // Book and nest all lists nested in the base list fHistList

  // 1. Book and nest list for QA histograms
  // 2. Book and nest list for control histograms
  // 3. Book and nest list for final results

  if (!fHistList) {
    std::cout << __LINE__ << ": Did not get " << fHistListName << std::endl;
    Fatal("BookAndNestAllLists", "Invalid Pointer");
  }
  // 1. Book and nest lists for QA histograms
  if (fFillQAHistograms) {
    fQAHistogramsList = new TList();
    fQAHistogramsList->SetName(fQAHistogramsListName);
    fQAHistogramsList->SetOwner(kTRUE);
    fHistList->Add(fQAHistogramsList);

    // centrality correlation QA histograms
    fCenCorQAHistogramsList = new TList();
    fCenCorQAHistogramsList->SetName(fCenCorQAHistogramsListName);
    fCenCorQAHistogramsList->SetOwner(kTRUE);
    fQAHistogramsList->Add(fCenCorQAHistogramsList);

    // multiplicity correlation QA histograms
    fMulCorQAHistogramsList = new TList();
    fMulCorQAHistogramsList->SetName(fMulCorQAHistogramsListName);
    fMulCorQAHistogramsList->SetOwner(kTRUE);
    fQAHistogramsList->Add(fMulCorQAHistogramsList);

    // filterbit QA histograms
    fFBScanQAHistogramsList = new TList();
    fFBScanQAHistogramsList->SetName(fFBScanQAHistogramsListName);
    fFBScanQAHistogramsList->SetOwner(kTRUE);
    fQAHistogramsList->Add(fFBScanQAHistogramsList);

    // self correlation QA histograms
    fSelfCorQAHistogramsList = new TList();
    fSelfCorQAHistogramsList->SetName(fSelfCorQAHistogramsListName);
    fSelfCorQAHistogramsList->SetOwner(kTRUE);
    fQAHistogramsList->Add(fSelfCorQAHistogramsList);
  }

  // 2. Book and nest lists for control histograms
  fControlHistogramsList = new TList();
  fControlHistogramsList->SetName(fControlHistogramsListName);
  fControlHistogramsList->SetOwner(kTRUE);
  fHistList->Add(fControlHistogramsList);

  // track control histograms
  fTrackControlHistogramsList = new TList();
  fTrackControlHistogramsList->SetName(fTrackControlHistogramsListName);
  fTrackControlHistogramsList->SetOwner(kTRUE);
  fControlHistogramsList->Add(fTrackControlHistogramsList);

  // event control histograms
  fEventControlHistogramsList = new TList();
  fEventControlHistogramsList->SetName(fEventControlHistogramsListName);
  fEventControlHistogramsList->SetOwner(kTRUE);
  fControlHistogramsList->Add(fEventControlHistogramsList);

  // 3. Book and nest lists for final results
  fFinalResultsList = new TList();
  fFinalResultsList->SetName(fFinalResultsListName);
  fFinalResultsList->SetOwner(kTRUE);
  fHistList->Add(fFinalResultsList);
}

void AliAnalysisTaskAR::BookQAHistograms() {
  // Book all QA histograms

  // book centrality estimator correlation QA histograms
  for (int cen = 0; cen < LAST_ECENESTIMATORS * (LAST_ECENESTIMATORS - 1) / 2;
       ++cen) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fCenCorQAHistograms[cen][ba] =
          new TH2D(fCenCorQAHistogramNames[cen][ba][kNAME],
                   fCenCorQAHistogramNames[cen][ba][kTITLE],
                   fCenCorQAHistogramBins[cen][kBIN],
                   fCenCorQAHistogramBins[cen][kLEDGE],
                   fCenCorQAHistogramBins[cen][kUEDGE],
                   fCenCorQAHistogramBins[cen][kBIN + LAST_EBINS],
                   fCenCorQAHistogramBins[cen][kLEDGE + LAST_EBINS],
                   fCenCorQAHistogramBins[cen][kUEDGE + LAST_EBINS]);
      fCenCorQAHistograms[cen][ba]->SetOption("colz");
      fCenCorQAHistograms[cen][ba]->GetXaxis()->SetTitle(
          fCenCorQAHistogramNames[cen][ba][kXAXIS]);
      fCenCorQAHistograms[cen][ba]->GetYaxis()->SetTitle(
          fCenCorQAHistogramNames[cen][ba][kYAXIS]);
      fCenCorQAHistogramsList->Add(fCenCorQAHistograms[cen][ba]);
    }
  }

  // book multiplicity estimator correlation QA histograms
  for (int mul = 0; mul < kMulEstimators * (kMulEstimators - 1) / 2; ++mul) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fMulCorQAHistograms[mul][ba] =
          new TH2D(fMulCorQAHistogramNames[mul][ba][kNAME],
                   fMulCorQAHistogramNames[mul][ba][kTITLE],
                   fMulCorQAHistogramBins[mul][kBIN],
                   fMulCorQAHistogramBins[mul][kLEDGE],
                   fMulCorQAHistogramBins[mul][kUEDGE],
                   fMulCorQAHistogramBins[mul][kBIN + LAST_EBINS],
                   fMulCorQAHistogramBins[mul][kLEDGE + LAST_EBINS],
                   fMulCorQAHistogramBins[mul][kUEDGE + LAST_EBINS]);
      fMulCorQAHistograms[mul][ba]->SetOption("colz");
      fMulCorQAHistograms[mul][ba]->GetXaxis()->SetTitle(
          fMulCorQAHistogramNames[mul][ba][kXAXIS]);
      fMulCorQAHistograms[mul][ba]->GetYaxis()->SetTitle(
          fMulCorQAHistogramNames[mul][ba][kYAXIS]);
      fMulCorQAHistogramsList->Add(fMulCorQAHistograms[mul][ba]);
    }
  }

  // book filter bit scan QA histogram
  fFBScanQAHistogram =
      new TH1D(fFBScanQAHistogramName[kNAME], fFBScanQAHistogramName[kTITLE],
               fFBScanQAHistogramBin[kBIN], fFBScanQAHistogramBin[kLEDGE],
               fFBScanQAHistogramBin[kUEDGE]);

  // set labels of filter bit scan QA histograms
  // filterbits are powers of 2, i.e 1,2,4,...
  // label the bins accordingly up to the hardcoded maximum filter bit
  int fb = 1;
  for (int i = 0; i < kMaxFilterbit; ++i) {
    fFBScanQAHistogram->GetXaxis()->SetBinLabel(i + 1, Form("%d", fb));
    fb *= 2;
  }
  fFBScanQAHistogram->SetFillColor(kFillColor[kAFTER]);
  fFBScanQAHistogramsList->Add(fFBScanQAHistogram);

  // book track scan filterbit QA histograms
  for (int track = 0; track < LAST_ETRACK; ++track) {
    for (int fb = 0; fb < kNumberofTestFilterBit; ++fb) {
      fFBTrackScanQAHistograms[track][fb] =
          new TH1D(fFBTrackScanQAHistogramNames[track][fb][kNAME],
                   fFBTrackScanQAHistogramNames[track][fb][kTITLE],
                   fFBTrackScanQAHistogramBins[track][kBIN],
                   fFBTrackScanQAHistogramBins[track][kLEDGE],
                   fFBTrackScanQAHistogramBins[track][kUEDGE]);
      fFBTrackScanQAHistograms[track][fb]->SetFillColor(kFillColor[kAFTER]);
      fFBTrackScanQAHistograms[track][fb]->GetXaxis()->SetTitle(
          fFBTrackScanQAHistogramNames[track][fb][kXAXIS]);
      fFBTrackScanQAHistograms[track][fb]->GetYaxis()->SetTitle(
          fFBTrackScanQAHistogramNames[track][fb][kYAXIS]);
      fFBScanQAHistogramsList->Add(fFBTrackScanQAHistograms[track][fb]);
    }
  }

  // book self correlation QA histograms
  for (int var = 0; var < kKinematic; ++var) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fSelfCorQAHistograms[var][ba] =
          new TH1D(fSelfCorQAHistogramNames[var][ba][kNAME],
                   fSelfCorQAHistogramNames[var][ba][kTITLE],
                   fSelfCorQAHistogramBins[var][kBIN],
                   fSelfCorQAHistogramBins[var][kLEDGE],
                   fSelfCorQAHistogramBins[var][kUEDGE]);
      fSelfCorQAHistograms[var][ba]->SetFillColor(kFillColor[ba]);
      fSelfCorQAHistograms[var][ba]->SetMinimum(0.0);
      fSelfCorQAHistograms[var][ba]->GetXaxis()->SetTitle(
          fSelfCorQAHistogramNames[var][ba][kXAXIS]);
      fSelfCorQAHistograms[var][ba]->GetYaxis()->SetTitle(
          fSelfCorQAHistogramNames[var][ba][kYAXIS]);
      fSelfCorQAHistogramsList->Add(fSelfCorQAHistograms[var][ba]);
    }
  }
}

void AliAnalysisTaskAR::BookControlHistograms() {
  // Book all control histograms

  // book histogram for counting trackcuts
  // add 2 bins manually for filterbit and primary only cut
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    fTrackCutsCounter[mode] =
        new TH1D(fTrackCutsCounterNames[mode], fTrackCutsCounterNames[mode],
                 2 * LAST_ETRACK + 2, 0, 2 * LAST_ETRACK + 2);
    fTrackCutsCounter[mode]->SetFillColor(kFillColor[kAFTER]);
    for (int bin = 0; bin < LAST_ETRACK; ++bin) {
      for (int mm = 0; mm < LAST_EMINMAX; ++mm) {
        fTrackCutsCounter[mode]->GetXaxis()->SetBinLabel(
            2 * bin + mm + 1, fTrackCutsCounterBinNames[bin][mm]);
      }
    }
    fTrackCutsCounter[mode]->GetXaxis()->SetBinLabel(2 * LAST_ETRACK + 1,
                                                     "Filterbit");
    fTrackCutsCounter[mode]->GetXaxis()->SetBinLabel(2 * LAST_ETRACK + 2,
                                                     "PrimaryOnly");
    fTrackControlHistogramsList->Add(fTrackCutsCounter[mode]);
  }
  // book track control histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    for (int var = 0; var < LAST_ETRACK; ++var) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        fTrackControlHistograms[mode][var][ba] =
            new TH1D(fTrackControlHistogramNames[mode][var][ba][kNAME],
                     fTrackControlHistogramNames[mode][var][ba][kTITLE],
                     fTrackControlHistogramBins[var][kBIN],
                     fTrackControlHistogramBins[var][kLEDGE],
                     fTrackControlHistogramBins[var][kUEDGE]);
        fTrackControlHistograms[mode][var][ba]->SetFillColor(kFillColor[ba]);
        fTrackControlHistograms[mode][var][ba]->SetMinimum(0.0);
        fTrackControlHistograms[mode][var][ba]->GetXaxis()->SetTitle(
            fTrackControlHistogramNames[mode][var][ba][kXAXIS]);
        fTrackControlHistograms[mode][var][ba]->GetYaxis()->SetTitle(
            fTrackControlHistogramNames[mode][var][ba][kYAXIS]);
        fTrackControlHistogramsList->Add(
            fTrackControlHistograms[mode][var][ba]);
      }
    }
  }

  // book histogram for counting event cuts
  // add 4 bins by hand for centrality/multiplicity correlation cuts
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    fEventCutsCounter[mode] =
        new TH1D(fEventCutsCounterNames[mode], fEventCutsCounterNames[mode],
                 2 * (LAST_EEVENT + 2), 0, 2 * (LAST_EEVENT + 2));
    fEventCutsCounter[mode]->SetFillColor(kFillColor[kAFTER]);
    for (int bin = 0; bin < LAST_EEVENT; ++bin) {
      for (int mm = 0; mm < LAST_EMINMAX; ++mm) {
        fEventCutsCounter[mode]->GetXaxis()->SetBinLabel(
            2 * bin + mm + 1, fEventCutsCounterBinNames[bin][mm]);
      }
    }
    fEventCutsCounter[mode]->GetXaxis()->SetBinLabel(2 * LAST_EEVENT + 1,
                                                     "CenCorCut[kMIN]");
    fEventCutsCounter[mode]->GetXaxis()->SetBinLabel(2 * LAST_EEVENT + 2,
                                                     "CenCorCut[kMAX]");
    fEventCutsCounter[mode]->GetXaxis()->SetBinLabel(2 * (LAST_EEVENT + 1) + 1,
                                                     "MulCorCut[kMIN]");
    fEventCutsCounter[mode]->GetXaxis()->SetBinLabel(2 * (LAST_EEVENT + 1) + 2,
                                                     "MulCorCut[kMAX]");
    fEventControlHistogramsList->Add(fEventCutsCounter[mode]);
  }
  // book event control histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    for (int var = 0; var < LAST_EEVENT; ++var) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        fEventControlHistograms[mode][var][ba] =
            new TH1D(fEventControlHistogramNames[mode][var][ba][kNAME],
                     fEventControlHistogramNames[mode][var][ba][kTITLE],
                     fEventControlHistogramBins[var][kBIN],
                     fEventControlHistogramBins[var][kLEDGE],
                     fEventControlHistogramBins[var][kUEDGE]);
        fEventControlHistograms[mode][var][ba]->SetFillColor(kFillColor[ba]);
        fEventControlHistograms[mode][var][ba]->SetMinimum(0.0);
        fEventControlHistograms[mode][var][ba]->GetXaxis()->SetTitle(
            fEventControlHistogramNames[mode][var][ba][kXAXIS]);
        fEventControlHistograms[mode][var][ba]->GetYaxis()->SetTitle(
            fEventControlHistogramNames[mode][var][ba][kYAXIS]);
        fEventControlHistogramsList->Add(
            fEventControlHistograms[mode][var][ba]);
      }
    }
  }
}

void AliAnalysisTaskAR::BookFinalResultHistograms() {
  // Book final result histograms

  Color_t colorFinalResult = kBlue - 10;

  // book final result histograms
  for (int var = 0; var < LAST_EFINALHIST; ++var) {
    fFinalResultHistograms[var] = new TH1D(
        fFinalResultHistogramNames[var][0], fFinalResultHistogramNames[var][1],
        fFinalResultHistogramBins[var][kBIN],
        fFinalResultHistogramBins[var][kLEDGE],
        fFinalResultHistogramBins[var][kUEDGE]);
    fFinalResultHistograms[var]->SetFillColor(colorFinalResult);
    fFinalResultHistograms[var]->GetXaxis()->SetTitle(
        fFinalResultHistogramNames[var][2]);
    fFinalResultsList->Add(fFinalResultHistograms[var]);
  }
}

void AliAnalysisTaskAR::BookFinalResultProfiles() {
  // Book final result profiles

  // book final result profiles
  for (int var = 0; var < LAST_EFINALPROFILE; ++var) {
    fFinalResultProfiles[var] = new TProfile(
        fFinalResultProfileNames[var][0], fFinalResultProfileNames[var][1],
        fFinalResultProfileBins[var][kBIN],
        fFinalResultProfileBins[var][kLEDGE],
        fFinalResultProfileBins[var][kUEDGE], nullptr);
    fFinalResultProfiles[var]->GetXaxis()->SetTitle(
        fFinalResultProfileNames[var][2]);
    fFinalResultProfiles[var]->Sumw2();
    fFinalResultsList->Add(fFinalResultProfiles[var]);

    // set bin labels, i.e. the name of the correlator like <<2>>_{-2,2}
    TString BinLabel = "";
    if (!fCorrelators.empty()) {
      for (std::size_t bin = 0; bin < fCorrelators.size(); bin++) {
        BinLabel = Form("#LT#LT%d#GT#GT", int(fCorrelators.at(bin).size()));
        BinLabel += "_{";
        for (auto Harmonic : fCorrelators.at(bin)) {
          BinLabel += Form("%d,", Harmonic);
        }
        BinLabel += "}";
        fFinalResultProfiles[var]->GetXaxis()->SetBinLabel(bin + 1, BinLabel);
      }
    }
  }
}

void AliAnalysisTaskAR::BookMCOnTheFlyObjects() {
  // book objects need for MC analysis

  // check if fMCFlowHarmonics is empty
  if (fMCFlowHarmonics.empty()) {
    std::cout << __LINE__ << ": no flow harmonics defined" << std::endl;
    Fatal("BookMCObjects", "Invalid Pointer");
  }

  // base setup for the pdf for MC analysis with flow harmonics
  // generate formula, i.e. fourier series
  // set flow harmonics as parameters as given by fMCFlowHarmonics
  // leave symmetry planes and set them later on a event by event basis

  // generate formula, f(phi) = [1 + 2*Sum_{i=1}{N} v_i
  // cos(i*(phi-Psi_i))]/(2pi)
  TString Formula = "1+";
  for (std::size_t i = 0; i < fMCFlowHarmonics.size(); ++i) {
    // we set 2n parameters
    // [2i] -> v_i
    // [2i+1] -> Psi_i
    Formula += Form("2*[%d]*TMath::Cos(%d*(x-[%d]))", int(2 * i), int(i + 1),
                    int(2 * i + 1));
    if (i < fMCFlowHarmonics.size() - 1) {
      Formula += "+";
    }
  }
  Formula = "(" + Formula + ")/TMath::TwoPi()";
  // create TF1 object with formaul
  fMCPdf = new TF1(fMCPdfName, Formula, 0., TMath::TwoPi());

  // set flow harmonics
  for (std::size_t i = 0; i < fMCFlowHarmonics.size(); ++i) {
    fMCPdf->SetParameter(2 * i, fMCFlowHarmonics.at(i));
  }
}

void AliAnalysisTaskAR::UserExec(Option_t *) {
  // if we do MC analysis locally, call MCOnTheFlyExec and bail out
  if (fMCOnTheFly) {
    MCOnTheFlyExec();
    return;
  }

  // general strategy
  // Get pointer(s) to event (reconstructed,simulated)
  // Fill event objects
  // Check event cut
  // Start Analysis either
  // 		over AOD only or
  // 		over AOD and MC (TBI over MC only)
  // PostData

  // Get pointers to event
  // get pointer to AOD event
  AliAODEvent *aAOD = dynamic_cast<AliAODEvent *>(InputEvent());
  // get pointer to MC event
  AliMCEvent *aMC = MCEvent();

  // bail out of it AOD event is invald
  // this means we cannot process MC only
  if (!aAOD) {
    return;
  }

  // compute event objects
  // this requries an inital loop over all tracks in the event
  FillEventObjects(aAOD, aMC);

  // fill event histograms before cut
  if (fFillQAHistograms) {
    FillEventQAHistograms(kBEFORE, aAOD);
    FillEventQAHistograms(kBEFORE, aMC);
  }
  FillEventControlHistograms(kBEFORE, aAOD);
  FillEventControlHistograms(kBEFORE, aMC);

  // check if event survives cut
  if (!SurviveEventCut(aAOD)) {
    return;
  }

  // fill event histograms after cut
  FillEventControlHistograms(kAFTER, aAOD);
  FillEventControlHistograms(kAFTER, aMC);
  if (fFillQAHistograms) {
    FillEventQAHistograms(kAFTER, aAOD);
    FillEventQAHistograms(kAFTER, aMC);
  }

  // start analysis over AODEvent only
  if (aAOD && !aMC) {

    // clear vectors holding kinematics and weights
    ClearVectors();

    // get number of all tracks in current event
    Int_t nTracks = aAOD->GetNumberOfTracks();
    AliAODTrack *aTrack = nullptr;

    for (int iTrack = 0; iTrack < nTracks; ++iTrack) {

      // getting a pointer to a track
      aTrack = dynamic_cast<AliAODTrack *>(aAOD->GetTrack(iTrack));

      // protect against invalid pointers
      if (!aTrack) {
        continue;
      }

      // fill QA track scan histograms
      if (fFillQAHistograms) {
        FillFBScanQAHistograms(aTrack);
      }

      // fill track control histogram before track cut
      FillTrackControlHistograms(kBEFORE, aTrack);
      if (!SurviveTrackCut(aTrack, kTRUE)) {
        continue;
      }
      // fill track control histogram after track cut
      FillTrackControlHistograms(kAFTER, aTrack);

      // check if we do a monte carlo closure
      if (!fMCClosure) {

        // fill kinematic variables into event objects
        FillTrackObjects(aTrack);

      } else {
        // Monte Carlo Closure
        Int_t AcceptanceBin[kKinematic];
        Double_t Acceptance[kKinematic];
        Bool_t AcceptParticle = kTRUE;
        // get bin of the acceptance histogram according to kinematic
        // variables
        if (fAcceptanceHistogram[kPT]) {
          AcceptanceBin[kPT] = fAcceptanceHistogram[kPT]->FindBin(aTrack->Pt());
        }
        if (fAcceptanceHistogram[kPHI]) {
          AcceptanceBin[kPHI] =
              fAcceptanceHistogram[kPHI]->FindBin(aTrack->Phi());
        }
        if (fAcceptanceHistogram[kETA]) {
          AcceptanceBin[kETA] =
              fAcceptanceHistogram[kETA]->FindBin(aTrack->Eta());
        }
        // get acceptance
        for (int k = 0; k < kKinematic; ++k) {
          if (fAcceptanceHistogram[k]) {
            Acceptance[k] =
                fAcceptanceHistogram[k]->GetBinContent(AcceptanceBin[k]);
          } else {
            Acceptance[k] = 1.;
          }
        }
        // test if particle gets accepted
        for (int k = 0; k < kKinematic; ++k) {
          if (gRandom->Uniform() >= Acceptance[k]) {
            AcceptParticle = kFALSE;
          }
        }
        // if particle is accepted, push back its kinematic variables and
        // weights into event objects
        if (AcceptParticle) {
          FillTrackObjects(aTrack);
        }
      }
    }

    // aggregate weights
    AggregateWeights();
    // calculate qvectors
    CalculateQvectors();

    // fill final result profile
    FillFinalResultProfile(kHARDATA);

    // if option is given, repeat with weights resetted
    if (fUseWeightsAggregated && fResetWeightsAggregated) {
      ResetWeights();
      AggregateWeights();
      CalculateQvectors();
      FillFinalResultProfile(kHARDATARESET);
    }
  }

  // start analysis over MC and AOD to compute efficiencies
  if (aMC && aAOD) {

    // reset vectors holding kinematics and weights
    ClearVectors();

    // get number of all particles in current event
    Int_t nParticles = aMC->GetNumberOfTracks();
    AliAODMCParticle *MCParticle = nullptr;
    AliAODTrack *aTrack = nullptr;
    // loop over all particles in the event
    for (Int_t iParticle = 0; iParticle < nParticles; iParticle++) {

      // getting a pointer to a MCparticle
      MCParticle = dynamic_cast<AliAODMCParticle *>(aMC->GetTrack(iParticle));

      // protect against invalid pointers
      if (!MCParticle) {
        continue;
      }

      // fill control histogram before cutting
      FillTrackControlHistograms(kBEFORE, MCParticle);

      // get corresponding AODTrack, if it exists
      aTrack = dynamic_cast<AliAODTrack *>(
          aAOD->GetTrack(fLookUpTable->GetValue(iParticle)));
      if (aTrack) {
        // if it exists, fill control histogram before cutting
        FillTrackControlHistograms(kBEFORE, aTrack);
      } else {
        // bail out, if there is no corresponding track
        continue;
      }

      // cut MC particle
      if (!SurviveTrackCut(MCParticle, kTRUE)) {
        continue;
      }

      // fill control histogram after cutting
      FillTrackControlHistograms(kAFTER, MCParticle);

      // cut AOD Track
      if (!SurviveTrackCut(aTrack, kTRUE)) {
        continue;
      }

      // fill control histogram after cutting
      FillTrackControlHistograms(kAFTER, aTrack);
    }
  }

  // PostData
  PostData(1, fHistList);
}

void AliAnalysisTaskAR::MCOnTheFlyExec() {
  // call this method for local monte carlo analysis

  // reset angles and weights
  ClearVectors();

  // set symmetry plane
  MCPdfSymmetryPlanesSetup();
  // loop over all particles in an event
  Double_t Phi = 0.0;
  Int_t AcceptanceBin = 0;
  for (int i = 0; i < GetMCNumberOfParticlesPerEvent(); ++i) {
    Phi = fMCPdf->GetRandom();

    if (fUseWeights[kPHI]) {
      AcceptanceBin = fAcceptanceHistogram[kPHI]->FindBin(Phi);
      if (gRandom->Uniform() <=
          fAcceptanceHistogram[kPHI]->GetBinContent(AcceptanceBin)) {
        fKinematics[kPHI].push_back(Phi);
        fKinematicWeights[kPHI].push_back(
            fWeightHistogram[kPHI]->GetBinContent(AcceptanceBin));
      }
    } else {
      fKinematics[kPHI].push_back(Phi);
    }
  }

  // compute Q-vectors
  AggregateWeights();
  CalculateQvectors();
  // fill data into final result profile
  FillFinalResultProfile(kHARDATA);
  // if option is given, reset weight and recompute Q-vectors
  if (fUseWeightsAggregated && fResetWeightsAggregated) {
    ResetWeights();
    AggregateWeights();
    CalculateQvectors();
    FillFinalResultProfile(kHARDATARESET);
  }
}

void AliAnalysisTaskAR::ClearVectors() {
  // clear vectors holding kinematics and weights of an event
  fWeightsAggregated.clear();
  for (int k = 0; k < kKinematic; ++k) {
    fKinematics[k].clear();
    fKinematicWeights[k].clear();
  }
}

void AliAnalysisTaskAR::FillTrackObjects(AliAODTrack *track) {
  // fill kinematic variables and weights into event objects

  fKinematics[kPT].push_back(track->Pt());
  if (fWeightHistogram[kPT]) {
    fKinematicWeights[kPT].push_back(fWeightHistogram[kPT]->GetBinContent(
        fWeightHistogram[kPT]->FindBin(track->Pt())));
  }
  fKinematics[kPHI].push_back(track->Phi());
  if (fWeightHistogram[kPHI]) {
    fKinematicWeights[kPHI].push_back(fWeightHistogram[kPHI]->GetBinContent(
        fWeightHistogram[kPHI]->FindBin(track->Phi())));
  }
  fKinematics[kETA].push_back(track->Eta());
  if (fWeightHistogram[kETA]) {
    fKinematicWeights[kETA].push_back(fWeightHistogram[kETA]->GetBinContent(
        fWeightHistogram[kETA]->FindBin(track->Eta())));
  }
}

void AliAnalysisTaskAR::AggregateWeights() {
  // aggregate all kinematic weights into one vector
  Double_t w[kKinematic];
  Double_t tmp;
  fWeightsAggregated.clear();

  for (std::size_t i = 0; i < fKinematics[kPHI].size(); ++i) {
    tmp = 1.;
    for (int k = 0; k < kKinematic; ++k) {
      w[k] = 1.;
      if (fUseWeights[k] && !fKinematicWeights[k].empty()) {
        w[k] *= fKinematicWeights[k].at(i);
      }
    }
    for (int k = 0; k < kKinematic; ++k) {
      tmp *= w[k];
    }
    fWeightsAggregated.push_back(tmp);
  }

  // if we use at least one kinematic weight or at least one kinematic weight
  // should be reset, aggregate that into one boolean for easier checking
  for (int k = 0; k < kKinematic; ++k) {
    if (fUseWeights[k]) {
      fUseWeightsAggregated = kTRUE;
    }
    if (fResetWeights[k]) {
      fResetWeightsAggregated = kTRUE;
    }
  }
}

void AliAnalysisTaskAR::ResetWeights() {
  // reset weights kinematic weights
  for (int k = 0; k < kKinematic; ++k) {
    if (fResetWeights[k] && !fKinematicWeights[k].empty()) {
      for (std::size_t i = 0; i < fKinematicWeights[k].size(); ++i) {
        fKinematicWeights[k].at(i) = 1.;
      }
    }
  }
}

Int_t AliAnalysisTaskAR::IndexCorHistograms(Int_t i, Int_t j, Int_t N) {
  // helper function for computing index of centrality estimator correlation
  // histograms project 2D indeces of the entries above the diagonal to a 1D
  // index
  Int_t Index = 0;
  for (int k = 0; k < i; ++k) {
    Index += N - (k + 1);
  }
  Index += j - i - 1;
  return Index;
}

void AliAnalysisTaskAR::FillEventControlHistograms(kBeforeAfter BA,
                                                   AliVEvent *ave) {

  // fill event control histograms
  AliAODEvent *AODEvent = dynamic_cast<AliAODEvent *>(ave);
  AliMCEvent *MCEvent = dynamic_cast<AliMCEvent *>(ave);

  // AOD event
  if (AODEvent) {
    // get centrality percentile
    AliMultSelection *AMS = dynamic_cast<AliMultSelection *>(
        AODEvent->FindListObject("MultSelection"));

    // get primary vertex object
    AliAODVertex *PrimaryVertex = AODEvent->GetPrimaryVertex();
    if (!AMS || !PrimaryVertex) {
      std::cout << __LINE__ << ": did not get pointers" << std::endl;
      Fatal("FillEventControlHistograms", "Invalid pointers");
    }

    // fill control histograms
    fEventControlHistograms[kRECO][kMUL][BA]->Fill(fMultiplicity[kMUL]);
    fEventControlHistograms[kRECO][kMULQ][BA]->Fill(fMultiplicity[kMULQ]);
    fEventControlHistograms[kRECO][kMULW][BA]->Fill(fMultiplicity[kMULW]);
    fEventControlHistograms[kRECO][kMULREF][BA]->Fill(fMultiplicity[kMULREF]);
    fEventControlHistograms[kRECO][kNCONTRIB][BA]->Fill(
        fMultiplicity[kNCONTRIB]);
    fEventControlHistograms[kRECO][kCEN][BA]->Fill(
        fCentrality[fCentralityEstimator]);
    fEventControlHistograms[kRECO][kX][BA]->Fill(PrimaryVertex->GetX());
    fEventControlHistograms[kRECO][kY][BA]->Fill(PrimaryVertex->GetY());
    fEventControlHistograms[kRECO][kZ][BA]->Fill(PrimaryVertex->GetZ());
    fEventControlHistograms[kRECO][kVPOS][BA]->Fill(
        std::sqrt(PrimaryVertex->GetX() * PrimaryVertex->GetX() +
                  PrimaryVertex->GetY() * PrimaryVertex->GetY() +
                  PrimaryVertex->GetZ() * PrimaryVertex->GetZ()));
  }

  // MC event
  if (MCEvent) {
    fEventControlHistograms[kSIM][kMUL][BA]->Fill(MCEvent->GetNumberOfTracks());
    // AliVVertex *avtx = (AliVVertex *)MCEvent->GetPrimaryVertex();
    // if (avtx) {
    //   fEventControlHistograms[kSIM][kX][BA]->Fill(avtx->GetX());
    //   fEventControlHistograms[kSIM][kY][BA]->Fill(avtx->GetY());
    //   fEventControlHistograms[kSIM][kZ][BA]->Fill(avtx->GetZ());
    // }
  }
}

void AliAnalysisTaskAR::FillTrackControlHistograms(kBeforeAfter BA,
                                                   AliVParticle *avp) {
  // fill track control histograms

  // aod track
  AliAODTrack *track = dynamic_cast<AliAODTrack *>(avp);
  if (track) {
    fTrackControlHistograms[kRECO][kPT][BA]->Fill(track->Pt());
    fTrackControlHistograms[kRECO][kPHI][BA]->Fill(track->Phi());
    fTrackControlHistograms[kRECO][kETA][BA]->Fill(track->Eta());
    fTrackControlHistograms[kRECO][kCHARGE][BA]->Fill(track->Charge());
    fTrackControlHistograms[kRECO][kTPCNCLS][BA]->Fill(track->GetTPCNcls());
    fTrackControlHistograms[kRECO][kITSNCLS][BA]->Fill(track->GetITSNcls());
    fTrackControlHistograms[kRECO][kCHI2PERNDF][BA]->Fill(track->Chi2perNDF());
    fTrackControlHistograms[kRECO][kDCAZ][BA]->Fill(track->ZAtDCA());
    fTrackControlHistograms[kRECO][kDCAXY][BA]->Fill(track->DCA());
  }

  // MC particle
  AliAODMCParticle *MCParticle = dynamic_cast<AliAODMCParticle *>(avp);
  if (MCParticle) {
    // fill track control histograms
    fTrackControlHistograms[kSIM][kPT][BA]->Fill(MCParticle->Pt());
    fTrackControlHistograms[kSIM][kPHI][BA]->Fill(MCParticle->Phi());
    fTrackControlHistograms[kSIM][kETA][BA]->Fill(MCParticle->Eta());
    fTrackControlHistograms[kSIM][kCHARGE][BA]->Fill(MCParticle->Charge() / 3.);
  }
}

void AliAnalysisTaskAR::FillEventQAHistograms(kBeforeAfter BA, AliVEvent *ave) {
  // fill event QA control histograms

  AliAODEvent *AODEvent = dynamic_cast<AliAODEvent *>(ave);
  AliMCEvent *MCEvent = dynamic_cast<AliMCEvent *>(ave);

  if (AODEvent) {
    // fill centrality estimator correlation histograms
    for (int i = 0; i < LAST_ECENESTIMATORS; ++i) {
      for (int j = i + 1; j < LAST_ECENESTIMATORS; ++j) {
        fCenCorQAHistograms[IndexCorHistograms(i, j, LAST_ECENESTIMATORS)][BA]
            ->Fill(fCentrality[i], fCentrality[j]);
      }
    }

    // file multiplicity correlation histograms
    for (int i = 0; i < kMulEstimators; ++i) {
      for (int j = i + 1; j < kMulEstimators; ++j) {
        fMulCorQAHistograms[IndexCorHistograms(i, j, kMulEstimators)][BA]->Fill(
            fMultiplicity[i], fMultiplicity[j]);
      }
    }

    // search for self correlations with nested loop
    Int_t nTracks = AODEvent->GetNumberOfTracks();
    AliAODTrack *aTrack1 = nullptr;
    AliAODTrack *aTrack2 = nullptr;
    // starting a loop over the first track
    for (Int_t iTrack1 = 0; iTrack1 < nTracks; iTrack1++) {
      aTrack1 = dynamic_cast<AliAODTrack *>(AODEvent->GetTrack(iTrack1));
      if (!aTrack1 || !SurviveTrackCut(aTrack1, kFALSE)) {
        continue;
      }
      // starting a loop over the second track
      for (Int_t iTrack2 = iTrack1 + 1; iTrack2 < nTracks; iTrack2++) {
        aTrack2 = dynamic_cast<AliAODTrack *>(AODEvent->GetTrack(iTrack2));
        if (!aTrack2 || !SurviveTrackCut(aTrack2, kFALSE)) {
          continue;
        }
        // compute differences
        fSelfCorQAHistograms[kPHI][BA]->Fill(aTrack1->Phi() - aTrack2->Phi());
        fSelfCorQAHistograms[kPT][BA]->Fill(aTrack1->Pt() - aTrack2->Pt());
        fSelfCorQAHistograms[kETA][BA]->Fill(aTrack1->Eta() - aTrack2->Eta());
      }
    }
  }

  if (MCEvent) {
    // TBI
  }
}

void AliAnalysisTaskAR::FillFBScanQAHistograms(AliAODTrack *track) {
  // fill filter bit scan QA histograms

  // check for filterbits of the track
  // filterbits are powers of 2, i.e. 1,2,4,8,...
  int fb = 1;
  for (int i = 0; i < kMaxFilterbit; ++i) {
    if (track->TestFilterBit(fb)) {
      fFBScanQAHistogram->Fill(i);
    }
    fb *= 2;
  }

  // scan kinematic variables for different filterbits
  for (int fb = 0; fb < kNumberofTestFilterBit; ++fb) {
    if (track->TestFilterBit(kTestFilterbit[fb])) {
      fFBTrackScanQAHistograms[kPT][fb]->Fill(track->Pt());
      fFBTrackScanQAHistograms[kPHI][fb]->Fill(track->Phi());
      fFBTrackScanQAHistograms[kETA][fb]->Fill(track->Eta());
      fFBTrackScanQAHistograms[kCHARGE][fb]->Fill(track->Charge());
      fFBTrackScanQAHistograms[kTPCNCLS][fb]->Fill(track->GetTPCNcls());
      fFBTrackScanQAHistograms[kITSNCLS][fb]->Fill(track->GetITSNcls());
      fFBTrackScanQAHistograms[kCHI2PERNDF][fb]->Fill(track->Chi2perNDF());
      fFBTrackScanQAHistograms[kDCAZ][fb]->Fill(track->ZAtDCA());
      fFBTrackScanQAHistograms[kDCAXY][fb]->Fill(track->DCA());
    }
  }
}

Bool_t AliAnalysisTaskAR::SurviveEventCut(AliVEvent *ave) {
  // Check if the current event survives event cuts
  // return flag at the end, if one cut is not passed, set it to kFALSE
  Bool_t Flag = kTRUE;

  // check AOD event
  AliAODEvent *aAOD = dynamic_cast<AliAODEvent *>(ave);
  if (aAOD) {

    // cut on multiplicity
    // number of total tracks of the event
    if (fMultiplicity[kMUL] < fEventCuts[kMUL][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kMUL + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (fMultiplicity[kMUL] > fEventCuts[kMUL][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kMUL + kMAX + 0.5);
      Flag = kFALSE;
    }
    // number of tracks that survive track cuts
    if (fMultiplicity[kMULQ] < fEventCuts[kMULQ][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kMULQ + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (fMultiplicity[kMULQ] > fEventCuts[kMULQ][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kMULQ + kMAX + 0.5);
      Flag = kFALSE;
    }
    // sum of weighted surviving tracks
    if (fMultiplicity[kMULW] < fEventCuts[kMULW][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kMULW + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (fMultiplicity[kMULW] > fEventCuts[kMULW][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kMULW + kMAX + 0.5);
      Flag = kFALSE;
    }
    // numbers of contriubters to the vertex
    if (fMultiplicity[kNCONTRIB] < fEventCuts[kNCONTRIB][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kNCONTRIB + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (fMultiplicity[kNCONTRIB] > fEventCuts[kNCONTRIB][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kNCONTRIB + kMAX + 0.5);
      Flag = kFALSE;
    }

    // cut event if it is not within the reference centrality percentile
    if (fMultiplicity[kMULREF] < fEventCuts[kMULREF][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kMULREF + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (fMultiplicity[kMULREF] > fEventCuts[kMULREF][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kMULREF + kMAX + 0.5);
      Flag = kFALSE;
    }

    // Get primary vertex
    AliAODVertex *PrimaryVertex = aAOD->GetPrimaryVertex();
    if (!PrimaryVertex) {
      return kFALSE;
    }

    // cut event if it is not within the centrality percentile
    if (fCentrality[fCentralityEstimator] < fEventCuts[kCEN][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kCEN + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (fCentrality[fCentralityEstimator] > fEventCuts[kCEN][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kCEN + kMAX + 0.5);
      Flag = kFALSE;
    }

    // cut event if primary vertex is too out of center
    if (PrimaryVertex->GetX() < fEventCuts[kX][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kX + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (PrimaryVertex->GetX() > fEventCuts[kX][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kX + kMAX + 0.5);
      Flag = kFALSE;
    }
    if (PrimaryVertex->GetY() < fEventCuts[kY][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kY + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (PrimaryVertex->GetY() > fEventCuts[kY][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kY + kMAX + 0.5);
      Flag = kFALSE;
    }
    if (PrimaryVertex->GetZ() < fEventCuts[kZ][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kZ + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (PrimaryVertex->GetZ() > fEventCuts[kZ][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kZ + kMAX + 0.5);
      Flag = kFALSE;
    }

    // additionally cut on absolute value of the vertex postion
    // there are suspicous events with |r_v|=0 that we do not trust
    Double_t VPos = std::sqrt(PrimaryVertex->GetX() * PrimaryVertex->GetX() +
                              PrimaryVertex->GetY() * PrimaryVertex->GetY() +
                              PrimaryVertex->GetZ() * PrimaryVertex->GetZ());
    if (VPos < fEventCuts[kVPOS][kMIN]) {
      fEventCutsCounter[kRECO]->Fill(2 * kVPOS + kMIN + 0.5);
      Flag = kFALSE;
    }
    if (VPos > fEventCuts[kVPOS][kMAX]) {
      fEventCutsCounter[kRECO]->Fill(2 * kVPOS + kMAX + 0.5);
      Flag = kFALSE;
    }

    // cut on centrality estimator correlation
    // ugly! cut on fundamental observerables instead but there are some
    // really weird events we need to get rid off
    // cut away all events that are above the line
    // y=mx+t
    // and below
    // y=(x-t)/m
    // this gives a nice and symmetric cone around the diagonal y=x
    // set m>1 such that the cone gets wider for larger centralities
    Double_t m_cen = fCenCorCut[0];
    Double_t t_cen = fCenCorCut[1];
    for (int i = 0; i < LAST_ECENESTIMATORS; ++i) {
      for (int j = i + 1; j < LAST_ECENESTIMATORS; ++j) {
        if (fCentrality[j] > m_cen * fCentrality[i] + t_cen) {
          fEventCutsCounter[kRECO]->Fill(2 * LAST_EEVENT + kMAX + 0.5);
          Flag = kFALSE;
        }
        if (fCentrality[j] < (fCentrality[i] - t_cen) / m_cen) {
          fEventCutsCounter[kRECO]->Fill(2 * LAST_EEVENT + kMIN + 0.5);
          Flag = kFALSE;
        }
      }
    }
    // Double_t cenDiff = 0;
    // for (int i = 0; i < LAST_ECENESTIMATORS; ++i) {
    //   for (int j = i + 1; j < LAST_ECENESTIMATORS; ++j) {
    //     // protect against division by zero
    //     if (fCentrality[i] > 0. && fCentrality[j] > 0.) {
    //       if (fCenCorCutMode == kDIFFABS) {
    //         cenDiff = std::abs(fCentrality[i] - fCentrality[j]);
    //       } else if (fCenCorCutMode == kDIFFREL) {
    //         cenDiff = std::abs(fCentrality[i] - fCentrality[j]) /
    //                   (fCentrality[i] + fCentrality[j]);
    //       } else {
    //         Fatal("SurviveEventCut", "No centrality difference");
    //       }
    //       if (cenDiff > fCenCorCut) {
    //         fEventCutsCounter[kRECO]->Fill(2 * LAST_EEVENT + 0.5);
    //         Flag = kFALSE;
    //       }
    //     }
    //   }
    // }

    // cut on multiplicity correlation
    // ugly! cut on fundamental observerables instead but there are some
    // really weird events we need to get rid off
    // logic is same as above
    Double_t m_mul = fMulCorCut[0];
    Double_t t_mul = fMulCorCut[1];
    for (int i = 0; i < kMulEstimators; ++i) {
      for (int j = i + 1; j < kMulEstimators; ++j) {
        // skip kMul since it is a bad multiplicity estimate
        if (i == kMUL || j == kMUL) {
          continue;
          ;
        }
        if (fMultiplicity[j] > m_mul * fMultiplicity[i] + t_mul) {
          fEventCutsCounter[kRECO]->Fill(2 * (LAST_EEVENT + 1) + kMAX + 0.5);
          Flag = kFALSE;
        }
        if (fMultiplicity[j] < (fMultiplicity[i] - t_mul) / m_mul) {
          fEventCutsCounter[kRECO]->Fill(2 * (LAST_EEVENT + 1) + kMIN + 0.5);
          Flag = kFALSE;
        }
      }
    }
    // Double_t mulDiff = 0;
    // for (int i = 0; i < kMulEstimators; ++i) {
    //   for (int j = i + 1; j < kMulEstimators; ++j) {
    //     // exclude kMUL, since it is not a good estimate
    //     if (i == kMUL || j == kMUL) {
    //       continue;
    //     }
    //     // protect against division by zero
    //     if (fMultiplicity[i] > 0. && fMultiplicity[j] > 0.) {
    //       if (fMulCorCutMode == kDIFFABS) {
    //         mulDiff = std::abs(fMultiplicity[i] - fMultiplicity[j]);
    //       } else if (fMulCorCutMode == kDIFFREL) {
    //         mulDiff = std::abs(fMultiplicity[i] - fMultiplicity[j]) /
    //                   (fMultiplicity[i] + fMultiplicity[j]);
    //       } else {
    //         Fatal("SurviveEventCut", "No multiplicity difference");
    //       }
    //     }
    //     if (mulDiff > fMulCorCut) {
    //       fEventCutsCounter[kRECO]->Fill(2 * LAST_EEVENT + 1.5);
    //       Flag = kFALSE;
    //     }
    //   }
    // }
  }

  // check MC event
  AliMCEvent *aMC = dynamic_cast<AliMCEvent *>(ave);
  if (aMC) {

    // cut on multiplicity
    // if ((aMC->GetNumberOfTracks() < fEventCuts[kMUL][kMIN]) ||
    //     (aMC->GetNumberOfTracks() > fEventCuts[kMUL][kMAX])) {
    //   fEventCutsCounter[kSIM]->Fill(kMUL + 0.5);
    //   Flag = kFALSE;
    // }

    // // cut event if primary vertex is too out of center
    // if ((aMC->GetPrimaryVertex()->GetX() < fEventCuts[kX][kMIN]) ||
    //     (aMC->GetPrimaryVertex()->GetX() > fEventCuts[kX][kMAX])) {
    //   fEventCutsCounter[kSIM]->Fill(kX + 0.5);
    //   Flag = kFALSE;
    // }
    // if ((aMC->GetPrimaryVertex()->GetY() < fEventCuts[kY][kMIN]) ||
    //     (aMC->GetPrimaryVertex()->GetY() > fEventCuts[kY][kMAX])) {
    //   fEventCutsCounter[kSIM]->Fill(kY + 0.5);
    //   Flag = kFALSE;
    // }
    // if ((aMC->GetPrimaryVertex()->GetZ() < fEventCuts[kZ][kMIN]) ||
    //     (aMC->GetPrimaryVertex()->GetZ() > fEventCuts[kZ][kMAX])) {
    //   fEventCutsCounter[kSIM]->Fill(kZ + 0.5);
    //   Flag = kFALSE;
    // }
  }

  return Flag;
}

Bool_t AliAnalysisTaskAR::SurviveTrackCut(AliVParticle *avp,
                                          Bool_t FillCounter) {
  // check if current track survives track cut
  // return flag at the end, if one cut fails, set it to false
  Bool_t Flag = kTRUE;

  // check AOD track
  AliAODTrack *aTrack = dynamic_cast<AliAODTrack *>(avp);
  if (aTrack) {

    // cut PT
    if (aTrack->Pt() < fTrackCuts[kPT][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kPT + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (aTrack->Pt() > fTrackCuts[kPT][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kPT + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut PHI
    if (aTrack->Phi() < fTrackCuts[kPHI][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kPHI + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (aTrack->Phi() > fTrackCuts[kPHI][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kPHI + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut ETA
    if (aTrack->Eta() < fTrackCuts[kETA][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kETA + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (aTrack->Eta() > fTrackCuts[kETA][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kETA + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut on absolute value of CHARGE
    // set kMIN value larger than 0 and all neutral particles are cut away
    if (std::abs(aTrack->Charge()) <= fTrackCuts[kCHARGE][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kCHARGE + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (std::abs(aTrack->Charge()) >= fTrackCuts[kCHARGE][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kCHARGE + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut on number of clusters in the TPC
    if (aTrack->GetTPCNcls() < fTrackCuts[kTPCNCLS][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kTPCNCLS + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (aTrack->GetTPCNcls() > fTrackCuts[kTPCNCLS][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kTPCNCLS + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut on number of clusters in the ITS
    if (aTrack->GetITSNcls() < fTrackCuts[kITSNCLS][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kITSNCLS + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (aTrack->GetITSNcls() > fTrackCuts[kITSNCLS][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kITSNCLS + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut on chi2 / NDF of the track fit
    if (aTrack->Chi2perNDF() < fTrackCuts[kCHI2PERNDF][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kCHI2PERNDF + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (aTrack->Chi2perNDF() > fTrackCuts[kCHI2PERNDF][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kCHI2PERNDF + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut DCA in z direction
    if (aTrack->ZAtDCA() < fTrackCuts[kDCAZ][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kDCAZ + kMIN + 0.5);
        // if track is not constrained it returns dummy value -999
        // makes the counter blow up
      }
      Flag = kFALSE;
    }
    if (aTrack->ZAtDCA() > fTrackCuts[kDCAZ][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kDCAZ + kMAX + 0.5);
        // if track is not constrained it returns dummy value -999
        // makes the counter blow up
      }
      Flag = kFALSE;
    }
    // cut DCA in xy plane
    if (aTrack->DCA() < fTrackCuts[kDCAXY][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kDCAXY + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (aTrack->DCA() > fTrackCuts[kDCAXY][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * kDCAXY + kMAX + 0.5);
      }
      Flag = kFALSE;
    }

    // cut with filtertbit
    // filter bit 128 denotes TPC-only tracks, use only them for the
    // analysis, for hybrid tracks use filterbit 782
    // for more information about filterbits see the online week
    // the filterbits can change from run to run
    if (!aTrack->TestFilterBit(fFilterbit)) {
      if (FillCounter) {
        fTrackCutsCounter[kRECO]->Fill(2 * LAST_ETRACK + 0.5);
      }
      Flag = kFALSE;
    }

    // if set, cut all non-primary particles away
    if (fPrimaryOnly) {
      if (aTrack->GetType() != AliAODTrack::kPrimary) {
        if (FillCounter) {
          fTrackCutsCounter[kRECO]->Fill(2 * LAST_ETRACK + 1.5);
        }
        Flag = kFALSE;
      }
    }
  }

  // check MC particle
  AliAODMCParticle *MCParticle = dynamic_cast<AliAODMCParticle *>(avp);
  if (MCParticle) {
    // cut PT
    if (MCParticle->Pt() < fTrackCuts[kPT][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kSIM]->Fill(2 * kPT + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (MCParticle->Pt() > fTrackCuts[kPT][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kSIM]->Fill(2 * kPT + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut PHI
    if (MCParticle->Phi() < fTrackCuts[kPHI][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kSIM]->Fill(2 * kPHI + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (MCParticle->Phi() > fTrackCuts[kPHI][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kSIM]->Fill(2 * kPHI + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut ETA
    if (MCParticle->Eta() < fTrackCuts[kETA][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kSIM]->Fill(2 * kETA + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (MCParticle->Eta() > fTrackCuts[kETA][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kSIM]->Fill(2 * kETA + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // cut on absolute value of CHARGE
    // set kMIN value larger than 0 and all neutral particles are cut away
    if (std::abs(MCParticle->Charge() / 3) <= fTrackCuts[kCHARGE][kMIN]) {
      if (FillCounter) {
        fTrackCutsCounter[kSIM]->Fill(2 * kCHARGE + kMIN + 0.5);
      }
      Flag = kFALSE;
    }
    if (std::abs(MCParticle->Charge() / 3) >= fTrackCuts[kCHARGE][kMAX]) {
      if (FillCounter) {
        fTrackCutsCounter[kSIM]->Fill(2 * kCHARGE + kMAX + 0.5);
      }
      Flag = kFALSE;
    }
    // if set, cut all non-primary particles away
    if (fPrimaryOnly) {
      if (MCParticle->IsPrimary()) {
        // if (MCParticle->IsPhysicalPrimary())
        fTrackCutsCounter[kSIM]->Fill(2 * LAST_ETRACK + 1.5);
        Flag = kFALSE;
      }
    }
  }
  return Flag;
}

void AliAnalysisTaskAR::FillEventObjects(AliAODEvent *aAOD, AliMCEvent *aMC) {
  // get/compute event variables and fill them into data members

  // get centralities
  AliMultSelection *aMS =
      dynamic_cast<AliMultSelection *>(aAOD->FindListObject("MultSelection"));
  for (int cen = 0; cen < LAST_ECENESTIMATORS; ++cen) {
    fCentrality[cen] = aMS->GetMultiplicityPercentile(kCenEstimatorNames[cen]);
  }

  // multiplicity as number of tracks
  fMultiplicity[kMUL] = aAOD->GetNumberOfTracks();

  // multiplicity as number of contributors to the primary vertex
  AliAODVertex *PrimaryVertex = aAOD->GetPrimaryVertex();
  fMultiplicity[kNCONTRIB] = PrimaryVertex->GetNContributors();

  // reference multiplicity
  // combined reference multiplicity (tracklets + ITSTPC) in |eta|<0.8
  AliAODHeader *Header = dynamic_cast<AliAODHeader *>(aAOD->GetHeader());
  fMultiplicity[kMULREF] = Header->GetRefMultiplicityComb08();

  // multiplicity as number of tracks that survive track cuts
  fMultiplicity[kMULQ] = 0;
  // multiplicity as the weighted sum of all surviving tracks
  fMultiplicity[kMULW] = 0;
  Double_t w = 1.;

  AliAODTrack *aTrack = nullptr;
  if (0 != fLookUpTable->GetSize()) {
    fLookUpTable->Delete();
  }
  for (int iTrack = 0; iTrack < fMultiplicity[kMUL]; ++iTrack) {

    // getting pointer to a track
    aTrack = dynamic_cast<AliAODTrack *>(aAOD->GetTrack(iTrack));

    // protect against invalid pointers
    if (!aTrack) {
      continue;
    }

    if (!SurviveTrackCut(aTrack, kFALSE)) {
      continue;
    }
    fMultiplicity[kMULQ] += 1;

    w = 1.;
    if (fUseWeights[kPT] && fWeightHistogram[kPT]) {
      w *= fWeightHistogram[kPT]->GetBinContent(
          fWeightHistogram[kPT]->FindBin(aTrack->Pt()));
    }
    if (fUseWeights[kPHI] && fWeightHistogram[kPHI]) {
      w *= fWeightHistogram[kPHI]->GetBinContent(
          fWeightHistogram[kPHI]->FindBin(aTrack->Phi()));
    }
    if (fUseWeights[kETA] && fWeightHistogram[kETA]) {
      w *= fWeightHistogram[kETA]->GetBinContent(
          fWeightHistogram[kETA]->FindBin(aTrack->Eta()));
    }
    fMultiplicity[kMULW] += w;

    // since we are already looping over the events, create a look up table if
    // we also have a monte carlo event
    if (aMC) {
      // "key" = label, "value" = iTrack
      fLookUpTable->Add(aTrack->GetLabel(), iTrack);
    }
  }
}

void AliAnalysisTaskAR::MCPdfSymmetryPlanesSetup() {
  // set symmetry planes randomly on a event by event basis
  Double_t Psi = gRandom->Uniform(0., TMath::TwoPi());
  for (std::size_t i = 0; i < fMCFlowHarmonics.size(); ++i) {
    fMCPdf->SetParameter(2 * i + 1, Psi);
  }
}

Int_t AliAnalysisTaskAR::GetMCNumberOfParticlesPerEvent() {
  // compute number of paritcles per event, if set the number can fluctuate

  if (!fMCNumberOfParticlesPerEventFluctuations) {
    return fMCNumberOfParticlesPerEvent;
  }

  return gRandom->Uniform(fMCNumberOfParticlesPerEventRange[kMIN],
                          fMCNumberOfParticlesPerEventRange[kMAX]);
};

void AliAnalysisTaskAR::CalculateQvectors() {
  // Calculate all Q-vectors

  // Make sure all Q-vectors are initially zero
  for (Int_t h = 0; h < kMaxHarmonic; h++) {
    for (Int_t p = 0; p < kMaxPower; p++) {
      fQvector[h][p] = TComplex(0., 0.);
    }
  }

  // Calculate Q-vectors for available angles and weights
  Double_t dPhi = 0.;
  Double_t wPhi = 1.;         // particle weight
  Double_t wPhiToPowerP = 1.; // particle weight raised to power p
  for (std::size_t i = 0; i < fKinematics[kPHI].size(); i++) {
    dPhi = fKinematics[kPHI].at(i);
    if (fUseWeightsAggregated) {
      wPhi = fWeightsAggregated.at(i);
    }
    for (Int_t h = 0; h < kMaxHarmonic; h++) {
      for (Int_t p = 0; p < kMaxPower; p++) {
        if (fUseWeightsAggregated) {
          wPhiToPowerP = pow(wPhi, p);
        }
        fQvector[h][p] += TComplex(wPhiToPowerP * TMath::Cos(h * dPhi),
                                   wPhiToPowerP * TMath::Sin(h * dPhi));
      }
    }
  }
}

void AliAnalysisTaskAR::FillFinalResultProfile(kFinalProfile fp) {
  // fill final result profiles

  Double_t corr = 0.0;
  Double_t weight = 0.0;
  Int_t index = 0;

  // loop over all correlators
  for (auto Corr : fCorrelators) {
    // protect against insufficient amount of statistics i.e. number of
    // paritcles is lower then the order of correlator due to track cuts
    if (fKinematics[kPHI].size() < Corr.size()) {
      return;
    }

    // compute correlator
    switch (static_cast<int>(Corr.size())) {
    case 2:
      corr = Two(Corr.at(0), Corr.at(1)).Re();
      weight = Two(0, 0).Re();
      break;
    case 3:
      corr = Three(Corr.at(0), Corr.at(1), Corr.at(2)).Re();
      weight = Three(0, 0, 0).Re();
      break;
    case 4:
      corr = Four(Corr.at(0), Corr.at(1), Corr.at(2), Corr.at(3)).Re();
      weight = Four(0, 0, 0, 0).Re();
      break;
    case 5:
      corr =
          Five(Corr.at(0), Corr.at(1), Corr.at(2), Corr.at(3), Corr.at(4)).Re();
      weight = Five(0, 0, 0, 0, 0).Re();
      break;
    case 6:
      corr = Six(Corr.at(0), Corr.at(1), Corr.at(2), Corr.at(3), Corr.at(4),
                 Corr.at(5))
                 .Re();
      weight = Six(0, 0, 0, 0, 0, 0).Re();
      break;
    default:
      corr = Recursion(Corr.size(), Corr.data()).Re();
      weight = Recursion(Corr.size(), std::vector<Int_t>(Corr.size(), 0).data())
                   .Re();
    }

    // fill final result profile
    fFinalResultProfiles[fp]->Fill(index + 0.5, corr / weight, weight);
    index++;
  }
}

TComplex AliAnalysisTaskAR::Q(Int_t n, Int_t p) {
  // return Qvector from fQvector array

  if (n > kMaxHarmonic || p > kMaxPower) {
    std::cout << __LINE__ << ": running out of bounds" << std::endl;
    Fatal("Q", "Running out of bounds in fQvector");
  }
  if (n >= 0) {
    return fQvector[n][p];
  }
  return TComplex::Conjugate(fQvector[-n][p]);
}

TComplex AliAnalysisTaskAR::Two(Int_t n1, Int_t n2) {
  // Generic two-particle correlation <exp[i(n1*phi1+n2*phi2)]>.
  TComplex two = Q(n1, 1) * Q(n2, 1) - Q(n1 + n2, 2);
  return two;
}

TComplex AliAnalysisTaskAR::Three(Int_t n1, Int_t n2, Int_t n3) {
  // Generic three-particle correlation <exp[i(n1*phi1+n2*phi2+n3*phi3)]>.
  TComplex three = Q(n1, 1) * Q(n2, 1) * Q(n3, 1) - Q(n1 + n2, 2) * Q(n3, 1) -
                   Q(n2, 1) * Q(n1 + n3, 2) - Q(n1, 1) * Q(n2 + n3, 2) +
                   2. * Q(n1 + n2 + n3, 3);
  return three;
}

TComplex AliAnalysisTaskAR::Four(Int_t n1, Int_t n2, Int_t n3, Int_t n4) {
  // Generic four-particle correlation
  // <exp[i(n1*phi1+n2*phi2+n3*phi3+n4*phi4)]>.
  TComplex four =
      Q(n1, 1) * Q(n2, 1) * Q(n3, 1) * Q(n4, 1) -
      Q(n1 + n2, 2) * Q(n3, 1) * Q(n4, 1) -
      Q(n2, 1) * Q(n1 + n3, 2) * Q(n4, 1) -
      Q(n1, 1) * Q(n2 + n3, 2) * Q(n4, 1) + 2. * Q(n1 + n2 + n3, 3) * Q(n4, 1) -
      Q(n2, 1) * Q(n3, 1) * Q(n1 + n4, 2) + Q(n2 + n3, 2) * Q(n1 + n4, 2) -
      Q(n1, 1) * Q(n3, 1) * Q(n2 + n4, 2) + Q(n1 + n3, 2) * Q(n2 + n4, 2) +
      2. * Q(n3, 1) * Q(n1 + n2 + n4, 3) - Q(n1, 1) * Q(n2, 1) * Q(n3 + n4, 2) +
      Q(n1 + n2, 2) * Q(n3 + n4, 2) + 2. * Q(n2, 1) * Q(n1 + n3 + n4, 3) +
      2. * Q(n1, 1) * Q(n2 + n3 + n4, 3) - 6. * Q(n1 + n2 + n3 + n4, 4);

  return four;
}

TComplex AliAnalysisTaskAR::Five(Int_t n1, Int_t n2, Int_t n3, Int_t n4,
                                 Int_t n5) {
  // Generic five-particle correlation
  // <exp[i(n1*phi1+n2*phi2+n3*phi3+n4*phi4+n5*phi5)]>.
  TComplex five = Q(n1, 1) * Q(n2, 1) * Q(n3, 1) * Q(n4, 1) * Q(n5, 1) -
                  Q(n1 + n2, 2) * Q(n3, 1) * Q(n4, 1) * Q(n5, 1) -
                  Q(n2, 1) * Q(n1 + n3, 2) * Q(n4, 1) * Q(n5, 1) -
                  Q(n1, 1) * Q(n2 + n3, 2) * Q(n4, 1) * Q(n5, 1) +
                  2. * Q(n1 + n2 + n3, 3) * Q(n4, 1) * Q(n5, 1) -
                  Q(n2, 1) * Q(n3, 1) * Q(n1 + n4, 2) * Q(n5, 1) +
                  Q(n2 + n3, 2) * Q(n1 + n4, 2) * Q(n5, 1) -
                  Q(n1, 1) * Q(n3, 1) * Q(n2 + n4, 2) * Q(n5, 1) +
                  Q(n1 + n3, 2) * Q(n2 + n4, 2) * Q(n5, 1) +
                  2. * Q(n3, 1) * Q(n1 + n2 + n4, 3) * Q(n5, 1) -
                  Q(n1, 1) * Q(n2, 1) * Q(n3 + n4, 2) * Q(n5, 1) +
                  Q(n1 + n2, 2) * Q(n3 + n4, 2) * Q(n5, 1) +
                  2. * Q(n2, 1) * Q(n1 + n3 + n4, 3) * Q(n5, 1) +
                  2. * Q(n1, 1) * Q(n2 + n3 + n4, 3) * Q(n5, 1) -
                  6. * Q(n1 + n2 + n3 + n4, 4) * Q(n5, 1) -
                  Q(n2, 1) * Q(n3, 1) * Q(n4, 1) * Q(n1 + n5, 2) +
                  Q(n2 + n3, 2) * Q(n4, 1) * Q(n1 + n5, 2) +
                  Q(n3, 1) * Q(n2 + n4, 2) * Q(n1 + n5, 2) +
                  Q(n2, 1) * Q(n3 + n4, 2) * Q(n1 + n5, 2) -
                  2. * Q(n2 + n3 + n4, 3) * Q(n1 + n5, 2) -
                  Q(n1, 1) * Q(n3, 1) * Q(n4, 1) * Q(n2 + n5, 2) +
                  Q(n1 + n3, 2) * Q(n4, 1) * Q(n2 + n5, 2) +
                  Q(n3, 1) * Q(n1 + n4, 2) * Q(n2 + n5, 2) +
                  Q(n1, 1) * Q(n3 + n4, 2) * Q(n2 + n5, 2) -
                  2. * Q(n1 + n3 + n4, 3) * Q(n2 + n5, 2) +
                  2. * Q(n3, 1) * Q(n4, 1) * Q(n1 + n2 + n5, 3) -
                  2. * Q(n3 + n4, 2) * Q(n1 + n2 + n5, 3) -
                  Q(n1, 1) * Q(n2, 1) * Q(n4, 1) * Q(n3 + n5, 2) +
                  Q(n1 + n2, 2) * Q(n4, 1) * Q(n3 + n5, 2) +
                  Q(n2, 1) * Q(n1 + n4, 2) * Q(n3 + n5, 2) +
                  Q(n1, 1) * Q(n2 + n4, 2) * Q(n3 + n5, 2) -
                  2. * Q(n1 + n2 + n4, 3) * Q(n3 + n5, 2) +
                  2. * Q(n2, 1) * Q(n4, 1) * Q(n1 + n3 + n5, 3) -
                  2. * Q(n2 + n4, 2) * Q(n1 + n3 + n5, 3) +
                  2. * Q(n1, 1) * Q(n4, 1) * Q(n2 + n3 + n5, 3) -
                  2. * Q(n1 + n4, 2) * Q(n2 + n3 + n5, 3) -
                  6. * Q(n4, 1) * Q(n1 + n2 + n3 + n5, 4) -
                  Q(n1, 1) * Q(n2, 1) * Q(n3, 1) * Q(n4 + n5, 2) +
                  Q(n1 + n2, 2) * Q(n3, 1) * Q(n4 + n5, 2) +
                  Q(n2, 1) * Q(n1 + n3, 2) * Q(n4 + n5, 2) +
                  Q(n1, 1) * Q(n2 + n3, 2) * Q(n4 + n5, 2) -
                  2. * Q(n1 + n2 + n3, 3) * Q(n4 + n5, 2) +
                  2. * Q(n2, 1) * Q(n3, 1) * Q(n1 + n4 + n5, 3) -
                  2. * Q(n2 + n3, 2) * Q(n1 + n4 + n5, 3) +
                  2. * Q(n1, 1) * Q(n3, 1) * Q(n2 + n4 + n5, 3) -
                  2. * Q(n1 + n3, 2) * Q(n2 + n4 + n5, 3) -
                  6. * Q(n3, 1) * Q(n1 + n2 + n4 + n5, 4) +
                  2. * Q(n1, 1) * Q(n2, 1) * Q(n3 + n4 + n5, 3) -
                  2. * Q(n1 + n2, 2) * Q(n3 + n4 + n5, 3) -
                  6. * Q(n2, 1) * Q(n1 + n3 + n4 + n5, 4) -
                  6. * Q(n1, 1) * Q(n2 + n3 + n4 + n5, 4) +
                  24. * Q(n1 + n2 + n3 + n4 + n5, 5);
  return five;
}

TComplex AliAnalysisTaskAR::Six(Int_t n1, Int_t n2, Int_t n3, Int_t n4,
                                Int_t n5, Int_t n6) {
  // Generic six-particle correlation
  // <exp[i(n1*phi1+n2*phi2+n3*phi3+n4*phi4+n5*phi5+n6*phi6)]>.
  TComplex six =
      Q(n1, 1) * Q(n2, 1) * Q(n3, 1) * Q(n4, 1) * Q(n5, 1) * Q(n6, 1) -
      Q(n1 + n2, 2) * Q(n3, 1) * Q(n4, 1) * Q(n5, 1) * Q(n6, 1) -
      Q(n2, 1) * Q(n1 + n3, 2) * Q(n4, 1) * Q(n5, 1) * Q(n6, 1) -
      Q(n1, 1) * Q(n2 + n3, 2) * Q(n4, 1) * Q(n5, 1) * Q(n6, 1) +
      2. * Q(n1 + n2 + n3, 3) * Q(n4, 1) * Q(n5, 1) * Q(n6, 1) -
      Q(n2, 1) * Q(n3, 1) * Q(n1 + n4, 2) * Q(n5, 1) * Q(n6, 1) +
      Q(n2 + n3, 2) * Q(n1 + n4, 2) * Q(n5, 1) * Q(n6, 1) -
      Q(n1, 1) * Q(n3, 1) * Q(n2 + n4, 2) * Q(n5, 1) * Q(n6, 1) +
      Q(n1 + n3, 2) * Q(n2 + n4, 2) * Q(n5, 1) * Q(n6, 1) +
      2. * Q(n3, 1) * Q(n1 + n2 + n4, 3) * Q(n5, 1) * Q(n6, 1) -
      Q(n1, 1) * Q(n2, 1) * Q(n3 + n4, 2) * Q(n5, 1) * Q(n6, 1) +
      Q(n1 + n2, 2) * Q(n3 + n4, 2) * Q(n5, 1) * Q(n6, 1) +
      2. * Q(n2, 1) * Q(n1 + n3 + n4, 3) * Q(n5, 1) * Q(n6, 1) +
      2. * Q(n1, 1) * Q(n2 + n3 + n4, 3) * Q(n5, 1) * Q(n6, 1) -
      6. * Q(n1 + n2 + n3 + n4, 4) * Q(n5, 1) * Q(n6, 1) -
      Q(n2, 1) * Q(n3, 1) * Q(n4, 1) * Q(n1 + n5, 2) * Q(n6, 1) +
      Q(n2 + n3, 2) * Q(n4, 1) * Q(n1 + n5, 2) * Q(n6, 1) +
      Q(n3, 1) * Q(n2 + n4, 2) * Q(n1 + n5, 2) * Q(n6, 1) +
      Q(n2, 1) * Q(n3 + n4, 2) * Q(n1 + n5, 2) * Q(n6, 1) -
      2. * Q(n2 + n3 + n4, 3) * Q(n1 + n5, 2) * Q(n6, 1) -
      Q(n1, 1) * Q(n3, 1) * Q(n4, 1) * Q(n2 + n5, 2) * Q(n6, 1) +
      Q(n1 + n3, 2) * Q(n4, 1) * Q(n2 + n5, 2) * Q(n6, 1) +
      Q(n3, 1) * Q(n1 + n4, 2) * Q(n2 + n5, 2) * Q(n6, 1) +
      Q(n1, 1) * Q(n3 + n4, 2) * Q(n2 + n5, 2) * Q(n6, 1) -
      2. * Q(n1 + n3 + n4, 3) * Q(n2 + n5, 2) * Q(n6, 1) +
      2. * Q(n3, 1) * Q(n4, 1) * Q(n1 + n2 + n5, 3) * Q(n6, 1) -
      2. * Q(n3 + n4, 2) * Q(n1 + n2 + n5, 3) * Q(n6, 1) -
      Q(n1, 1) * Q(n2, 1) * Q(n4, 1) * Q(n3 + n5, 2) * Q(n6, 1) +
      Q(n1 + n2, 2) * Q(n4, 1) * Q(n3 + n5, 2) * Q(n6, 1) +
      Q(n2, 1) * Q(n1 + n4, 2) * Q(n3 + n5, 2) * Q(n6, 1) +
      Q(n1, 1) * Q(n2 + n4, 2) * Q(n3 + n5, 2) * Q(n6, 1) -
      2. * Q(n1 + n2 + n4, 3) * Q(n3 + n5, 2) * Q(n6, 1) +
      2. * Q(n2, 1) * Q(n4, 1) * Q(n1 + n3 + n5, 3) * Q(n6, 1) -
      2. * Q(n2 + n4, 2) * Q(n1 + n3 + n5, 3) * Q(n6, 1) +
      2. * Q(n1, 1) * Q(n4, 1) * Q(n2 + n3 + n5, 3) * Q(n6, 1) -
      2. * Q(n1 + n4, 2) * Q(n2 + n3 + n5, 3) * Q(n6, 1) -
      6. * Q(n4, 1) * Q(n1 + n2 + n3 + n5, 4) * Q(n6, 1) -
      Q(n1, 1) * Q(n2, 1) * Q(n3, 1) * Q(n4 + n5, 2) * Q(n6, 1) +
      Q(n1 + n2, 2) * Q(n3, 1) * Q(n4 + n5, 2) * Q(n6, 1) +
      Q(n2, 1) * Q(n1 + n3, 2) * Q(n4 + n5, 2) * Q(n6, 1) +
      Q(n1, 1) * Q(n2 + n3, 2) * Q(n4 + n5, 2) * Q(n6, 1) -
      2. * Q(n1 + n2 + n3, 3) * Q(n4 + n5, 2) * Q(n6, 1) +
      2. * Q(n2, 1) * Q(n3, 1) * Q(n1 + n4 + n5, 3) * Q(n6, 1) -
      2. * Q(n2 + n3, 2) * Q(n1 + n4 + n5, 3) * Q(n6, 1) +
      2. * Q(n1, 1) * Q(n3, 1) * Q(n2 + n4 + n5, 3) * Q(n6, 1) -
      2. * Q(n1 + n3, 2) * Q(n2 + n4 + n5, 3) * Q(n6, 1) -
      6. * Q(n3, 1) * Q(n1 + n2 + n4 + n5, 4) * Q(n6, 1) +
      2. * Q(n1, 1) * Q(n2, 1) * Q(n3 + n4 + n5, 3) * Q(n6, 1) -
      2. * Q(n1 + n2, 2) * Q(n3 + n4 + n5, 3) * Q(n6, 1) -
      6. * Q(n2, 1) * Q(n1 + n3 + n4 + n5, 4) * Q(n6, 1) -
      6. * Q(n1, 1) * Q(n2 + n3 + n4 + n5, 4) * Q(n6, 1) +
      24. * Q(n1 + n2 + n3 + n4 + n5, 5) * Q(n6, 1) -
      Q(n2, 1) * Q(n3, 1) * Q(n4, 1) * Q(n5, 1) * Q(n1 + n6, 2) +
      Q(n2 + n3, 2) * Q(n4, 1) * Q(n5, 1) * Q(n1 + n6, 2) +
      Q(n3, 1) * Q(n2 + n4, 2) * Q(n5, 1) * Q(n1 + n6, 2) +
      Q(n2, 1) * Q(n3 + n4, 2) * Q(n5, 1) * Q(n1 + n6, 2) -
      2. * Q(n2 + n3 + n4, 3) * Q(n5, 1) * Q(n1 + n6, 2) +
      Q(n3, 1) * Q(n4, 1) * Q(n2 + n5, 2) * Q(n1 + n6, 2) -
      Q(n3 + n4, 2) * Q(n2 + n5, 2) * Q(n1 + n6, 2) +
      Q(n2, 1) * Q(n4, 1) * Q(n3 + n5, 2) * Q(n1 + n6, 2) -
      Q(n2 + n4, 2) * Q(n3 + n5, 2) * Q(n1 + n6, 2) -
      2. * Q(n4, 1) * Q(n2 + n3 + n5, 3) * Q(n1 + n6, 2) +
      Q(n2, 1) * Q(n3, 1) * Q(n4 + n5, 2) * Q(n1 + n6, 2) -
      Q(n2 + n3, 2) * Q(n4 + n5, 2) * Q(n1 + n6, 2) -
      2. * Q(n3, 1) * Q(n2 + n4 + n5, 3) * Q(n1 + n6, 2) -
      2. * Q(n2, 1) * Q(n3 + n4 + n5, 3) * Q(n1 + n6, 2) +
      6. * Q(n2 + n3 + n4 + n5, 4) * Q(n1 + n6, 2) -
      Q(n1, 1) * Q(n3, 1) * Q(n4, 1) * Q(n5, 1) * Q(n2 + n6, 2) +
      Q(n1 + n3, 2) * Q(n4, 1) * Q(n5, 1) * Q(n2 + n6, 2) +
      Q(n3, 1) * Q(n1 + n4, 2) * Q(n5, 1) * Q(n2 + n6, 2) +
      Q(n1, 1) * Q(n3 + n4, 2) * Q(n5, 1) * Q(n2 + n6, 2) -
      2. * Q(n1 + n3 + n4, 3) * Q(n5, 1) * Q(n2 + n6, 2) +
      Q(n3, 1) * Q(n4, 1) * Q(n1 + n5, 2) * Q(n2 + n6, 2) -
      Q(n3 + n4, 2) * Q(n1 + n5, 2) * Q(n2 + n6, 2) +
      Q(n1, 1) * Q(n4, 1) * Q(n3 + n5, 2) * Q(n2 + n6, 2) -
      Q(n1 + n4, 2) * Q(n3 + n5, 2) * Q(n2 + n6, 2) -
      2. * Q(n4, 1) * Q(n1 + n3 + n5, 3) * Q(n2 + n6, 2) +
      Q(n1, 1) * Q(n3, 1) * Q(n4 + n5, 2) * Q(n2 + n6, 2) -
      Q(n1 + n3, 2) * Q(n4 + n5, 2) * Q(n2 + n6, 2) -
      2. * Q(n3, 1) * Q(n1 + n4 + n5, 3) * Q(n2 + n6, 2) -
      2. * Q(n1, 1) * Q(n3 + n4 + n5, 3) * Q(n2 + n6, 2) +
      6. * Q(n1 + n3 + n4 + n5, 4) * Q(n2 + n6, 2) +
      2. * Q(n3, 1) * Q(n4, 1) * Q(n5, 1) * Q(n1 + n2 + n6, 3) -
      2. * Q(n3 + n4, 2) * Q(n5, 1) * Q(n1 + n2 + n6, 3) -
      2. * Q(n4, 1) * Q(n3 + n5, 2) * Q(n1 + n2 + n6, 3) -
      2. * Q(n3, 1) * Q(n4 + n5, 2) * Q(n1 + n2 + n6, 3) +
      4. * Q(n3 + n4 + n5, 3) * Q(n1 + n2 + n6, 3) -
      Q(n1, 1) * Q(n2, 1) * Q(n4, 1) * Q(n5, 1) * Q(n3 + n6, 2) +
      Q(n1 + n2, 2) * Q(n4, 1) * Q(n5, 1) * Q(n3 + n6, 2) +
      Q(n2, 1) * Q(n1 + n4, 2) * Q(n5, 1) * Q(n3 + n6, 2) +
      Q(n1, 1) * Q(n2 + n4, 2) * Q(n5, 1) * Q(n3 + n6, 2) -
      2. * Q(n1 + n2 + n4, 3) * Q(n5, 1) * Q(n3 + n6, 2) +
      Q(n2, 1) * Q(n4, 1) * Q(n1 + n5, 2) * Q(n3 + n6, 2) -
      Q(n2 + n4, 2) * Q(n1 + n5, 2) * Q(n3 + n6, 2) +
      Q(n1, 1) * Q(n4, 1) * Q(n2 + n5, 2) * Q(n3 + n6, 2) -
      Q(n1 + n4, 2) * Q(n2 + n5, 2) * Q(n3 + n6, 2) -
      2. * Q(n4, 1) * Q(n1 + n2 + n5, 3) * Q(n3 + n6, 2) +
      Q(n1, 1) * Q(n2, 1) * Q(n4 + n5, 2) * Q(n3 + n6, 2) -
      Q(n1 + n2, 2) * Q(n4 + n5, 2) * Q(n3 + n6, 2) -
      2. * Q(n2, 1) * Q(n1 + n4 + n5, 3) * Q(n3 + n6, 2) -
      2. * Q(n1, 1) * Q(n2 + n4 + n5, 3) * Q(n3 + n6, 2) +
      6. * Q(n1 + n2 + n4 + n5, 4) * Q(n3 + n6, 2) +
      2. * Q(n2, 1) * Q(n4, 1) * Q(n5, 1) * Q(n1 + n3 + n6, 3) -
      2. * Q(n2 + n4, 2) * Q(n5, 1) * Q(n1 + n3 + n6, 3) -
      2. * Q(n4, 1) * Q(n2 + n5, 2) * Q(n1 + n3 + n6, 3) -
      2. * Q(n2, 1) * Q(n4 + n5, 2) * Q(n1 + n3 + n6, 3) +
      4. * Q(n2 + n4 + n5, 3) * Q(n1 + n3 + n6, 3) +
      2. * Q(n1, 1) * Q(n4, 1) * Q(n5, 1) * Q(n2 + n3 + n6, 3) -
      2. * Q(n1 + n4, 2) * Q(n5, 1) * Q(n2 + n3 + n6, 3) -
      2. * Q(n4, 1) * Q(n1 + n5, 2) * Q(n2 + n3 + n6, 3) -
      2. * Q(n1, 1) * Q(n4 + n5, 2) * Q(n2 + n3 + n6, 3) +
      4. * Q(n1 + n4 + n5, 3) * Q(n2 + n3 + n6, 3) -
      6. * Q(n4, 1) * Q(n5, 1) * Q(n1 + n2 + n3 + n6, 4) +
      6. * Q(n4 + n5, 2) * Q(n1 + n2 + n3 + n6, 4) -
      Q(n1, 1) * Q(n2, 1) * Q(n3, 1) * Q(n5, 1) * Q(n4 + n6, 2) +
      Q(n1 + n2, 2) * Q(n3, 1) * Q(n5, 1) * Q(n4 + n6, 2) +
      Q(n2, 1) * Q(n1 + n3, 2) * Q(n5, 1) * Q(n4 + n6, 2) +
      Q(n1, 1) * Q(n2 + n3, 2) * Q(n5, 1) * Q(n4 + n6, 2) -
      2. * Q(n1 + n2 + n3, 3) * Q(n5, 1) * Q(n4 + n6, 2) +
      Q(n2, 1) * Q(n3, 1) * Q(n1 + n5, 2) * Q(n4 + n6, 2) -
      Q(n2 + n3, 2) * Q(n1 + n5, 2) * Q(n4 + n6, 2) +
      Q(n1, 1) * Q(n3, 1) * Q(n2 + n5, 2) * Q(n4 + n6, 2) -
      Q(n1 + n3, 2) * Q(n2 + n5, 2) * Q(n4 + n6, 2) -
      2. * Q(n3, 1) * Q(n1 + n2 + n5, 3) * Q(n4 + n6, 2) +
      Q(n1, 1) * Q(n2, 1) * Q(n3 + n5, 2) * Q(n4 + n6, 2) -
      Q(n1 + n2, 2) * Q(n3 + n5, 2) * Q(n4 + n6, 2) -
      2. * Q(n2, 1) * Q(n1 + n3 + n5, 3) * Q(n4 + n6, 2) -
      2. * Q(n1, 1) * Q(n2 + n3 + n5, 3) * Q(n4 + n6, 2) +
      6. * Q(n1 + n2 + n3 + n5, 4) * Q(n4 + n6, 2) +
      2. * Q(n2, 1) * Q(n3, 1) * Q(n5, 1) * Q(n1 + n4 + n6, 3) -
      2. * Q(n2 + n3, 2) * Q(n5, 1) * Q(n1 + n4 + n6, 3) -
      2. * Q(n3, 1) * Q(n2 + n5, 2) * Q(n1 + n4 + n6, 3) -
      2. * Q(n2, 1) * Q(n3 + n5, 2) * Q(n1 + n4 + n6, 3) +
      4. * Q(n2 + n3 + n5, 3) * Q(n1 + n4 + n6, 3) +
      2. * Q(n1, 1) * Q(n3, 1) * Q(n5, 1) * Q(n2 + n4 + n6, 3) -
      2. * Q(n1 + n3, 2) * Q(n5, 1) * Q(n2 + n4 + n6, 3) -
      2. * Q(n3, 1) * Q(n1 + n5, 2) * Q(n2 + n4 + n6, 3) -
      2. * Q(n1, 1) * Q(n3 + n5, 2) * Q(n2 + n4 + n6, 3) +
      4. * Q(n1 + n3 + n5, 3) * Q(n2 + n4 + n6, 3) -
      6. * Q(n3, 1) * Q(n5, 1) * Q(n1 + n2 + n4 + n6, 4) +
      6. * Q(n3 + n5, 2) * Q(n1 + n2 + n4 + n6, 4) +
      2. * Q(n1, 1) * Q(n2, 1) * Q(n5, 1) * Q(n3 + n4 + n6, 3) -
      2. * Q(n1 + n2, 2) * Q(n5, 1) * Q(n3 + n4 + n6, 3) -
      2. * Q(n2, 1) * Q(n1 + n5, 2) * Q(n3 + n4 + n6, 3) -
      2. * Q(n1, 1) * Q(n2 + n5, 2) * Q(n3 + n4 + n6, 3) +
      4. * Q(n1 + n2 + n5, 3) * Q(n3 + n4 + n6, 3) -
      6. * Q(n2, 1) * Q(n5, 1) * Q(n1 + n3 + n4 + n6, 4) +
      6. * Q(n2 + n5, 2) * Q(n1 + n3 + n4 + n6, 4) -
      6. * Q(n1, 1) * Q(n5, 1) * Q(n2 + n3 + n4 + n6, 4) +
      6. * Q(n1 + n5, 2) * Q(n2 + n3 + n4 + n6, 4) +
      24. * Q(n5, 1) * Q(n1 + n2 + n3 + n4 + n6, 5) -
      Q(n1, 1) * Q(n2, 1) * Q(n3, 1) * Q(n4, 1) * Q(n5 + n6, 2) +
      Q(n1 + n2, 2) * Q(n3, 1) * Q(n4, 1) * Q(n5 + n6, 2) +
      Q(n2, 1) * Q(n1 + n3, 2) * Q(n4, 1) * Q(n5 + n6, 2) +
      Q(n1, 1) * Q(n2 + n3, 2) * Q(n4, 1) * Q(n5 + n6, 2) -
      2. * Q(n1 + n2 + n3, 3) * Q(n4, 1) * Q(n5 + n6, 2) +
      Q(n2, 1) * Q(n3, 1) * Q(n1 + n4, 2) * Q(n5 + n6, 2) -
      Q(n2 + n3, 2) * Q(n1 + n4, 2) * Q(n5 + n6, 2) +
      Q(n1, 1) * Q(n3, 1) * Q(n2 + n4, 2) * Q(n5 + n6, 2) -
      Q(n1 + n3, 2) * Q(n2 + n4, 2) * Q(n5 + n6, 2) -
      2. * Q(n3, 1) * Q(n1 + n2 + n4, 3) * Q(n5 + n6, 2) +
      Q(n1, 1) * Q(n2, 1) * Q(n3 + n4, 2) * Q(n5 + n6, 2) -
      Q(n1 + n2, 2) * Q(n3 + n4, 2) * Q(n5 + n6, 2) -
      2. * Q(n2, 1) * Q(n1 + n3 + n4, 3) * Q(n5 + n6, 2) -
      2. * Q(n1, 1) * Q(n2 + n3 + n4, 3) * Q(n5 + n6, 2) +
      6. * Q(n1 + n2 + n3 + n4, 4) * Q(n5 + n6, 2) +
      2. * Q(n2, 1) * Q(n3, 1) * Q(n4, 1) * Q(n1 + n5 + n6, 3) -
      2. * Q(n2 + n3, 2) * Q(n4, 1) * Q(n1 + n5 + n6, 3) -
      2. * Q(n3, 1) * Q(n2 + n4, 2) * Q(n1 + n5 + n6, 3) -
      2. * Q(n2, 1) * Q(n3 + n4, 2) * Q(n1 + n5 + n6, 3) +
      4. * Q(n2 + n3 + n4, 3) * Q(n1 + n5 + n6, 3) +
      2. * Q(n1, 1) * Q(n3, 1) * Q(n4, 1) * Q(n2 + n5 + n6, 3) -
      2. * Q(n1 + n3, 2) * Q(n4, 1) * Q(n2 + n5 + n6, 3) -
      2. * Q(n3, 1) * Q(n1 + n4, 2) * Q(n2 + n5 + n6, 3) -
      2. * Q(n1, 1) * Q(n3 + n4, 2) * Q(n2 + n5 + n6, 3) +
      4. * Q(n1 + n3 + n4, 3) * Q(n2 + n5 + n6, 3) -
      6. * Q(n3, 1) * Q(n4, 1) * Q(n1 + n2 + n5 + n6, 4) +
      6. * Q(n3 + n4, 2) * Q(n1 + n2 + n5 + n6, 4) +
      2. * Q(n1, 1) * Q(n2, 1) * Q(n4, 1) * Q(n3 + n5 + n6, 3) -
      2. * Q(n1 + n2, 2) * Q(n4, 1) * Q(n3 + n5 + n6, 3) -
      2. * Q(n2, 1) * Q(n1 + n4, 2) * Q(n3 + n5 + n6, 3) -
      2. * Q(n1, 1) * Q(n2 + n4, 2) * Q(n3 + n5 + n6, 3) +
      4. * Q(n1 + n2 + n4, 3) * Q(n3 + n5 + n6, 3) -
      6. * Q(n2, 1) * Q(n4, 1) * Q(n1 + n3 + n5 + n6, 4) +
      6. * Q(n2 + n4, 2) * Q(n1 + n3 + n5 + n6, 4) -
      6. * Q(n1, 1) * Q(n4, 1) * Q(n2 + n3 + n5 + n6, 4) +
      6. * Q(n1 + n4, 2) * Q(n2 + n3 + n5 + n6, 4) +
      24. * Q(n4, 1) * Q(n1 + n2 + n3 + n5 + n6, 5) +
      2. * Q(n1, 1) * Q(n2, 1) * Q(n3, 1) * Q(n4 + n5 + n6, 3) -
      2. * Q(n1 + n2, 2) * Q(n3, 1) * Q(n4 + n5 + n6, 3) -
      2. * Q(n2, 1) * Q(n1 + n3, 2) * Q(n4 + n5 + n6, 3) -
      2. * Q(n1, 1) * Q(n2 + n3, 2) * Q(n4 + n5 + n6, 3) +
      4. * Q(n1 + n2 + n3, 3) * Q(n4 + n5 + n6, 3) -
      6. * Q(n2, 1) * Q(n3, 1) * Q(n1 + n4 + n5 + n6, 4) +
      6. * Q(n2 + n3, 2) * Q(n1 + n4 + n5 + n6, 4) -
      6. * Q(n1, 1) * Q(n3, 1) * Q(n2 + n4 + n5 + n6, 4) +
      6. * Q(n1 + n3, 2) * Q(n2 + n4 + n5 + n6, 4) +
      24. * Q(n3, 1) * Q(n1 + n2 + n4 + n5 + n6, 5) -
      6. * Q(n1, 1) * Q(n2, 1) * Q(n3 + n4 + n5 + n6, 4) +
      6. * Q(n1 + n2, 2) * Q(n3 + n4 + n5 + n6, 4) +
      24. * Q(n2, 1) * Q(n1 + n3 + n4 + n5 + n6, 5) +
      24. * Q(n1, 1) * Q(n2 + n3 + n4 + n5 + n6, 5) -
      120. * Q(n1 + n2 + n3 + n4 + n5 + n6, 6);
  return six;
}

TComplex AliAnalysisTaskAR::Recursion(Int_t n, Int_t *harmonic,
                                      Int_t mult /* = 1*/, Int_t skip /*= 0*/) {
  // Calculate multi-particle correlators by using recursion (an improved
  // faster version) originally developed by Kristjan Gulbrandsen
  // (gulbrand@nbi.dk).

  Int_t nm1 = n - 1;
  TComplex c(Q(harmonic[nm1], mult));
  if (nm1 == 0)
    return c;
  c *= Recursion(nm1, harmonic);
  if (nm1 == skip)
    return c;

  Int_t multp1 = mult + 1;
  Int_t nm2 = n - 2;
  Int_t counter1 = 0;
  Int_t hhold = harmonic[counter1];
  harmonic[counter1] = harmonic[nm2];
  harmonic[nm2] = hhold + harmonic[nm1];
  TComplex c2(Recursion(nm1, harmonic, multp1, nm2));
  Int_t counter2 = n - 3;
  while (counter2 >= skip) {
    harmonic[nm2] = harmonic[counter1];
    harmonic[counter1] = hhold;
    ++counter1;
    hhold = harmonic[counter1];
    harmonic[counter1] = harmonic[nm2];
    harmonic[nm2] = hhold + harmonic[nm1];
    c2 += Recursion(nm1, harmonic, multp1, counter2);
    --counter2;
  }
  harmonic[nm2] = harmonic[counter1];
  harmonic[counter1] = hhold;

  if (mult == 1)
    return c - c2;
  return c - Double_t(mult) * c2;
}

Double_t AliAnalysisTaskAR::CombinatorialWeight(Int_t n) {
  // calculate combinatrial weight for Qvectors
  // used mainly for nested loops
  if (n >= static_cast<Int_t>(fKinematics[kPHI].size())) {
    std::cout << __LINE__ << ": Two few particles for this correlator"
              << std::endl;
    Fatal("Combinatorial weight",
          "order of correlator is larger then number of particles");
  }
  Double_t w = 1.0;
  for (int i = 0; i < n; ++i) {
    w *= (fKinematics[kPHI].size() - i);
  }
  return w;
}

TComplex AliAnalysisTaskAR::TwoNestedLoops(Int_t n1, Int_t n2) {
  // Calculation of <cos(n1*phi1+n2*phi2)> and <sin(n1*phi1+n2*phi2)>
  // with two nested loops

  TComplex Two(0., 0.);

  Double_t phi1 = 0., phi2 = 0.; // particle angle
  Double_t w1 = 1., w2 = 1.;     // particle weight
  for (std::size_t i1 = 0; i1 < fKinematics[kPHI].size(); i1++) {

    phi1 = fKinematics[kPHI].at(i1);
    if (fUseWeightsAggregated) {
      w1 *= fWeightsAggregated.at(i1);
    }
    for (std::size_t i2 = 0; i2 < fKinematics[kPHI].size(); i2++) {
      if (i2 == i1) {
        continue;
      } // Get rid of autocorrelations
      phi2 = fKinematics[kPHI].at(i2);
      if (fUseWeightsAggregated) {
        w2 *= fWeightsAggregated.at(i2);
      }
      Two += TComplex(TMath::Cos(n1 * w1 * phi1 + n2 * w2 * phi2),
                      TMath::Sin(n1 * w1 * phi1 + n2 * w2 * phi2));
    }
  }
  return Two / CombinatorialWeight(2);
}

TComplex AliAnalysisTaskAR::ThreeNestedLoops(Int_t n1, Int_t n2, Int_t n3) {
  // Calculation of <cos(n1*phi1+n2*phi2+n3*phi3)> and
  // <sin(n1*phi1+n2*phi2+n3*phi3)> with three nested loops.

  TComplex Q(0., 0.);
  Double_t phi1 = 0., phi2 = 0., phi3 = 0.; // particle angle
  Double_t w1 = 1., w2 = 1., w3 = 1.;       // particle weight
  for (std::size_t i1 = 0; i1 < fKinematics[kPHI].size(); i1++) {
    phi1 = fKinematics[kPHI].at(i1);
    if (fUseWeightsAggregated) {
      w1 = fWeightsAggregated.at(i1);
    }
    for (std::size_t i2 = 0; i2 < fKinematics[kPHI].size(); i2++) {
      if (i2 == i1) {
        continue;
      } // Get rid of autocorrelations
      phi2 = fKinematics[kPHI].at(i2);
      if (fUseWeightsAggregated) {
        w2 = fWeightsAggregated.at(i2);
      }
      for (std::size_t i3 = 0; i3 < fKinematics[kPHI].size(); i3++) {
        if (i3 == i1 || i3 == i2) {
          continue;
        } // Get rid of autocorrelations
        phi3 = fKinematics[kPHI].at(i3);
        if (fUseWeightsAggregated) {
          w3 = fWeightsAggregated.at(i3);
        }
        Q += TComplex(
            TMath::Cos(n1 * w1 * phi1 + n2 * w2 * phi2 + n3 * w3 * phi3),
            TMath::Sin(n1 * w1 * phi1 + n2 * w2 * phi2 + n3 * w3 * phi3));
      }
    }
  }
  return Q / CombinatorialWeight(3);
}

TComplex AliAnalysisTaskAR::FourNestedLoops(Int_t n1, Int_t n2, Int_t n3,
                                            Int_t n4) {
  // Calculation of <cos(n1*phi1+n2*phi2+n3*phi3+n4*phi4)> and
  // <sin(n1*phi1+n2*phi2+n3*phi3+n4*phi4)> with four nested loops.

  TComplex Q(0., 0.);
  Double_t phi1 = 0., phi2 = 0., phi3 = 0., phi4 = 0.; // particle angle
  Double_t w1 = 1., w2 = 1., w3 = 1., w4 = 1.;         // particle weight
  for (std::size_t i1 = 0; i1 < fKinematics[kPHI].size(); i1++) {
    phi1 = fKinematics[kPHI].at(i1);
    if (fUseWeightsAggregated) {
      w1 = fWeightsAggregated.at(i1);
    }
    for (std::size_t i2 = 0; i2 < fKinematics[kPHI].size(); i2++) {
      if (i2 == i1) {
        continue;
      } // Get rid of autocorrelations
      phi2 = fKinematics[kPHI].at(i2);
      if (fUseWeightsAggregated) {
        w2 = fWeightsAggregated.at(i2);
      }
      for (std::size_t i3 = 0; i3 < fKinematics[kPHI].size(); i3++) {
        if (i3 == i1 || i3 == i2) {
          continue;
        } // Get rid of autocorrelations
        phi3 = fKinematics[kPHI].at(i3);
        if (fUseWeightsAggregated) {
          w3 = fWeightsAggregated.at(i3);
        }
        for (std::size_t i4 = 0; i4 < fKinematics[kPHI].size(); i4++) {
          if (i4 == i1 || i4 == i2 || i4 == i3) {
            continue;
          } // Get rid of autocorrelations
          phi4 = fKinematics[kPHI].at(i4);
          if (fUseWeightsAggregated) {
            w4 = fWeightsAggregated.at(i4);
          }
          Q += TComplex(TMath::Cos(n1 * w1 * phi1 + n2 * w2 * phi2 +
                                   n3 * w3 * phi3 + n4 * w4 * phi4),
                        TMath::Sin(n1 * w1 * phi1 + n2 * w2 * phi2 +
                                   n3 * w3 * phi3 + n4 * w4 * phi4));
        }
      }
    }
  }
  return Q / CombinatorialWeight(4);
}

void AliAnalysisTaskAR::SetCenCorQAHistogramBinning(
    kCenEstimators cen1, Int_t xnbins, Double_t xlowerEdge, Double_t xupperEdge,
    kCenEstimators cen2, Int_t ynbins, Double_t ylowerEdge,
    Double_t yupperEdge) {
  if (cen1 >= LAST_ECENESTIMATORS || cen2 >= LAST_ECENESTIMATORS) {
    std::cout << __LINE__ << ": running out of bounds" << std::endl;
    Fatal("SetCenCorQAHistogramBinning",
          "Running out of bounds in SetCenCorQAHistogramBinning");
  }
  if (xupperEdge < xlowerEdge && yupperEdge < ylowerEdge) {
    std::cout << __LINE__ << ": upper edge has to be larger than the lower edge"
              << std::endl;
    Fatal("SetCenCorQAHistogramBinning",
          ": upper edge has to be larger than the lower edge");
  }
  this->fCenCorQAHistogramBins[IndexCorHistograms(
      cen1, cen2, LAST_ECENESTIMATORS)][kBIN] = xnbins;
  this->fCenCorQAHistogramBins[IndexCorHistograms(
      cen1, cen2, LAST_ECENESTIMATORS)][kLEDGE] = xlowerEdge;
  this->fCenCorQAHistogramBins[IndexCorHistograms(
      cen1, cen2, LAST_ECENESTIMATORS)][kUEDGE] = xupperEdge;
  this->fCenCorQAHistogramBins[IndexCorHistograms(
      cen1, cen2, LAST_ECENESTIMATORS)][kBIN + LAST_EBINS] = ynbins;
  this->fCenCorQAHistogramBins[IndexCorHistograms(
      cen1, cen2, LAST_ECENESTIMATORS)][kLEDGE + LAST_EBINS] = ylowerEdge;
  this->fCenCorQAHistogramBins[IndexCorHistograms(
      cen1, cen2, LAST_ECENESTIMATORS)][kUEDGE + LAST_EBINS] = yupperEdge;
}

void AliAnalysisTaskAR::SetMulCorQAHistogramBinning(
    kCenEstimators mul1, Int_t xnbins, Double_t xlowerEdge, Double_t xupperEdge,
    kCenEstimators mul2, Int_t ynbins, Double_t ylowerEdge,
    Double_t yupperEdge) {
  if (mul1 >= kMulEstimators || mul2 >= kMulEstimators) {
    std::cout << __LINE__ << ": running out of bounds" << std::endl;
    Fatal("SetMulCorQAHistogramBinning",
          "Running out of bounds in SetMulCorQAHistogramBinning");
  }
  if (xupperEdge < xlowerEdge && yupperEdge < ylowerEdge) {
    std::cout << __LINE__ << ": upper edge has to be larger than the lower edge"
              << std::endl;
    Fatal("SetMulCorQAHistogramBinning",
          ": upper edge has to be larger than the lower edge");
  }
  this->fMulCorQAHistogramBins[IndexCorHistograms(
      mul1, mul2, LAST_ECENESTIMATORS)][kBIN] = xnbins;
  this->fMulCorQAHistogramBins[IndexCorHistograms(
      mul1, mul2, LAST_ECENESTIMATORS)][kLEDGE] = xlowerEdge;
  this->fMulCorQAHistogramBins[IndexCorHistograms(
      mul1, mul2, LAST_ECENESTIMATORS)][kUEDGE] = xupperEdge;
  this->fMulCorQAHistogramBins[IndexCorHistograms(
      mul1, mul2, LAST_ECENESTIMATORS)][kBIN + LAST_EBINS] = ynbins;
  this->fMulCorQAHistogramBins[IndexCorHistograms(
      mul1, mul2, LAST_ECENESTIMATORS)][kLEDGE + LAST_EBINS] = ylowerEdge;
  this->fMulCorQAHistogramBins[IndexCorHistograms(
      mul1, mul2, LAST_ECENESTIMATORS)][kUEDGE + LAST_EBINS] = yupperEdge;
}

void AliAnalysisTaskAR::SetAcceptanceHistogram(kTrack kinematic,
                                               const char *Filename,
                                               const char *Histname) {
  // set a acceptance histogram for monte carlo closure

  // check if index is out of range
  if (kinematic > kKinematic) {
    std::cout << __LINE__ << ": Out of range" << std::endl;
    Fatal("SetAccpetanceHistogram", "Out of range");
  }
  // check if file exists
  if (gSystem->AccessPathName(Filename, kFileExists)) {
    std::cout << __LINE__ << ": File does not exist" << std::endl;
    Fatal("SetAcceptanceHistogram", "Invalid file name");
  }
  TFile *file = new TFile(Filename, "READ");
  if (!file) {
    std::cout << __LINE__ << ": Cannot open file" << std::endl;
    Fatal("SetAcceptanceHistogram", "ROOT file cannot be read");
  }
  this->fAcceptanceHistogram[kinematic] =
      dynamic_cast<TH1D *>(file->Get(Histname));
  if (!fAcceptanceHistogram[kinematic]) {
    std::cout << __LINE__ << ": No acceptance histogram" << std::endl;
    Fatal("SetAcceptanceHistogram", "Cannot get acceptance histogram");
  }
  // keeps the histogram in memory after we close the file
  this->fAcceptanceHistogram[kinematic]->SetDirectory(0);
  file->Close();
}

void AliAnalysisTaskAR::SetWeightHistogram(kTrack kinematic,
                                           const char *Filename,
                                           const char *Histname) {
  // set weight histogram

  // check if index is out of range
  if (kinematic > kKinematic) {
    std::cout << __LINE__ << ": Out of range" << std::endl;
    Fatal("SetAccpetanceHistogram", "Out of range");
  }
  // check if file exists
  if (gSystem->AccessPathName(Filename, kFileExists)) {
    std::cout << __LINE__ << ": File does not exist" << std::endl;
    Fatal("SetWeightHistogram", "Invalid file name");
  }
  TFile *file = new TFile(Filename, "READ");
  if (!file) {
    std::cout << __LINE__ << ": Cannot open file" << std::endl;
    Fatal("SetWeightHistogram", "ROOT file cannot be read");
  }
  this->fWeightHistogram[kinematic] = dynamic_cast<TH1D *>(file->Get(Histname));
  if (!fWeightHistogram[kinematic]) {
    std::cout << __LINE__ << ": No acceptance histogram" << std::endl;
    Fatal("SetWeightHistogram", "Cannot get weight histogram");
  }
  // keeps the histogram in memory after we close the file
  this->fWeightHistogram[kinematic]->SetDirectory(0);
  file->Close();
  this->fUseWeights[kinematic] = kTRUE;
}

void AliAnalysisTaskAR::GetPointers(TList *histList) {
  // Initialize pointer for base list fHistList so we can initialize all of
  // objects and call terminate off-line

  fHistList = histList;
  if (!fHistList) {
    std::cout << __LINE__ << ": Did not get " << fHistListName << std::endl;
    Fatal("GetPointers", "Invalid Pointer");
  }

  // initialize all other objects
  this->GetPointersForControlHistograms();
  this->GetPointersForQAHistograms();
  this->GetPointersForFinalResultHistograms();
  this->GetPointersForFinalResultProfiles();
}

void AliAnalysisTaskAR::GetPointersForQAHistograms() {
  // get pointers for QA Histograms

  // get pointer for fControlHistograms
  fQAHistogramsList =
      dynamic_cast<TList *>(fHistList->FindObject(fQAHistogramsListName));
  // if the pointer is null, then there was no QA
  if (!fQAHistogramsList) {
    return;
  }

  // get pointer for fCenCorQAHistogramsList
  fCenCorQAHistogramsList = dynamic_cast<TList *>(
      fQAHistogramsList->FindObject(fCenCorQAHistogramsListName));
  if (!fCenCorQAHistogramsList) {
    std::cout << __LINE__ << ": Did not get " << fCenCorQAHistogramsListName
              << std::endl;
    Fatal("GetPointersForQAHistograms", "Invalid Pointer");
  }

  // get pointers for centrality correlation histograms
  for (int cen = 0; cen < LAST_ECENESTIMATORS * (LAST_ECENESTIMATORS - 1) / 2;
       ++cen) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fCenCorQAHistograms[cen][ba] =
          dynamic_cast<TH2D *>(fCenCorQAHistogramsList->FindObject(
              fCenCorQAHistogramNames[cen][ba][kNAME]));
      if (!fCenCorQAHistograms[cen][ba]) {
        std::cout << __LINE__ << ": Did not get "
                  << fCenCorQAHistogramNames[cen][ba][kNAME] << std::endl;
        Fatal("GetPointersForQAHistograms", "Invalid Pointer");
      }
    }
  }

  // get pointer for fMulCorQAHistogramsList
  fMulCorQAHistogramsList = dynamic_cast<TList *>(
      fQAHistogramsList->FindObject(fMulCorQAHistogramsListName));
  if (!fMulCorQAHistogramsList) {
    std::cout << __LINE__ << ": Did not get " << fMulCorQAHistogramsList
              << std::endl;
    Fatal("GetPointersForQAHistograms", "Invalid Pointer");
  }

  // get pointers for multiplicity correlation histograms
  for (int mul = 0; mul < kMulEstimators; ++mul) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fMulCorQAHistograms[mul][ba] =
          dynamic_cast<TH2D *>(fMulCorQAHistogramsList->FindObject(
              fMulCorQAHistogramNames[mul][ba][kNAME]));
      if (!fMulCorQAHistograms[mul][ba]) {
        std::cout << __LINE__ << ": Did not get "
                  << fMulCorQAHistogramNames[mul][ba][kNAME] << std::endl;
        Fatal("GetPointersForQAHistograms", "Invalid Pointer");
      }
    }
  }

  // get pointer for fFBScanQAHistogramsList
  fFBScanQAHistogramsList = dynamic_cast<TList *>(
      fQAHistogramsList->FindObject(fFBScanQAHistogramsListName));
  if (!fFBScanQAHistogramsList) {
    std::cout << __LINE__ << ": Did not get " << fFBScanQAHistogramsListName
              << std::endl;
    Fatal("GetPointersForQAHistograms", "Invalid Pointer");
  }

  // get pointer for filter bit scan histogram
  fFBScanQAHistogram = dynamic_cast<TH1D *>(
      fFBScanQAHistogramsList->FindObject(fFBScanQAHistogramName[kNAME]));
  if (!fFBScanQAHistogram) {
    std::cout << __LINE__ << ": Did not get " << fFBScanQAHistogramName
              << std::endl;
    Fatal("GetPointersForQAHistograms", "Invalid Pointer");
  }

  // get pointer track scan filterbit QA histograms
  for (int track = 0; track < LAST_ETRACK; ++track) {
    for (int fb = 0; fb < kNumberofTestFilterBit; ++fb) {
      fFBTrackScanQAHistograms[track][fb] =
          dynamic_cast<TH1D *>(fFBScanQAHistogramsList->FindObject(
              fFBTrackScanQAHistogramNames[track][fb][kNAME]));
      if (!fFBTrackScanQAHistograms[track][fb]) {
        std::cout << __LINE__ << ": Did not get "
                  << fFBTrackScanQAHistogramNames[track][fb][kNAME]
                  << std::endl;
        Fatal("GetPointersForQAHistograms", "Invalid Pointer");
      }
    }
  }

  // get pointer for fSelfCorQAHistogramsList
  fSelfCorQAHistogramsList = dynamic_cast<TList *>(
      fQAHistogramsList->FindObject(fSelfCorQAHistogramsListName));
  if (!fSelfCorQAHistogramsList) {
    std::cout << __LINE__ << ": Did not get " << fSelfCorQAHistogramsListName
              << std::endl;
    Fatal("GetPointersForQAHistograms", "Invalid Pointer");
  }
  // get pointers for self correlation QA histograms
  for (int var = 0; var < kKinematic; ++var) {
    for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
      fSelfCorQAHistograms[var][ba] =
          dynamic_cast<TH1D *>(fSelfCorQAHistogramsList->FindObject(
              fSelfCorQAHistogramNames[var][ba][kNAME]));
      if (!fSelfCorQAHistograms[var][ba]) {
        std::cout << __LINE__ << ": Did not get "
                  << fSelfCorQAHistogramNames[var][ba][kNAME] << std::endl;
        Fatal("GetPointersForQAHistograms", "Invalid Pointer");
      }
    }
  }
}

void AliAnalysisTaskAR::GetPointersForControlHistograms() {
  // get pointers for Control Histograms

  // get pointer for fControlHistograms
  fControlHistogramsList =
      dynamic_cast<TList *>(fHistList->FindObject(fControlHistogramsListName));
  if (!fControlHistogramsList) {
    std::cout << __LINE__ << ": Did not get " << fControlHistogramsListName
              << std::endl;
    Fatal("GetPointersForControlHistograms", "Invalid Pointer");
  }

  // get pointer for fTrackControlHistogramsList
  fTrackControlHistogramsList = dynamic_cast<TList *>(
      fControlHistogramsList->FindObject(fTrackControlHistogramsListName));
  if (!fTrackControlHistogramsList) {
    std::cout << __LINE__ << ": Did not get " << fTrackControlHistogramsListName
              << std::endl;
    Fatal("GetPointersForControlHistograms", "Invalid Pointer");
  }

  // get pointers for track cut counter histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    fTrackCutsCounter[mode] = dynamic_cast<TH1D *>(
        fTrackControlHistogramsList->FindObject(fTrackCutsCounterNames[mode]));
  }
  // get all pointers for track control histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    for (int var = 0; var < LAST_ETRACK; ++var) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        fTrackControlHistograms[mode][var][ba] =
            dynamic_cast<TH1D *>(fTrackControlHistogramsList->FindObject(
                fTrackControlHistogramNames[mode][var][ba][kNAME]));
        if (!fTrackControlHistograms[mode][var][ba]) {
          std::cout << __LINE__ << ": Did not get "
                    << fTrackControlHistogramNames[mode][var][ba][kNAME]
                    << std::endl;
          Fatal("GetPointersForControlHistograms", "Invalid Pointer");
        }
      }
    }
  }

  // get pointer for fEventControlHistogramsList
  fEventControlHistogramsList = dynamic_cast<TList *>(
      fControlHistogramsList->FindObject(fEventControlHistogramsListName));
  if (!fEventControlHistogramsList) {
    std::cout << __LINE__ << ": Did not get " << fEventControlHistogramsListName
              << std::endl;
    Fatal("GetPointersForControlHistograms", "Invalid Pointer");
  }

  // get pointers for event cut counter histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    fEventCutsCounter[mode] = dynamic_cast<TH1D *>(
        fEventControlHistogramsList->FindObject(fEventCutsCounterNames[mode]));
  }
  // get all pointers for event control histograms
  for (int mode = 0; mode < LAST_EMODE; ++mode) {
    for (int var = 0; var < LAST_EEVENT; ++var) {
      for (int ba = 0; ba < LAST_EBEFOREAFTER; ++ba) {
        fEventControlHistograms[mode][var][ba] =
            dynamic_cast<TH1D *>(fEventControlHistogramsList->FindObject(
                fEventControlHistogramNames[mode][var][ba][kNAME]));
        if (!fEventControlHistograms[mode][var][ba]) {
          std::cout << __LINE__ << ": Did not get "
                    << fEventControlHistogramNames[mode][var][ba][kNAME]
                    << std::endl;
          Fatal("GetPointersForControlHistograms", "Invalid Pointer");
        }
      }
    }
  }
}

void AliAnalysisTaskAR::GetPointersForFinalResultHistograms() {
  // Get pointers for final result Histograms

  // Get pointer for fFinalResultsList
  fFinalResultsList =
      dynamic_cast<TList *>(fHistList->FindObject(fFinalResultsListName));
  if (!fFinalResultsList) {
    std::cout << __LINE__ << ": Did not get " << fFinalResultsListName
              << std::endl;
    Fatal("GetPointersForOutputHistograms", "Invalid Pointer");
  }

  // get all pointers for final result histograms
  for (int var = 0; var < LAST_EFINALHIST; ++var) {
    fFinalResultHistograms[var] = dynamic_cast<TH1D *>(
        fFinalResultsList->FindObject(fFinalResultHistogramNames[var][kNAME]));
    if (!fFinalResultHistograms[var]) {
      std::cout << __LINE__ << ": Did not get "
                << fFinalResultHistogramNames[var][kNAME] << std::endl;
      Fatal("GetPointersForOutputHistograms", "Invalid Pointer");
    }
  }
}

void AliAnalysisTaskAR::GetPointersForFinalResultProfiles() {
  // Get pointers for final result Histograms

  // Get pointer for fFinalResultsList
  fFinalResultsList =
      dynamic_cast<TList *>(fHistList->FindObject(fFinalResultsListName));
  if (!fFinalResultsList) {
    std::cout << __LINE__ << ": Did not get " << fFinalResultsListName
              << std::endl;
    Fatal("GetPointersForOutputHistograms", "Invalid Pointer");
  }

  // get all pointers for final result histograms
  for (int var = 0; var < LAST_EFINALPROFILE; ++var) {
    fFinalResultProfiles[var] = dynamic_cast<TProfile *>(
        fFinalResultsList->FindObject(fFinalResultProfileNames[var][0]));
    if (!fFinalResultProfiles[var]) {
      std::cout << __LINE__ << ": Did not get "
                << fFinalResultProfileNames[var][0] << std::endl;
      Fatal("GetPointersForOutputProfiles", "Invalid Pointer");
    }
  }
}
