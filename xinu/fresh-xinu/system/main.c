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

	// basic_test();
	//delete_test();
	// write_test();
	// multi_open_test();
	// size_test();
	// directory_test();
	// remove_dir_test();
	updated_sizing_test();

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
