#ifndef DATA_HDF_HDF_ALN_GROUP_H_
#define DATA_HDF_HDF_ALN_GROUP_H_

#include "HDFAtom.h"
#include "HDFArray.h"
#include "HDFGroup.h"

class HDFAlnGroupGroup {
 public:
  HDFGroup alnGroup;
  HDFAtom<string> masterDatasetAtom;
  HDFAtom<unsigned int>    nRowAtom;
  HDFArray<unsigned int> idArray;
  HDFArray<string> pathArray;
  HDFAtom<uint64_t>    idLastRowAtom;

  bool Create(HDFGroup &parent) {
    parent.AddGroup("AlnGroup");
    if (alnGroup.Initialize(parent.group, "AlnGroup") == 0) { return 0; }
    masterDatasetAtom.Create(alnGroup.group, "MasterDataset", "ID");
    idArray.Create(alnGroup, "ID");
    pathArray.Create(alnGroup, "Path");
    nRowAtom.Create(alnGroup.group, "nRow");
    nRowAtom.Write(0);
    return true;
  }
  
  int AddPath(string path) {
    pathArray.Write(&path, 1);
    unsigned int id = pathArray.size();
    idArray.Write(&id, 1);
    int size = pathArray.size();
    nRowAtom.Write(size);
    return size;
  }

  int Initialize(HDFGroup &parent) {
    if (alnGroup.Initialize(parent.group, "AlnGroup") == 0) { return 0; }
    if (masterDatasetAtom.Initialize(alnGroup.group, "MasterDataset") == 0) { return 0; }
    if (idArray.Initialize(alnGroup, "ID") == 0) { return 0; }
    if (pathArray.Initialize(alnGroup, "Path") == 0) { return 0; }
    if (nRowAtom.Initialize(alnGroup.group, "nRow") == 0) { return 0; }
                        
    return 1;
  }
  
  void Read(AlnGroup &aln) {
    masterDatasetAtom.Read(aln.masterDataset);
    aln.nRow = idArray.size();
    if (aln.nRow > 0) {
      aln.id.resize(aln.nRow);
      idArray.Read(0,aln.nRow, &aln.id[0]);
      aln.path.resize(aln.nRow);
      unsigned int i;
      for (i = 0; i < aln.nRow; i++) {
        pathArray.Read(i, i+1, &aln.path[i]);
      }
    }
  }
  
  ~HDFAlnGroupGroup() {
    alnGroup.Close();
  }

};


#endif
