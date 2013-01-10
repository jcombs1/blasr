#ifndef DATASTRUCTURES_ALIGNMENT_CMP_INDEXED_STRING_TABLE_H_
#define DATASTRUCTURES_ALIGNMENT_CMP_INDEXED_STRING_TABLE_H_

#include <vector>
#include <string>
#include <map>
using namespace std;

class CmpIndexedStringTable {
 public:

	void resize(int size) {
		names.resize(size);
		ids.resize(size);
	}

	void StoreArrayIndexMap() {
		int i;
		for (i = 0; i < ids.size(); i++) {
			idToArrayIndex[ids[i]] = i;
		}
	}

	bool GetNameAtIndex(int index, string &name) {
		map<int,int>::iterator mapIt;
		mapIt = idToArrayIndex.find(index);
		if (mapIt != idToArrayIndex.end()) {
			name = names[mapIt->second];
			return true;
		}
		else {
			return false;
		}
	}

	bool GetIndexOfId(int id, int &index) {
		map<int,int>::iterator mapIt;
		mapIt = idToArrayIndex.find(id);
		if (mapIt != idToArrayIndex.end()) {
			index = mapIt->second;
			return true;
		}
		else {
			return false;
		}
	}
	vector<int> ids;
	vector<string> names;
	map<int,int> idToArrayIndex;
	int lastRow;
};




#endif
