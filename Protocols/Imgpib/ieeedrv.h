#define WRITECOMCODE  0
#define READCOMCODE   1
#define WRITEDATCODE  2
#define READDATCODE   3

/** Completion codes for Driver calls **/

#define BADHW	255		/* return code for chip failure */

#define CCDOK	0		/* normal termination, no error */
#define	CCDCOM	1		/* unrecognised local command code */
#define CCDTCS	2		/* TCSET not followed by value */
#define CCDTOS	3       /* TOSET not followed by value */
#define CCDBUS	4		/* appears to be nothing on the bus */
#define CCDEOI	5		/* EOI indicated end of incoming data */
#define CCDTC	6		/* terminating character found */
#define CCDTO1	7		/* timed out at start of accept */
#define CCDTO2	8		/* timed out at end of accept */
#define CCDTO3	9		/* timed out at start of send */
#define CCDTO4	10		/* timed out at end of send */
#define CCDHRD	11		/* wrong/missing interface hardware */
 	     
#define CCDINS	13		/* not correctly installed (copy protection) */
#define CC_UNSUPPORTED 14  /* unsupported local command */
#define FUNCTION_UNKNOWN 15  /* Function other than readcom/writecom etc requested */
#define OUTBUFFER_SMALL 16   /* Not enough room for the requested reply */

/** IEEE bus codes **/

#define TAD  0x40    /* Primary Talk Address */
#define LAD  0x20    /* Primary Listen Address */
#define SAD  0x60    /* Secondary Address */
#define UNL  0x3F    /* Un Listen */
#define UNT  0x5F    /* Un Talk */

/** Driver Codes **/

#define TCDIS   0x86   /* Terminal Character disable */
#define TOSET   0x87   /* Time out set */
#define EXTOEN  0x92   /* Extended time out enable */
#define TOEN    0x88   /* Time out enable */

// Commands sent from w32ifapp.exe to w32intf.vxd

#define W32IF_PASS_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 1, METHOD_NEITHER, FILE_ANY_ACCESS)
