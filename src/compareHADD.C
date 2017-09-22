#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#include "TFile.h"
#include "TChain.h"

#include "include/doGlobalDebug.h"
#include "include/checkMakeDir.h"
#include "include/returnFileList.h"
#include "include/returnRootFileContentsList.h"

int compareHADD(const std::string unHaddFileDir, const std::string haddFileDir)
{
  if(!checkDir(unHaddFileDir)){
    std::cout << "unHaddFileDir \'" << unHaddFileDir << "\' is not a directory - return 1" << std::endl;
    return 1;
  }
  if(!checkDir(haddFileDir)){
    std::cout << "haddFileDir \'" << haddFileDir << "\' is not a directory - return 1" << std::endl;
    return 1;
  }
  
  std::vector<std::string> unHaddFiles = returnFileList(unHaddFileDir, ".root");
  std::vector<std::string> haddFiles = returnFileList(haddFileDir, ".root");

  unsigned int pos = 0; 
  while(unHaddFiles.size() > pos){
    if(unHaddFiles.at(pos).find("/failed/") != std::string::npos) unHaddFiles.erase(unHaddFiles.begin()+pos);
    else ++pos;
  }

  pos = 0; 
  while(haddFiles.size() > pos){
    if(haddFiles.at(pos).find("/failed/") != std::string::npos) haddFiles.erase(haddFiles.begin()+pos);
    else ++pos;
  }

  if(unHaddFiles.size() == 0){
    std::cout << "unHaddFileDir \'" << unHaddFileDir << "\' returns no .root files, return 1" << std::endl;
    return 1;
  }

  if(haddFiles.size() == 0){
    std::cout << "haddFileDir \'" << haddFileDir << "\' returns no .root files, return 1" << std::endl;
    return 1;
  }

  TFile* tempUnHaddFile_p = new TFile(unHaddFiles.at(0).c_str(), "READ");
  TFile* tempHaddFile_p = new TFile(haddFiles.at(0).c_str(), "READ");
  std::vector<std::string> ttreeNamesUnHadd = returnRootFileContentsList(tempUnHaddFile_p, "TTree");
  std::vector<std::string> ttreeNamesHadd = returnRootFileContentsList(tempHaddFile_p, "TTree");


  std::vector< std::vector<std::string> > ttreeListOfBranches;

  if(ttreeNamesUnHadd.size() != ttreeNamesHadd.size()){
    std::cout << "Mismatch in ttree number between file sets from \'" << unHaddFileDir << "\' and \'" << haddFileDir << "\'.return 1" << std::endl;
    return 1;
  }

  if(doGlobalDebug) std::cout << __FILE__ << ", " << __LINE__ << std::endl;

  for(unsigned int i = 0; i < ttreeNamesUnHadd.size(); ++i){
    bool nameShared = false;
    for(unsigned int j = 0; j < ttreeNamesHadd.size(); ++j){
      if(ttreeNamesUnHadd.at(i).size() == ttreeNamesHadd.at(j).size() && ttreeNamesUnHadd.at(i).find(ttreeNamesHadd.at(j)) != std::string::npos){
	nameShared = true;
	break;
      }
    }

    if(!nameShared){
      std::cout << "Note! - TTree \'" << ttreeNamesUnHadd.at(i) << "\' is not found in haddFileDir \'" << haddFileDir << "\'. return 1" << std::endl;
      return 1;
    }
  }

  for(unsigned int i = 0; i < ttreeNamesUnHadd.size(); ++i){
    TTree* temp_p = (TTree*)tempUnHaddFile_p->Get(ttreeNamesUnHadd.at(i).c_str());
    std::vector<std::string> tempVect;
    TObjArray* branchList = (TObjArray*)temp_p->GetListOfBranches();

    for(int j = 0; j < branchList->GetEntries(); ++j){
      tempVect.push_back(std::string(branchList->At(j)->GetName()));
    }

    ttreeListOfBranches.push_back(tempVect);
  }


  tempUnHaddFile_p->Close();
  delete tempUnHaddFile_p;

  tempHaddFile_p->Close();
  delete tempHaddFile_p;

  const unsigned int nTTree = ttreeNamesUnHadd.size();

  if(doGlobalDebug) std::cout << __FILE__ << ", " << __LINE__ << std::endl;

  unsigned long long unHaddTTreeEntryCount[nTTree];
  unsigned long long haddTTreeEntryCount[nTTree];

  for(unsigned int i = 0; i < nTTree; ++i){
    unHaddTTreeEntryCount[i] = 0;
    haddTTreeEntryCount[i] = 0;
  }

  std::cout << "Processing unhadd files..." << std::endl;
  for(unsigned int i = 0; i < unHaddFiles.size(); ++i){
    tempUnHaddFile_p = new TFile(unHaddFiles.at(i).c_str(), "READ");

    std::cout << " " << i << "/" << unHaddFiles.size() << std::endl;

    for(unsigned int j = 0; j < nTTree; ++j){
      TTree* temp_p = (TTree*)tempUnHaddFile_p->Get(ttreeNamesUnHadd.at(j).c_str());
      unHaddTTreeEntryCount[j] += temp_p->GetEntries();

      TObjArray* branchList = (TObjArray*)temp_p->GetListOfBranches();
      if(branchList->GetEntries() != (int)ttreeListOfBranches.at(j).size()){
	std::cout << "BranchList size is not equal, return 1" << std::endl;
	return 1;
      }


      if(doGlobalDebug) std::cout << __FILE__ << ", " << __LINE__ << ", " << ttreeNamesUnHadd.at(j) << ", " << j << "/" << nTTree << std::endl;
      
      for(int k = 0; k < branchList->GetEntries(); ++k){
	bool isFound = false;
	std::string tempStr = std::string(branchList->At(k)->GetName());

	if(doGlobalDebug) std::cout << __FILE__ << ", " << __LINE__ << ", " << tempStr << std::endl;

	for(unsigned int l = 0; l < ttreeListOfBranches.at(j).size(); ++l){

	  if(ttreeListOfBranches.at(j).at(l).size() == tempStr.size() && tempStr.find(ttreeListOfBranches.at(j).at(l)) != std::string::npos){
	    isFound = true;
	    break;
	  }
	}

	if(!isFound){
	  std::cout << "Not all branches found, return 1" << std::endl;
	  return 1;
	}
      }

    }
    
    tempUnHaddFile_p->Close();
    delete tempUnHaddFile_p;
  }
  std::cout << "Completed processing unhadd files." << std::endl;

  if(doGlobalDebug) std::cout << __FILE__ << ", " << __LINE__ << std::endl;

  std::cout << "Processing hadd files..." << std::endl;
  for(unsigned int i = 0; i < haddFiles.size(); ++i){
    tempHaddFile_p = new TFile(haddFiles.at(i).c_str(), "READ");

    std::cout << " " << i << "/" << haddFiles.size() << std::endl;

    for(unsigned int j = 0; j < nTTree; ++j){
      TTree* temp_p = (TTree*)tempHaddFile_p->Get(ttreeNamesHadd.at(j).c_str());
      haddTTreeEntryCount[j] += temp_p->GetEntries();

      TObjArray* branchList = (TObjArray*)temp_p->GetListOfBranches();
      if(branchList->GetEntries() != (int)ttreeListOfBranches.at(j).size()){
	std::cout << "BranchList size is not equal, return 1" << std::endl;
	return 1;
      }
      
      for(int k = 0; k < branchList->GetEntries(); ++k){
	bool isFound = false;
	std::string tempStr = std::string(branchList->At(k)->GetName());

	for(unsigned int l = 0; l < ttreeListOfBranches.at(j).size(); ++l){
	  if(ttreeListOfBranches.at(j).at(l).size() == tempStr.size() && tempStr.find(ttreeListOfBranches.at(j).at(l)) != std::string::npos){
	    isFound = true;
	    break;
	  }
	}

	if(!isFound){
	  std::cout << "Not all branches found, return 1" << std::endl;
	  return 1;
	}
      }
    }
    
    tempHaddFile_p->Close();
    delete tempHaddFile_p;
  }
  std::cout << "Completed processing hadd files." << std::endl;

  if(doGlobalDebug) std::cout << __FILE__ << ", " << __LINE__ << std::endl;

  std::cout << "Comparing ttree entries..." << std::endl;
  bool goodEntries = true;
  for(unsigned int j = 0; j < nTTree; ++j){
    std::cout << ttreeNamesUnHadd.at(j) << ": " << unHaddTTreeEntryCount[j] << ", " << haddTTreeEntryCount[j] << std::endl;
    if(unHaddTTreeEntryCount[j] - haddTTreeEntryCount[j] != 0) goodEntries = false;
  }
  if(goodEntries) std::cout << "TTree Entries GOOD" << std::endl;
  else std::cout << "TTree Entries FAILED" << std::endl;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ./compareHADD.exe <unHaddFileDir> <haddFileDir>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += compareHADD(argv[1], argv[2]);
  return retVal;
}
