#include <xinu.h>
/*
 *  rfstests.c - File containing various tests for the remote file system.
 */

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
    control(RFILESYS, RFS_CTL_DEL, filename, 0);
    kprintf("Seeking to the beginning of the file and reading...\n");

    seek(dp, 0);
    memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

	kprintf("The read message from %s was %s\n", filename, read_msg);
    kprintf("\n");

    freemem(read_msg, 64);
    
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
    uint32 size = control(RFILESYS, RFS_CTL_SIZE, "newfile2", 0);
    kprintf("The returned size is %u bytes\n", size);

    kprintf("Option 2) calling on rfl pseudo-device directly:\n");
    size = control(dp, RFS_CTL_SIZE, 0, 0);
    kprintf("The returned size is %u bytes\n", size);

    kprintf("Going to try calling control() to get size of a rfl device that has been closed...\n");
    close(dp);

    size = control(dp, RFS_CTL_SIZE, 0, 0);
    kprintf("The returned size is %u bytes, should be %u\n", size, SYSERR);

    kprintf("-----------------Finished size_test-----------------\n");
    return;
}

/*
 *  directory_test - a test to open a directory and iterate over its contents
 */
void directory_test() {
    kprintf("-----------------Beginning directory_test-----------------\n");
    kprintf("Creating a new directory called \"mydir\"\n");
    uint32 status = control(RFILESYS, RFS_CTL_MKDIR, "mydir", 0);
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
    kprintf("-----------------Finished directory_test-----------------\n");
}

/*
 *  remove_dir_test - a test to delete a directory, or to confirm failure when
 *  directory is not empty.
 */
void remove_dir_test() {
    kprintf("-----------------Beginning remove_dir_test-----------------\n");
    kprintf("Creating a new directory called \"mydirtoremove\"\n");
    uint32 status = control(RFILESYS, RFS_CTL_MKDIR, "mydirtoremove", 0);
    if (status == SYSERR) {
        kprintf("ERROR with creating directory!\n");
    }
    kprintf("\n");

    kprintf("We will now try to delete this directory...\n");
    status = control(RFILESYS, RFS_CTL_RMDIR, "mydirtoremove", 0);
    if (status == SYSERR) {
        kprintf("ERROR with removing directory!\n");
    }

    kprintf("Creating a new directory called \"mydirtoremove\"\n");
    status = control(RFILESYS, RFS_CTL_MKDIR, "mydirtoremove", 0);
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

    kprintf("We will now try to delete this directory...\n");
    status = control(RFILESYS, RFS_CTL_RMDIR, "mydirtoremove", 0);
    if (status == SYSERR) {
        kprintf("ERROR with removing directory!\n");
    }

    kprintf("-----------------Finished remove_dir_test-----------------\n");
}

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
    control(RFILESYS, RFS_CTL_SIZE, filename, 0);
    kprintf("Size is %u bytes (should be same as before)!\n", (&rfltab[0])->rfsize );

    kprintf("Calling from device directly, now should update...\n");
    control(dp, RFS_CTL_SIZE, filename, 0);
    kprintf("Size is %u bytes (should be different now)!\n", (&rfltab[0])->rfsize);

    kprintf("Reading again...\n");
    seek(dp, 0);
    memset(read_msg, 0, 64);
    read(dp, read_msg, 64);
    kprintf("The read message from %s was %s\n", filename, read_msg);
    
    kprintf("-----------------Finished %s-----------------\n", test_name);
    return;
}