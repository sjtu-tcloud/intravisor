//local store for capabilitites, relative address usualy provided via AUX
unsigned long local_cap_store = 0xe001000;


// hostcalls

int host_write(char *ptr, int size);
int host_cap_prb(char *key, void *location, long *size);
int host_cap_file_prb(char *key, void *location, long *size);
int host_cap_map(void **location, long *size);
int host_cap_unmap(void **location);
int host_cap_list();
void host_exit();

extern void copy_to_cap(void *dst_cap_location, void *src, int len);
extern void copy_from_cap(void *dst, void *src_cap_location, int len);