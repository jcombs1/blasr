#ifndef UTILS_PROTECTED_NEW_H_
#define UTILS_PROTECTED_NEW_H_

#include <iostream>

using namespace std;

template<typename T>
T*ProtectedNew(unsigned long size) {
	T* ptr;
	ptr = new T[size];
	if (ptr == NULL) {
		cout << "Error allocating " << size * sizeof(T) << " bytes.";
		exit(0);
	}
	return ptr;
}

#endif
