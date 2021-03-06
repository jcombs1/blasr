#ifndef DATA_HDF_HDF_PLS_READER_H_
#define DATA_HDF_HDF_PLS_READER_H_

#include "data/hdf/HDFArray.h"
#include "data/hdf/HDF2DArray.h"
#include "data/hdf/HDFAtom.h"
#include "data/hdf/PlatformId.h"
#include "data/hdf/HDFGroup.h"
#include "data/hdf/HDFBasReader.h"
#include "data/hdf/HDFPulseDataFile.h"
#include "data/hdf/DatasetCollection.h"
#include "data/hdf/HDFZMWReader.h"
#include "datastructures/reads/PulseFile.h"
#include "HDFScanDataReader.h"
#include "FASTQSequence.h"
#include <sstream>
#include <vector>
#include <set>
using namespace H5;
using namespace std;
/*
 * Interface for reading pulse information from a .pls.h5 file.
 * To read both pls and bas information, use the HDFBasReader.
 */

class HDFPlsReader : public DatasetCollection, public HDFPulseDataFile  {
	DNALength curPos;
	int curRead;
	int nReads;

	HDFGroup pulseCallsGroup;

  int meanSignalNDims, midSignalNDims, maxSignalNDims;
  
	HDF2DArray<uint16_t>   meanSignalMatrix;
	HDF2DArray<uint16_t>   midSignalMatrix;
	HDF2DArray<uint16_t>   maxSignalMatrix;
	HDFArray<uint16_t>     meanSignalArray;
	HDFArray<uint16_t>     midSignalArray;
	HDFArray<uint16_t>     maxSignalArray;
	HDFArray<uint16_t>     plsWidthInFramesArray;
	HDFArray<unsigned int> startFrameArray;
	HDFArray<float>        classifierQVArray;


 public:
 HDFPlsReader() : HDFPulseDataFile(), DatasetCollection() {
		fieldNames.push_back("MeanSignal");
		fieldNames.push_back("MidSignal");
		fieldNames.push_back("MaxSignal");
		fieldNames.push_back("StartFrame");
		fieldNames.push_back("ClassifierQV");
		fieldNames.push_back("WidthInFrames");
		fieldNames.push_back("FrameRate");
		fieldNames.push_back("WhenStarted");
		fieldNames.push_back("NumEvent"); // this is read from the zmw, but control it here.
		InitializeAllFields(false);
	}

	int InitializeCommon() {
		return 1;
	}

	int Initialize(string hdfPlsFileName) {
		/*
		 * Initialize access to the HDF file.
		 */

		if (HDFPulseDataFile::Initialize(hdfPlsFileName) == 0) {
			return 0;
		}

		if (pulseDataGroup.ContainsObject("PulseCalls") == 0 or
				pulseCallsGroup.Initialize(pulseDataGroup.group, "PulseCalls") == 0) {
			return 0;
		}
		zmwReader.Initialize(&pulseCallsGroup);


		//
		// Initialize arrays for reading.  This has been preconfigured to 
		//
		if (!InitializePulseGroup()) return 0;

    if ( (meanSignalNDims = GetDatasetNDim(pulseCallsGroup.group, "MeanSignal")) == 1) {
      if (!meanSignalArray.Initialize(pulseCallsGroup, "MeanSignal"))    return 0;
    }
    else if (meanSignalNDims == 2) {
      if (!meanSignalMatrix.Initialize(pulseCallsGroup, "MeanSignal"))    return 0;
    }
    
    if ((midSignalNDims = GetDatasetNDim(pulseCallsGroup.group, "MidSignal")) == 1) {
      if (!midSignalArray.Initialize(pulseCallsGroup, "MidSignal"))     return 0;
    }
    else if (midSignalNDims == 2) {
      if (!midSignalMatrix.Initialize(pulseCallsGroup, "MidSignal"))     return 0;
    }

    if ((maxSignalNDims = GetDatasetNDim(pulseCallsGroup.group, "MaxSignal")) == 1) {
      if (!maxSignalArray.Initialize(pulseCallsGroup, "MaxSignal")) return 0;
    }
    else if (maxSignalNDims == 2) {
      if (!maxSignalMatrix.Initialize(pulseCallsGroup, "MaxSignal"))     return 0;
    }
    
    //
    // Astro pulse files may not have the ClassifierQV dataset, check
    // to see if it exists, and then try and initialize it.
    //
    if (pulseCallsGroup.ContainsObject("ClassifierQV")) {
      if (!classifierQVArray.Initialize(pulseCallsGroup, "ClassifierQV"))  return 0;
    }

    //
    // These are all required fields.
    //
		if (!startFrameArray.Initialize(pulseCallsGroup, "StartFrame"))    return 0;
		if (!plsWidthInFramesArray.Initialize(pulseCallsGroup, "WidthInFrames")) return 0;

		return 1;
	}

  int  GetDatasetNDim(CommonFG &parentGroup, string datasetName) {
    HDFData tmpDataset;
    tmpDataset.InitializeDataset(parentGroup, datasetName);
    DataSpace dataspace = tmpDataset.dataset.getSpace();
    int nDims = dataspace.getSimpleExtentNdims();
    dataspace.close();
    tmpDataset.dataset.close();
    return nDims;
  }

	void GetAllMeanSignal(vector<uint16_t> &meanSignal) {
    if (meanSignalNDims == 1) {
      CheckMemoryAllocation(meanSignalArray.arrayLength, maxAllocNElements, "MeanSignal");
      meanSignal.resize(meanSignalArray.arrayLength);
      meanSignalArray.Read(0, meanSignalArray.arrayLength, &meanSignal[0]);
    }   
    else if (meanSignalNDims == 2) {
      CheckMemoryAllocation(meanSignalMatrix.GetNCols() * meanSignalMatrix.GetNRows(), maxAllocNElements, "MeanSignal");
      meanSignal.resize(meanSignalMatrix.GetNCols() * meanSignalMatrix.GetNRows());
      meanSignalMatrix.Read(0, meanSignalMatrix.GetNRows(), &meanSignal[0]);
    } 
	}

	void GetAllMidSignal(vector<uint16_t> &midSignal) {
    if (midSignalNDims == 1) {
      CheckMemoryAllocation(midSignalArray.arrayLength, maxAllocNElements, "MidSignal");
      midSignal.resize(midSignalArray.arrayLength);
      midSignalArray.Read(0, midSignalArray.arrayLength, &midSignal[0]);
    }
    else if (midSignalNDims == 2) {
      CheckMemoryAllocation(midSignalMatrix.GetNCols() * midSignalMatrix.GetNRows(), maxAllocNElements, "MidSignal");
      midSignal.resize(midSignalMatrix.GetNCols() * midSignalMatrix.GetNRows());
      midSignalMatrix.Read(0, midSignalMatrix.GetNRows(), &midSignal[0]);
    }
	}

	void GetAllMaxSignal(vector<uint16_t> &maxSignal) {
    if (maxSignalNDims == 1) {
      CheckMemoryAllocation(maxSignalArray.arrayLength, maxAllocNElements, "MaxSignal");
      maxSignal.resize(maxSignalArray.arrayLength);
      maxSignalArray.Read(0, maxSignalArray.arrayLength, &maxSignal[0]);
    }
    else if (maxSignalNDims == 2) {
      CheckMemoryAllocation(maxSignalMatrix.GetNCols() * maxSignalMatrix.GetNRows(), maxAllocNElements, "MaxSignal");
      maxSignal.resize(maxSignalMatrix.GetNCols() * maxSignalMatrix.GetNRows());
      maxSignalMatrix.Read(0, maxSignalMatrix.GetNRows(), &maxSignal[0]);
    }
	}

	void GetAllStartFrames(vector<UInt> &startFrame) {
		CheckMemoryAllocation(startFrameArray.arrayLength, maxAllocNElements, "StartFrame");
		startFrame.resize(startFrameArray.arrayLength);
		startFrameArray.Read(0,startFrameArray.arrayLength, &startFrame[0]);
	}

	void GetAllPlsWidthInFrames(vector<uint16_t> &widthInFrames) {
		CheckMemoryAllocation(plsWidthInFramesArray.arrayLength, maxAllocNElements, "WidthInFrames (pulse)");
		widthInFrames.resize(plsWidthInFramesArray.arrayLength);
		plsWidthInFramesArray.Read(0,plsWidthInFramesArray.arrayLength, &widthInFrames[0]);
	}
	
	void GetAllClassifierQV(vector<float> &classifierQV) {
		CheckMemoryAllocation(classifierQVArray.arrayLength, maxAllocNElements, "ClassifierQV (pulse)");
		classifierQV.resize(classifierQVArray.arrayLength);
		classifierQVArray.Read(0, classifierQVArray.arrayLength, &classifierQV[0]);
	}

	void GetAllNumEvent(vector<int> &numEvent) {
		CheckMemoryAllocation(zmwReader.numEventArray.arrayLength, maxAllocNElements, "NumEvent (pulse)");
		numEvent.resize(zmwReader.numEventArray.arrayLength);
		zmwReader.numEventArray.Read(0, zmwReader.numEventArray.arrayLength, &numEvent[0]);
	}

	void ReadPulseFile(PulseFile &pulseFile) {
		if (scanDataReader.fileHasScanData) {
			scanDataReader.Read(pulseFile.scanData);
		}
		if (includedFields["StartFrame"]) {
			GetAllStartFrames(pulseFile.startFrame);
		}
		if (includedFields["WidthInFrames"]) {
			GetAllPlsWidthInFrames(pulseFile.plsWidthInFrames);
		}
		if (includedFields["MeanSignal"]) {
			GetAllMeanSignal(pulseFile.meanSignal);
      pulseFile.meanSignalNDims = meanSignalNDims;
		}
		if (includedFields["MidSignal"]) {
			GetAllMidSignal(pulseFile.midSignal);
      pulseFile.midSignalNDims = midSignalNDims;
		}
		if (includedFields["MaxSignal"]) {
			GetAllMaxSignal(pulseFile.maxSignal);
      pulseFile.maxSignalNDims = maxSignalNDims;
		}
    // By default, always get the num event.  This is used
    // later to copy reads from the pls file.
    GetAllNumEvent(pulseFile.numEvent);
    int i;

    if (pulseFile.numEvent.size() > 0) {
      pulseFile.pulseStartPositions.resize(pulseFile.numEvent.size());
      pulseFile.pulseStartPositions[0] = 0;
      for (i = 1; i < pulseFile.numEvent.size(); i++) {
        pulseFile.pulseStartPositions[i] = pulseFile.pulseStartPositions[i-1] + pulseFile.numEvent[i-1];
      }
    }
    
		if (includedFields["ClassifierQV"]) {
			GetAllClassifierQV(pulseFile.classifierQV);
		}
	}

  int GetReadAt(int holeNumber, int* &basToPlsIndex, SMRTSequence &read) {
    if (preparedForRandomAccess == false) {
      PrepareForRandomAccess();
    }
    curRead = holeNumber;
    curPos  = eventOffset[holeNumber];
    zmwReader.curZMW = holeNumber;
    GetNextFlattenedToBase(read, basToPlsIndex);
  }

  template<typename T_FieldType>
  void StoreField(vector<T_FieldType> &source, int *basToPlsIndex, T_FieldType *dest, int destLength) {
    int i;
    for (i = 0 ; i < destLength; i++) {
      dest[i] = source[basToPlsIndex[i]];
    }
  }
  
  void ReadSignal(string fieldName,
                  HDFArray<HalfWord> &signalArray, 	HDF2DArray<HalfWord> &signalMatrix, 
                  int plsSeqLength, int nDims, 
                  Nucleotide *basSeq, int basSeqLength,
                  int *basToPlsIndex, HalfWord* dest) {
    

		if (includedFields[fieldName]) {
      vector<HalfWord> signal;
      if (nDims == 2) {
        signal.resize(plsSeqLength * 4);
        signalMatrix.Read(curPos, curPos + plsSeqLength, &signal[0]); // read off one row.
        int i;
        for (i = 0; i < basSeqLength; i++) {
          dest[i] = signal[basToPlsIndex[i]*4 + NucToHdfColumnOrder[basSeq[i]]];
        }
      }
      else {
        signal.resize(plsSeqLength);
        signalArray.Read(curPos, curPos + plsSeqLength, &signal[0]);
        int i;
        for (i = 0; i < basSeqLength; i++) {
          dest[i] = signal[basToPlsIndex[i]];
        }
      }
    }
  }

	int GetNextFlattenedToBase(SMRTSequence &read, int* basToPlsIndex) {

    /*
     * Get the pulse values, but only store values that correspond to called bases. 
     * This requires that the read has the read.seq field assigned.
     */
    assert(read.seq != NULL);
    int seqLength;
    
    //
    // Find out how many pulses are called for this read.
		zmwReader.numEventArray.Read(curRead, curRead+1, &seqLength);
    
    if (includedFields["StartFrame"]) {
      vector<unsigned int> pulseStartFrame;
      pulseStartFrame.resize(seqLength);
      startFrameArray.Read(curPos, curPos + seqLength, &pulseStartFrame[0]);
      read.startFrame = new unsigned int[read.length];
      StoreField(pulseStartFrame, basToPlsIndex, read.startFrame, read.length);
    }

    if (includedFields["WidthInFrames"]) {
      vector<HalfWord> pulseWidthInFrames;
      pulseWidthInFrames.resize(seqLength);
      plsWidthInFramesArray.Read(curPos, curPos + seqLength, &pulseWidthInFrames[0]);
      read.widthInFrames = new HalfWord[read.length];
      StoreField(pulseWidthInFrames, basToPlsIndex, read.widthInFrames, read.length);
    }
    
		if (includedFields["MidSignal"]) {
      read.midSignal = new HalfWord[read.length];
      ReadSignal("MidSignal", midSignalArray, midSignalMatrix, seqLength, midSignalNDims, 
                 read.seq, read.length, basToPlsIndex, read.midSignal);
    }
    
		if (includedFields["MaxSignal"]) {
      read.maxSignal = new HalfWord[read.length];
      ReadSignal("MaxSignal", maxSignalArray, maxSignalMatrix, seqLength, maxSignalNDims, 
                 read.seq, read.length, basToPlsIndex, read.maxSignal);
		}

		if (includedFields["MeanSignal"]) {
      read.meanSignal = new HalfWord[read.length];
      ReadSignal("MeanSignal", meanSignalArray, meanSignalMatrix, seqLength, meanSignalNDims, 
                 read.seq, read.length, basToPlsIndex, read.meanSignal);
		}

		if (includedFields["ClassifierQV"]) {
      vector<float> pulseClassifierQV;
      pulseClassifierQV.resize(seqLength);
      classifierQVArray.Read(curPos, curPos + seqLength, &pulseClassifierQV[0]);
      read.classifierQV = new float[read.length];
      StoreField(pulseClassifierQV, basToPlsIndex, read.classifierQV, read.length);
		}
    
    curRead++;
    curPos += seqLength;
    
	}
	

	void Close() {
		
	}
};


#endif
