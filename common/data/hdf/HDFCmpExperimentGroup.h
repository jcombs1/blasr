#ifndef DATA_HDF_HDF_CMP_EXPERIMENT_GROUP_H_
#define DATA_HDF_HDF_CMP_EXPERIMENT_GROUP_H_

#include "data/hdf/HDFGroup.h"
#include "data/hdf/HDFArray.h"
#include "data/hdf/HDFAtom.h"
#include "data/hdf/HDFCmpSupportedFields.h"
#include "Types.h"
#include <map>

class HDFCmpExperimentGroup {
 public:
  
  bool Create(HDFGroup &parent, string experimentGroupName) {
    parent.AddGroup(experimentGroupName);
    if (experimentGroup.Initialize(parent.group, experimentGroupName) == 0) { return 0; }
    masterDatasetAtom.Create(experimentGroup.group, "MasterDataset");    
    masterDatasetAtom.Write("ID");
    alignmentArray.Create(experimentGroup, "AlnArray");
    return true;
  }
  

  void AddAlignment(vector<unsigned char> &alignment, unsigned int &offsetBegin, unsigned int &offsetEnd) {
    int curNRows = alignmentArray.size();
    offsetBegin = offsetEnd = 0;
    if (alignment.size() == 0 ) {
      // 
      // Do not increment anything.
      //
      return;
    }
    offsetBegin = alignmentArray.size();
    offsetEnd   = offsetBegin + alignment.size();
    alignmentArray.Write(&alignment[0], alignment.size());
  }

  
  int Initialize(HDFGroup &refGroup, string experimentGroupName, set<string> &fieldNames) {
    //
    // Normal initialization that prepares for reading alignments
    //
    Initialize(refGroup, experimentGroupName);

    //
    // Field initialization for reading in pulse/quality information.
    //
    set<string>::iterator fieldNameIt, fieldEnd;
    fieldEnd = fieldNames.end();
    for (fieldNameIt = fieldNames.begin(); fieldNameIt != fieldEnd; ++fieldNameIt) {
      if (supportedFields.find(*fieldNameIt) != supportedFields.end() and
          experimentGroup.ContainsObject(*fieldNameIt)) {
        fields[*fieldNameIt]->Initialize(experimentGroup, *fieldNameIt);
      }
      else {
        cout << "Unable to initialize requested field " << *fieldNameIt << " in experiment group " << experimentGroupName << endl;
      }
    }
    return 1;
  }

  int Initialize(HDFGroup &refGroup, string experimentGroupName) {
    
    if (experimentGroup.Initialize(refGroup.group, experimentGroupName) == 0) { return 0; }
    
    if (experimentGroup.ContainsAttribute("MasterDataset")) {
      if (masterDatasetAtom.Initialize(experimentGroup.group, "MasterDataset") == 0) {
        return 0; 
      }
    }
    if (experimentGroup.ContainsAttribute("nrow")) {
      nRowAtom.Initialize(experimentGroup.group, "nRow");
    }
    if (alignmentArray.Initialize(experimentGroup, "AlnArray") == 0) { return 0; }
    return 1;
  }

  HDFCmpSupportedFields supportedFields;

    HDFArray<unsigned int> startTimeOffset;
    HDFArray<HalfWord> ipd;
    HDFArray<HalfWord> preBaseFrames;
    HDFArray<char>  deletionTag;
    HDFArray<char>  substitutionTag;
    HDFArray<UChar> qualityValue;
    HDFArray<UChar> deletionQV;
    HDFArray<UChar> insertionQV;
    HDFArray<UChar> classifierQV;
    HDFArray<UChar> substitutionQV;
    HDFArray<UChar> mergeQV;
    HDFArray<HalfWord> light;
    HDFArray<HalfWord> widthInFrames;
    HDFArray<HalfWord> pulseWidth;
    HDFArray<unsigned int> startTime;
    HDFArray<float> pkmid;
    HDFArray<float> pkmax;
    HDFArray<float> pkmean;
  HDFArray<uint32_t>   numPasses;
  HDFArray<uint32_t> pulseIndex;

  void Read();
  HDFAtom<string> masterDatasetAtom;
  HDFAtom<UInt>   nRowAtom;
  HDFGroup experimentGroup;
  HDFArray<unsigned char> alignmentArray;
  map<string, HDFData*> fields;
  HDFAtom<int>    alignmentArrayLastRow;
  
  HDFCmpExperimentGroup() {
    fields["StartTimeOffset"] = &this->startTimeOffset;
    fields["QualityValue"] = &this->qualityValue;
    fields["IPD"] = &this->ipd;
    fields["PreBaseFrames"] = &this->preBaseFrames;
    fields["DeletionQV"] = &this->deletionQV;
    fields["InsertionQV"] = &this->insertionQV;
    fields["ClassifierQV"] = &this->classifierQV;
    fields["SubstitutionQV"] = &this->substitutionQV;
    fields["Light"] = &this->light;
    fields["WidthInFrames"] = &this->widthInFrames;
    fields["PulseWidth"] = &this->pulseWidth;
    fields["StartTime"] = &this->startTime;
    fields["pkmid"] = &this->pkmid;
    fields["pkmax"] = &this->pkmax;
    fields["pkmean"] = &this->pkmean;
    fields["DeletionTag"] = &this->deletionTag;
    fields["SubstitutionTag"] = &this->substitutionTag;
    fields["PulseIndex"] = &this->pulseIndex;
    fields["MergeQV"] = &this->mergeQV;
  }
};
#endif
