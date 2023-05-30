#include "monitor.h"

#define CF_NAME_LEN		10
#define MAX_CF_FILES	10

struct cap_files_store_s {
	void *ptr;
	void *loc;
	char name[CF_NAME_LEN];
	int size;
};

static struct cap_files_store_s cap_files[MAX_CF_FILES];
static struct cap_files_store_s cvm_regions[MAX_CVMS][MAX_CF_FILES];
static pthread_mutex_t cf_store_lock;


void init_cap_files_store() {
	memset(cap_files, 0, MAX_CF_FILES*sizeof(struct cap_files_store_s));

	if (pthread_mutex_init(&cf_store_lock, NULL) != 0) {
		printf("\n mutex init failed\n");
		while(1);
	}

}

/**
\brief Hostcall function to 'advertise' a shared buffer. Intravisor creates a record inside a cap_file_store about published buffer: key, address, size
\param[in] ptr beginning of the advertised buffer
\param[in] size size of the buffer
\param[in] key identifier of the buffer
\param[out] ret: index in the table. not used anymore
*/	
int host_cap_file_adv(void *ptr, long size, char *key) {
#ifdef CF_DEBUG
	printf("CF_ADV: %p, %ld, %s\n", ptr, size, key);
#endif
	pthread_mutex_lock(&cf_store_lock);
	int i = 0;
	for(i = 0; i < MAX_CF_FILES; i++) {
		if(cap_files[i].ptr == 0)
			break;
	}
	
	if(i == MAX_CF_FILES) {
		printf("need more cap streams/files, die\n");
		while(1);
	}

	cap_files[i].ptr = ptr;
	cap_files[i].size = size;
	snprintf(cap_files[i].name, CF_NAME_LEN, "%s", key);

	pthread_mutex_unlock(&cf_store_lock);

#ifdef CF_DEBUG
	printf("MON: CF[%d] %p %s %d\n", i, cap_files[i].ptr, cap_files[i].name, cap_files[i].size);
#endif
	return i;
}

/**
\brief Hostcall function to 'probe' a shared buffer. Intravisor retrives a shared buffer, creates a capability, and stores it at location
\param[in] key Indentifier of the shared buffer
\param[in] location address inside cVM where to store the capability
\param[in] size address inside cVM where to store the size of the buffer
\param[out] ret: always 0
*/
int host_cap_file_prb(char *key, void *location, long *size) {
//todo: We should check that location and size are located inside the cVM that issued host_cap_file_prb 
#ifdef CF_DEBUG
	printf("MON: CF: probe for key %s, store at %p\n", key, location);
#endif
	pthread_mutex_lock(&cf_store_lock);

	struct cap_files_store_s *target_region = cap_files;

	int i;
	for(i = 0; i < MAX_CF_FILES; i++) {
		if(strncmp(target_region[i].name, key, CF_NAME_LEN) == 0) {
			// printf("match keys: |%s| vs |%s|\n", cap_files[i].name, key);
			break;
		} else {
			// printf("miss match: |%s| vs |%s|\n", cap_files[i].name, key);
		}
	}
	
	if(i == MAX_CF_FILES) {
		printf("wrong cap key %s ", key);
		return 0;
	}

	host_reg_cap(target_region[i].ptr, target_region[i].size, location);

	char *ptr = (char *)location;

	if(size) 
		*size = target_region[i].size;
	target_region[i].loc = location;

	pthread_mutex_unlock(&cf_store_lock);
	return 0;
}

int find_target_cvm(void **location) {
	// find target cvm
	int cvm_counter = 0;
	for (cvm_counter; cvm_counter < MAX_CVMS; cvm_counter++) {
		// printf("CVM: %p\n", (void *)(cvms[cvm_counter].cmp_begin));
		if (location >= cvms[cvm_counter].cmp_begin && location < cvms[cvm_counter].cmp_end) {
			// printf("Found the CVM: %d\n", cvm_counter);
			break;
		}
	}

	if (cvm_counter == MAX_CVMS) {
		printf("Cannot find the target CVM\n");
		return -1;
	}

	return cvm_counter;
}

int host_cvm_region_prb(char *key, void *location, long *size) {
//todo: We should check that location and size are located inside the cVM that issued host_cap_file_prb 
#ifdef CF_DEBUG
	printf("MON: CF: probe for key %s, store at %p\n", key, location);
#endif
	pthread_mutex_lock(&cf_store_lock);

	// find target cvm
	int cvm_counter = find_target_cvm(location);

	if (cvm_counter < 0) {
		return -1;
	}

	struct cap_files_store_s *target_region = cvm_regions[cvm_counter];

	int i;
	for(i = 0; i < MAX_CF_FILES; i++) {
		if(strncmp(target_region[i].name, key, CF_NAME_LEN) == 0) {
			// printf("match keys: |%s| vs |%s|\n", cap_files[i].name, key);
			break;
		} else {
			// printf("miss match: |%s| vs |%s|\n", cap_files[i].name, key);
		}
	}
	
	if(i == MAX_CF_FILES) {
		printf("wrong cap key %s ", key);
		return 0;
	}

	// printf("i: %d and localtion is at %p\n", i, location);

	host_reg_cap(target_region[i].ptr, target_region[i].size, location);

	// printf("MON: CF: probe for key %s, store at %p\n", key, location);

	int *ptr2 = (int *)location;
	printf("the mapped cap probed is: 0x %08x %08x %08x %08x\n", ptr2[0], ptr2[1], ptr2[2], ptr2[3]);

	if(size) 
		*size = target_region[i].size;
	target_region[i].loc = location;

	pthread_mutex_unlock(&cf_store_lock);
	return 0;
}

/**
\brief Hostcall function to destroy a shared buffer (capfile)
\param[in] key Indentifier
\param[out] ret: always 0
*/
int host_cap_file_revoke(char *key) {
#ifdef CF_DEBUG
	printf("MON: CF: revoke cap file with key %s\n", key);
#endif
	pthread_mutex_lock(&cf_store_lock);
	int i;
	for(i = 0; i < MAX_CF_FILES; i++) {
		if(strncmp(cap_files[i].name, key, CF_NAME_LEN) == NULL)
			break;
	}
	
	if(i == MAX_CF_FILES) {
		printf("wrong cap key %s ", key); while(1);
	}

	host_purge_cap(cap_files[i].loc);

	memset(cap_files[i].name, 0, CF_NAME_LEN);
	cap_files[i].size = 0;
	cap_files[i].loc = 0;
	cap_files[i].ptr = 0;

	pthread_mutex_unlock(&cf_store_lock);

//todo: remove the original cap from the CPU contextes of threads inside cVM
	return 0;
}


/**
\brief Hostcall function to 'probe' a shared buffer. Intravisor retrives a shared buffer, creates a capability, and stores it at location
\param[in] location address inside cVM where to store the capability
\param[in] size address inside cVM where to store the size of the buffer
\param[out] ret: always 0
*/
int host_cap_map(void **location, long *region_size) {
//todo: We should check that location and size are located inside the cVM that issued host_cap_file_prb 
#ifdef CF_DEBUG
	printf("MON: CF: probe for key %p, store at %p\n", location, size);
#endif
	int i = 0;
	int size = *region_size;

	// find target cvm
	int cvm_counter = find_target_cvm(location);

	if (cvm_counter < 0) {
		return -1;
	}

	for(i = 0; i < MAX_CF_FILES; i++) {
		if(cvm_regions[cvm_counter][i].ptr == 0)
			break;
	}
	
	if(i == MAX_CF_FILES) {
		printf("need more cap streams/files, die\n");
		return -1;
	}

	void *ptr = malloc(size);
	if(!ptr) {
		printf("cannot alloc %d bytes for %s key\n", size, "mmap");
		return -1;
	}

	memset(ptr, 0, size);

	*location = ptr;

	printf("Host Cap Map: %p with size %d\n", ptr, size);


	// create name for the cap file
	char name[CF_NAME_LEN] = "mmap";
	int prefix_size = sizeof("mmap") - 1;
	int current_id = counter;
	int remain_space = CF_NAME_LEN - prefix_size - 1;
	char tmp[CF_NAME_LEN];

	int target = 0;
	while (current_id > 0 && target < remain_space) {
		tmp[target] = '0' + current_id % 10;
		current_id /= 10;
		target++;
	}

	for (int j = 0; j < remain_space; j++) {
		// 11 -> i: 2 ; remain_space = 5 ; j = 3
		if (target + j >= remain_space) {
			name[prefix_size + j] = tmp[remain_space - j - 1];
		} else {
			name[prefix_size + j] = '0';
		}
	}
	

	name[CF_NAME_LEN - 1] = '\0';
	// printf("name: %s\n", name);

	cvm_regions[cvm_counter][i].ptr = ptr;
	cvm_regions[cvm_counter][i].size = size;
	strcpy(cvm_regions[cvm_counter][i].name, name);
	// host_cap_file_adv(ptr, size, name);

	int res = counter;
	counter++;

	return res;
}

/**
\brief Hostcall function to 'probe' a shared buffer. Intravisor retrives a shared buffer, creates a capability, and stores it at location
\param[in] location address inside cVM where to store the capability
\param[out] ret: always 0
*/
int host_cap_unmap(void **location) {
//todo: We should check that location and size are located inside the cVM that issued host_cap_file_prb 
#ifdef CF_DEBUG
	printf("MON: CF: probe for key, store at %p\n", location);
#endif
	printf("Host Cap Unmap: %p\n", *location);

	// find target cvm
	int cvm_counter = find_target_cvm(location);

	if (cvm_counter < 0) {
		return -1;
	}

	struct cap_files_store_s *target_region = cvm_regions[cvm_counter];

	int i;
	void *address = *location;
	for(i = 0; i < MAX_CF_FILES; i++) {
		// printf("CF[%d] %s %p %d\n", i, target_region[i].name, target_region[i].ptr, target_region[i].size);

		if(target_region[i].ptr == *location) {
			target_region[i].ptr = 0;
			target_region[i].size = 0;
			target_region[i].loc = 0;
			target_region[i].name[0] = 0;
		}
	}
	free(address);

	return 0;
}

int host_cap_list(unsigned long cmp_begin) {
//todo: We should check that location and size are located inside the cVM that issued host_cap_file_prb 
	pthread_mutex_lock(&cf_store_lock);

	// find target cvm
	int cvm_counter = find_target_cvm((void **)cmp_begin);

	if (cvm_counter < 0) {
		return -1;
	}

	printf("---------- Looking up CVM%d Regions ----------\n", cvm_counter);

	int i = 0;
	int flag = 0;

	printf("Regions      name      address   size\n");
	for(; i < MAX_CF_FILES; i++) {
		struct cap_files_store_s *cf = &cvm_regions[cvm_counter][i];
		if (cf->ptr != 0) {
			flag = 1;
			printf("Region[%d] %s %p %d\n", i, cf->name, cf->ptr, cf->size);
		}
	}

	if (!flag) {
		printf("No region found\n");
	}

	printf("---\n\n");

	pthread_mutex_unlock(&cf_store_lock);

	return 0;
}