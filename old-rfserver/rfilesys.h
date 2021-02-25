/* rfilesys.h - definitions for remote file system pseudo-devices */

#ifndef	Nrfl
#define	Nrfl	10
#endif

/* Control block for a remote file pseudo-device */

#define	RF_NAMLEN	128		/* maximum length of file name	*/
#define	RF_DATALEN	1024		/* Maximum data in read or write*/
#define	RF_MODE_R	0x01		/* Bit to grant read access	*/
#define	RF_MODE_W	0x02		/* Bit to grant write access	*/
#define	RF_MODE_RW	0x03		/* Mask for read and write bits	*/
#define	RF_MODE_N	0x04		/* Bit to for "new" mode	*/
#define	RF_MODE_O	0x08		/* Bit to for "old" mode	*/
					/* (new and old are mutually	*/
					/* exclusive)			*/
#define	RF_MODE_NO	0x0C		/* Mask for "n" and "o" bits	*/

/* Global data for the remote server */

#ifndef	RF_SERVER_IP
#define	RF_SERVER_IP	"255.255.255.255"
#endif

#ifndef	RF_SERVER_PORT
#define	RF_SERVER_PORT	33123
#endif

#ifndef	RF_LOC_PORT
#define	RF_LOC_PORT	33123
#endif

struct	rfdata	{
	int32	rf_seq;			/* next sequence number to use	*/
	uint32	rf_ser_ip;		/* server IP address		*/
	uint16	rf_ser_port;		/* server UDP port		*/
	uint16	rf_loc_port;		/* local (client) UPD port	*/
	sid32	rf_mutex;		/* mutual exclusion for access	*/
	bool8	rf_registered;		/* has UDP port been registered?*/
};

extern	struct	rfdata	Rf_data;

/* Definition of the control block for a remote file pseudo-device */

#define	RF_FREE	0			/* Entry is currently unused	*/
#define	RF_USED	1			/* Entry is currently in use	*/

struct	rflcblk	{
	int32	rfstate;		/* entry is free or used	*/
	int32	rfdev;			/* device number of this dev.	*/
	char	rfname[RF_NAMLEN];	/* Name of the file		*/
	uint32	rfpos;			/* current file position	*/
	uint32	rfmode;			/* mode: read access, write	*/
					/*	access or both		*/
};

extern	struct	rflcblk	rfltab[];	/* remote file control blocks	*/

/* Definintions of parameters used during server access */

#define	RF_RETRIES	3		/* time to retry sending a msg	*/
#define	RF_TIMEOUT	1000		/* wait one second for a reply	*/

/* Control functions for a remote file pseudo device */

#define	RFS_CTL_DEL	1		/* Delete a file		*/
#define	RFS_CTL_TRUNC	2		/* Truncate a file		*/
#define	RFS_CTL_MKDIR	3		/* make a sirectory		*/
#define	RFS_CTL_RMDIR	4		/* remove a directory		*/
#define RFS_CTL_SIZE	5		/* Obtain the size of a file	*/

/************************************************************************/
/*									*/
/*	Definintion of messages exchanged with the remote server	*/
/*									*/
/************************************************************************/

/* Values for the type field in messages */

#define	RF_MSG_RESPONSE	0x0100		/* Bit that indicates response	*/

#define	RF_MSG_RREQ	0x0001		/* Read Request and response	*/
#define	RF_MSG_RRES	(RF_MSG_RREQ | RF_MSG_RESPONSE)

#define	RF_MSG_WREQ	0x0002		/* Write Request and response	*/
#define	RF_MSG_WRES	(RF_MSG_WREQ | RF_MSG_RESPONSE)

#define	RF_MSG_OREQ	0x0003		/* Open request and response 	*/
#define	RF_MSG_ORES	(RF_MSG_OREQ | RF_MSG_RESPONSE)

#define	RF_MSG_DREQ	0x0004		/* Delete request and response 	*/
#define	RF_MSG_DRES	(RF_MSG_DREQ | RF_MSG_RESPONSE)

#define	RF_MSG_TREQ	0x0005		/* Truncate request &response	*/
#define	RF_MSG_TRES	(RF_MSG_TREQ | RF_MSG_RESPONSE)

#define	RF_MSG_SREQ	0x0006		/* Size request and response	*/
#define	RF_MSG_SRES	(RF_MSG_SREQ | RF_MSG_RESPONSE)

#define	RF_MSG_MREQ	0x0007		/* Mkdir request and response 	*/
#define	RF_MSG_MRES	(RF_MSG_MREQ | RF_MSG_RESPONSE)

#define	RF_MSG_XREQ	0x0008		/* Rmdir request and response 	*/
#define	RF_MSG_XRES	(RF_MSG_XREQ | RF_MSG_RESPONSE)

#define	RF_MIN_REQ	RF_MSG_RREQ	/* Minimum request type		*/
#define	RF_MAX_REQ	RF_MSG_XREQ	/* Maximum request type		*/

/* Message header fields present in each message */

#define	RF_MSG_HDR			/* Common message fields	*/\
	uint16	rf_type;		/* message type			*/\
	uint16	rf_status;		/* 0 in req, status in response	*/\
	uint32	rf_seq;			/* message sequence number	*/\
	char	rf_name[RF_NAMLEN];	/* null-terminated file name	*/

/* The standard header present in all messages with no extra fields */

/************************************************************************/
/*									*/
/*				Header					*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_hdr {			/* header fields present in each*/
	RF_MSG_HDR			/*   remote file system message	*/
};
#pragma pack()

/************************************************************************/
/*									*/
/*				Read					*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_rreq	{		/* remote file read request	*/
	RF_MSG_HDR			/* header fields		*/
	uint32	rf_pos;			/* position in file to read	*/
	uint32	rf_len;			/* number of bytes to read	*/
					/*   (between 1 and 1024)	*/
};
#pragma pack()

#pragma pack(2)
struct	rf_msg_rres	{		/* remote file read reply	*/
	RF_MSG_HDR			/* header fields		*/
	uint32	rf_pos;			/* position in file		*/
	uint32	rf_len;			/* number of bytes that follow	*/
					/*   (0 for EOF)		*/
	char	rf_data[RF_DATALEN];	/* array containing data from	*/
					/*   the file			*/
};
#pragma pack()

/************************************************************************/
/*									*/
/*				Write					*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_wreq	{		/* remote file write request	*/
	RF_MSG_HDR			/* header fields		*/
	uint32	rf_pos;			/* position in file		*/
	uint32	rf_len;			/* number of valid bytes in	*/
					/*   array that follows		*/
	char	rf_data[RF_DATALEN];	/* array containing data to be	*/
					/*   written to the file	*/
};
#pragma pack()

#pragma pack(2)
struct	rf_msg_wres	{		/* remote file write response	*/
	RF_MSG_HDR			/* header fields		*/
	uint32	rf_pos;			/* original position in file	*/
	uint32	rf_len;			/* number of bytes written	*/
};
#pragma pack()

/************************************************************************/
/*									*/
/*				Open					*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_oreq	{		/* remote file open request	*/
	RF_MSG_HDR			/* header fields		*/
	int32	rf_mode;		/* Xinu mode bits		*/
};
#pragma pack()

#pragma pack(2)
struct	rf_msg_ores	{		/* remote file open response	*/
	RF_MSG_HDR			/* header fields		*/
	int32	rf_mode;		/* Xinu mode bits		*/
};
#pragma pack()

/************************************************************************/
/*									*/
/*				Size					*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_sreq	{		/* remote file size request	*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()

#pragma pack(2)
struct	rf_msg_sres	{		/* remote file status response	*/
	RF_MSG_HDR			/* header fields		*/
	uint32	rf_size;		/* size of file in bytes	*/
};
#pragma pack()

/************************************************************************/
/*									*/
/*				Delete					*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_dreq	{		/* remote file delete message	*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()

#pragma pack(2)
struct	rf_msg_dres	{		/* remote file delete message	*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()

/************************************************************************/
/*									*/
/*				Truncate				*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_treq	{		/* remote file truncate request	*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()

#pragma pack(2)
struct	rf_msg_tres	{		/* remote file truncate response*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()

/************************************************************************/
/*									*/
/*				Mkdir					*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_mreq	{		/* remote file mkdir request	*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()

#pragma pack(2)
struct	rf_msg_mres	{		/* remote file mkdir response	*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()

/************************************************************************/
/*									*/
/*				Rmdir					*/
/*									*/
/************************************************************************/

#pragma pack(2)
struct	rf_msg_xreq	{		/* remote file rmdir request	*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()

#pragma pack(2)
struct	rf_msg_xres	{		/* remote file rmdir response	*/
	RF_MSG_HDR			/* header fields		*/
};
#pragma pack()
