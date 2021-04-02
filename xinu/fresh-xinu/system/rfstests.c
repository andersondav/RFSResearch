#include <xinu.h>
/*
 *  rfstests.c - File containing various tests for the remote file system.
 */

/*
 *  num_digits - helper function to get number of digits a number has
 */
int num_digits(int num) {
    uint32 result = 0;
    while (num != 0) {
        result++;
        num /= 10;
    }

    return result;
}

/*
 *  message_generator - helper function for generating file contents
 */
char * message_generator(uint32 count, char * filename) {
    char * msg = (char *) getmem(count);
    uint32 bytes_written = 0;
    char * to = msg;

    char * format = "Message for file %s (%d bytes):\n";

    sprintf(to, format, filename, count);

    bytes_written = strlen(format)-4 + strlen(filename) + num_digits(count);
    // kprintf("%d bytes written\n", bytes_written);
    to += bytes_written;
    // kprintf("A random char is %c\n", (char) (rand()%26) + 'a');
    for (int i = bytes_written; i < count-1; i++) {
        *to++ = (char) (rand()%26) + 'a';
    }
    *to++ = '\0';
    return msg;
}

/*
 *  basic_test - a basic test to open, write, and read from a remote file.
 */
void basic_test() {
    kprintf("-----------------Beginning basic_test-----------------\n");
    kprintf("Opened a file called newfile\n");
    int dp = open(RFILESYS, "newfile", "rw");
    if (dp == SYSERR) {
        kprintf("Open failed.\n");
        return;
    }

    kprintf("Writing a basic message \"Hello World!\" to newfile\n");
	char * msg = "Hello World!\0";
	write(dp, msg, strlen(msg));
	seek(dp, 0);

	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

    kprintf("Seeking to the beginning of the file and reading...\n");
	kprintf("The read message from newfile was %s\n", read_msg);

    freemem(read_msg, 64);
    close(dp);
    
    kprintf("-----------------Finished basic_test-----------------\n");
    return;
}

/*
 *  write_test - a test to explore how writing to a file is implemented
 *  in regards to overwriting data
 */
void write_test() {
    kprintf("-----------------Beginning write_test-----------------\n");
    kprintf("Opened a file called write_test_file\n");
    int dp = open(RFILESYS, "write_test_file", "rw");
    if (dp == SYSERR) {
        kprintf("Open failed.\n");
        return;
    }

    kprintf("Writing a basic message \"Hello write_test_file!\" to write_test_file\n");
	char * msg = "Hello World!\0";
	write(dp, msg, strlen(msg));
	seek(dp, 0);

	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

    kprintf("Seeking to the beginning of the file and reading...\n");
	kprintf("The read message from write_test_file was %s\n", read_msg);
    kprintf("\n");

    kprintf("Now going to seek again to the beginning, and overwrite a shorter message...\n");
    seek(dp, 0);
    msg = "Short\0";
    write(dp, msg, strlen(msg));

    seek(dp, 0);
    memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

    kprintf("Seeking to the beginning of the file and reading...\n");
	kprintf("The read message from write_test_file was %s\n", read_msg);
    kprintf("\n");

    freemem(read_msg, 64);
    
    kprintf("-----------------Finished write_test-----------------\n");
    return;
}

/*
 *  delete_test() - a test to see what happens when deleting an open file
 */
void delete_test() {
    char * test_name = "delete_test";
    char * filename = "delete_test_file";
    char * message = "Hello delete_test_file!";
    kprintf("-----------------Beginning %s-----------------\n", test_name);
    kprintf("Opened a file called %s\n", filename);
    int dp = open(RFILESYS, filename, "rw");
    if (dp == SYSERR) {
        kprintf("Open failed.\n");
        return;
    }

    kprintf("Writing a basic message \"%s\" to %s\n", message, filename);
	write(dp, message, strlen(message));

    kprintf("Seeking to the beginning of the file and reading...\n");

    seek(dp, 0);
	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

	kprintf("The read message from %s was %s\n", filename, read_msg);

    kprintf("Now deleting the file...\n");
    control(RFILESYS, RFS_CTL_DEL, (int) filename, 0);
    kprintf("Seeking to the beginning of the file and reading...\n");

    seek(dp, 0);
    memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

	kprintf("The read message from %s was %s\n", filename, read_msg);
    kprintf("\n");

    freemem(read_msg, 64);
    close(dp);
    
    kprintf("-----------------Finished %s-----------------\n", test_name);
    return;
}

/*
 *  multi_open_test - a test to open a file, do some operations on it, then close and re-open it.
 */
void multi_open_test() {
    kprintf("-----------------Beginning multi_open_test-----------------\n");
    kprintf("Opened a file called newfile\n");
    int dp = open(RFILESYS, "newfile", "rw");
    if (dp == SYSERR) {
        kprintf("Open failed.\n");
        return;
    }

    kprintf("Writing a basic message \"Hello World!\" to newfile\n");
	char * msg = "Hello World!\0";
	write(dp, msg, strlen(msg));
	seek(dp, 0);

	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

    kprintf("Seeking to the beginning of the file and reading...\n");
	kprintf("The read message from newfile was %s\n", read_msg);

    freemem(read_msg, 64);

    kprintf("Closing the file...\n");
    close(dp);

    kprintf("Reopening the file and reading...\n");
    dp = open(RFILESYS, "newfile", "rw");

    memset(read_msg, 0, 64);
    read(dp, read_msg, 64);
    kprintf("The read message from newfile was %s\n", read_msg);

    freemem(read_msg, 64);
    close(dp);
    
    kprintf("-----------------Finished multi_open_test-----------------\n");
    return;
}

/*
 *  size_test - a test to retrieve the size of a file in 2 ways.
 *    1) call on RFILESYS with specified filename as argument
 *    2) call on rfl pseudo-device directly
 */
void size_test() {
    kprintf("-----------------Beginning size_test-----------------\n");
    kprintf("Opened a file called newfile2\n");
    int dp = open(RFILESYS, "newfile2", "rw");
    if (dp == SYSERR) {
        kprintf("Open failed.\n");
        return;
    }

    kprintf("Writing a basic message \"Hello World a second time!\" to newfile2\n");
	char * msg = "Hello World a second time!\0";
	write(dp, msg, strlen(msg));
	seek(dp, 0);

	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

    kprintf("Seeking to the beginning of the file and reading...\n");
	kprintf("The read message from newfile was %s\n", read_msg);

    kprintf("The size of newfile2 should be %u bytes\n", strlen(read_msg));
    freemem(read_msg, 64);
    kprintf("\n");

    kprintf("Option 1) calling on RFILESYS with specified filename as arg:\n");
    uint32 size = control(RFILESYS, RFS_CTL_SIZE, (int) "newfile2", 0);
    kprintf("The returned size is %u bytes\n", size);

    kprintf("Option 2) calling on rfl pseudo-device directly:\n");
    size = control(dp, RFS_CTL_SIZE, 0, 0);
    kprintf("The returned size is %u bytes\n", size);

    kprintf("Going to try calling control() to get size of a rfl device that has been closed...\n");
    close(dp);

    size = control(dp, RFS_CTL_SIZE, 0, 0);
    kprintf("The returned size is %u bytes, should be %u\n", size, SYSERR);

    freemem(read_msg, 64);
    close(dp);

    kprintf("-----------------Finished size_test-----------------\n");
    return;
}

/*
 *  directory_test - a test to open a directory and iterate over its contents
 */
void directory_test() {
    kprintf("-----------------Beginning directory_test-----------------\n");
    kprintf("Creating a new directory called \"mydir\"\n");
    uint32 status = control(RFILESYS, RFS_CTL_MKDIR, (int) "mydir", 0);
    if (status == SYSERR) {
        kprintf("ERROR with creating directory!\n");
    }
    kprintf("\n");

    kprintf("Creating a new file in the created directory\n");
    int dp = open(RFILESYS, "mydir/mydirsfile", "rw");
    if (dp == SYSERR) {
        kprintf("ERROR with opening file!\n");
    }
    kprintf("\n");

    kprintf("Writing a basic message \"Hello World from inside the directory!\" to the file\n");
	char * msg = "Hello World from inside the directory!\0";
	write(dp, msg, strlen(msg));
	seek(dp, 0);
    kprintf("\n");

	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

    kprintf("Seeking to the beginning of the file and reading...\n");
	kprintf("The read message from the file was %s\n", read_msg);

    freemem(read_msg, 64);

    kprintf("We will now loop through the files in this directory...\n");
    int dp2 = open(RFILESYS, "mydir", "rw");
    memset(read_msg, 0, 64);

    read(dp2, read_msg, 64);
    while (strlen(read_msg) != 0) {
        kprintf("Found %s in mydir\n");
        memset(read_msg, 0, 64);
        read(dp2, read_msg, 64);
    }

    freemem(read_msg, 64);
    close(dp);
    close(dp2);

    kprintf("-----------------Finished directory_test-----------------\n");
}

/*
 *  remove_dir_test - a test to delete a directory, or to confirm failure when
 *  directory is not empty.
 */
void remove_dir_test() {
    kprintf("-----------------Beginning remove_dir_test-----------------\n");
    kprintf("Creating a new directory called \"mydirtoremove\"\n");
    uint32 status = control(RFILESYS, RFS_CTL_MKDIR, (int) "mydirtoremove", 0);
    if (status == SYSERR) {
        kprintf("ERROR with creating directory!\n");
    }
    kprintf("\n");

    kprintf("We will now try to delete this directory...\n");
    status = control(RFILESYS, RFS_CTL_RMDIR, (int) "mydirtoremove", 0);
    if (status == SYSERR) {
        kprintf("ERROR with removing directory!\n");
    }

    kprintf("Creating a new directory called \"mydirtoremove\"\n");
    status = control(RFILESYS, RFS_CTL_MKDIR, (int) "mydirtoremove", 0);
    if (status == SYSERR) {
        kprintf("ERROR with creating directory!\n");
    }
    kprintf("\n");

    kprintf("Creating a new file in the created directory\n");
    int dp = open(RFILESYS, "mydirtoremove/mydirsfile", "rw");
    if (dp == SYSERR) {
        kprintf("ERROR with opening file!\n");
    }
    kprintf("\n");

    kprintf("Writing a basic message \"Hello World from inside the directory!\" to the file\n");
	char * msg = "Hello World from inside the directory!\0";
	write(dp, msg, strlen(msg));
	seek(dp, 0);
    kprintf("\n");

	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

    kprintf("Seeking to the beginning of the file and reading...\n");
	kprintf("The read message from the file was %s\n", read_msg);
    kprintf("\n");

    freemem(read_msg, 64);
    close(dp);

    kprintf("We will now try to delete this directory...\n");
    status = control(RFILESYS, RFS_CTL_RMDIR, (int) "mydirtoremove", 0);
    if (status == SYSERR) {
        kprintf("ERROR with removing directory!\n");
    }

    kprintf("-----------------Finished remove_dir_test-----------------\n");
}

/*
 *  updated_sizing_test - a test to see if reading now returns file size
 */
void updated_sizing_test() {
    char * test_name = "updated_sizing_test";
    char * filename = "updated_sizing_file";
    char * message = "Hello updated_sizing_file!";
    kprintf("-----------------Beginning %s-----------------\n", test_name);
    kprintf("Opened a file called %s\n", filename);
    int dp = open(RFILESYS, filename, "rw");
    if (dp == SYSERR) {
        kprintf("Open failed.\n");
        return;
    }

    kprintf("Writing a basic message \"%s\" to %s\n", message, filename);
	write(dp, message, strlen(message));

    kprintf("Seeking to the beginning of the file and reading...\n");

    seek(dp, 0);
	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

	kprintf("The read message from %s was %s\n", filename, read_msg);

    kprintf("Sleeping for 20 secs while I append to file directly...\n");
    sleepms(20000);

    kprintf("Now going to retrieve file size to see if it updates...\n");
    kprintf("Calling from RFILESYS, shouldn't update from here...\n");
    control(RFILESYS, RFS_CTL_SIZE, (int) filename, 0);
    kprintf("Size is %u bytes (should be same as before)!\n", (&rfltab[0])->rfsize );

    kprintf("Calling from device directly, now should update...\n");
    control(dp, RFS_CTL_SIZE, (int) filename, 0);
    kprintf("Size is %u bytes (should be different now)!\n", (&rfltab[0])->rfsize);

    kprintf("Reading again...\n");
    seek(dp, 0);
    memset(read_msg, 0, 64);
    read(dp, read_msg, 64);
    kprintf("The read message from %s was %s\n", filename, read_msg);

    freemem(read_msg, 64);
    close(dp);
    
    kprintf("-----------------Finished %s-----------------\n", test_name);
    return;
}

/*
 *  cache_test1 - a basic test to ensure reading works with cache implementation
 */
void cache_test1() {
    char * test_name = "cache_test1";
    char * filename = "cache_test1_file";
    char * message = "Hello cache_test1_file!";
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    kprintf("Going to write %s into the file...\n", message);
    uint32 status = write(dp, message, strlen(message));
    if (status == SYSERR) {
        kprintf("Error with write()!\n");
        return;
    }

    kprintf("Seeking back to beginning of file...\n");
    seek(dp, 0);

    char * read_msg = getmem(64);
	memset(read_msg, 0, 64);

    kprintf("Reading from file...\n");
    status = read(dp, read_msg, 64);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Read %s\n", read_msg);

    freemem(read_msg, 64);
    close(dp);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/*
 *  cache_test2 - a test to ensure cached data persists
 */
void cache_test2() {
    char * test_name = "cache_test2";
    char * filename = "cache_test2_file";
    char * message = "Hello cache_test2_file!";
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    kprintf("Going to write %s into the file...\n", message);
    uint32 status = write(dp, message, strlen(message));
    if (status == SYSERR) {
        kprintf("Error with write()!\n");
        return;
    }

    kprintf("Seeking back to beginning of file...\n");
    seek(dp, 0);

    char * read_msg = getmem(64);
	memset(read_msg, 0, 64);

    kprintf("Reading from file...\n");
    status = read(dp, read_msg, 64);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Read %s\n", read_msg);

    kprintf("Sleeping for 5s so we can see server output\n");
    sleepms(5000);

    kprintf("Seeking back to beginning of file, should read from cache now...\n");
    seek(dp, 0);

	memset(read_msg, 0, 64);

    kprintf("Reading from file...\n");
    status = read(dp, read_msg, 64);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Read %s\n", read_msg);

    freemem(read_msg, 64);
    close(dp);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/*
 *  cache_test3 - a test for reading from multiple cache blocks
 */
void cache_test3() {
    char * test_name = "cache_test3";
    char * filename = "cache_test3_file";
    char * message = message_generator(1201, filename);
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    kprintf("Going to write %s\ninto the file...\n", message);

    /* first 1000 bytes */
    uint32 status = write(dp, message, 1000);
    if (status == SYSERR) {
        kprintf("Error with write()!\n");
        return;
    }

    /* last 201 bytes */
    status = write(dp, message+1000, 201);
    if (status == SYSERR) {
        kprintf("Error with write()!\n");
        return;
    }

    kprintf("Seeking back to beginning of file...\n");
    seek(dp, 0);

    char * read_msg = getmem(64);
	memset(read_msg, 0, 64);

    kprintf("Reading from file (block 0)...\n");
    status = read(dp, read_msg, 64);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Read %s\n", read_msg);
    kprintf("Expected to read ");
    for (int i = 0; i<64; i++) {
        kprintf("%c", message[i]);
    }
    kprintf("\n");

    kprintf("Seeking to somewhere in the second block...\n");
    seek(dp, 1024);

	memset(read_msg, 0, 64);

    kprintf("Reading from file (block 1)...\n");
    status = read(dp, read_msg, 64);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Read %s\n", read_msg);
    kprintf("Expected to read ");
    for (int i = 1024; i<1088; i++) {
        kprintf("%c", message[i]);
    }
    kprintf("\n");

    freemem(read_msg, 64);
    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/*
 *  cache_test4 - a test for performing a read that requires 2 cache blocks
 */
void cache_test4() {
    char * test_name = "cache_test4";
    char * filename = "cache_test4_file";
    char * message = message_generator(2048, filename);
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    kprintf("Going to write %s\ninto the file...\n", message);

    /* first 1024 bytes */
    uint32 status = write(dp, message, 1024);
    if (status == SYSERR) {
        kprintf("Error with write()!\n");
        return;
    }

    /* last 1024 bytes */
    status = write(dp, message+1024, 1024);
    if (status == SYSERR) {
        kprintf("Error with write()!\n");
        return;
    }

    kprintf("Seeking to position 900...\n");
    seek(dp, 900);

    char * read_msg = getmem(512);
	memset(read_msg, 0, 512);

    kprintf("Reading bytes 900 through 1411...\n");
    status = read(dp, read_msg, 512);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Read %s\n", read_msg);
    kprintf("Expected to read ");
    for (int i = 900; i<1412; i++) {
        kprintf("%c", message[i]);
    }
    kprintf("\n");

    kprintf("Sleeping...\n");
    sleepms(10000);

    kprintf("Seeking to position 750...\n");
    seek(dp, 750);

	memset(read_msg, 0, 512);

    kprintf("Reading bytes 750 through 1261... (shouldn't require server)\n");
    status = read(dp, read_msg, 512);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Read %s\n", read_msg);
    kprintf("Expected to read ");
    for (int i = 750; i<1262; i++) {
        kprintf("%c", message[i]);
    }
    kprintf("\n");

    freemem(read_msg, 512);
    close(dp);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/*
 *  cache_test5 - a test for retrieving blocks in the linked list portion of the cache
 */
void cache_test5() {
    char * test_name = "cache_test5";
    char * filename = "cache_test5_file";

    /* In the array portion of the cache there are 10 blocks of size 1024 */
    /* so anything beyond 10*1024 = 10240 bytes will be cached in the list portion */
    char * message = message_generator(12288, filename);
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    uint32 status;
    kprintf("Going to write message into the file...\n");
    /* writing 1024 bytes at a time */
    for (int i = 0; i < 12; i++) {
        status = write(dp, message+(i*1024), 1024);
        if (status == SYSERR) {
            kprintf("Error with write()!\n");
            return;
        }
    }

    kprintf("Seeking to position 10300...\n");
    seek(dp, 10300);

    char * read_msg = getmem(512);
	memset(read_msg, 0, 512);

    kprintf("Reading bytes 10300 through 10811...\n");
    status = read(dp, read_msg, 512);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Read %s\n", read_msg);
    kprintf("Expected to read ");
    for (int i = 10300; i<10812; i++) {
        kprintf("%c", message[i]);
    }
    kprintf("\n");

    freemem(read_msg, 512);
    close(dp);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/*
 *  cache_test6 - a test that tests whether linked list portion of the cache maintains sorted order
 */
void cache_test6() {
    char * test_name = "cache_test6";
    char * filename = "cache_test6_file";

    /* In the array portion of the cache there are 10 blocks of size 1024 */
    /* so anything beyond 10*1024 = 10240 bytes will be cached in the list portion */
    uint32 num_blocks = 5;
    char * message = message_generator(10240 + num_blocks*1024-57, filename);
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    uint32 status;
    kprintf("Going to write message into the file...\n");
    /* writing 1024 bytes at a time */
    for (int i = 0; i < 10+num_blocks; i++) {
        status = write(dp, message+(i*1024), 1024 <= strlen(message+(i*1024)) ? 1024 : strlen(message+(i*1024)));
        if (status == SYSERR) {
            kprintf("Error with write()!\n");
            return;
        }
    }

    kprintf("Going to cache 2nd, then 1st, then 5th, then 3rd, then 4th list blocks...\n");
    uint32 order[5] = {1, 0, 4, 2, 3};
    for (int i=0; i < 5; i++) {
        kprintf("Seeking to position in block %d...\n", order[i]);

        uint32 read_start = 10240 + order[i]*1024 + 60;
        seek(dp, read_start);

        char * read_msg = getmem(512);
        memset(read_msg, 0, 512);

        kprintf("Reading bytes %d through %d...\n", read_start, read_start + 511);
        status = read(dp, read_msg, 512);
        if (status == SYSERR) {
            kprintf("Error with read()!\n");
            return;
        }

        // kprintf("Read %s\n", read_msg);
        // kprintf("Expected to read ");
        // for (int i = read_start; i<read_start + 512; i++) {
        //     kprintf("%c", message[i]);
        // }
        // kprintf("\n");

        kprintf("Comparison result: %s\n", strncmp(read_msg, &message[read_start], 512) == 0 ? "Equal!" : "Not equal!");

        freemem(read_msg, 512);
    }

    
    close(dp);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/*
 *  cache_test7 - a test that reads across multiple cache blocks and utilizes the cache list
 */
void cache_test7() {
    char * test_name = "cache_test7";
    char * filename = "cache_test7_file";

    /* In the array portion of the cache there are 10 blocks of size 1024 */
    /* so anything beyond 10*1024 = 10240 bytes will be cached in the list portion */
    uint32 num_blocks = 5;
    char * message = message_generator(10240 + num_blocks*1024-57, filename);
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    uint32 status;
    kprintf("Going to write message into the file...\n");
    /* writing 1024 bytes at a time */
    for (int i = 0; i < 10+num_blocks; i++) {
        status = write(dp, message+(i*1024), 1024 <= strlen(message+(i*1024)) ? 1024 : strlen(message+(i*1024)));
        if (status == SYSERR) {
            kprintf("Error with write()!\n");
            return;
        }
    }

    kprintf("Going to do a read that uses last array cache block and first list cache block\n");
    uint32 read_start = 10040;
    seek(dp, read_start);

    char * read_msg = getmem(750);
    memset(read_msg, 0, 750);

    kprintf("Reading bytes %d through %d...\n", read_start, read_start+749); 
    status = read(dp, read_msg, 750);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Comparison result: %s\n", strncmp(read_msg, &message[read_start], 750) == 0 ? "Equal!" : "Not equal!");

    kprintf("Going to do a read that uses firs and second list cache blocks\n");
    read_start = 11140;
    seek(dp, read_start);

    memset(read_msg, 0, 750);
    
    kprintf("Reading bytes %d through %d...\n", read_start, read_start+749); 
    status = read(dp, read_msg, 750);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Comparison result: %s\n", strncmp(read_msg, &message[read_start], 750) == 0 ? "Equal!" : "Not equal!");

    freemem(read_msg, 750);
    close(dp);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/*
 *  devnum_test - a test to ensure we can access a remote file's cache from a chosen block's rfl_devnum
 */
void devnum_test() {
    char * test_name = "devnum_test";
    char * filename = "devnum_test_file";

    /* In the array portion of the cache there are 10 blocks of size 1024 */
    /* so anything beyond 10*1024 = 10240 bytes will be cached in the list portion */
    uint32 num_blocks = 5;
    char * message = message_generator(10240 + num_blocks*1024-57, filename);
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    uint32 status;
    kprintf("Going to write message into the file...\n");
    /* writing 1024 bytes at a time */
    for (int i = 0; i < 10+num_blocks; i++) {
        status = write(dp, message+(i*1024), 1024 <= strlen(message+(i*1024)) ? 1024 : strlen(message+(i*1024)));
        if (status == SYSERR) {
            kprintf("Error with write()!\n");
            return;
        }
    }

    kprintf("Going to do a read to get a block in linked list portion of cache\n");
    uint32 read_start = 10040;
    seek(dp, read_start);

    char * read_msg = getmem(750);
    memset(read_msg, 0, 750);

    kprintf("Reading bytes %d through %d...\n", read_start, read_start+749); 
    status = read(dp, read_msg, 750);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Comparison result: %s\n", strncmp(read_msg, &message[read_start], 750) == 0 ? "Equal!" : "Not equal!");

    struct rfs_cblock *desired_block = (&rfltab[RFILE0-RFILE0])->cache_list;
    kprintf("Expected address of block: %d\n", (int) desired_block);
    kprintf("Address of block calculated from its devnum: %d\n", (int) (&rfltab[desired_block->rfl_devnum - RFILE0])->cache_list);

    kprintf("Going to do a read to get a block in array portion of cache\n");
    read_start = 0;
    seek(dp, read_start);

    memset(read_msg, 0, 750);
    
    kprintf("Reading bytes %d through %d...\n", read_start, read_start+749); 
    status = read(dp, read_msg, 750);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Comparison result: %s\n", strncmp(read_msg, &message[read_start], 750) == 0 ? "Equal!" : "Not equal!");

    desired_block = (&rfltab[RFILE0-RFILE0])->cache[0];
    kprintf("Expected address of block: %d\n", (int) desired_block);
    kprintf("Address of block calculated from its devnum: %d\n", (int) (&rfltab[desired_block->rfl_devnum - RFILE0])->cache[desired_block->file_start / RF_DATALEN]);

    freemem(read_msg, 750);
    close(dp);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/*
 *  lru_test1 - a test that tests the basic functionality of the rfs's lru list
 */
void lru_test1() {
    char * test_name = "lru_test1";
    char * filename = "lru_test1_file";
    char * filename2 = "lru_test1_file2";
    char * filename3 = "lru_test1_file3";

    /* In the array portion of the cache there are 10 blocks of size 1024 */
    /* so anything beyond 10*1024 = 10240 bytes will be cached in the list portion */
    uint32 num_blocks = 5;
    char * message = message_generator(10240 + num_blocks*1024-57, filename);
    char * message2 = message_generator(10240 + num_blocks*1024-89, filename2);
    char * message3 = message_generator(10240 + num_blocks*1024-43, filename3);
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    kprintf("Opening file named %s\n", filename2);
    uint32 dp2 = open(RFILESYS, filename2, "rw");
    if (dp2 == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    kprintf("Opening file named %s\n", filename3);
    uint32 dp3 = open(RFILESYS, filename3, "rw");
    if (dp3 == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    uint32 status;
    kprintf("Going to write messages into the files...\n");
    /* writing 1024 bytes at a time */
    for (int i = 0; i < 10+num_blocks; i++) {
        status = write(dp, message+(i*1024), 1024 <= strlen(message+(i*1024)) ? 1024 : strlen(message+(i*1024)));
        if (status == SYSERR) {
            kprintf("Error with write()!\n");
            return;
        }
    }

    /* writing 1024 bytes at a time */
    for (int i = 0; i < 10+num_blocks; i++) {
        status = write(dp2, message2+(i*1024), 1024 <= strlen(message2+(i*1024)) ? 1024 : strlen(message2+(i*1024)));
        if (status == SYSERR) {
            kprintf("Error with write()!\n");
            return;
        }
    }

    /* writing 1024 bytes at a time */
    for (int i = 0; i < 10+num_blocks; i++) {
        status = write(dp3, message3+(i*1024), 1024 <= strlen(message3+(i*1024)) ? 1024 : strlen(message3+(i*1024)));
        if (status == SYSERR) {
            kprintf("Error with write()!\n");
            return;
        }
    }

    /* reading blocks from both remote files */
    uint32 read_locs[9][2] = { {0, 3}, {1, 7}, {2, 4}, {1, 8}, {0, 1}, {2, 8}, {0, 3}, {1, 13}, {2, 9} };
    char * read_msg = getmem(512);

    for (uint32 i = 0; i < 9; i++) {
        uint32 file = read_locs[i][0] + RFILE0;
        uint32 byte = read_locs[i][1] * RF_DATALEN;

        memset(read_msg, 0, 512);
        kprintf("Reading file %d, block %d:\n", read_locs[i][0], read_locs[i][1]);
        char *comp_result;
        if (read_locs[i][0] == 0) {
            seek(dp, byte);
            read(dp, read_msg, 512);
            comp_result = strncmp(read_msg, &message[byte], 512) == 0 ? "Equal" : "Not equal";
        }
        else if (read_locs[i][0] == 1) {
            seek(dp2, byte);
            read(dp2, read_msg, 512);
            comp_result = strncmp(read_msg, &message2[byte], 512) == 0 ? "Equal" : "Not equal";
        }
        else {
            seek(dp3, byte);
            read(dp3, read_msg, 512);
            comp_result = strncmp(read_msg, &message3[byte], 512) == 0 ? "Equal" : "Not equal";
        }

        kprintf("Comparison result: %s\n", comp_result);
        print_lru_list();
    }

    
    freemem(read_msg, 512);
    close(dp);
    close(dp2);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}

/* 
 *  trunc_test - a test of improved truncation functionality
 */
void trunc_test() {
    char * test_name = "trunc_test";
    char * filename = "trunc_test";
    char * message = message_generator(2048, filename);
    kprintf("-----------------Beginning %s-----------------\n", test_name);

    kprintf("Opening file named %s\n", filename);
    uint32 dp = open(RFILESYS, filename, "rw");
    if (dp == NULL) {
        kprintf("Error with open()!\n");
        return;
    }

    kprintf("Going to write message into the file...\n");

    /* first 1024 bytes */
    uint32 status = write(dp, message, 1024);
    if (status == SYSERR) {
        kprintf("Error with write()!\n");
        return;
    }

    /* last 1024 bytes */
    status = write(dp, message+1024, 1024);
    if (status == SYSERR) {
        kprintf("Error with write()!\n");
        return;
    }

    kprintf("Seeking to position 900...\n");
    seek(dp, 900);

    char * read_msg = getmem(512);
	memset(read_msg, 0, 512);

    kprintf("Reading bytes 900 through 1411...\n");
    status = read(dp, read_msg, 512);
    if (status == SYSERR) {
        kprintf("Error with read()!\n");
        return;
    }

    kprintf("Comparison result: %s\n", strncmp(&message[900], read_msg, 512) == 0 ? "Equal" : "Not equal");

    kprintf("File size is %d\n", control(RFILESYS, RFS_CTL_SIZE, (int32) filename, 0));

    kprintf("Truncating file to be 67 bytes...\n");
    control(RFILESYS, RFS_CTL_TRUNC, (int32) filename, 67);
    
    kprintf("New file size is %d\n", control(dp, RFS_CTL_SIZE, 0, 0));

    kprintf("Truncating file to be 105 bytes...\n");
    control(dp, RFS_CTL_TRUNC, 105, 0);

    kprintf("New file size is %d\n", (&rfltab[0])->rfsize);
    kprintf("New file size is %d\n", control(dp, RFS_CTL_SIZE, 0, 0));

    freemem(read_msg, 512);
    close(dp);

    kprintf("-----------------Finished %s-----------------\n", test_name);
}