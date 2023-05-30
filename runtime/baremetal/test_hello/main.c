#include "crt.h"
#include "hostcalls.h"

#define Log(buf) \
	host_write("test_hello: ", sizeof("test_hello: ")); \
	host_write(buf, sizeof(buf));

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
	char cap[16];
	char cap_file[16];
	long size = 1024;

	Log("DEMO test_hello starts\n");

	Log("---------- Memory allocation part ----------\n");

	Log("Let's look the memory regions of this demo\n");

	host_cap_list();

	Log("Let's map a memory region named 'cap'\n");

	int id = host_cap_map((void **)&cap, &size);

	host_cap_list();

	char hello[] = "Dynamic demo\n\n";

	char cap_name[10]; 

	build_char_id(cap_name, id, 10);


	Log("Let's probe the capability\n");

	host_cap_prb(cap_name, cap, &size);

	Log("Let's write \"Dynamic demo\" into the memory region\n");

	copy_to_cap(cap, hello, sizeof(hello));

	Log("Let's read the capability: ");

	char dst[256];
	copy_from_cap(dst, cap, 0x20);
	host_write(dst, 256);

	Log("Let's unmap this memory region\n");

	host_cap_unmap((void **)&cap);

	host_cap_list();


	Log("---------- Security part ----------\n");


	Log("Let's get 'test1', the cap_file shared between cvms\n");

	host_cap_file_prb("test1", cap_file, &size);

	Log("Let's write the content of 'cap' to the cap_file 'test1'\n");

	copy_to_cap(cap_file, cap, 16);

	Log("Start looping and wating for test_mmap...\n\n");
	while (1) {}

	host_cap_unmap(&cap);

	host_exit();
}

