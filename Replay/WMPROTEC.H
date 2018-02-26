/**
***		WMPROTEC.H			Header file for Windmill Software Copy Protection
***
***  (C) Copyright Windmill Software Ltd, 1996. Al rights reserved.
***
***  March 1996 - First Version.
**/



#ifdef __cplusplus
extern "C" {
#endif 


/**  EnableWindmillOnThisMachine
***
***  Install program calls this function when ready to install Windmill from the distribution
***  floppy onto the user's hard disk. Arguments:
***
***		hWndParent		HWND of parent for error message box
***
***		nDrive			0 if distribution floppy in drive A:
***						1 if in drive B:
***				 		No other values supported.
***
***  This checks that the floppy looks like the original one made by Windmill Software Ltd.
***  If not, it displays a message telling the user he must use the original disk, then
***  returns FALSE. If disk is OK, it writes the characteristic value for this machine
***  to the hard disk and returns TRUE.
***
***  Returns: TRUE if OK to proceed, FALSE if program should abort.
***
***  NOTE: Message to user displayed from within this function.
**/

BOOL _cdecl  EnableWindmillOnThisMachine ( HWND hWndParent, short nDrive );



/**  IsValidReleaseCopy
***
***  Called by all programs at run time to check that the user has a properly installed copy of
***  the software. If installed correctly, the assigned serial number is returned by reference.
***  Arguments :
***
***		hWndParent		HWND of parent for error message box (can be NULL)
***
***     pwSerialNumber	Pointer to Word in which the serial number is returned by reference.
***
***  Returns: TRUE if OK to proceed, FALSE if program should abort.
***
***  NOTE: Message to user displayed from within this function.
***
**/


BOOL _cdecl  IsValidReleaseCopy ( HWND hWndParent, WORD *punSerialNumber );


#ifdef __cplusplus
}
#endif 

