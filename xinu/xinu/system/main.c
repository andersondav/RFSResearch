/*  main.c  - main */

#include <xinu.h>

process	main(void)
{
    
    	kprintf("\nHello World!\n");
    	kprintf("\nI'm the first XINU app and running function main() in system/main.c.\n");
    	kprintf("\nI was created by nulluser() in system/initialize.c using create().\n");
    	kprintf("\nMy creator will turn itself into the do-nothing null process.\n");
    	kprintf("\nI will create a second XINU app that runs shell() in shell/shell.c as an example.\n");
    	kprintf("\nYou can do something else, or do nothing; it's completely up to you.\n");
    	kprintf("\n...creating a shell\n");

	/* Run the Xinu shell */
	recvclr();
	kprintf("Hello Anderson David!\n");

	// uint32 xxx;
	// xxx = getlocalip();
	// kprintf("The local ip is %u\n", xxx);
	// kprintf("Please enter IP address:\n");

	// int x = 0;
	// char * nums[4];
	// char c;
	// for (int i = 0; i < 4; i++) {
	// 	nums[i] = getmem(4);
	// 	nums[i][3] = '\0';
	// }

	// while (x < 12) {
	// 	char c = getc(CONSOLE);
	// 	int num, offset;
	// 	num = x / 3;
	// 	offset = x % 3;
	// 	nums[num][offset] = c;
	// 	x++;
	// }
	// kprintf("\n\nYour entered ip is %s.%s.%s.%s\n\n", nums[0], nums[1], nums[2], nums[3]);
	// unsigned int c0, c1, c2, c3;
	// c0 = atoi(nums[0]);
	// c1 = atoi(nums[1]);
	// c2 = atoi(nums[2]);
	// c3 = atoi(nums[3]);
	// kprintf("Your entered ip is %d.%d.%d.%d\n", c0, c1, c2, c3);
	// uint32 ip = (uint32) c3+c2*256+c1*256*256+c0*256*256*256;
	// kprintf("In integer form, your ip is %u\n\n", ip);

	int dp = open(RFILESYS, "newfile", "rw");

	char * msg = "Hello World!\0";
	write(dp, msg, strlen(msg));
	seek(dp, 0);

	char * read_msg = getmem(64);
	memset(read_msg, 0, 64);
	read(dp, read_msg, 64);

	kprintf("The read message from newfile was %s\n", read_msg);
	kprintf("\n");

	int dp2 = open(RFILESYS, "newfile2", "rw");
	msg = "Hello World from file 2!\0";
	write(dp2, msg, strlen(msg));

	seek(dp2, 0);

	memset(read_msg, 0, 64);
	read(dp2, read_msg, 64);
	kprintf("The read message from newfile2 was %s\n", read_msg);
	kprintf("\n");

	kprintf("Obtaining file size without passing in arg, but which file I don't know...\n");
	uint32 size = control(RFILESYS, RFS_CTL_SIZE, "\0", 0);
	kprintf("File size is %u\n", size);
	kprintf("\n");

	// close(dp);
	// kprintf("Closed newfile\n");
	// kprintf("\n");

	kprintf("Obtaining file size with passing in arg \"newfile2\", but which file I don't know...\n");
	size = control(RFILESYS, RFS_CTL_SIZE, "newfile2", 0);
	kprintf("File size is %u\n", size);
	kprintf("\n");

	kprintf("Obtaining file size with passing in arg \"newfile\", but which file I don't know...\n");
	size = control(RFILESYS, RFS_CTL_SIZE, "newfile", 0);
	kprintf("File size is %u\n", size);
	kprintf("\n");

	kprintf("Trying to call control on the remote file newfile2 directly...\n");
	size = control(dp2, RFS_CTL_SIZE, 0, 0);
	kprintf("File size is %u\n", size);
	kprintf("\n");

	kprintf("Trying to call control on the remote file newfile directly...\n");
	size = control(dp, RFS_CTL_SIZE, 0, 0);
	kprintf("File size is %u\n", size);
	kprintf("\n");

	kprintf("Trying a different control call on remote file newfile2...Should fail\n");
	uint32 status = control(dp2, RFS_CTL_DEL, 0, 0);
	kprintf("Status is %s\n", (status == SYSERR ? "SYSERR" : "OK"));

	// kprintf("Closing newfile2...\n");
	// close(dp2);
	// kprintf("\n");

	// kprintf("Re-opening newfile2, but now it will be the only file open and should take the first spot in the rfctlblk array");
	// dp2 = open(RFILESYS, "newfile2", "rw");
	// kprintf("Obtaining file size, should now be of newfile2\n");
	// size = control(RFILESYS, RFS_CTL_SIZE, 0, 0);
	// kprintf("File size is %u\n", size);
	// kprintf("\n");
	// recvclr();
	// resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	// while (TRUE) {
	// 	receive();
	// 	sleepms(200);
	// 	kprintf("\n\nMain process recreating shell\n\n");
	// 	resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	// }
	return OK;
    
}
