#ifndef ALIGNMENT_MAPPING_SEMAPHORE_H_
#define ALIGNMENT_MAPPING_SEMAPHORE_H_
#include <vector>
#include <pthread.h>
#include <semaphore.h>

class MappingSemaphores {
  public:
	sem_t reader;
	sem_t writer;
	sem_t unaligned;
  sem_t hitCluster;
	MappingSemaphores& operator=(MappingSemaphores &rhs) {
		return *this;
	}

	void InitializeAll() {
		sem_init(&reader, 0, 1);
		sem_init(&writer, 0, 1);
		sem_init(&unaligned, 0, 1);
    sem_init(&hitCluster, 0, 1);
	}
};

#endif
