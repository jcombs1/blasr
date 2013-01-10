#ifndef HDF_REF_GROUP_H_
#define HDF_REF_GROUP_H_

#include "HDFAtom.h"
#include "HDFArray.h"
#include "HDFGroup.h"
#include "datastructures/saf/RefGroup.h"

class HDFRefGroupGroup {
 public:
	HDFGroup refGroup;
	HDFAtom<uint32_t> nRow;
	HDFAtom<string>   masterDatasetAtom;
	HDFArray<uint32_t>  idArray;
	HDFAtom<uint64_t> idArrayLastRow;
	HDFArray<string> pathArray;

	HDFArray<uint32_t> refInfoIdArray;

	~HDFRefGroupGroup() {
		refGroup.Close();
	}
	
  bool Create(HDFGroup &parent) {
    parent.AddGroup("RefGroup");
		if (refGroup.Initialize(parent.group, "RefGroup") == 0) {
      return 0;
    }
    masterDatasetAtom.Create(refGroup.group, "MasterDataset", "ID");
    nRow.Create(refGroup.group, "nRow");
    idArray.Create(refGroup, "ID");
    pathArray.Create(refGroup, "Path");
    refInfoIdArray.Create(refGroup, "RefInfoID");
    return true;
  }

  int AddPath(string path, unsigned int refInfoId) {
    pathArray.Write(&path, 1);
    unsigned int numPath = pathArray.size();
    idArray.Write(&numPath, 1);
    refInfoIdArray.Write(&refInfoId, 1);
    nRow.Write(numPath);
    return numPath;
  }

	int Initialize(HDFGroup &rootGroup) {
		refGroup.Initialize(rootGroup.group, "RefGroup");
		
		if (masterDatasetAtom.Initialize(refGroup.group, "MasterDataset") == 0) { return 0; }
		if (nRow.Initialize(refGroup, "nRow") == 0) { return 0; }
		if (idArray.Initialize(refGroup, "ID") == 0) { return 0; }
		if (pathArray.Initialize(refGroup, "Path") == 0) { return 0; }
		if (refInfoIdArray.Initialize(refGroup, "RefInfoID") == 0) { return 0; }
		
		return 1;
	}

	void Read(RefGroup &refGroup) {
		masterDatasetAtom.Read(refGroup.masterDataset);
		nRow.Read(refGroup.nRow);
		int i;
		refGroup.path.resize(refGroup.nRow);
		for (i = 0; i < refGroup.nRow; i++) {
			pathArray.Read(i, i+1, &refGroup.path[i]);
		}

		int idArrayNElem = idArray.arrayLength;
		refGroup.id.resize(idArrayNElem);
		idArray.Read(0, idArrayNElem, &refGroup.id[0]);

		int refIDNElem = refInfoIdArray.arrayLength;
		refGroup.refInfoId.resize(refIDNElem);
		refInfoIdArray.Read(0, refIDNElem, &refGroup.refInfoId[0]);
	}
};

#endif
