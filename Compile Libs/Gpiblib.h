/**	GPIBLIB.H	Header file for GPIB / Windows Library
***
***	Nigel Roberts	15 August, 1990
***
***  This is the header file to be included in programs which are to link
***  with the GPIBLIB library.
**/


/*  GPIBCOM Local Command Codes  */

#define SIFC   0x80	/*  Send InterFace Clear signal		*/
#define RENT   0x81	/*  Set REN line True			*/
#define RENF   0x82	/*  Set REN line False			*/
#define EXPP   0x83	/*  Execute a Parallel Poll		*/
#define TCSET  0x84	/*  Set new Terminating Character	*/
#define TCEN   0x85	/*  Enable T.C. detection on input	*/
#define TCDIS  0x86	/*  Disable T.C. detection		*/
#define TOSET  0x87	/*  Set TimeOut period			*/
#define TOEN   0x88	/*  Enable timeouts during transfers	*/
#define TODIS  0x89	/*  Disable timeouts			*/
#define EOIEN  0x8A	/*  Send EOI with last byte output	*/
#define EOIDIS 0x8B	/*  Disable sending of EOI		*/
#define SRQWT  0x8C	/*  Wait for SRQ signal to arrive	*/
#define ATNF   0x8D	/*  Set ATN line False			*/
#define INTEN  0x8E	/*  Enable Interrupts during transfers	*/
#define INTDIS 0x8F	/*  Disable Interrupts during transfers	*/
#define DMAEN  0x90	/*  Enable DMA during transfers			*/
#define DMADIS 0x91	/*  Disable DMA during transfers		*/
#define EXTOEN 0x92	/*  Enable machine independed extended TimeOuts	*/
#define EXWAIT 0x93	/*  Wait for period of extended Timeout		*/


/*  The IEEE-488 Bus Command Codes recognised by Microlink */

#define UNT    0x5F	/*  Untalk				*/
#define UNL    0x3F	/*  Unlisten				*/
#define GET    0x08	/*  Group Execute Trigger		*/


/*  Addressing Base Values  */

#define TAD    0x40	/*  Talk Address			*/
#define LAD    0x20	/*  Listen Address			*/
#define SAD    0x60	/*  Secondary Address			*/


#define GPIB_NOT_OPEN	0x7FFF



extern short WINAPI  GPIBOpen ( void );
short _declspec(dllexport) WINAPI  GPIBRequest ( short nUserNo );
short _declspec(dllexport) WINAPI  GPIBIsWanted ( void );
short _declspec(dllexport) WINAPI GPIBRelease ( void );
short _declspec(dllexport) WINAPI GPIBReadCom ( unsigned char far *lpbyBuffer, unsigned short 
								nRequest );
short _declspec(dllexport) WINAPI GPIBReadDat ( unsigned char far *lpbyBuffer, unsigned short 
								nRequest );
short _declspec(dllexport) WINAPI GPIBWriteCom ( unsigned char far *lpbyBuffer, unsigned short 
								nRequest );
short _declspec(dllexport) WINAPI GPIBWriteDat ( unsigned char far *lpbyBuffer, unsigned short 
								nRequest );
short _declspec(dllexport) WINAPI GPIBSendCom ( unsigned char byValue );
short _declspec(dllexport) WINAPI GPIBCompCode ( void );
short _declspec(dllexport) WINAPI GPIBClose ( short nUserNo );



