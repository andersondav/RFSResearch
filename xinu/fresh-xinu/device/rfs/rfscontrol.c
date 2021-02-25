/* rfscontrol.c - rfscontrol */

#include <xinu.h>

#define isRFILE(N) ((N >= RFILE0 && N <= RFILE9) ? 1 : 0)

/*------------------------------------------------------------------------
 * rfscontrol  -  Provide control functions for the remote file system
 *------------------------------------------------------------------------
 */
devcall	rfscontrol (
	 struct dentry	*devptr,	/* Entry in device switch table	*/
	 int32	func,			/* A control function		*/
	 int32	arg1,			/* Argument #1			*/
	 int32	arg2			/* Argument #2			*/
	)
{
	int32	len;			/* Length of name		*/
	struct	rf_msg_sreq msg;	/* Buffer for size request	*/
	struct	rf_msg_sres resp;	/* Buffer for size response	*/
	struct	rflcblk	*rfptr;		/* Pointer to entry in rfltab	*/
	char	*to, *from;		/* Used during name copy	*/
	int32	retval;			/* Return value			*/

	/* Wait for exclusive access */

	wait(Rf_data.rf_mutex);

	/* control function only available to rfl device if func is RFS_CTL_SIZE */
	if ( (isRFILE(devptr->dvnum) && (func != RFS_CTL_SIZE || (struct rflcblk *) (&rfltab[devptr->dvminor])->rfstate == RF_FREE)) ) {
		signal(Rf_data.rf_mutex);
		return SYSERR;
	}
	else if (devptr->dvnum == RFILESYS && func == RFS_CTL_SIZE) { /* if file size request, populate name field from arg 1 */
		/* set request file name to file name passed in arg1 */
		from = (char *) arg1;
		to = msg.rf_name;
		memset(to, NULLCH, RF_NAMLEN);
		len = 0;
		while ( (*to++ = *from++) ) {
			len++;
			if (len >= (RF_NAMLEN - 1) ) {
				signal(Rf_data.rf_mutex);
				return SYSERR;
			}
		}
	}
	else { /* populate name normally */
		/* Check length and copy (needed for size) */
		rfptr = &rfltab[devptr->dvminor];
		from = rfptr->rfname;
		to = msg.rf_name;
		len = 0;
		memset(to, NULLCH, RF_NAMLEN);	/* Start name as all zeroes	*/
		while ( (*to++ = *from++) ) {	/* Copy name to message		*/
			len++;
			if (len >= (RF_NAMLEN - 1) ) {
				signal(Rf_data.rf_mutex);
				return SYSERR;
			}
		}
	}

	

	switch (func) {

	/* Delete a file */

	case RFS_CTL_DEL:
		if (rfsndmsg(RF_MSG_DREQ, (char *)arg1) == SYSERR) {
			signal(Rf_data.rf_mutex);
			return SYSERR;
		}
		break;

	/* Truncate a file */

	case RFS_CTL_TRUNC:
		if (rfsndmsg(RF_MSG_TREQ, (char *)arg1) == SYSERR) {
			signal(Rf_data.rf_mutex);
			return SYSERR;
		}
		break;



	/* Make a directory */

	case RFS_CTL_MKDIR:
		if (rfsndmsg(RF_MSG_MREQ, (char *)arg1) == SYSERR) {
			signal(Rf_data.rf_mutex);
			return SYSERR;
		}
		break;

	/* Remove a directory */

	case RFS_CTL_RMDIR:
		if (rfsndmsg(RF_MSG_XREQ, (char *)arg1) == SYSERR) {
			signal(Rf_data.rf_mutex);
			return SYSERR;
		}
		break;

	/* Obtain current file size (non-standard message size) */

	case RFS_CTL_SIZE:

		/* Hand-craft a size request message */

		msg.rf_type = htons(RF_MSG_SREQ);
		msg.rf_status = htons(0);
		msg.rf_seq = 0;		/* Rfscomm will set the seq num	*/

		/* Send the request to server and obtain a response	*/

		retval = rfscomm( (struct rf_msg_hdr *)&msg,
					sizeof(struct rf_msg_sreq),
				  (struct rf_msg_hdr *)&resp,
					sizeof(struct rf_msg_sres) );
		if ( (retval == SYSERR) || (retval == TIMEOUT) ) {
			signal(Rf_data.rf_mutex);
			return SYSERR;
		} else {
			/* update file size if called by file device directly */
			if (isRFILE(devptr->dvnum)) {
				(&rfltab[devptr->dvminor])->rfsize = ntohl(resp.rf_size);
				kprintf("Updated file size to be %u bytes\n", (&rfltab[devptr->dvminor])->rfsize);
			}

			signal(Rf_data.rf_mutex);
			return ntohl(resp.rf_size);
		}

	default:
		kprintf("rfscontrol: function %d not valid\n", func);
		signal(Rf_data.rf_mutex);
		return SYSERR;
	}

	signal(Rf_data.rf_mutex);
	return OK;
}
