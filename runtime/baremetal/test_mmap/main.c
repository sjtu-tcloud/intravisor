#include "crt.h"
#include "hostcalls.h"

#define MSG "DEMO test_mmap starts\n"
#define Log(buf) \
	host_write("test_mmap: ", sizeof("test_mmap: ")); \
	host_write(buf, sizeof(buf));

void build_address_string(char *buf, char *cap, long size) {
	int i;
	buf[0] = '0';
	buf[1] = 'x';
	for (i = 0; i < size; i++) {
		char num = (cap[i] & 0xf0) >> 4;
		buf[2 * i + 2] = (num <= 9 && num >= 0) ? num + '0' : num - 10 + 'a';
		num = cap[i] & 0x0f;
		buf[2 * i + 3] = (num <= 9 && num >= 0) ? num + '0' : num - 10 + 'a';
	}
	buf[2 * i] = '\n';
	buf[2 * i + 1] = '\0';
}

int fill_buffer(char *buf, const char *content, int size) {
	int i;
	for (i = 0; i < size; i++) {
		buf[i] = content[i];
	}
	return i;
}

void build_char_id(char *buf, int id, int size) {
	int i;
	buf[0] = 'm';
	buf[1] = 'm';
	buf[2] = 'a';
	buf[3] = 'p';

	for (i = size - 2; i >= 4; i--) {
		buf[i] = '0' + id % 10;
		id /= 10;
	}

	buf[size-1] = '\0';
}

void hello_c() {
	char buf[32];
	char cap[16]; //place to store the capability
	char target[16];
	long size = 1024;

	Log(MSG);

	Log("Wating for test_hello to be looping...\n");

	// wating for the other cvm to be ready
	for (int i = 0; i < 100000000; i++) {}

	// Log("Let's look the memory regions of this demo\n");

	// host_cap_list();

	// Log("Let's map a memory region\n");

	// int id = host_cap_map((void **)&cap, &size);

	// host_cap_list();

	// char hello[] = "Dynamic demo\n\n";

	// char cap_name[10]; 

	// build_char_id(cap_name, id, 10);


	// Log("Let's probe the capability\n");

	// host_cap_prb(cap_name, cap, &size);

	// Log("Let's write \"Dynamic demo\" into the memory region\n");

	// copy_to_cap(cap, hello, sizeof(hello));

	// Log("Let's read the capability: ");

	char dst[256];
	// copy_from_cap(dst, cap, 0x20);
	// host_write(dst, 256);

	// Log("Let's unmap this memory region\n");

	// host_cap_unmap((void **)&cap);

	// host_cap_list();

	Log("Let's probe the 'test1' capability\n");

	host_cap_file_prb("test1", cap, &size);


	Log("For security, test_mmap can' t get the memory regions of other cvm\n");
	Log("Even if test_mmap get the capability content, it also can' t access the memory region\n");
	Log("Let's look at this demo\n");

	Log("Just copy the capability content:in 'test1' into our \"capability\"\n");

	copy_from_cap(target, cap, 16);

	Log("Let's read the capability. An In-address space security exception will generate\n");

	copy_from_cap(dst, target, 0x20);
	host_write(dst, 32);
	

	host_exit();
}

