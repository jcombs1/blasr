#ifndef DATA_HDF_HDF_ALN_GROUP_H_
#define DATA_HDF_HDF_ALN_GROUP_H_

#include "HDFAtom.h"
#include "HDFArray.h"
#include "HDFGroup.h"

class HDFAlnGroup {
 public:
  HDFGroup alnGroup;
  HDFAtom<string> masterDatsetAtom;
  HDFAtom<int>    nRowAtom;
  HDFArray<unsigned int> idArray;
  HDFArray<string> pathArray;

  void Initialize(HDFGroup &parent) {
    alnGroup.Initialize(parent.group, "AlnGroup");
    masterDatasetAtom.Initialize(alnGroup.group, "MasterDataset");
    idArray.Initialize(alnGroup.group, "ID");

  }
  
  void Read(AlnGroup &aln) {
    masterDatasetAtom.Read(aln.masterDataset);
    int idNElem = idArray.arrayLength;
    int pathNElem = pathArray.arrayLength;
    aln.nRow = idNElem;
    if (aln.nRow > 0) {

      aln.id.resize(idNElem);
      idArray.Read(0,aln.nRow);

      aln.path.resize(pathNElem);
      unsigned int i;
      for (i = 0; i < pathNElem; i++) {
        pathArray.Read(i, i+1, &aln.path[i]);
      }
    }
  }

  int AddPath(string &path) {
    int id;
    pathArray.Write(&path, 1);
    id = pathArray.size();
    idArray.Write(&id, 1);
    return id;
  }

  ~HDFAlnGroup() {
    alnGroup.Close();
  }

};


#endif
