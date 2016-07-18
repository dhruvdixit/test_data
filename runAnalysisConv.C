#include <ctime>
#include "TGrid.h"

AliAnalysisGrid* CreateAlienHandler(const char* uniqueName, 
				    const char* gridDir, 
				    const char* gridMode, 
				    const char* runNumbers, 
				    const char* pattern, 
				    TString additionalCode, 
				    TString additionalHeaders, 
				    Int_t maxFilesPerWorker, 
				    Int_t workerTTL, 
				    Bool_t isMC);
                                    
//______________________________________________________________________________
void runAnalysisConv(
         const char*    dataType            = "ESD",                       // set the analysis type, AOD, ESD or sESD
         Bool_t         useGrid             = kFALSE,                      // local or grid
         const char*    gridMode            = "test",                      // set the grid run mode (can be "full", "test", "offline", "submit" or "terminate")
         const char*    pattern             = "*/AliESDs.root", 		   // file pattern (here one can specify subdirs like passX etc.) (used on grid)
         const char*    gridDir             = "/alice/data/2013/LHC13g/", // dir on alien, where the files live (used on grid)
         const char*    runNumbers          = "197583 197669",             // considered run numbers (used on grid)
         UInt_t         numLocalFiles       = 20,                          // number of files analyzed locally  
         const char*    runPeriod           = "LHC15n",                    // set the run period (used on grid)
         const char*    uniqueName          = "GammaCalo",     // sets base string for the name of the task on the grid
         Bool_t         isMC                = kFALSE,                      // trigger, if MC handler should be used

         // These two settings depend on the dataset and your quotas on the AliEN services
         Int_t          maxFilesPerWorker   = 4,
         Int_t          workerTTL           = 7200

         )
{

  // Some pre-settings and constants

  gSystem->SetFPEMask();
  gSystem->Setenv("ETRAIN_ROOT", ".");
  gSystem->Setenv("ETRAIN_PERIOD", runPeriod);
  // change this objects to strings
  TString usedData(dataType);
  cout << dataType << " analysis chosen" << endl;
 
  // Load necessary libraries
  LoadLibs();

  // Create analysis manager
  AliAnalysisManager* mgr = new AliAnalysisManager(uniqueName);

  // Check type of input and create handler for it
  TString localFiles("-1");
  if(usedData == "AOD")
  {
    localFiles = "files_aod.txt";
    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/train/AddAODHandler.C");
    AliAODInputHandler* aodH = AddAODHandler();
  }
  else if((usedData == "ESD"))
  {
    if (usedData == "ESD")
      localFiles = "/home/dhruv/test_data/test_15n_244340.txt";
    
    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/train/AddESDHandler.C");
    AliESDInputHandler* esdH = AddESDHandler();
  }
  else
  {
    cout << "Data type not recognized! You have to specify ESD, AOD, or sESD!\n";
  }
  
  if(!useGrid)
    cout << "Using " << localFiles.Data() << " as input file list.\n";

  // Create MC handler, if MC is demanded
  if (isMC && (usedData != "AOD"))
  {
    AliMCEventHandler* mcH = new AliMCEventHandler();
    mcH->SetPreReadMode(AliMCEventHandler::kLmPreRead);
    mcH->SetReadTR(kTRUE);
    mgr->SetMCtruthEventHandler(mcH); 
  }

  gROOT->LoadMacro("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C");
  AliPhysicsSelectionTask* physSelTask = AddTaskPhysicsSelection(isMC);
  
  //========= Add PID Reponse to ANALYSIS manager ====
  if(!(AliPIDResponse*)mgr->GetTask("PIDResponseTask")){
    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
    AddTaskPIDResponse(isMC,kFALSE,kTRUE);
    // 		AddTaskPIDResponse(isMC);
  }//========= Add PID Reponse to ANALYSIS manager ====
  
  gROOT->LoadMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/AddTaskEmcalSetup.C");
  AliEmcalSetupTask *setupTask = AddTaskEmcalSetup(0,0,"uselocal",0,1);
  
  //	gROOT->LoadMacro("$ALICE_ROOT/PWG/EMCAL/macros/AddTaskEmcalTriggerMaker.C");
  //	AliEmcalTriggerMaker *emcalTriggers = AddTaskEmcalTriggerMaker("EmcalTriggers");
  
  gROOT->LoadMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/AddTaskEMCALTender.C");
  AliAnalysisTask *emcalTender = AddTaskEMCALTender( 	kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, AliEMCALRecoUtils::kNoCorrection, kTRUE, 0.5, 0.1, AliEMCALRecParam::kClusterizerv2, kFALSE, kFALSE, -500e-8, 500e-8, 1e6   );//inital
  //AliAnalysisTask *emcalTender = AddTaskEMCALTender(kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, kFALSE, kFALSE, kFALSE, kFALSE, kFALSE, AliEMCALRecoUtils::kNoCorrection, kTRUE, 0.5, 0.1, AliEMCALRecParam::kClusterizerv2, kFALSE, kFALSE, -500e-8, 500e-8, 1e6   );//First Change
  //AliAnalysisTask *emcalTender = AddTaskEMCALTender(kTRUE, kTRUE, kTRUE, kTRUE, kTRUE, kFALSE, kFALSE, kFALSE, kFALSE, kTRUE, AliEMCALRecoUtils::kNoCorrection, kTRUE, 0.5, 0.1, AliEMCALRecParam::kClusterizerv2, kFALSE, kFALSE, -500e-8, 500e-8, 1e6   );//Second Change
  
// 	gROOT->LoadMacro("$ALICE_PHYSICS/PWGGA/GammaConv/macros/AddTask_GammaConvV1_pp.C");
// 	AliAnalysisTask *taskF = AddTask_GammaConvV1_pp(107,0,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC13g");
	
	gROOT->LoadMacro("$ALICE_PHYSICS/PWGGA/GammaConv/macros/AddTask_GammaCalo_pp.C");
// 	AliAnalysisTask *taskD = AddTask_GammaConvCalo_pp(4, 0, 1, 1, "MCSpectraInput.root", "000000006008400001001500000", 0, "LHC13g", kFALSE, kFALSE, kTRUE, kFALSE, kFALSE, 3., "LHC13g");
//	AliAnalysisTask *taskF = AddTask_GammaCalo_pp(60, 0, 1, 1 );                                
 	//AliAnalysisTask *taskF = AddTask_GammaCalo_pp(201, 0, 1, 1, "MCSpectraInput.root", "000000006008400001001500000", "LHC15n", kFALSE, kFALSE, 2, kFALSE, kFALSE );

	AliAnalysisTask *taskF = AddTask_GammaCalo_pp(201, 0, 1, 1, "MCSpectraInput.root", "000000006008400001001500000", "LHC15n", kFALSE, kFALSE, 3, kFALSE, kFALSE );

// 	gROOT->LoadMacro("$ALICE_PHYSICS/PWGGA/GammaConv/macros/AddTask_GammaCalo_pp.C");
// 	AliAnalysisTask *taskE = AddTask_GammaCalo_pp(41, 0, 1, 1, "MCSpectraInput.root", "000000006008400001001500000", "LHC13g", kFALSE, kFALSE, 0, kFALSE, kFALSE );//

//   gROOT->LoadMacro("$ALICE_PHYSICS/PWGGA/GammaConv/macros/AddTask_GammaCaloMerged_pp.C");
//   AliAnalysisTask *taskE = AddTask_GammaCaloMerged_pp(15, 0, 1, 1);
//   AliAnalysisTask *taskF = AddTask_GammaCaloMerged_pp(16, 0, 1, 1);
  
//    	gROOT->LoadMacro("$ALICE_PHYSICS/PWGGA/GammaConv/macros/AddTask_GammaConvCalo_pp.C");
//     AliAnalysisTask *taskA = AddTask_GammaConvCalo_pp(41, 0, 1, 1, "MCSpectraInput.root", "000000006008400001001500000", 2, "LHC13g", kFALSE,	kFALSE,  kFALSE, kFALSE, kTRUE);
//   AliAnalysisTask *taskB = AddTask_GammaConvCalo_pp(40, 0, 1, 1, "MCSpectraInput.root", "000000006008400001001500000", 2, "LHC13g", kFALSE, kFALSE,  kFALSE, kFALSE, kTRUE);
//   AliAnalysisTask *taskC = AddTask_GammaConvCalo_pp(41, 0, 1, 1, "MCSpectraInput.root", "000000006008400001001500000", 2, "LHC13g", kFALSE, kFALSE, kFALSE, kFALSE, kTRUE);
//   AliAnalysisTask *taskD = AddTask_GammaConvCalo_pp(153, 0, 1, 1, "MCSpectraInput.root", "000000006008400001001500000", 0, "LHC13g", kFALSE, kFALSE, kFALSE, kFALSE, kTRUE);
	
	// 	AliAnalysisTask *taskI = AddTask_GammaCalo_pp(9,kTRUE,1,1 );
// 	AliAnalysisTask *taskH = AddTask_GammaCalo_pp(5,kTRUE,1,1 );
// 	AliAnalysisTask *taskF = AddTask_GammaCalo_pp(6,kTRUE,1,1 );
// 	AliAnalysisTask *taskG = AddTask_GammaCalo_pp(7,kTRUE,1,1 );

// 	gROOT->LoadMacro("$ALICE_PHYSICS/PWGGA/EMCALTasks/macros/AddTaskEMCALMesonGGSDM.C");
// 	AliAnalysisTask *taskE = AddTaskEMCALMesonGGSDM(0);
	
	
// 	AliAnalysisTask *taskF = AddTask_GammaConvV1_pp(48,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskF = AddTask_GammaConvV1_pp(49,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskF = AddTask_GammaConvV1_pp(50,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskF = AddTask_GammaConvV1_pp(51,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskG = AddTask_GammaConvV1_pp(31,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskA = AddTask_GammaConvV1_pp(33,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskB = AddTask_GammaConvV1_pp(35,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskC = AddTask_GammaConvV1_pp(37,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskD = AddTask_GammaConvV1_pp(39,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
// 	AliAnalysisTask *taskE = AddTask_GammaConvV1_pp(41,kTRUE,1,1,"MCSpectraInput.root","0000000060084001001500000","LHC14e2b");
	
// 	gROOT->LoadMacro("$ALICE_PHYSICS/PWGGA/GammaConv/macros/AddTask_GammaConvNeutralMesonPiPlPiMiPiZero_ConvMode_pp.C");
// 	AliAnalysisTask *taskA = AddTask_GammaConvNeutralMesonPiPlPiMiPiZero_ConvMode_pp(1);


	
	mgr->SetUseProgressBar(1, 100);
        
	if (!mgr->InitAnalysis()) 
		return;
	mgr->PrintStatus();

	if (useGrid) 
	{  // GRID CALCULATION

		AliAnalysisGrid *plugin = CreateAlienHandler(uniqueName, gridDir, gridMode, runNumbers, pattern, maxFilesPerWorker, workerTTL, isMC);
		mgr->SetGridHandler(plugin);

		// start analysis
		cout << "Starting GRID Analysis...";
		mgr->SetDebugLevel(0);
		mgr->StartAnalysis("grid");
	}
	else
	{  // LOCAL CALCULATION

		TChain* chain = 0;
		if (usedData == "AOD") 
		{
		gROOT->LoadMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/CreateAODChain.C");
		chain = CreateAODChain(localFiles.Data(), numLocalFiles);
		}
		else
		{  // ESD or skimmed ESD
		gROOT->LoadMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/CreateESDChain.C");
		chain = CreateESDChain(localFiles.Data(), numLocalFiles);
		}
		
		// start analysis
		cout << "Starting LOCAL Analysis...";
		mgr->SetDebugLevel(2);
		mgr->StartAnalysis("local", chain);
	}
}

//______________________________________________________________________________
void LoadLibs()
{

  // Load common libraries (better too many than too few)
	gSystem->Load("libCore.so");  
	gSystem->Load("libTree.so");
	gSystem->Load("libGeom.so");
	gSystem->Load("libVMC.so");
	gSystem->Load("libGui.so");
	gSystem->Load("libPhysics.so");
	gSystem->Load("libMinuit");
	gSystem->Load("libMinuit2");
	gSystem->Load("libSTEERBase.so");
	gSystem->Load("libESD");
	gSystem->Load("libAOD");
	gSystem->Load("libOADB");
	gSystem->Load("libANALYSIS");
	gSystem->Load("libANALYSISalice");  
	gSystem->Load("libPWGGAGammaConv.so");
	gSystem->Load("libEve.so");   
	gSystem->Load("libCDB.so");
	gSystem->Load("libProof.so");
	gSystem->Load("libRAWDatabase.so");
	gSystem->Load("libSTEER.so");
	gSystem->Load("libEVGEN.so");
	gSystem->Load("libESDfilter.so");	
	gSystem->Load("libTender.so");
	gSystem->Load("libTOFbase.so");
	gSystem->Load("libTOFsim.so");
	gSystem->Load("libTOFrec.so");
	gSystem->Load("libTRDbase.so");
	gSystem->Load("libVZERObase.so");
	gSystem->Load("libVZEROrec.so");
	gSystem->Load("libTenderSupplies.so");
	gSystem->Load("libXMLParser.so");
	gSystem->Load("libSTAT.so");
	gSystem->Load("libRAWDatarec.so");
	gSystem->Load("libANALYSIScalib.so");
	gSystem->Load("libCORRFW.so");
	gSystem->Load("libPWGUDbase.so");
	gSystem->Load("libTPCbase.so");
	gSystem->Load("libTPCrec.so");
	gSystem->Load("libTPCcalib.so");
	gSystem->Load("libTRDrec.so");
	gSystem->Load("libITSbase.so");
	gSystem->Load("libITSrec.so");
	gSystem->Load("libHMPIDbase.so");
	gSystem->Load("libPWGmuon.so");
	gSystem->Load("libPWGPP.so");
	gSystem->Load("libPWGHFbase.so");
	gSystem->Load("libPWGDQdielectron.so");
	gSystem->Load("libPWGHFhfe.so");
	gSystem->Load("libPHOSUtils.so");
	gSystem->Load("libPHOSbase.so");
	gSystem->Load("libPHOSpi0Calib.so");
	gSystem->Load("libPHOSrec.so");
	gSystem->Load("libPWGGAPHOSTasks.so");
	gSystem->Load("libPHOSshuttle.so");
	gSystem->Load("libPHOSsim.so");
	gSystem->Load("libPWGCaloTrackCorrBase.so");
	gSystem->Load("libEMCALUtils.so");
	gSystem->Load("libEMCALraw.so");
	gSystem->Load("libEMCALbase.so");
	gSystem->Load("libEMCALrec.so");
	gSystem->Load("libPWGTools.so");
	gSystem->Load("libPWGEMCAL.so");
	gSystem->Load("libPWGGAUtils.so");
	gSystem->Load("libPWGGAEMCALTasks.so");
	gSystem->Load("libPWGGAGammaConv.so");
	gSystem->Load("libPWGGAPHOSTasks.so");
	gSystem->Load("libPWGCFCorrelationsBase.so");
	gSystem->Load("libPWGCFCorrelationsDPhi.so");
  
}

AliAnalysisGrid* CreateAlienHandler(const char* uniqueName, const char* gridDir, const char* gridMode, const char* runNumbers, 
                                     const char* pattern, Int_t maxFilesPerWorker, 
                                     Int_t workerTTL, Bool_t isMC)
{
	TDatime currentTime;
	TString tmpName(uniqueName);

	// Only add current date and time when not in terminate mode! In this case the exact name has to be supplied by the user
	if(strcmp(gridMode, "terminate"))
	{
		tmpName += "_";
		tmpName += currentTime.GetDate();
		tmpName += "_";
		tmpName += currentTime.GetTime();
	}

	TString tmpAdditionalLibs("");
	tmpAdditionalLibs = "libTree.so libVMC.so libGui.so libXMLParser.so libMinuit.so libMinuit2.so libProof.so libGeom.so libPhysics.so libSTEERBase.so libESD.so libAOD.so libOADB.so libANALYSIS.so libANALYSISalice.so libCDB.so libRAWDatabase.so libSTEER.so libEVGEN.so libESDfilter.so libCORRFW.so libTOFbase.so libTOFsim.so libTOFrec.so libRAWDatarec.so libTPCbase.so libTPCrec.so libTPCcalib.so libITSbase.so libITSrec.so libTRDbase.so libTRDrec.so libSTAT.so libHMPIDbase.so libVZERObase.so libVZEROrec.so libTENDER.so libTENDERSupplies.so libPWGmuon.so libPWGPP.so libPWGHFbase.so libPWGDQdielectron.so libPWGHFhfe.so libPHOSUtils.so libPHOSbase.so libPHOSpi0Calib.so libPHOSrec.so libPWGGAPHOSTasks.so libPHOSshuttle.so libPHOSsim.so libPWGCaloTrackCorrBase.so libEMCALUtils.so libEMCALraw.so libEMCALbase.so libEMCALrec.so libPWGTools.so libPWGEMCAL.so libPWGGAUtils.so libPWGGAEMCALTasks.so libPWGGAPHOSTasks.so libPWGCFCorrelationsBase.so libPWGCFCorrelationsDPhi.so libPWGGAGammaConv.so";


	TString macroName("");
	TString execName("");
	TString jdlName("");
	macroName = Form("%s.C", tmpName.Data());
	execName = Form("%s.sh", tmpName.Data());
	jdlName = Form("%s.jdl", tmpName.Data());

	AliAnalysisAlien *plugin = new AliAnalysisAlien();
	plugin->SetOverwriteMode();
	plugin->SetRunMode(gridMode);
		
	// Here you can set the (Ali)ROOT version you want to use
	plugin->SetAPIVersion("V1.1x");
	plugin->SetROOTVersion("v5-34-08-4");
	plugin->SetAliROOTVersion("v5-05-63-AN");

	plugin->SetGridDataDir(gridDir); // e.g. "/alice/sim/LHC10a6"
	plugin->SetDataPattern(pattern); //dir structure in run directory
	if (!isMC)
	plugin->SetRunPrefix("000");

	plugin->AddRunList(runNumbers);

	plugin->SetGridWorkingDir(Form("work/%s",tmpName.Data()));
	plugin->SetGridOutputDir("output2"); // In this case will be $HOME/work/output

	plugin->SetAdditionalLibs(tmpAdditionalLibs.Data());
	plugin->AddExternalPackage("boost::v1_43_0");
	plugin->AddExternalPackage("cgal::v3.6");
	plugin->AddExternalPackage("fastjet::v2.4.2");

	plugin->SetDefaultOutputs(kTRUE);
	//plugin->SetMergeExcludes("");
	plugin->SetAnalysisMacro(macroName.Data());
	plugin->SetSplitMaxInputFileNumber(maxFilesPerWorker);
	plugin->SetExecutable(execName.Data());
	plugin->SetTTL(workerTTL);
	plugin->SetInputFormat("xml-single");
	plugin->SetJDLName(jdlName.Data());
	plugin->SetPrice(1);      
	plugin->SetSplitMode("se");

	return plugin;
}
