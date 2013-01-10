#ifndef DATA_HDF_HDF_CMP_FILE_H_
#define DATA_HDF_HDF_CMP_FILE_H_

#include "H5Cpp.h"
#include <iostream>
#include <assert.h>
#include "datastructures/alignment/CmpFile.h"
#include "datastructures/alignment/Alignment.h"
#include "datastructures/alignment/AlignmentCandidate.h"
#include "datastructures/alignment/CmpAlignment.h"
#include "datastructures/alignment/CmpReadGroupTable.h"
#include "datastructures/alignment/CmpRefSeqTable.h"
#include "datastructures/alignment/ByteAlignment.h"
#include "datastructures/saf/RefInfo.h"
#include "data/hdf/HDFAtom.h"
#include "data/hdf/HDFArray.h"
#include "data/hdf/HDF2DArray.h"
#include "data/hdf/HDFCmpData.h"
#include "data/hdf/HDFCmpRefAlignmentGroup.h"
#include "data/hdf/HDFCmpExperimentGroup.h"
#include "HDFAlnGroupGroup.h"
#include "HDFAlnInfoGroup.h"
#include "HDFRefGroupGroup.h"
#include "HDFRefInfoGroup.h"
#include "HDFMovieInfoGroup.h"
#include "HDFCmpRootGroup.h"
#include "HDFCmpSupportedFields.h"
#include "HDFFileLogGroup.h"
#include "SMRTSequence.h"
#include <sstream>
#include <map>

using namespace H5;
using namespace std;

template<typename T_Alignment> 
class HDFCmpFile : public HDFCmpData {
public:
  map<int,int>  movieNameIdToArrayIndex,  readGroupPathIdToArrayIndex, refGroupIdToArrayIndex;
  map<string, int> refNameToArrayIndex;
  map<string,string> readGroupPathToReadGroup;

  HDFAlnGroupGroup alnGroupGroup;
  HDFAlnInfoGroup alnInfoGroup;
  HDFMovieInfoGroup movieInfoGroup;
  HDFRefGroupGroup refGroupGroup;
  HDFRefInfoGroup refInfoGroup;
  HDFCmpRootGroup<T_Alignment > rootGroup;
  set<string> includedFields;
  HDFCmpSupportedFields supportedFields;
  HDFAtom<string> readTypeAtom;
  HDFFileLogGroup fileLogGroup;


  void AstroInitializeColumnNameMap() {
    CmpAlignmentBase::columnNameToIndex["AlignmentId"] = 0;
    CmpAlignmentBase::columnNameToIndex["ReadGroupId"] = 1;
    CmpAlignmentBase::columnNameToIndex["MovieId"] = 2;
    CmpAlignmentBase::columnNameToIndex["RefSeqId"] = 3;    
    CmpAlignmentBase::columnNameToIndex["tStart"] = 4;
    CmpAlignmentBase::columnNameToIndex["tEnd"] = 5;
    CmpAlignmentBase::columnNameToIndex["AlignedStrand"] = 6;
    CmpAlignmentBase::columnNameToIndex["ExpId"] = 7;
    CmpAlignmentBase::columnNameToIndex["RunId"] = 8;     
    CmpAlignmentBase::columnNameToIndex["Panel"] = 9;
    CmpAlignmentBase::columnNameToIndex["x"] = 10;
    CmpAlignmentBase::columnNameToIndex["y"] = 11;
    CmpAlignmentBase::columnNameToIndex["SubreadId"] = 12;
    CmpAlignmentBase::columnNameToIndex["rStart"] = 13;     
    CmpAlignmentBase::columnNameToIndex["rEnd"] = 14;
    CmpAlignmentBase::columnNameToIndex["Z"] = 15;
    CmpAlignmentBase::columnNameToIndex["nM"] = 16;
    CmpAlignmentBase::columnNameToIndex["nMM"] = 17;
    CmpAlignmentBase::columnNameToIndex["nIns"] = 18;     
    CmpAlignmentBase::columnNameToIndex["nDel"] = 19;
    CmpAlignmentBase::columnNameToIndex["offset_begin"] = 20;
    CmpAlignmentBase::columnNameToIndex["offset_end"] = 21;
  }

  void SpringfieldInitializeColumnNameMap() {
    CmpAlignmentBase::columnNameToIndex["AlignmentId"] = 0;
    CmpAlignmentBase::columnNameToIndex["ReadGroupId"] = 1;
      CmpAlignmentBase::columnNameToIndex["AlnGroupID"] = 1;
    CmpAlignmentBase::columnNameToIndex["AlnGroupId"] = 1;
    CmpAlignmentBase::columnNameToIndex["MovieId"] = 2;
    CmpAlignmentBase::columnNameToIndex["RefSeqId"] = 3;    
    CmpAlignmentBase::columnNameToIndex["tStart"] = 4;
    CmpAlignmentBase::columnNameToIndex["tEnd"] = 5;
    CmpAlignmentBase::columnNameToIndex["RCRefStrand"] = 6;
    CmpAlignmentBase::columnNameToIndex["HoleNumber"] = 7;
    CmpAlignmentBase::columnNameToIndex["SetNumber"] = 8;     
    CmpAlignmentBase::columnNameToIndex["StrobeNumber"] = 9;
    CmpAlignmentBase::columnNameToIndex["SubreadId"] = 10;
    CmpAlignmentBase::columnNameToIndex["rStart"] = 11;     
    CmpAlignmentBase::columnNameToIndex["rEnd"] = 12;
    CmpAlignmentBase::columnNameToIndex["MapQV"] = 13;
    CmpAlignmentBase::columnNameToIndex["nM"] = 14;
    CmpAlignmentBase::columnNameToIndex["nMM"] = 15;
    CmpAlignmentBase::columnNameToIndex["nIns"] = 16;     
    CmpAlignmentBase::columnNameToIndex["nDel"] = 17;
    CmpAlignmentBase::columnNameToIndex["offset_begin"] = 18;
    CmpAlignmentBase::columnNameToIndex["offset_end"] = 19;
    CmpAlignmentBase::columnNameToIndex["nBackRead"] = 20;
    CmpAlignmentBase::columnNameToIndex["nReadOverlap"] = 21;

    // All synonyms, the code requiring these should be refactored so
    // that it no synonyms are needed.
    //
      CmpAlignmentBase::columnNameToIndex["AlnID"] = 0;
      CmpAlignmentBase::columnNameToIndex["AlnGroupID"] = 1;
      CmpAlignmentBase::columnNameToIndex["MovieID"] = 2;
      CmpAlignmentBase::columnNameToIndex["RefGroupID"] = 3;
    CmpAlignmentBase::columnNameToIndex["RefGroupId"] = 3;
      CmpAlignmentBase::columnNameToIndex["MoleculeID"] = 10;
      CmpAlignmentBase::columnNameToIndex["Offset_begin"] = 18;
      CmpAlignmentBase::columnNameToIndex["Offset_end"] = 19;
    
  }


  int Initialize(string &hdfCmpFileName, set<string> includedFieldsP, unsigned int flags=H5F_ACC_RDONLY) {
    Initialize(hdfCmpFileName, flags);
    includedFields = includedFieldsP;
  }

  void Create(string &hdfCmpFileName) {
    H5File newFile(hdfCmpFileName.c_str(), H5F_ACC_TRUNC, FileCreatPropList::DEFAULT, FileAccPropList::DEFAULT);  
    hdfCmpFile.openFile(hdfCmpFileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);    
    rootGroup.Initialize(hdfCmpFile);
    refGroupGroup.Create(rootGroup.rootGroup);
    alnGroupGroup.Create(rootGroup.rootGroup);
    refInfoGroup.Create(rootGroup.rootGroup);
    movieInfoGroup.Create(rootGroup.rootGroup);
    alnInfoGroup.Create(rootGroup.rootGroup);
    fileLogGroup.Create(rootGroup.rootGroup);

    //
    // Add atom data
    //
    readTypeAtom.Create(rootGroup.rootGroup.group, "ReadType");
    readTypeAtom.Write("standard");

    // Add some proxy data for now.
    HDFAtom<string> primaryVersionAtom, versionAtom;
    primaryVersionAtom.Create(rootGroup.rootGroup.group, "PrimaryVersion");
    primaryVersionAtom.Write("1.3.1.0.104787");
    versionAtom.Create(rootGroup.rootGroup.group, "Version");
    versionAtom.Write("1.3.1.SF");
  }

  void SetReadType(string readType) {
    readTypeAtom.Write(readType.c_str());
  }

  void GenerateNextRefGroupName(string &name) { 
    stringstream nameStrm;
    nameStrm << "ref";
    nameStrm.width(6);
    nameStrm.fill('0');
    nameStrm << right << refGroupIdToArrayIndex.size() + 1;
    name = nameStrm.str();
  }


  int AddReference(string refName, unsigned int length, string md5, string &refGroupName) {
    //
    // Adding a reference requires:
    //
    // 1. Creating a new refgroup name. 
    // 2. Create the new reference group to add alignments to.
    // 3. Adding this new name to the set of paths of ref groups.
    // 4. Adding information for this ref group.
    //

    // 1.
    this->GenerateNextRefGroupName(refGroupName);

    // 2.
    HDFCmpRefAlignmentGroup *newGroup = new HDFCmpRefAlignmentGroup;
    newGroup->Create(rootGroup.rootGroup, refGroupName);
    refAlignGroups.push_back(newGroup);
    unsigned int id = refAlignGroups.size();

    // 3.
    refGroupGroup.AddPath("/" + refGroupName, id);
    refGroupIdToArrayIndex[id] = id - 1;
    refNameToArrayIndex[refName] = id - 1;
    // 4.
    refInfoGroup.AddRefInfo(refName, id, length, md5);
    return id;
  }

  int StoreAlnArray(vector<unsigned char> &alnArray, string refName, string &experimentName,
                    unsigned int &offsetBegin,
                    unsigned int &offsetEnd) {
    //
    // First find the reference group.
    //
    assert(refNameToArrayIndex.find(refName) != refNameToArrayIndex.end());
    int refIndex = refNameToArrayIndex[refName];
    assert(refIndex < refAlignGroups.size());
    HDFCmpExperimentGroup *expGroup;
    expGroup = refAlignGroups[refIndex]->GetExperimentGroup(experimentName);
    expGroup->AddAlignment(alnArray, offsetBegin, offsetEnd);
  }

  int Initialize(string &hdfCmpFileName, unsigned int flags=H5F_ACC_RDONLY) {
    /*
     * Initialize access to the HDF file.
     */
    try {
      hdfCmpFile.openFile(hdfCmpFileName.c_str(), flags, H5P_DEFAULT);
    }
    catch (Exception &e) {
      cout << e.getDetailMsg() << endl;
      return 0;
    }

    rootGroup.Initialize(hdfCmpFile);
    readTypeAtom.Initialize(rootGroup.rootGroup, "ReadType");

    if (alnGroupGroup.Initialize(rootGroup.rootGroup)  == 0) { return 0; }
    if (refInfoGroup.Initialize(rootGroup.rootGroup)   == 0) { return 0; }
    if (refGroupGroup.Initialize(rootGroup.rootGroup)  == 0) { return 0; }
    if (movieInfoGroup.Initialize(rootGroup.rootGroup) == 0) { return 0; }
    if (alnInfoGroup.Initialize(rootGroup.rootGroup)   == 0) { return 0; }
    if (fileLogGroup.Initialize(rootGroup.rootGroup)   == 0) { return 0; }
    
    SpringfieldInitializeColumnNameMap();    

    return 1;
  }

  unsigned int GetAlignmentIndexSize() {
    if (alnInfoGroup.alnInfoGroup.groupIsInitialized == false) {
      cout << "ERROR, getting the size of an index before initializing the cmp.h5 file." << endl;
      exit(0);
    }
    return alnInfoGroup.alnIndexArray.GetNRows();
  }

  // Add synonym 
  unsigned int GetNAlignments() {
    return GetAlignmentIndexSize();
  }

  static void ParseReadGroupPath(string &path, string &refName, string &readGroupName){ 
    int delimPos;
    delimPos = path.find_last_of('/');
    if (delimPos != path.npos) {
      refName = path.substr(0, delimPos);
      //
      // Check the ref name to see if it has a preceeding '/'
      //
      int firstSlashPos;
      firstSlashPos = refName.find_first_of('/');
      readGroupName = path.substr(delimPos+1, path.size());
    } 
    else {
      refName ="";
      readGroupName = "";
    }
  }

  void StorePlatformId(CmpFile &cmpFile) {
    //
    // For now, hard wire as a springfield.  Later if the fields
    // change per platform, this may be updated.
    //
    cmpFile.platformId = Springfield;
  }

  void ReadAlignmentDescriptions(CmpFile &cmpFile) {
    
    //
    // Gather run information.
    //
    rootGroup.ReadAttributes(cmpFile);

    string readTypeString;
    readTypeAtom.Read(readTypeString);
    cmpFile.StoreReadType(readTypeString);
    //
    // Read in the groups that describe what alignments are present.
    //
    
    alnGroupGroup.Read(cmpFile.alnGroup);
    alnInfoGroup.Read(cmpFile.alnInfo);
    refGroupGroup.Read(cmpFile.refGroup);
    movieInfoGroup.Read(cmpFile.movieInfo);
    refInfoGroup.Read(cmpFile.refInfo);
    StorePlatformId(cmpFile);
    
  }

  void ReadStructure(CmpFile &cmpFile) {
    /*
     * Now for every reference group in the cmp file, create a group.
     */
    map<string,int> refNameToArrayIndex;

    unsigned int refSeqIndex;

    for (refSeqIndex = 0; refSeqIndex < cmpFile.refGroup.path.size(); refSeqIndex++) {
      HDFCmpRefAlignmentGroup* refAlignGroup;
      refAlignGroup = new HDFCmpRefAlignmentGroup;
      refAlignGroup->Initialize(rootGroup.rootGroup.group, cmpFile.refGroup.path[refSeqIndex]);
      int refAlignGroupIndex = refAlignGroups.size();
      refAlignGroups.push_back(refAlignGroup);        
      //
      // Allow one to index directly into this group given a string
      // representing the reference name.
      //
      refNameToArrayIndex[cmpFile.refGroup.path[refSeqIndex]] = refAlignGroupIndex;
      refGroupIdToArrayIndex[cmpFile.refGroup.id[refSeqIndex]] = refAlignGroupIndex;
    }

    //
    // Build a map from movie name to index.  This allows translation
    // of the group name to an index when storing all movie groups
    // that exist under ref groups.
    //
    int movieIndex;
    map<string,int> movieNameToId;
    for (movieIndex = 0; movieIndex < cmpFile.movieInfo.name.size(); movieIndex++) {
      movieNameToId[cmpFile.movieInfo.name[movieIndex]] = cmpFile.movieInfo.id[movieIndex];
    }

    /*
     * Now that all ref groups are created, add groups for movies that
     * are aligned to the ref groups.
     */

    unsigned int readGroupIndex;
    for (readGroupIndex = 0; readGroupIndex < cmpFile.alnGroup.path.size(); readGroupIndex++) {
      string refName, movieName;
      ParseReadGroupPath(cmpFile.alnGroup.path[readGroupIndex], refName, movieName);

      //
      // Create an index that allows one to immediately find the movie
      // name given a readGroupIndex
      //
      readGroupPathToReadGroup[cmpFile.alnGroup.path[readGroupIndex]] = movieName;

      //
      // Look up the group where this alignment should be found.
      unsigned int refGroupArrayIndex;
      if (refNameToArrayIndex.find(refName) != refNameToArrayIndex.end()) {
        refGroupArrayIndex = refNameToArrayIndex[refName];
      }
      else {
        cout << "ERROR! The reference name '" << refName << "' does not have an entry though it is "
             << " specified in the path for " << cmpFile.readGroupTable.names[readGroupIndex] 
             << endl;
        assert(0);
      }
      HDFCmpExperimentGroup *alnGroupPtr;
      alnGroupPtr = refAlignGroups[refGroupArrayIndex]->InitializeExperimentGroup(movieName, includedFields);
      refAlignGroups[refGroupArrayIndex]->movieIdToIndex[movieNameToId[movieName]] = refAlignGroups[refGroupArrayIndex]->readGroups.size()-1;
    }
  }

  void Read(CmpFile &cmpFile) {
    
    ReadAlignmentDescriptions(cmpFile);
    ReadStructure(cmpFile);

    /*
     * Now that the alignment indices are all read in, read the base-by-base alignments.
     */
    
    
    unsigned int alignmentIndex;

    for (alignmentIndex = 0; alignmentIndex < cmpFile.alnInfo.alignments.size(); alignmentIndex++) {
      unsigned int alnGroupId = cmpFile.alnInfo.alignments[alignmentIndex].GetAlnGroupId();
      unsigned int refGroupId = cmpFile.alnInfo.alignments[alignmentIndex].GetRefGroupId();
      unsigned int movieId    = cmpFile.alnInfo.alignments[alignmentIndex].GetMovieId();
      string refSeqName, readGroupName;

      //
      // Make sure the refGroupId specified in the alignment index exists in the alignment groups.
      //
      int refGroupArrayIndex;
      if (refGroupIdToArrayIndex.find(refGroupId) == refGroupIdToArrayIndex.end()) {
          cout << "ERROR! Alignment " << cmpFile.alnInfo.alignments[alignmentIndex].GetAlignmentId()
               << " has ref seq id " << refGroupId << " that does not exist in the HDF file." << endl;
          assert(0);
      }
      else {
        refGroupArrayIndex = refGroupIdToArrayIndex[refGroupId];
      }
      
      //
      // Point to the refGroup that this alignment is part of.
      //
      HDFCmpRefAlignmentGroup* refAlignGroup = refAlignGroups[refGroupArrayIndex];

      //
      // Now locate the movie group that is part of this ref align group.
      //
      string movieName;
      if (cmpFile.movieInfo.FindMovie(movieId, movieName) == 0) {
        cout << "ERROR! Alignment " << cmpFile.alnInfo.alignments[alignmentIndex].GetAlignmentId() 
             << " specifies a movie id " << movieId << " that is not listed in the movie group." << endl;
        assert(0);
      }
      
      if (refAlignGroup->experimentNameToIndex.find(movieName) ==
          refAlignGroup->experimentNameToIndex.end()) {
        cout << "Internal ERROR! The movie name " << movieName << " is specified as part of "
             << " the path in alignment " << cmpFile.alnInfo.alignments[alignmentIndex].GetAlignmentId() 
             << " though it does not exist in the ref align group specified for this alignment." << endl;
        assert(0);
      }
      
      int experimentIndex;
      experimentIndex = refAlignGroup->movieIdToIndex[movieId];

      unsigned int alignmentLength = cmpFile.alnInfo.alignments[alignmentIndex].GetOffsetEnd() - 
        cmpFile.alnInfo.alignments[alignmentIndex].GetOffsetBegin();

      vector<unsigned char> alignmentArray;
      vector<UChar> fieldArray;

      if (alignmentArray.size() < alignmentLength) {
        alignmentArray.resize(alignmentLength);
      }
      
      /* 
       * Read in the base by base alignment.
       */

        refAlignGroup->readGroups[experimentIndex]->alignmentArray.Read(cmpFile.alnInfo.alignments[alignmentIndex].GetOffsetBegin(),
                                                                      cmpFile.alnInfo.alignments[alignmentIndex].GetOffsetEnd(),
                                                                      &alignmentArray[0]);

      cmpFile.alnInfo.alignments[alignmentIndex].StoreAlignmentArray(&alignmentArray[0], alignmentLength);
      
      /*
       * Read in all additional fields such as quality values, etc..
       */
      set<string>::iterator fieldIt, fieldEnd;
      fieldEnd = includedFields.end();

      for (fieldIt = includedFields.begin(); fieldIt != fieldEnd; ++fieldIt) {
        if (fieldArray.size() < alignmentLength) {
          fieldArray.resize(alignmentLength);
        }
        HDFArray<UChar>* fieldArrayPtr = dynamic_cast<HDFArray<UChar>* >(refAlignGroup->readGroups[experimentIndex]->fields[*fieldIt]);

        int ob = cmpFile.alnInfo.alignments[alignmentIndex].GetOffsetBegin();
        int oe = cmpFile.alnInfo.alignments[alignmentIndex].GetOffsetEnd();
        fieldArrayPtr->Read(ob, oe, &fieldArray[0]);

        cmpFile.alnInfo.alignments[alignmentIndex].StoreField(*fieldIt, &fieldArray[0], alignmentLength);
      }
    }
  }

  void IncludeField(string fieldName) {
    if (supportedFields.find(fieldName) == supportedFields.end()) {
      cout << "ERROR, attempting to include field " << fieldName << " that is not supported." << endl;
      exit(1);
    }
    includedFields.insert(fieldName);
  }

  template<typename T_Value, typename T_QV>
  void StoreQualityValueFromAlignment(vector<T_Value> &fieldValues,
                                      vector<int> &baseToAlignmentMap,
                                      T_QV *qv) {
    int i;
    for (i = 0; i < baseToAlignmentMap.size();i++) {
      qv[i] = fieldValues[baseToAlignmentMap[i]];
    }
  }
  /*
  void ReadAlignment(int alignmentIndex, CmpAlignment &cmpAlignment) {
    alnInfoGroup.ReadCmpAlignment(alignmentIndex, cmpAlignment);
    ReadAlignmentArray(alignmentIndex, cmpAlignment.alignmentArray);
  }
  */
  void ReadAlignment(int alignmentIndex, T_Alignment &alignment) {
    CmpAlignment cmpAln;
    ReadAlignment(alignmentIndex, cmpAln);

    string   refSequence;
    string   readSequence;
    readSequence.resize(cmpAln.alignmentArray.size());
    refSequence.resize(cmpAln.alignmentArray.size());

    ByteAlignmentToQueryString(&cmpAln.alignmentArray[0], cmpAln.alignmentArray.size(), &readSequence[0]);
    ByteAlignmentToRefString(&cmpAln.alignmentArray[0], cmpAln.alignmentArray.size(), &refSequence[0]);         
    string ungappedRead, ungappedRef;    
    RemoveGaps(readSequence, ungappedRead);
    RemoveGaps(refSequence, ungappedRef);

    GappedStringsToAlignment(readSequence, refSequence, alignment);
    FASTASequence qAlignedSeq, rAlignedSeq;
    qAlignedSeq.seq = (Nucleotide*) &ungappedRead[0];
    qAlignedSeq.length = ungappedRead.size();
    rAlignedSeq.seq = (Nucleotide*) &ungappedRef[0];
    rAlignedSeq.length = ungappedRef.size();

    alignment.tAlignedSeq.Copy(rAlignedSeq);
    alignment.qAlignedSeq.Copy(qAlignedSeq);

    unsigned int qStart = cmpAln.GetQueryStart();
    unsigned int tStart = cmpAln.GetRefStart();
    
    alignment.tPos = cmpAln.GetRefStart();
    alignment.qPos = cmpAln.GetQueryStart();
    alignment.nIns   = cmpAln.GetNInsertions();
    alignment.nDel   = cmpAln.GetNDeletions();
    alignment.nMatch = cmpAln.GetNMatch();
    alignment.nMismatch=cmpAln.GetNMismatch();
    alignment.qStrand= 0;
    alignment.tStrand = cmpAln.GetTStrand();
    alignment.pctSimilarity = ((float)alignment.nMatch) / (alignment.nMatch + alignment.nMismatch + alignment.nIns + alignment.nDel);
    alignment.mapQV  = cmpAln.GetMapQV();
  }

  void ReadAlignmentArray(int alignmentIndex, ByteAlignment &alignmentArray) {
    CmpAlignment cmpAlignment;
    alnInfoGroup.ReadCmpAlignment(alignmentIndex, cmpAlignment);

    //
    // Cache some stats about the read, and where it was aligned to.
    //
    int queryStart = cmpAlignment.GetQueryStart();
    int queryEnd   = cmpAlignment.GetQueryEnd();
    int refGroupId = cmpAlignment.GetRefGroupId();
    int movieId    = cmpAlignment.GetMovieId();

    int refGroupIndex  = refGroupIdToArrayIndex[refGroupId];
    int readGroupIndex = refAlignGroups[refGroupIndex]->movieIdToIndex[movieId];
    HDFCmpExperimentGroup* expGroup = refAlignGroups[refGroupIndex]->readGroups[readGroupIndex];

    int offsetBegin = cmpAlignment.GetOffsetBegin();
    int offsetEnd   = cmpAlignment.GetOffsetEnd();

    int alignedSequenceLength = offsetEnd - offsetBegin;
    
    if (alignedSequenceLength >= 0) {
      alignmentArray.resize(alignedSequenceLength);
    }
    else {
      return;
    }
  
    //
    // Read the alignment string.  All alignments 
    //
    refAlignGroups[refGroupIndex]->readGroups[readGroupIndex]->alignmentArray.Read(offsetBegin, 
                                                                                   offsetEnd, 
                                                                                   &alignmentArray[0]);
  }

  void ImportReadFromCmpH5(int alignmentIndex, CmpAlignment &cmpAlignment, SMRTSequence &read) {
    alnInfoGroup.ReadCmpAlignment(alignmentIndex, cmpAlignment);

    //
    // Cache some stats about the read, and where it was aligned to.
    //
    int queryStart = cmpAlignment.GetQueryStart();
    int queryEnd   = cmpAlignment.GetQueryEnd();
    read.holeNumber = cmpAlignment.GetHoleNumber();
    int refGroupId = cmpAlignment.GetRefGroupId();
    int movieId    = cmpAlignment.GetMovieId();

    int refGroupIndex  = refGroupIdToArrayIndex[refGroupId];
    int readGroupIndex = refAlignGroups[refGroupIndex]->movieIdToIndex[movieId];
    HDFCmpExperimentGroup* expGroup = refAlignGroups[refGroupIndex]->readGroups[readGroupIndex];

    int offsetBegin = cmpAlignment.GetOffsetBegin();
    int offsetEnd   = cmpAlignment.GetOffsetEnd();

    int alignedSequenceLength = offsetEnd - offsetBegin;
    string   alignedSequence;
    string   readSequence;
    vector<unsigned char> byteAlignment;
    
    if (alignedSequenceLength >= 0) {
      alignedSequence.resize(alignedSequenceLength);
      byteAlignment.resize(alignedSequenceLength);
    }
  
    //
    // Read the alignment string.  All alignments 
    //
    refAlignGroups[refGroupIndex]->readGroups[readGroupIndex]->alignmentArray.Read(offsetBegin, 
                                                                                   offsetEnd, 
                                                                                   &byteAlignment[0]);

    //
    // Convert to something we can compare easily.
    //
    ByteAlignmentToQueryString(&byteAlignment[0], byteAlignment.size(), &alignedSequence[0]);

    
    // 
    // Initialize the sequence of the read.
    //
    RemoveGaps(alignedSequence, readSequence);
    
    //
    // Make space for the sequence and all fields.
    //
    read.length = readSequence.size();
    read.Allocate(read.length);
    memcpy(read.seq, readSequence.c_str(), readSequence.size() * sizeof(char));

    vector<int> baseToAlignmentMap;
    CreateSequenceToAlignmentMap(byteAlignment, baseToAlignmentMap);
    
    //
    // Read in the quality values
    //


    vector<unsigned char> storedQVArray;
    
    vector<UChar> qvValues;
    vector<HalfWord> frameValues;
    int length = offsetEnd - offsetBegin;
    qvValues.resize(length);
    frameValues.resize(length);
    int i;

    
    if (expGroup->experimentGroup.ContainsObject("QualityValue")) {
      expGroup->qualityValue.Read(offsetBegin, offsetEnd, &qvValues[0]);
      StoreQualityValueFromAlignment(qvValues, baseToAlignmentMap, &read.qual.data[0]);
      int i;
      for (i= 0; i < read.length; i++) {
        assert(read.qual[i] < 100);
      }
    }
    
    if (expGroup->experimentGroup.ContainsObject("InsertionQV")) {
      expGroup->insertionQV.Read(offsetBegin, offsetEnd, &qvValues[0]);
      StoreQualityValueFromAlignment(qvValues, baseToAlignmentMap, &read.insertionQV.data[0]);
    }
    
    if (expGroup->experimentGroup.ContainsObject("SubstitutionQV")) {
      expGroup->substitutionQV.Read(offsetBegin, offsetEnd, &qvValues[0]);
      StoreQualityValueFromAlignment(qvValues, baseToAlignmentMap, &read.substitutionQV.data[0]);
    }

    if (expGroup->experimentGroup.ContainsObject("DeletionQV")) {
      expGroup->deletionQV.Read(offsetBegin, offsetEnd, &qvValues[0]);
      StoreQualityValueFromAlignment(qvValues, baseToAlignmentMap, &read.deletionQV.data[0]);
    }

    if (expGroup->experimentGroup.ContainsObject("DeletionTag")) {
      vector<char> deletionTagValues;
      deletionTagValues.resize(offsetEnd-offsetBegin);
      expGroup->deletionTag.Read(offsetBegin, offsetEnd, &deletionTagValues[0]);
      StoreQualityValueFromAlignment(deletionTagValues, baseToAlignmentMap, read.deletionTag);
    }

    if (expGroup->experimentGroup.ContainsObject("SubstitutionTag")) {
      vector<char> substitutionTagValues;
      substitutionTagValues.resize(offsetEnd-offsetBegin);
      expGroup->substitutionTag.Read(offsetBegin, offsetEnd, &substitutionTagValues[0]);
      StoreQualityValueFromAlignment(substitutionTagValues, baseToAlignmentMap, read.substitutionTag);
    }

    if (expGroup->experimentGroup.ContainsObject("PulseIndex")) {
      vector<uint32_t> pulseIndexValues;
      pulseIndexValues.resize(offsetEnd-offsetBegin);
      expGroup->pulseIndex.Read(offsetBegin, offsetEnd, &pulseIndexValues[0]);
      StoreQualityValueFromAlignment(pulseIndexValues, baseToAlignmentMap, read.pulseIndex);
    }
    
    if (expGroup->experimentGroup.ContainsObject("PreBaseFrames")) {
      expGroup->preBaseFrames.Read(offsetBegin, offsetEnd, &frameValues[0]);
      StoreQualityValueFromAlignment(frameValues, baseToAlignmentMap, read.preBaseFrames);
    }

    if (expGroup->experimentGroup.ContainsObject("WidthInFrames")) {
      expGroup->widthInFrames.Read(offsetBegin, offsetEnd, &frameValues[0]);
      StoreQualityValueFromAlignment(frameValues, baseToAlignmentMap, read.widthInFrames);
    }
    
  }

  void ReadReadGroupPathTable(CmpIndexedStringTable &readGroupPathTable) {
    readGroupPathIdLastRow.Read(readGroupPathTable.lastRow);
    readGroupPathTable.resize(readGroupPathTable.lastRow); // resizes all tables
    readGroupPathIdArray.Read(0, readGroupPathTable.lastRow, &readGroupPathTable.ids[0]);
    readGroupPathArray.Read(0, readGroupPathTable.lastRow, &readGroupPathTable.names[0]);
    readGroupPathTable.StoreArrayIndexMap();
  }

  void ReadRefSeqTable(CmpIndexedStringTable &refSeqTable) {
    refSeqNameIdLastRow.Read(refSeqTable.lastRow);
    refSeqTable.resize(refSeqTable.lastRow);
    refSeqNameIdArray.Read(0, refSeqTable.lastRow, &refSeqTable.ids[0]);
    refSeqNameArray.Read(0, refSeqTable.lastRow, &refSeqTable.names[0]);
    refSeqTable.StoreArrayIndexMap();
  }

  void ReadMovieNameTable(CmpIndexedStringTable &movieTable) {
    movieNameIdLastRow.Read(movieTable.lastRow);
    movieTable.resize(movieTable.lastRow);
    movieNameIdArray.Read(0, movieTable.lastRow, &movieTable.ids[0]);
    movieNameArray.Read(0,   movieTable.lastRow, &movieTable.names[0]);
    movieTable.StoreArrayIndexMap();
  }
  
  
};


#endif
