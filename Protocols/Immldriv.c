/**	IMMLDRIV.C		Main module for IML Protocol Driver
***
***	(C) Copyright Windmill Software Ltd, 1991-94. All rights reserved.
***
***  This is the common section for the family of IML Protocol Drivers.
***
***  This file implements these standard parts:
***
***     - start-up code
***     - read device details from IML.INI file
***     - create window & run "About" box
***     - interface to COMMSLIB
***     - tidy up and shutdown
***
***  Each protocol for linking to a remote Microlink (eg. GPIB, Serial,
***  NetBIOS, etc) has a source file which implements a standard set of
***  functions (OpenDriverInterface, SendDriverCommand, etc). A complete
***  driver is produced by linking this standard IMMLDRIV module with a
***  specific protocol module (eg. IMGPIB, IMSERL, etc).
***
***  Associated files (common to all protocols):
***     IMMLDRIV.C      this file (COMMSLIB & Windows interface)
***     IMMLDRIV.H      header file for coonstants, prototypes, structs
***     IMMLDLG.H       header file for About box constants
***
***  For each protocol (where xxxx = GPIB or SERL or NETB):
***     IMxxxx.C        source code for low level protocol functions
***     IMxxxx.RC       resources
***     IMxxxx.ICO      icon
***     IMxxxx.DLG      dialog (About box)
***     IMxxxx.DEF      linker def file
***     IMxxxx.MAK      build file
***
***
***		 MUST USE COMMSLIB 4.00 OR LATER
***     =================================
***
***
***  Version History:
***
***    Feb 93   Stuart Grace   First version. Based loosely on code from
***						       IMWMAIN.C from WINAPP KIT 2.04. For use with
***			    	    	      IMGPIB P1.00  and  IMSERL P1.00
***
***  Spring 95  Stuart Grace   Extended to include some new functionality
***                             required for use with IMNETB (for NetBIOS).
***
***    Nov 95   Stuart Grace   Correction to bug SFR PROTOCS/1.
***                             (Arguments to MAKELONG were reversed, so that
***                              error codes were returned incorrectly).
***
***    May 96   Stuart Grace   Altered to use new interface to COMMSLIB 4.01 (using AckIMLCommand).
***                             This seems to cure any intermittent hanging problems under Win95.
***                             Also modified message loop so it only uses PeekMessage when necessary;
***								This imposes less CPU load and is recommended by Microsoft.
***
***    Aug 96	Stuart Grace   Fix for bug SFR PROTOCS/2 - overwriting ulComNumber and unCommandFlags
***								statics for commands which were rejected - causing lock-ups in COMMSLIB.
***
***    Jan 99   BGO            Version 5.00 32 bit created
***    
***    May 00   BGO            PostMessages changed to SendMessages to
***                            force immediate initialisation of the
***                            serial hardware. Especially matters when
***                            error correcting used since the protocol
***                            message must be sent immediately.
***
***    Jan 03   BGO			   Version 5.1 IML.INI moved to Windmill dir
***                            About Box Warnings cleared up
**/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define IML_INTERPRETER		/* activate correct sections of imlinter.h */

#include "\Windmill 32\Compile Libs\imlinter.h"           /* COMMSLIB include file */
#include "\Windmill 32\Compile Libs\imslib.h"

#include "immldriv.h"
#include "immldlg.h"		/* include file for About box controls */








/** Here are global variables that relate to the Windows side of things...
**/

char	szCommand [ MAX_COMMAND + 1 ];		/* copy of incoming command */
char	szReply [ MAX_REPLY + 1 ];			/* outgoing reply 	 */

WORD	wIMLWaitingMessage;		/* message: command has been sent to us	 */
WORD	wIMLAbortMessage;		/* message: abort current command	 */
WORD	wIMLReplyMessage;		/* message: look for reply to command	 */

short   nIMLHandle;				/* (for generic interface to kernel)	 */
HWND	hWndMain;				/* handle of our main Window		 */
HINSTANCE	hInst;          	/* instance handle 			 */

BOOL	bTextReplyPending;		/* TRUE while waiting for text reply	 */
BOOL	bBDPReplyInProgress;	/* TRUE during a streaming reply	 */





/**  The next two strings must be defined in the low-level (protocol-specific)
***  section which this will be linked with, in order to give this section
***  it's specific "personality".
**/

extern char *szDriverName;      /* Import our program name from low-level */
                                /*  (will use this for window class name) */

extern char *szTitle;		/* Import our window title from low-level */






/**  Here is the table of IML devices (=Microlinks) we are looking after.
***  Those devices numbers which are not our responsibility have the
***  hardware address (GPIB primary address/COM port/LANA) left at -1. Devices
***  which we have to look after will get hardware address and error log
***  level set from settings in IML.INI.
**/

IMLDEVICE   Microlink [ NUM_DEVICES ];



/**  This will hold the full path of the file to be used for error logging:
**/

char    szLogFile [ _MAX_PATH ];        /* used by low-level funcs */



/** For copy-protected drivers, this string can be modified by the Handlers
*** if necessary. For unprotected drivers, it will be patched during the disk
*** building process.
**/



/** Prototypes for the two window functions:
**/

LONG WINAPI DriverWndProc ( HWND hWnd, DWORD Message, WPARAM wParam, LPARAM lParam );
BOOL WINAPI AboutDriverProc ( HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam);

void  FindErrorLogFile ( HANDLE hInst );        /* local helper func */







/**
***  Windows 3.1 Main Program Body
***
***  The WinMain routine does these jobs:
***	- parses and checks command line args (from COMMSLIB)
***     - if another copy of this app is already running, it just passes
***        the command line info across (using a private message) then exits.
***	- if no other copy running, it registers and creates the program's
***	   main window
***	- registers required private messages
***	- runs the message dispatch loop until WM_QUIT is found.
**/

int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpszCmdLine, int nCmdShow )
{

	MSG		  msg;
	WNDCLASS  wndclass;
	char	  szLocal [ 50 ];
	short	  nCode, nIndex;
	short	  nMyDeviceNumber;
	HWND      hExistingWindow;

	

	bTextReplyPending   = FALSE;
	bBDPReplyInProgress = FALSE;

	hInst = hInstance;


	/** First we check the command line args. We expect them like this:
	***
	***	COMMSLIB   <device-num>
	***
	*** We show a message and terminate if the COMMSLIB keyword is missing,
	*** or if the device number is missing.
	**/

	strncpy ( szLocal, lpszCmdLine, 20 );		/* local copy of args */
	
	if ( strnicmp ( szLocal, "COMMSLIB", 8 ) != 0 ) {

		ErrorMessageBox ( IDS_LOADS_WHEN_NEEDED );
		return ( 0 );
	}

	if ( sscanf ( &( szLocal [ 8 ]), "%d", &nMyDeviceNumber ) == 0 ) {

		ErrorMessageBox ( IDS_DEV_NUMBER_MISSING );
		return ( 0 );
	}



  /** Look for another window with our class name to see if we need
    *** to register the class. If non found we register our class.
    **/

	hExistingWindow = FindWindow ( szDriverName, NULL );

	if ( hExistingWindow == NULL ) {


		/** No other window is using this class. Therefore this is
        *** the first instance of this Protocol Driver to be loaded.
        **/

        for ( nIndex = 0; nIndex < NUM_DEVICES; nIndex++ ) {

			Microlink [ nIndex ].nHardwareAddress = -1;
        }

		wndclass.style			= CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc	= (WNDPROC) DriverWndProc;
		wndclass.cbClsExtra		= 0;
		wndclass.cbWndExtra		= 0;
		wndclass.hInstance		= hInst;
		wndclass.hIcon			= LoadIcon ( hInst, MAKEINTRESOURCE ( DRIVICON ) );
		wndclass.hCursor		= LoadCursor ( NULL, IDC_ARROW );
		wndclass.hbrBackground  = (HBRUSH) (COLOR_WINDOW+1);
		wndclass.lpszMenuName	= NULL;
		wndclass.lpszClassName	= szDriverName;

		if ( !RegisterClass ( &wndclass ) ) {

			ErrorMessageBox ( IDS_ERR_REGISTER_CLASS );

			return ( 0 ); 	/* abort - failed to register class */
		}

	} else {

		/** This protocol manager is already running (has a window).
        *** All we have to do is ask that one to look after this
        *** device as well as the others.
        **/

        SendMessage ( hExistingWindow, MSG_USE_DEVICE, nMyDeviceNumber, 0L );

        return ( 0 );   /* All taken care of! */
	}


    /** Now create application's main window using this class
	**/

	hWndMain = CreateWindow (
					szDriverName,		/* Window class name      */
					szTitle,       		/* Window's title         */
                	WS_POPUP  |         /* Window normal type     */
					  WS_VISIBLE |		/*    visible		      */
                	  WS_SYSMENU,       /*    with system menu    */
                	1, 1,			  	/* Window X, Y            */
	                1, 1,			  	/* Window width, height   */
    	            NULL,               /* Parent window's handle */
        	        NULL,               /* default to class menu  */
            	    hInst,              /* Instance of window     */
                	NULL );			  	/* Create struct 	      */


	if ( hWndMain == NULL ) {

		ErrorMessageBox ( IDS_ERR_CREATE_WINDOW );

		return ( 0 );	  /* abort - failed to create our window */
	}

	wIMLWaitingMessage = RegisterWindowMessage ( "IML_COMMAND_WAITING" );
    wIMLAbortMessage   = RegisterWindowMessage ( "IML_ABORT_COMMAND"   );
    wIMLReplyMessage   = RegisterWindowMessage ( "IML_REPLY_WANTED"    );

	if ( ( wIMLWaitingMessage == 0 ) ||
         ( wIMLAbortMessage   == 0 ) ||
         ( wIMLReplyMessage   == 0 ) ) {

		ErrorMessageBox ( IDS_REG_MESSAGE_FAILED );

		return ( 0 );	   /* abort - failed to reg private message */
	}


	ShowWindow ( hWndMain, SW_SHOWMINNOACTIVE );	/* display as ICON */

    FindErrorLogFile ( hInstance );         /* locate our error log */

    /** Open our comms protocol interface.
    **/

    nCode = OpenDriverInterface ( &nIMLHandle );

	if ( nCode == FATAL_STARTUP_ERROR ) {	/* Was that a disaster? */

		return ( 0 );			/* Abort the whole program */
	}


    /** Tell our window which IML device it is driving
    **/

    SendMessage ( hWndMain, MSG_USE_DEVICE, nMyDeviceNumber, 0L );

	/**
	***     Determine if the user has a valid release copy
	***     of the software. If not put up MessageBox.
	**/
    if ( !IMSClear ( szDriverName ) ) {

		
		/**	Copy protection has been invalidated
		**/
		strcpy ( szLocal, "ERROR  ");
		strcat ( szLocal, szDriverName );

		MessageBox ( (HWND)hInstance, 
		"        This Program is NOT an Authorised Copy. \r\r    It will however function correctly for this Session. \r\r See Confiml Software Section for Authorisation Details",
					szLocal,
					 MB_OK );
						    		
	}
  

#if defined ( NETBIOS )         /* Only used in IMNETB Driver */


	while ( GetMessage ( &msg, NULL, 0, 0 ) ) {    /* Until WM_QUIT msg */

		TranslateMessage ( &msg );
		DispatchMessage ( &msg );
	}


#else


	/** The main message processing loop for SERIAL and GPIB.
	**/

	do {

        if ( bBDPReplyInProgress ) {	/* Do BDPs need to be collected in the background? */

			/** Allow Windows to pass a message to any WndProc that has work to do.
			*** If this returns true, then the message is for us, so we dispatch it
			*** to our own WndProc. If it returns false, other Apps may have used the opportunity.
			**/
			
			if ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE ) ) {

				if ( msg.message == WM_QUIT ) {
					return ( msg.wParam );
				}
   				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			/** Now allow ourselves time to sort out the BDP stuff.
			**/
        
			BackgroundProcess ();
			
		} else {

			/** No background processing required - sleep until a message arrives.
			**/

			WaitMessage ();
			GetMessage ( &msg, NULL, 0, 0 );

			if ( msg.message == WM_QUIT ) {
				return ( msg.wParam );
			}
    		TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	} while ( msg.message != WM_QUIT );


#endif

	return ( msg.wParam );

} /*  End of WinMain  */





/**  This little function is called from WinMain during start-up. It decides
***  the name of the file which will receive the error logging info, if any.
***  It does this by calling GetModuleFileName for this program, which
***  returns the full path of this .EXE file. It then just removes .EXE
***  from the end and replaces it with .LOG, using same path & filename.
***  Result is left in the global szLogFile.
**/

void  FindErrorLogFile ( HANDLE hInst )
{
	char	szExeDrive [ _MAX_DRIVE ];
	char	szExeDir   [ _MAX_DIR ];
	char	szExeName  [ _MAX_FNAME ];
	char	szExeExtn  [ _MAX_EXT ];


	GetModuleFileName ( hInst, szLogFile, _MAX_PATH );

	_splitpath ( szLogFile, szExeDrive, szExeDir, szExeName, szExeExtn );

	_makepath ( szLogFile, szExeDrive, szExeDir, szExeName, ".LOG" );

	return;
}






/**  Main Window Procedure. This has three parts:
***
***  (1) Deal with messages arriving from COMMSLIB:
***
***	IML_COMMAND_WAITING  (msg number in wIMLWaitingMessage) tells us
***		that a command is waiting for our attention. We get the
***		command from COMMSLIB and pass it on to the protocol section.
***     wParam contains flags (which tell what kind of reply if any
***     we should expect), and lParam has the device number in the
***     low word, and a commslib reference number in the high word.
***		If a text reply was expected, we see if one is available and
***	    pass it back to COMMSLIB if it is.
***
*** IML_ABORT_COMMAND  (msg number in wIMLAbortMessage) tells us to give
***     up on the text reply we are waiting for (ie. timeout). We just
***     reset our local flags and tell the protocol section to
***     cancel the pending reply and reset itself.
***
*** IML_REPLY_WANTED  (msg number in wIMLReplyMessage) tells us that
***     the application is looking for a text reply which we have
***     not yet supplied. This is our cue to check to see if the
***     reply is ready yet, and if so pass it on to COMMSLIB.
***
***  (2) Deal with messages sent here by our own WinMain or our own protocol
***      section:
***
*** MSG_USE_DEVICE  (which is WM_USER+1) is sent here by WinMain to
***		inform us of a device we must drive using the protocol.
***
***	MSG_BDP_READY  (which is WM_USER+2) comes from our own low-level
***		protocol section asking us to transfer a completed BDP
***		through to COMMSLIB.
***
***  (3) The rest is a normal window procedure, but with the window
***      permanently iconised:
***
***	WM_CREATE	register the window with COMMSLIB and add
***			  "About..." to system menu.
***
***	WM_SYSCOMMAND	load and show About box when requested.
***
***	WM_QUERYOPEN	reply with "NO! We will not open our window!"
***
***	WM_CLOSE	show an "Are you sure?" type message box, and if
***			  confirmed, tell COMMSLIB that we are away, tell
***			  the protocol section to shut down, and destroy
***			  the window.
***
***  Everything else is passed through to DefWindowProc, as there is no client
***  area (other than icon) and no menus (other than About box).
**/



long WINAPI DriverWndProc ( HWND hWnd, DWORD Message, WPARAM wParam, LPARAM
								lParam )

{

HDC         hDC;
PAINTSTRUCT    ps;
short		rc = 0;             /* return code                         */
WORD		unCommandFlags;
HMENU		hSysMenu;
LPIMLBDP	lpbdpData;
short		nCode, nIndex;
short       nDevice;
	char	szTemp [ 200 ];
static short    nTargetDevice;    /* device number for comnd being processd */
static DWORD	ulComNumber;      /* COMMSLIB ref number of cmd being procd */



 	/** First check for special private messages from COMMSLIB...
 	**/

 	if ( Message == wIMLWaitingMessage ) {


		/** COMMSLIB is saying "Command for You". If we are already in the middle of
		*** a previous command, we must return "Not Ready", to indicate the communication
		*** link is tied up by another application at the moment. The app should re-try later.
        **/

        if ( bTextReplyPending || bBDPReplyInProgress ) {

	        AckIMLCommand ( (DWORD)lParam, IML_NOT_READY );		/* Fix for SFR PROTOCS/2 */
        
			return ( 0L );		/* Busy doing previous command */
		}


        /** Not doing anything else, so go ahead with this command
        **/

        ulComNumber = (DWORD) lParam;	/* command serial number */
		unCommandFlags = LOWORD (wParam);	  	/* command flags	   */

		if ( GetIMLCommandToExecute ( ulComNumber, szCommand, MAX_COMMAND ) != 0 ) {

			ErrorMessageBox ( IDS_ERR_GET_COMMAND );
        
	        AckIMLCommand ( ulComNumber, IML_LINK_ERROR );
        
			return ( 0L );		/* We failed to retrieve command */
		}


        /** The target device for this command is embedded in the
        *** LOWORD of the command number.
   		**/

        nTargetDevice = LOWORD ( ulComNumber );


        /** Check if the command begins with tilde ( ~ ) in first char. If so,
        *** this is a local command aimed at this driver (not to be passed on to
        *** Microlink). Invoke a special function in the protocol section.
        **/

        if ( szCommand [ 0 ] == '~' ) {

			ProcessTildeCommand ( nTargetDevice, szCommand, szReply );

			ReplyToIMLCommand ( ulComNumber, szReply );

	        AckIMLCommand ( ulComNumber, OK );

			return ( 0L );
        }


		/** Normal IML command - send to the appropriate device.
        **/

		rc = SendDriverCommand ( nIMLHandle, nTargetDevice, szCommand, unCommandFlags );

        if ( rc != OK ) {

	        AckIMLCommand ( ulComNumber, rc );

			return ( 0L );      /* error while sending */
        }


		if ( ( unCommandFlags & IML_TEXT_REPLY ) != 0 ) {


			/** This command has a text reply. Let's check if it
       		*** is ready straight away...
			**/

			rc = GetDriverReply ( nIMLHandle, &nTargetDevice, szReply, sizeof ( szReply ), 0 );

            if ( rc == IML_NOT_READY ) {

				/** The reply is still working its way through the system. This is not
				*** an error, but the app must retry later. At this later time we will
				*** get a IML_REPLY_WANTED message. Just note this fact and return OK since
                *** the command was sent without error.
                **/

    	        bTextReplyPending = TRUE;   /* I'll be back!*/

		        AckIMLCommand ( ulComNumber, OK );
    	
				return ( 0L );
			}

			if ( rc != OK ) {		/* This is a genuine problem - notify COMMSLIB */
			
		        AckIMLCommand ( ulComNumber, rc );

				return ( 0L );
	       	}


            /** GetDriverReply returned OK so the reply is ready
			**/

			if ( ReplyToIMLCommand ( ulComNumber, szReply ) != 0 ) {

				ErrorMessageBox ( IDS_ERR_POST_REPLY );
			}
		}

		if ( ( unCommandFlags & IML_BIN_REPLY ) != 0 ) {

			/** This command has a binary packet stream reply. We must start looking out for
			*** these as a background job. The bBDPReplyInProgress flag tells the main message
			*** loop above to keep calling the BackgroundProcess function. We must post a
			*** message to it to ensure it wakes up  and comes out of GetMessage.
			**/

			bBDPReplyInProgress = TRUE;
			
			PostMessage ( hWnd, WM_NULL, 0 , 0L );		/* Send ourselves a dummy msg to wake up */
		}

        AckIMLCommand ( ulComNumber, OK );

		return ( 0L );
 	}


 	if ( Message == wIMLAbortMessage ) {


		/** COMMSLIB is saying "Abort the current command". If we are
        *** still waiting for a text reply to the previous command,
        *** tell the low-level routines to scrap this reply and reset
        *** ready for start of next command.
        **/

        if ( bTextReplyPending ) {

        	bTextReplyPending = FALSE;

	        AckIMLCommand ( ulComNumber, AbortDriverTextReply ( nTargetDevice ) );

			return ( 0L );
		}

		return ( 0L );		/* finished */
 	}


    if ( Message == wIMLReplyMessage ) {

		
		/** COMMSLIB is telling us that the app is polling for the
        *** pending text reply. We must check to see if it is ready
        *** yet. If it is, we pass it back to COMMSLIB and return OK.
        *** If it is not available, we pass back whatever excuse
        *** the low-level GetDriverReply function used.
        **/

		/** If there is no reply pending **/
        if ( !bTextReplyPending ) {     
	        AckIMLCommand ( ulComNumber, IML_NO_REPLY_EXPECTED );
    		return ( 0L );
		}


		rc = GetDriverReply ( nIMLHandle, &nTargetDevice,
					     	    szReply, sizeof ( szReply ), 0 );

        if ( rc == OK ) {
			/** The reply was found successfully! Pass it on. **/
			if ( ReplyToIMLCommand ( ulComNumber, szReply ) != 0 ) {
				ErrorMessageBox ( IDS_ERR_POST_REPLY );
			}
		} 
			

        /** This reply is still expected only if the protocol driver
        *** said it is still not ready. If the driver said anything
        *** else like finished or error, it is no longer pending.
        **/

        bTextReplyPending = ( rc == IML_NOT_READY );

        AckIMLCommand ( ulComNumber, rc );
    	
		return ( 0L );
 	}


 	switch ( Message ) {	/* process internal & windows messages */


		/**  MSG_USE_DEVICE is sent here by our own WinMain to tell us that
		***  this is a device we must drive using the protocol. This causes
		***  us to look in IML.INI file to find the details about this
		***  device, and stick them in the Microlink table.
		***
		***  It gets sent in two different cases:
		***
		***     - for the first device, when we have just been loaded by
		***	   COMMSLIB and have registerd our class, created window, etc.
		***
		***     - when we start up and find the window is already running. We
		***        just send this message to the existing window and then
		***	   quit. Hence all Microlinks using the same protocol are
		***	   managed by a single instance.
		***
		***  WPARAM is device number.
		***  LPARAM is reserved (set to 0).
		**/

    	case MSG_USE_DEVICE:	/* This is a device we must look after */

			nDevice = LOWORD (wParam);

			if ( nDevice >= NUM_DEVICES ) {

            	ErrorMessageBox ( IDS_TOO_MANY_DEVICES );

                break;  	/* cannot handle this many devices */
            }

            if ( RegisterIMLInterpreter ( hWnd, (short) wParam ) != 0 ) {

				ErrorMessageBox ( IDS_ERR_REG_COMMS );
			}

			{ 
			char    szPath [MAX_PATH];

			GetIMLINIPath ( szPath, MAX_PATH );

            /** Look in IML.INI for device details which we must
            *** transfer to the Microlink table. The relevent details
            *** will be in this form (not all fields needed):
            ***
            ***     [DeviceNN]        where NN is device number
            ***
            ***     Address=PP        where PP is GPIB primary address
            ***                                or COM port number
            ***
            ***     MicrolinkType=T   where T is 1 for RSI or 0 otherwise
            ***
            ***     ExtraInfo=E       where E is 1 for C protocol, 0 for N
            ***
            ***     Rate=R            where R is a code for baud rate
            ***
            ***     ErrorLog=L        where L is error logging level
            ***
            ***     ESC1=command      channel & text of ESC command to
            ***  to ESC4=command       to send to Microlink at start-up
            ***
            *** The ESC commands have a prefix of  "ESC n" prepended
            *** before they are sent, where n is device number. For
            *** example:
            ***			ESC1=0000 ABC DEF
            ***
            *** would get sent to device 2 as
            ***
            ***	        ESC 20000 ABC DEF    which is a valid command.
            **/

            sprintf ( szTemp, "Device%u", wParam );

            Microlink [ nDevice ].nHardwareAddress =
                			GetPrivateProfileInt ( szTemp, "Address", 0, szPath );

            Microlink [ nDevice ].nMicrolinkType =
                			GetPrivateProfileInt ( szTemp, "MicrolinkType", MLINK_DEFAULT, szPath );

            Microlink [ nDevice ].nExtraInfo =
                			GetPrivateProfileInt ( szTemp, "ExtraInfo", MLINK_DEFAULT, szPath );

            Microlink [ nDevice ].nRate =
                			GetPrivateProfileInt ( szTemp, "Rate", MLINK_DEFAULT, szPath );

            Microlink [ nDevice ].nErrorLogLevel =
                			GetPrivateProfileInt ( szTemp, "ErrorLog", 0, szPath );

            GetPrivateProfileString ( szTemp,
                                      "ESC1",
                                      "",
									  Microlink [ nDevice ].szEscape1,
                                      sizeof ( Microlink [ nDevice ].szEscape1 ),
                                      szPath );

            GetPrivateProfileString ( szTemp,
                                      "ESC2",
                                      "",
									  Microlink [ nDevice ].szEscape2,
                                      sizeof ( Microlink [ nDevice ].szEscape2 ),
                                      szPath );

            GetPrivateProfileString ( szTemp,
                                      "ESC3",
                                      "",
					  				  Microlink [ nDevice ].szEscape3,
                                      sizeof ( Microlink [ nDevice ].szEscape3 ),
                                      szPath );

            GetPrivateProfileString ( szTemp,
                                      "ESC4",
                                      "",
					  				  Microlink [ nDevice ].szEscape4,
                                      sizeof ( Microlink [ nDevice ].szEscape4 ),
                                      szPath );

			}

            /** Finally, tell the low-level comms routines that they
            *** will be dealing with this device in future.
            **/

            PrepareLinkToDevice ( nDevice );
            break;



        /**  MSG_BDP_READY comes from our own low-level protocol section
        ***  asking us to transfer a completed BDP through to COMMSLIB.
        ***
        ***  In principle, all we have to do is get the BDP from the protocol
        ***  section and pass it to COMMSLIB using the functions provided.
        ***  But it is the error handling which is complicated. Remember that
        ***  BDP transfers can be aborted from EITHER end. The possible
        ***  cases and their consequences are:
        ***
        *** (1) Normal BDP arrived & sent to COMMSLIB - carry on.
        ***
        *** (2) Comms link error occurred - tell COMMSLIB error occurred.
        ***
        *** (3) BDP with a terminal flag set - send to COMMSLIB but then tell
        ***                                 COMMSLIB there will be no more.
        ***
        *** (4) BDP arrived but COMMSLIB will not accept it - tell protocol
        ***                                   section to abort the transfer.
        ***
        **/

#if defined ( NETBIOS )         /* Only used in IMNETB Driver */


		case MSG_BDP_READY:

			if ( ! bBDPReplyInProgress ) {
				break;			/* no BDP link - do nothing */
			}

			nCode = GetBDPFromDriver ( wParam, lParam, &lpbdpData );

            if ( nCode == IML_LINK_ERROR ) {

				/** Case (2) above - error in comms.
                **/

                TerminateIMLBDPStream ( nTargetDevice, nCode );

				bBDPReplyInProgress = FALSE;      /* reply failed */
                break;
            }


            /** We have received a BDP from the Microlink, so pass it on to COMMSLIB.
            **/

			rc = SendIMLBinaryDataPacket ( lpbdpData, IML_FAIL_IF_FULL );

			if ( nCode != OK ) {

                /** Case (3) above - Microlink terminated the stream
                **/

				TerminateIMLBDPStream ( nTargetDevice, nCode );

				bBDPReplyInProgress = FALSE;      /* finished reply */

			} else {		/* are we able to continue? */

				switch ( rc ) {

		  	 	 case OK : 	/* sent packet - no problem */

                            SetBDPStatus ( BDP_STAT_GET_NEXT );
                            break;

		  	 	 case IML_NO_FREE_BUFFER :    /* Not yet accepted */

                            SetBDPStatus ( BDP_STAT_REPEAT );
							break;

			  	 default :	/* transfer has been aborted by App */

                            SetBDPStatus ( BDP_STAT_ABORT );

							bBDPReplyInProgress = FALSE;  /* abort reply */
							break;
				}
			
			}
            break;


#else      /* Serial & GPIB version */


	 	case MSG_BDP_READY:

			if ( ! bBDPReplyInProgress ) {
				break;			/* no BDP link - do nothing */
			}

			nCode = GetBDPFromDriver ( (short) wParam, lParam, &lpbdpData );

            if ( nCode == IML_LINK_ERROR ) {

				/** Case (2) above - error in comms.
                **/

                TerminateIMLBDPStream ( nTargetDevice, nCode );

				bBDPReplyInProgress = FALSE;      /* reply failed */
                break;
            }


            /** We have received a BDP from the Microlink, so pass it on to COMMSLIB.
            **/

			rc = SendIMLBinaryDataPacket ( lpbdpData, IML_FAIL_IF_FULL );

			if ( nCode != OK ) {

                /** Case (3) above - Microlink terminated the stream
                **/

				TerminateIMLBDPStream ( nTargetDevice, nCode );

				bBDPReplyInProgress = FALSE;      /* finished reply */

			} else {		/* are we able to continue? */

				switch ( rc ) {

			  	 case OK : break;	/* sent packet - no problem */

                 /** rc != OK means case (4) above - link being killed from the application end.
                 **/

			  	 case IML_NO_FREE_BUFFER :    /* App not accepting */

					TerminateIMLBDPStream ( nTargetDevice, BUFFER_OVERFLOW );

       	            AbortDriverBDPReply ( nTargetDevice );

					bBDPReplyInProgress = FALSE;  /* abort reply */
					break;

				  default :	/* transfer has been aborted by App */

					AbortDriverBDPReply ( nTargetDevice );

					bBDPReplyInProgress = FALSE;  /* abort reply */
					break;
				}
			}
            break;

#endif



        /**  MSG_NET_EVENT is sent by IMNBLIB.DLL when something has happened
        ***  in NetBIOS. We just pass this (with both params) to a special
        ***  routine in the low-level network module which will deal with
        ***  it.
        ***
        ***  NetBIOS version only - no equivalent in serial or GPIB.
        **/

#if defined ( NETBIOS )         /* Only used in IMNETB Driver */

		case MSG_NET_EVENT:

			HandleNetworkEvent ( wParam, lParam );
			break;

#endif                          /* End of conditional section */



    	case WM_COMMAND:      /* Windows messages for ordinary menu items */

			return ( DefWindowProc ( hWnd, Message, wParam, lParam ) );


    	case WM_SYSCOMMAND:   /* The Windows messages for System menu items */

			switch ( LOWORD (wParam) ) {

        		case IDM_ABOUT:


		 			DialogBox ( hInst, MAKEINTRESOURCE ( ABOUTBOX ), hWnd, (DLGPROC) AboutDriverProc );

					break;

				default: return ( DefWindowProc ( hWnd, Message, wParam, lParam ) );
			}
			break;

		/**  Before we allow the window to be closed, we first do an
		***  "Are you sure?" type message box, to prevent the user
		***  inadvertently cutting off the Microlinks while they are
		***  in use. If they are sure, we close the low-level stuff,
		***  sign-off with COMMSLIB (so it doesn't send us any more
		***  messages), and destroy the window.
		**/

    	case WM_CLOSE:  /* close the window  */

			LoadString ( hInst, IDS_CLOSE_WARNING, szTemp, sizeof ( szTemp ) );

			rc = MessageBox ( hWnd, szTemp, szTitle,  MB_SYSTEMMODAL | MB_ICONSTOP | MB_OKCANCEL );

			if ( rc == IDCANCEL ) {
				return ( 0L );		/* window closure cancelled */
			} 


			/** Close our low-level interface.
			**/

			CloseDriverInterface ( nIMLHandle );


			/** Now tell COMMSLIB which devices are no longer available.
			**/

			for ( nIndex = 0; nIndex < NUM_DEVICES; nIndex++ ) {

				if ( Microlink [ nIndex ].nHardwareAddress != -1 ) {

                	rc = UnRegisterIMLInterpreter ( nIndex );

					if ( rc != 0 ) {
						ErrorMessageBox ( IDS_ERR_UNREGISTER );
					}
				}
			}
    
			DestroyWindow ( hWnd );
			PostQuitMessage ( 0 );    /* Quit the application */
			break;

    	case WM_CREATE:	     /* Window is just being created */

			strcpy ( szReply, "" );
			hWndMain = hWnd;

			hSysMenu = GetSystemMenu ( hWnd, FALSE );
			AppendMenu ( hSysMenu, MF_SEPARATOR, 0, (LPSTR) NULL );
			AppendMenu ( hSysMenu, MF_STRING | MF_ENABLED, IDM_ABOUT, "About Microlink Driver..." );
            break;


    	case WM_QUERYOPEN:	/* windows is asking if we will open window */

			return ( 0L );	    /* Refuse to open from icon to window! */
    		break;


    	case WM_SIZE:     /*  code for sizing client area  */
			break;

    	case WM_PAINT:    /* code for the window's client area */
			hDC = BeginPaint ( hWnd, &ps );
			EndPaint ( hWnd, &ps );
            break;





        /**  If Windows is being shut-down, we must close down the interface
        ***  to release the comms channel (NetBIOS, GPIB Drivers, etc).
        **/

    	case WM_ENDSESSION:

			if ( (BOOL) LOWORD (wParam) ) {     /* If Windows is closing down: */

            	/** Close our low-level interface.
                **/

				CloseDriverInterface ( nIMLHandle );
            }
			break;


    	default:

			/** All other messages go through to Default message processing
			**/

			return ( DefWindowProc ( hWnd, Message, wParam, lParam ));

   	}

   	return ( 0L );

}     /* End of WndProc */








/**  "About Microlink Driver..." Dialog Window Procedure
***
***  This procedure drives the "About..." dialog box. It sets the dynamic
***  strings showing serial number and device numbers we are driving, then
***  just waits for OK button or the window being closed. Dialog template is
***  in protocol-specific .DLG file (included in an .RC file).
**/

BOOL WINAPI AboutDriverProc ( HWND hWndDlg, WORD Message, WPARAM wParam,
								LPARAM lParam )

{
	char	szTemp [ 4 * NUM_DEVICES ];	

    short   nIndex;



	switch ( Message ) {

	 case WM_INITDIALOG:	/* Initialise: set ser num string */

        strcpy ( szTemp, "" );    /* build list of device numbers */

        for ( nIndex = 0; nIndex < NUM_DEVICES; nIndex++ ) {

        	if ( Microlink [ nIndex ].nHardwareAddress != -1 ) {

            	/** This is one of our devices - add to list
                **/

                if ( strlen ( szTemp ) > 0 ) {

					strcat ( szTemp, " + " );
                }

                sprintf ( &( szTemp [ strlen ( szTemp ) ] ), "%d", nIndex );
            }
        }

        SetWindowText ( GetDlgItem ( hWndDlg, IDD_DEVICES ), szTemp );
        break;


   	 case WM_CLOSE:	 /* Closing the Dialog behaves the same as Cancel */

		PostMessage ( hWndDlg, WM_COMMAND, IDCANCEL, 0L );
		break;


   	 case WM_COMMAND:

   		switch ( LOWORD (wParam) ) {

           	case IDOK:     EndDialog ( hWndDlg, TRUE );
					       break;

           	case IDCANCEL: EndDialog ( hWndDlg, FALSE );
		    			   break;
		}
        break;

   	 default:
		return ( FALSE );
   	}

   	return ( TRUE );
}





/**  This just loads the string for an error message from the string table in
***  the resource file, and displays it in a SYSTEM-MODAL message box. This
***  is because errors in a WinApp device are serious or fatal for the IML
***  application software and should be read and acknowledged by the user
***  before any further processing takes place.
**/

short	ErrorMessageBox ( short nCode )

{

static char	szTemp [ 200 ];


	if ( nCode == 0 ) {		/* code zero is no error at all */
		return ( 0 );		/*   so just return */
	}

	LoadString ( hInst, nCode, szTemp, sizeof ( szTemp ) );

	MessageBox ( NULL, szTemp, szTitle, MB_SYSTEMMODAL | MB_OK | MB_ICONEXCLAMATION );

	return ( nCode );
}
