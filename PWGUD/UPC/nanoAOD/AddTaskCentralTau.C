AliAnalysisTaskCentralTau *AddTaskCentralTau(Float_t cutEta = 0.9){
  
  //--- get the current analysis manager ---//
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
      Error("AddTaskCentralTau", "No analysis manager found.");
      return 0;
   }
  
  // Check the analysis type using the event handlers connected to the analysis manager.
  //==============================================================================
  if (!mgr->GetInputEventHandler()) {
    Error("AddTaskCentralTau", "This task requires an input event handler.");
    return 0;
  }
	
  TString inputDataType = mgr->GetInputEventHandler()->GetDataType(); // can be "ESD" or "AOD"
  Bool_t isESD = kFALSE;
  if(inputDataType.Contains("ESD"))isESD = kTRUE;
  if (!isESD) {
      Error("AddTaskCentralTau", "Dataset type is not ESD.");
      return 0;
   }
  
  // Create tasks
  AliAnalysisTaskCentralTau *task = new AliAnalysisTaskCentralTau(inputDataType.Data());
  task->SetParameters(cutEta);
  mgr->AddTask(task);


   // Create containers for input/output
  AliAnalysisDataContainer *cinput = mgr->GetCommonInputContainer();
  AliAnalysisDataContainer *coutput = mgr->CreateContainer("ListHist", TList::Class(), AliAnalysisManager::kOutputContainer, Form("%s:Upc", AliAnalysisManager::GetCommonFileName()));  

  // Connect input/output
  mgr->ConnectInput(task, 0, cinput);
  mgr->ConnectOutput(task, 1, coutput);

return task;
}
