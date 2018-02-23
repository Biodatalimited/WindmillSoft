/**
***     ATSelect.c              IMLAPPS-IML Applications Toolkit
***
***     IMLPAPPS-ATK-P-1.01	Andrew Thomson
***
***     This file contains the source for the following toolkit
***     functions :
***
***             ATKSelectChannels
***             ATKSelectChannelsDialogFn
***
***
***     The functions in this module should be used by
***     Windmill applications to select a subset of channels
***     from a specified loaded setup file to be 'connected'
***     to the application. An array of connected channel
***     structures is created for use by the application.
***
***     This module has one entry point from the application.
***     This is via a call to the ATKSelectChannels function.
***
***     To allow the user to make the Connections selection...
***
***     Errors, if they occur, will be reported to the user. An
***     error code will also be returned from the function call
***     allowing the application to act upon the error.
***
***     The application MUST unlock the setup, using IMSUnlockSetup,
***     once it is no longer required. This allows the setup to be
***     edited by other applications.
***
***
***
***     To use this module :
***
***     - Include the function ATKSelectChannelsDialogFn in the
***       Export section of the application's DEF file.
***
***     - Include the line 'rcinclude ATSelect.rc' in the
***       application's Resource file.
***
***     - Specify the include file 'ATKit.h' in the module
***       which will call ATKLoadSetup. This should be located
***       after including <windows.h>
***
***      - Specify the include file 'IMSLIB.h' if you have not 
***        already done so.
***
***      - Include the following toolkit files in the build list :
***
***             ATSelect.c, ATError.c
***
***
***
***
***     Assoiciated Files
***
***             ATKIT.h       - Contains the Fn prototypes, structure
***                           declarations and manifest constants.
***
***             ATSELECT.dlg  - Resource script for the 'Channel Connections'
***                             dialog box.
***
***     VERSION HISTORY :
***
***     17 September 91    ATK-P-1.00   First Version Written.
***     1 October   92    ATK-P-1.01   Error Message if no suitable
***                                    channels available.
***
**/

#define NOKANJI
#define NOCOMMS
#define IML_APPLICATION

#include <windows.h>
#include <string.h>
#include "ATKit.h"
#include "\Windmill 32\Compile Libs\imslib.h"




/**     Intrinsic Function Prototypes
**/

short FillConnectedChannelList ( HWND hDlg,
                   PATKCHANINFO pciConnections, short nNumConnected );

short FillUnconnectedChannelList ( HWND hDlg, PATKCHANINFO pciConnections,
                      short nNumConnected, PCHANNELINFO lpSummary,
                      short nSummaryItems, short wChanTypes );

short ConnectChannels ( HWND hDlg, short nMaxConnections );

short DisconnectChannels ( HWND hDlg );

void  EnableConnectAll ( HWND hDlg, short nMaxConnections );

void  SelectAll ( HWND hDlg );

short CreateConnectionsArray ( HWND hDlg, PCHANNELINFO lpSummary,
  short nNumSummaryChans, HANDLE *phNewArrayHandle, short *pnNumElements );



/**     Intrinsic Variables used to pass
***	information in and out of the Dialog Function
**/
static  short   nDlgStatus;               /* Error code incurred in Dlg Fn  */
static  short   nNumConnectedChannels;    /* Number of Channels connected   */
static  HANDLE  hConnectionsArray;        /* Handle to Array of Connections */
static  WORD    wChannelTypes;            /* Permissable Channel types      */
static  short   nMaxConnections;          /* Max Permissable Connections    */

static  char    szIMSSetup [ MAX_NAME_LENGTH + 1 ];   /* IMS setup to use   */
static  DLGPROC lpfnATKSelectChannelsDialogFn;   /* dlg proc-instance addr  */




/**
***   ATKSelectChannels
***
***   This function is used to select a subset of channels from the
***   loaded setup specified by parameter szSetupName. The selected
***   channels may  then be  used the application as 'connections'.
***
***   The channel subset is returned as an array of ATKChanInfo structs
***   the handle to which is returned by reference in phConnected.
***   The calling application is responsible for freeing this memory
***   when it is no longer required.
***
***   The 'wType' parameter may be used to determine which types of
***   channels may be selected, and the 'nMaxChans' parameter determines
***   how many channels may be selected (i.e. The maximum number of
***   connections that the application can handle )
***
***
***
***   Arguments
***
***   hInst           The Instance handle of the calling Application.
***   hWndParent      The Window Handle of the calling Application.
***                     (usually the application's Window handle)
***   szSetupName     The IMS setup name from which channels may be
***                     selected.
***   phConnected	  Pointer to a handle for memory containing
***                   ATKCHANNEL data for connected channels. NULL if
***                   there are none.
***   phChans         Pointer to a short in which the number of selected
***                     channels is returned by reference.
***   nMaxChans       The maximum number of channels which may be
***                     selected.
***   wType           This is used to select which types of channels may
***                     be connected. The following constants may be
***                     used. The constants may be OR'ed to obtain
***                     combination types.:
***
***         TYPE_A          Multiplexable Analog Inputs
***         TYPE_I          Value Input Channels
***         TYPE_O          Value Output Channels
***         TYPE_V          Value Input and Output Chans
***         TYPE_B          Bit digital Inputs
***         TYPE_R          Bit digital Outputs
***         TYPE_T          Bit digital Inputs and Outputs
***         TYPE_M          Multiplexer control channels
***         TYPE_U          Unused dummy channels
***         TYPE_INPUT      All input types
***                  ( TYPE_A | TYPE_I | TYPE_V | TYPE_B | TYPE_T )
***         TYPE_OUTPUT     All output types
***                  ( TYPE_O | TYPE_R | TYPE_T | TYPE_V )
***
***    Return Value :  0     - No Error, one or more connections
***                            have been sucessfully made.
***
***      non-zero        - A predefined error code from the
***                       IMSLIB DLL or a from the toolkit
***                       see ATKit.h ). No Connections have
***                       been made.
**/
short ATKSelectChannels ( HANDLE hInst, HWND hWndParent, char *szSetupName,
	   PHANDLE phConnected, short *pnChans, short nMaxChans, WORD wType )
{
	short   nResult;


	if ( lpfnATKSelectChannelsDialogFn == NULL ) {

		/**  Create a procedure-instance address to bind
	***  the Dialog call-back function to the instance's
	***  data segment
	**/

		lpfnATKSelectChannelsDialogFn =
	 (DLGPROC) MakeProcInstance ( ATKSelectChannelsDialogFn, hInst );


	}


	/**     Set local variables which will be used by to pass
	***     information between this fn and the 'Select Channels'
	***     Dialog Handling function.
	**/
	nDlgStatus		= OK;	  /* Error code incurred in Dlg Fn  */
	wChannelTypes   = wType;  /* Permissable Channel types      */
	nMaxConnections = nMaxChans;   /* Max allowable connections */
	strcpy ( szIMSSetup, szSetupName );    /* The IMS setup name        */

	/**
	*** Dereference the pointers to the Channel array handle and
	*** the number of channels short. First check to see if the array
	*** handle pointer is NULL. If this is so then no array
	*** exists and will we need to create one later. Set the channels
	*** to 0.
	**/
	hConnectionsArray  = *phConnected;
	nNumConnectedChannels = ( hConnectionsArray == NULL ) ? 0 : *pnChans;


	/**     Invoke the modal-dialog box to obtain the connections
	**/
	nResult = DialogBox ( hInst, (LPSTR) "ATK_CONNECTIONS",
	                       hWndParent, lpfnATKSelectChannelsDialogFn );

	if ( nResult == FALSE ) {

		/**     The Dialog failed or was canceled, inform
		***	the user and the calling application which may
		***	have to deal with having no connections.
		**/
		ATKMessageBox ( hInst, hWndParent, nDlgStatus, (LPSTR) NULL,
			                                 MB_OK | MB_ICONEXCLAMATION );

		return ( nDlgStatus );

	} else {


		/** 
		***  pass back the number of channels and the handle of
		***  the ATK channel array.
		**/
		*phConnected = hConnectionsArray;
		*pnChans	 = nNumConnectedChannels;

		return ( OK );
	}
}



/**
***	ATKSelectChannelsDialogFn
***
***
***	This is the Dialog callback function for the
***	'Channel Connections' Dialog.
***
***	Current control settings are stored in static variables
***	declared at the beginning of this module,.
***
***	Arguments
***	hDlg          Identifies the dialog box that receives the message.
***	  wMsg			Specifies the message number.
***	  wParam        Specifies 32 bits of additional message-dependent
***	                  information.
***	  lParam        Specifies 32 bits of additional message-dependent
***	                  information.
***
***
***
***	Return Value
***	     FALSE  - If message not processed
***	     TRUE   - If message Processed
***
***
***
**/
short CALLBACK ATKSelectChannelsDialogFn ( HWND hDlg, DWORD iMessage,
                                          WPARAM wParam, LPARAM lParam )
{
	static  PATKCHANINFO    pciConnections;    /* Ptr to ATKChanInfo    */
	static  GLOBALHANDLE    ghSummary;         /* handle to IMS Summary */
	static  PCHANNELINFO    lpSummary;         /* Ptr to IMS Summary    */
	static  short           nSummaryItems;     /* Elements in Summary   */

	LOCALHANDLE             hNewArray;
	short                   hNumElements;
	short                   nReturn;

	char    szDescription [ ATK_DESC_LEN + 1 ];  /* IMS desc string     */
	char    szDlgDesc [ ATK_DLG_DESC_LEN + 1 ];  /* Dialog Desc         */





	switch ( iMessage ) {

		case ( WM_INITDIALOG ) :        /* Request to Initialise */

			/**     Request a summary from IMSLib **/
			ghSummary = IMSGetSetupSummary ( (LPSTR) szIMSSetup,
			                        &nSummaryItems );
			if ( ghSummary == NULL ) {

				/**     The summary could not be obtained, in this
				***     case, nSummaryItems contains an error code
				**/
				nDlgStatus = nSummaryItems;
				EndDialog ( hDlg, FALSE );
				return ( FALSE );
			}

			/**     Lock the summary array**/
			lpSummary = (PCHANNELINFO) GlobalLock ( ghSummary );
			if ( lpSummary == NULL ) {

				/**     No memory...**/
				GlobalFree ( ghSummary );
				nDlgStatus = ATK_ERR_NO_MEMORY_IN_DLG;
				EndDialog ( hDlg, FALSE );
				return ( FALSE );
			}


			/** Lock the Existing ATKChanArray,
			*** if it exists. If not set the pointer
			*** to NULL, to indicate we have no
			*** existing array.
			*/
			pciConnections = ( hConnectionsArray == NULL ) ? NULL :
				 (PATKCHANINFO) LocalLock ( hConnectionsArray );


			/**     Initialise the controls.....
			**/

			/**     Display the description from the IMS file
			**/
			IMSGetSetupDescription ( (LPSTR) szIMSSetup,
					(LPSTR) szDescription, ATK_DESC_LEN );

			SetDlgItemText ( hDlg, IDC_SETUP_COMMENT,
					(LPSTR) szDescription );

			/**     Append the setup name onto the end of the
			***     dialog description text.
			**/
			GetDlgItemText ( hDlg, IDC_DLG_DESC, (LPSTR) szDlgDesc,
					ATK_DLG_DESC_LEN );

			strcat ( szDlgDesc, szIMSSetup );

			SetDlgItemText ( hDlg, IDC_DLG_DESC, (LPSTR) szDlgDesc );

			/**     Fill the Connected and Unconnected List Boxes
			***
			***     To fill the Connected listbox we need only the
			***     current ATKInfoArray ( if one exists ). For the
			***     Unconnected listbox we also need the IMS summary
			***     array.
			**/
			nReturn = FillConnectedChannelList ( hDlg,
					  pciConnections, nNumConnectedChannels );

			if ( nReturn != OK ) {

				/**     Cannot initialise the dialog.
				***     Store the error for the caller
				***     and terminate the dialog
				**/
				GlobalFree ( ghSummary );

				nDlgStatus = ATK_ERR_NO_MEMORY_IN_DLG;
						EndDialog ( hDlg, FALSE );
						return ( FALSE );

			}

			nReturn = FillUnconnectedChannelList ( hDlg,
					pciConnections,
					nNumConnectedChannels, lpSummary,
					nSummaryItems, wChannelTypes );


			if ( nReturn != OK ) {

				/**     Cannot initialise the dialog.
				***     Store the error for the caller
				***     and terminate the dialog
				**/
				GlobalFree ( ghSummary );

				nDlgStatus = ATK_ERR_NO_MEMORY_IN_DLG;
						EndDialog ( hDlg, FALSE );
						return ( FALSE );

			}




			/**	Verify that atleast one of the listboxes
			***	contains atleast one channel. If both
			***	lists are empty it is pointless continuing.
			**/
			if ( ( SendDlgItemMessage ( hDlg, IDC_CONNECTED_LIST,
					LB_GETCOUNT, 0, 0 ) +
				SendDlgItemMessage ( hDlg, IDC_UNCONNECTED_LIST,
							LB_GETCOUNT, 0, 0 ) ) == 0 ) {


				/** Cannot initialise the dialog.
				*** Store the error for the caller
				*** and terminate the dialog
				**/
				GlobalFree ( ghSummary );

				nDlgStatus = ATK_ERR_NO_SUITABLE_CHANS;
				EndDialog ( hDlg, FALSE );

				return ( FALSE );

			}

			/**     Enable ( or disable ) the ConnectALL button
			**/
			EnableConnectAll ( hDlg, nMaxConnections );

			/**	Leave the summary open, we will need it when
			***	the user OK's the dialog to find required
			***	information on the 'connected' channels.
			***
			***	Return TRUE since we havn't used SetFocus
			**/
			return ( TRUE );

		case ( WM_COMMAND ) :

			switch ( LOWORD (wParam) ) {

				case IDC_CONNECTED_LIST :

					switch ( HIWORD ( wParam ) ) {

						case LBN_SELCHANGE :
							break;

						case LBN_DBLCLK :
							DisconnectChannels ( hDlg );
							/**  Enable ( or disable )
							***  the ConnectALL button
							**/
							EnableConnectAll ( hDlg,
							nMaxConnections );
							break;

					}
					return ( TRUE );

				case IDC_UNCONNECTED_LIST :

					switch ( HIWORD ( wParam ) ) {

						case LBN_SELCHANGE :
							break;

						case LBN_DBLCLK :
							ConnectChannels ( hDlg,
							nMaxConnections );
							/** Enable ( or disable )
							*** the ConnectALL button
							**/
							EnableConnectAll ( hDlg,
							nMaxConnections );
							break;

					}
					return ( TRUE );

				case IDC_CONNECT :
					ConnectChannels ( hDlg, nMaxConnections );
					/** Enable ( or disable )
					*** the ConnectALL button
					**/
					EnableConnectAll ( hDlg, nMaxConnections );
					return ( TRUE );

				case IDC_DISCONNECT :
					DisconnectChannels ( hDlg );
					/** Enable ( or disable )
					*** the ConnectALL button
					**/
					EnableConnectAll ( hDlg, nMaxConnections );
					return ( TRUE );

				case IDC_CONNECT_ALL :
					SelectAll ( hDlg );
					ConnectChannels  ( hDlg, nMaxConnections );
					EnableConnectAll ( hDlg, nMaxConnections );
					return ( TRUE );

				case IDOK :
					/**  Create an array of ATKChanInfo
					***  structures, with one structure
					***  for every item in the 'Connected'
					***  list box.
					**/
					nReturn = CreateConnectionsArray ( hDlg,
					lpSummary, nSummaryItems,
					&hNewArray, &hNumElements );

					if ( nReturn != OK ) {

						/** Error - Inform the user and
						*** break.
						**/
						ATKMessageBox ( (HANDLE) GetWindowLong ( hDlg,
							GWL_HINSTANCE ),
							hDlg, nReturn,
							(LPSTR) NULL,
							MB_OK | MB_ICONEXCLAMATION );
							break;

					} else {
						/** Destroy the old ATKChanInfo array
						*** and overwrite the handle with the
						*** handle of the new array.
						**/
						if ( hConnectionsArray != NULL ) {

							LocalUnlock ( hConnectionsArray );
							LocalFree   ( hConnectionsArray );
						}
						/** Store the handle and size of the
						*** new array in variables accessible
						*** to the caller fucntion.
						**/
						hConnectionsArray     = hNewArray;
						nNumConnectedChannels = hNumElements;

					}

					GlobalUnlock ( ghSummary );
					GlobalFree   ( ghSummary );
					EndDialog ( hDlg, TRUE );
					break;

				case IDCANCEL :

					if ( hConnectionsArray != NULL ) {
						LocalUnlock ( hConnectionsArray );
					}

					GlobalUnlock ( ghSummary );
					GlobalFree   ( ghSummary );
					EndDialog ( hDlg, FALSE );
					break;

			}
		break;
	}
	return ( FALSE );
}





/**
***     FillConnectedChannelList
***
***
***     This function uses an array of ATKChanInfo structs to
***     obtain all the presently connected channels. The channel
***     names are extracted from the array and added to the
***     'Connected channels' listbox.  The array of ATKChanInfo
***     structs would normally be passed into this module from
***     the application. It may also be empty, indicated by a
***     NULL pointer.
***
***     Arguments
***
***     hDlg                    Dialog Handle which owns the list box
***                             control
***
***     pciConnections          Near Pointer to the Array of Connected
***                             Channel structures.  If the pointer is
***                             NULL, no connected channels exist.
***
***     nNumConnected           The number of array components in the
***                             array.
***
***     Return Value :
***                     0        - List filled sucessufully.
***                     Non-zero - List not filled, the error code
***                                may be processed by ATKMessageBox
**/
short FillConnectedChannelList ( HWND hDlg,
		PATKCHANINFO pciConnections, short nNumConnected )  

{


	HWND    hWndListBox;    /* Window Handle to Connections List Box */
	short   nConnection;    /* used to index ATKChanInfo Array       */


	/**     First Check we have a ATKChanInfo Array
	***     containing Connected channel details.
	**/
	if ( pciConnections == NULL ) {

		/**     No connections, simply exit the fn **/
		return ( OK );
	}


	/**     Obtain a handle to the list box so we may send it
	***     messages
	**/
	hWndListBox = GetDlgItem ( hDlg, IDC_CONNECTED_LIST );


	/**     Add the channel string from each array component to the
	***     list box
	***
	**/
	for ( nConnection = 0; nConnection < nNumConnected; nConnection++ ) {

		SendMessage ( hWndListBox, LB_ADDSTRING, 0,
		     (LONG) (LPSTR) pciConnections [ nConnection ].szChannel );

	}

	return ( OK );
}






/**
***     FillUnconnectedChannelList
***
***
***     This function uses the IMS setup summary to obtain
***     all the channels of the specified type. Each channel
***     name is extracted from the summary and IF not present
***     in the ATKChanInfo array, added to the list box.
***
***     Arguments
***
***     hDlg                    Dialog Handle which owns the list box
***                             control
***
***      pciConnections         Near Pointer to the Array of Connected
***                             Channel structures.  If the pointer is
***                             NULL, no connected channels exist.
***
***     nNumConnected           The number of array components in the
***                             array.
***
***     lpSummary               Pointer to an IMS Summary array.
***                             The summary holds details on all the
***                             channels in this dialog.
***
***     nSummaryItems	        The size of IMS summary array
***
***     wChanTypes              The types of channels for which we will
***                             obtain summary information.
***                             ( See wType in ATKSelectChannels for desc.)
***
***     Return Value :
***                     0        - List filled sucessufully.
***                     Non-zero - List not filled, the error code
***                                may be processed by ATKMessageBox
**/
short FillUnconnectedChannelList ( HWND hDlg, PATKCHANINFO pciConnections,
                                   short nNumConnected, PCHANNELINFO lpSummary,
                                   short nSummaryItems, short wChanTypes )
{
	HWND    hWndListBox;    /* Window Handle to Connections List Box */
	short   nConnection;    /* used to index ATKChanInfo Array       */
	short   nChannel;       /* Used to index the IMS summary array   */

	BOOL    bConnected;
	BOOL    bPermissibleType;



	/**     Obtain a handle to the list box so we may send it
	***     messages
	**/
	hWndListBox = GetDlgItem ( hDlg, IDC_UNCONNECTED_LIST );



	/**     Interrogate the summary array, searching for
	***     enabled channels of the required type(s)
	**/
	for ( nChannel = 0; nChannel < nSummaryItems; nChannel++ ) {

		/**     Is this channel type required.**/
		switch ( lpSummary [ nChannel ].cTypeCode ) {

			case 'A' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_A );
				break;

			case 'I' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_I );
				break;

			case 'O' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_O );
				break;

			case 'V' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_V );
				break;

			case 'B' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_B );
				break;

			case 'R' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_R );
				break;

			case 'T' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_T );
				break;

			case 'M' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_M );
				break;

			case 'U' : bPermissibleType = (BOOL) ( wChanTypes & TYPE_U );
				break;

			default  : bPermissibleType = FALSE;
		}

		if ( ( !bPermissibleType ) ||
		        ( !lpSummary [ nChannel ].bEnabled ) ) {

			/**  The channel does not belong to
			***  one of the permissable channel types
			***  or is disabled so ignore it.
			**/
			continue;
		}



		/**     Is the channel already connected
		***     search the ATKChanInfo array which
		***     contains the connected channels. Set
		***     bConnected accordingly so we know
		***     on exit from the search whether to add
		***     the channel to the list or not.
		**/
		bConnected = FALSE;

		if ( pciConnections != NULL ) {

			for ( nConnection = 0; nConnection < nNumConnected;
			                nConnection++ ) {

				/**     Step thru' the ATKChanInfo array
				***     searching for a matching channel
				***     name.
				**/
				if ( lstrcmp ( lpSummary [ nChannel ].szName,
				         (LPSTR) pciConnections [ nConnection ].szChannel )
				                            == 0 ) {

					/** This channel is connected,
					*** set the 'bConnected' flag.
					**/
					bConnected = TRUE;
					break;
				}
			}
		}

		if ( !bConnected ) {

			/** Add to the Unconnected List Box
			**/
			SendMessage ( hWndListBox, LB_ADDSTRING, 0,
 			   (LONG) (LPSTR) lpSummary [ nChannel ].szName );
		}

		/**     Next Channel
		**/
	}


	return ( OK );
}


/**
***     ConnectChannels
***
***     This function moves all the selected channels in the
***     'Unconnected' list box and places them in the 'Connected'
***     list box provided that the resulting number of connected
***     entries does NOT exceed nMaxConnections.
***
***     If moving the selections would exceed nMaxConnections, a
***     message-box is presented and the selections ignored.
***
***     Arguments :
***
***     hDlg        The Window Handle of the Dialog
***                  owning the listboxes.
***
***     nMaxConnections         The maximum number of entries
***                             permitted in the Connections
***                             list box.
***     Return Value :
***
***     0                           Channels Connected if permissable.
***     ATK_ERR_NO_MEMORY_IN_DLG    Insufficient memory .
**/
short ConnectChannels ( HWND hDlg, short nMaxConnections )
{

	HWND            hWndConnectedListBox;
	HWND            hWndUnconnectedListBox;
	LOCALHANDLE     hSelections;
	long*   pSelections;
	long    lNumSelections;
	long    lSelection;
	long    lAlreadyConnected;
	char    szSelectionString [ MAX_NAME + 1 ];
	UINT temp;


	/**     Obtain the handle to the two list boxes
	**/
	hWndConnectedListBox   = GetDlgItem ( hDlg, IDC_CONNECTED_LIST );
	hWndUnconnectedListBox = GetDlgItem ( hDlg, IDC_UNCONNECTED_LIST );




	/** Create an integer buffer in which SendMessage will store the
	*** the current list box selections indexes.
	***
	*** First determine the  number of selections so we may
	*** allocate an appropriately sized buffer to store the indexes.
	*** We can determine this by sending a LB_GETSELCOUNT message to
	*** the list box.
	**/
	lNumSelections =  SendMessage ( hWndUnconnectedListBox,
	                                  LB_GETSELCOUNT, 0, 0L );

	hSelections = GlobalAlloc ( LMEM_MOVEABLE,
	                         lNumSelections * sizeof (long) );

	if ( hSelections == NULL ) {

		return ( ATK_ERR_NO_MEMORY_IN_DLG );
	}

	temp = GlobalSize ( hSelections );


	/**     Lock the buffer
	**/
	pSelections = (long *) GlobalLock ( hSelections );

	if ( pSelections == NULL ) {

		GlobalFree ( hSelections );
		return ( ATK_ERR_NO_MEMORY_IN_DLG );

	}



	/**     Get the array of selected items
	**/
	lNumSelections = SendMessage ( hWndUnconnectedListBox,
	                        LB_GETSELITEMS, lNumSelections,
	                                   (LONG)  pSelections );


	/**     Check the user has not made too many selections
	***     inform him if he has....
	**/
	lAlreadyConnected = SendMessage ( hWndConnectedListBox,
	                                    LB_GETCOUNT, 0, 0L );

	if ( ( lNumSelections + lAlreadyConnected ) > nMaxConnections ) {

		/**     Too many items have been selected,
		***     inform the user and ignore the selections
		**/
		ATKMessageBox ( (HINSTANCE) GetWindowLong ( hDlg, GWL_HINSTANCE ),
		            hDlg, ATK_ERR_TOO_MANY_CONNECTIONS,
		               (LPSTR) NULL,  MB_OK | MB_ICONEXCLAMATION );

		GlobalUnlock ( hSelections );
		GlobalFree   ( hSelections );

		return ( ATK_ERR_TOO_MANY_CONNECTIONS );
	}




	/**     Process the selections. This involves sending
	***     three messages :-
	***
	***        - Get the selected list box entry.
	***        - Add it to the Unconnected List box.
	***        - Delete the entry from the Connected List Box.
	***
	***     The final stage, deleting the the selected entries,
	***     should be performed in a seperate loop to avoid
	***     changing the 'Unconnected' list dynamically.
	**/
	for ( lSelection = 0; lSelection < lNumSelections; lSelection++ ) {

		/**     Obtain the selection and add to the new
		***     list box..
		**/
		SendMessage ( hWndUnconnectedListBox, LB_GETTEXT,
		      pSelections [ lSelection ],
		      (LONG) (LPSTR) szSelectionString );

		SendMessage ( hWndConnectedListBox, LB_ADDSTRING, 0,
		      (LONG) (LPSTR) szSelectionString );
	}

	for ( lSelection = ( lNumSelections - 1 ); lSelection >= 0;
	                                    lSelection-- ) {

		/**     Remove the selections from the Unconnected list
		***
		***     Remember to remove the selections from the
		***     end of the pSelections array first. Removing
		***     them from the beginning will shift the
		***     remaining selections down the list-box and
		***     invalidate the indexes in the pSelections
		***     array.
		**/
		SendMessage ( hWndUnconnectedListBox, LB_DELETESTRING,
		            pSelections [ lSelection ], 0L );
	}


	/**     Tidy up and exit....
	**/
	GlobalUnlock ( hSelections );
	GlobalFree   ( hSelections );

	return ( OK );
}


/**
***     DisconnectChannels
***
***     This function moves all the selected channels in the
***     'Connected' list box and places them in the 'Unconnected'
***     list box.
***
***     Arguements :
***
***             hDlg    The Window Handle of the Dialog
***                     owning the listboxes.
***
***     Return Value :
***
***     0                           Channels Disconnected.
***     ATK_ERR_NO_MEMORY_IN_DLG    Insufficient memory .
**/
short DisconnectChannels ( HWND hDlg )
{

	HWND           hWndConnectedListBox;
	HWND           hWndUnconnectedListBox;
	LOCALHANDLE    hSelections;
	long           *pSelections;
	long           lNumSelections;
	long           lSelection;
	char           szSelectionString [ MAX_NAME + 1 ];



	/**     Obtain the handle to the two list boxes
	**/
	hWndConnectedListBox   = GetDlgItem ( hDlg, IDC_CONNECTED_LIST );
	hWndUnconnectedListBox = GetDlgItem ( hDlg, IDC_UNCONNECTED_LIST );




	/**     Create an integer buffer in which SendMessage will store the
	***     the current list box selections indexes.
	***
	***     First determine the  number of selections so we may
	***     allocate an appropriately sized buffer to store the indexes.
	***     We can determine this by sending a LB_GETSELCOUNT message to
	***     the list box.
	**/
	lNumSelections = SendMessage ( hWndConnectedListBox,
	                              LB_GETSELCOUNT, 0, 0L );


	hSelections = LocalAlloc ( LMEM_MOVEABLE,
	                    (WORD) lNumSelections * sizeof (long) );

	if ( hSelections == NULL ) {

		return ( ATK_ERR_NO_MEMORY_IN_DLG );
	}




	/**     Lock the buffer
	**/
	pSelections = (long *) LocalLock ( hSelections );

	if ( pSelections == NULL ) {

		LocalFree ( hSelections );
		return ( ATK_ERR_NO_MEMORY_IN_DLG );

	}



	/**     Get the array of selected items
	**/
	lNumSelections = SendMessage ( hWndConnectedListBox,
	                            LB_GETSELITEMS, lNumSelections,
	                            (LONG) (long *) pSelections );






	/**     Process the selections. This involves sending
	***     three messages :-
	***
	***        - Get the selected list box entry.
	***        - Add it to the Unconnected List box.
	***        - Delete the entry from the Connected List Box.
	***
	***     The final stage, deleting the the selected entries,
	***     should be performed in a seperate loop to avoid
	***     changing the 'Connections' list dynamically.
	***
	**/
	for ( lSelection = 0; lSelection < lNumSelections; lSelection++ ) {

		SendMessage ( hWndConnectedListBox, LB_GETTEXT,
			pSelections [ lSelection ],
			(LONG) (LPSTR) szSelectionString );

		SendMessage ( hWndUnconnectedListBox, LB_ADDSTRING, 0,
			(LONG) (LPSTR) szSelectionString );

	}
	for ( lSelection = ( lNumSelections - 1 ); lSelection >= 0;
	                     lSelection-- ) {

		/**     Remove the selections from the Unconnected list
		***
		***     Rememeber to remove the selections from the
		***     end of the pSelections array first. Removing
		***     them from the beginning will shift the
		***     remaining selections down the list-box and
		***     invalidate the indexes in the pSelections
		***     array.
		**/
		SendMessage ( hWndConnectedListBox, LB_DELETESTRING,
		              pSelections [ lSelection ], 0L );


	}

	/**     Tidy up and exit....
	**/
	LocalUnlock ( hSelections );
	LocalFree   ( hSelections );

	return ( OK );
}




/**
***     EnableConnectAll
***
***
***     This function Enables ( or Disables ) the 'Connect All'
***     pushbutton. The button is enabled if connecting all the
***     currently unconnected channels would NOT exceed the maximum
***     permissible number of connections.
***
***     ARGUMENTS
***
***     hDlg                    The Window Handle of the Dialog
***                             owning the push-button.
***
***     nMaxConnections         The maximum permissable number of
***                             connections.
***
***
***     RETURN VALUE            void.
***
**/
void EnableConnectAll ( HWND hDlg, short nMaxConnections )
{

	short   nNumConnected;
	short   nNumUnconnected;
	HWND    hWndConnectedListBox;
	HWND    hWndUnconnectedListBox;



	/**     Obtain the handle to the two list boxes
	**/
	hWndConnectedListBox   = GetDlgItem ( hDlg, IDC_CONNECTED_LIST );
	hWndUnconnectedListBox = GetDlgItem ( hDlg, IDC_UNCONNECTED_LIST );



	/**     if the number of unconnected channels
	***     + the number of connected channels is less
	***     than or eqaul to the maximum connections then
	***     enable the ConnectAll button otherwise
	***     disable it..
	**/


	nNumConnected =   (short) SendMessage ( hWndConnectedListBox,
	                                       LB_GETCOUNT, 0, 0L );

	nNumUnconnected = (short) SendMessage ( hWndUnconnectedListBox,
	                                       LB_GETCOUNT, 0, 0L );

	if ( ( nNumConnected + nNumUnconnected <= nMaxConnections ) &&
	                                  ( nNumUnconnected > 0 ) ) {

		/**      Adding all unconnected channels will not
		***      exceed the maximum allowed...
		**/
		EnableWindow ( GetDlgItem ( hDlg, IDC_CONNECT_ALL ), TRUE );

	} else {
		EnableWindow ( GetDlgItem ( hDlg, IDC_CONNECT_ALL ), FALSE );
	}

	return;
}




/**
***     SelectAll
***
***     This function is used to select ALL items in the
***     'Unconnected' list box and should be used prior
***     to a call to the 'ConnectChannels' fn when the
***     user requests that all channels be connected.
***
***     ARGUMENTS
***
***     hDlg                    The Window Handle of the Dialog
***                             owning the push-button.
***
***     RETURN VALUE            void.
***
**/
void SelectAll ( HWND hDlg )
{
	short   nNumUnconnected;
	HWND    hWndUnconnectedListBox;



	/**     Obtain the handle to the two list boxes
	**/
	hWndUnconnectedListBox = GetDlgItem ( hDlg, IDC_UNCONNECTED_LIST );

	nNumUnconnected = (short) SendMessage ( hWndUnconnectedListBox,
	                                      LB_GETCOUNT, 0, 0L );


	/**     Selection Range is 0 to ( nNumUnconnected - 1 )
	**/
	SendMessage ( hWndUnconnectedListBox, LB_SELITEMRANGE, 1,
	                       MAKELONG ( 0, nNumUnconnected - 1 ) );


	return;
}




/**
***     CreateConnectionsArray
***
***
***     This function creates an array of ATKChanInfo structures.
***     One array component is created for EACH channel listed in
***     'Connections' listbox.
***     Each structure field is then initialised using either default
***     values or information stored in the IMS summary for that
***     channel.
***
***     The array handle and the size of the array are returned by
***     reference.
***
***     ARGUMENTS
***
***       hDlg                  Window handle of Dialog
***
***       lpSummary             Pointer to an IMS Summary array.
***                             The summary holds details on all the
***                             channels in this dialog.
***
***       nNumSummaryChans      The size of IMS summary array
***
***       phNewArrayHandle      Pointer to a LOCALHANDLE in which to
***                             store the new handle to the array of
***                             ATKChanInfo structures. These detail the
***                             connected channels.
***
***       pnNumElements         The number of structures in the
***                             ATKChanInfo array.
***
***     RETURN VALUE
***
***       OK                        Array created sucessfully.
***       ATK_ERR_NO_CONNECTIONS        No connections specified in dialog.
***       ATK_ERR_NO_MEMORY_IN_DLG      Insufficient memory.
***
**/
short CreateConnectionsArray ( HWND hDlg, PCHANNELINFO lpSummary,
			short nNumSummaryChans, HANDLE *phNewArrayHandle,
			short *pnNumElements )
{
	long            lNumConnected;
	HWND            hWndConnectedListBox;
	LOCALHANDLE     hNewArray;
	PATKCHANINFO    pNewChanArray;
	short           nConnection;
	short           nChannel;
	char            szConnection [ MAX_NAME + 1 ];



	/**     Default the values which are returned by reference
	***     incase the function exits prematurely with an error.
	**/
	*phNewArrayHandle = NULL;
	*pnNumElements    = 0;


	/**     Obtain the handle to the connected list box
	**/
	hWndConnectedListBox = GetDlgItem ( hDlg, IDC_CONNECTED_LIST );

	lNumConnected = SendMessage ( hWndConnectedListBox,
	                                       LB_GETCOUNT, 0, 0L );



	if ( lNumConnected == 0 ) {

		return ( ATK_ERR_NO_CONNECTIONS );
	}



	/**     Allocate the new array
	**/
	hNewArray = LocalAlloc ( LMEM_MOVEABLE, (long) (lNumConnected *
	                                sizeof ( ATKCHANINFO )) );

	if ( hNewArray == NULL ) {

		return ( ATK_ERR_NO_MEMORY_IN_DLG );
	}


	/**     Lock the new array
	**/
	pNewChanArray = (PATKCHANINFO) LocalLock ( hNewArray );

	if ( pNewChanArray == NULL ) {

		LocalFree ( hNewArray );
		return ( ATK_ERR_NO_MEMORY_IN_DLG );
	}




	for ( nConnection = 0; nConnection < lNumConnected; nConnection++ ) {


		/**     Get the connected channel's name
		**/
		SendMessage ( hWndConnectedListBox, LB_GETTEXT,
		         (WORD) nConnection, (LONG) (LPSTR) szConnection );



		/**     Find the connection's entry in the IMS Summary
		**/
		for ( nChannel = 0; nChannel < nNumSummaryChans; nChannel++ ){

			if ( lstrcmp  ( szConnection,
				lpSummary [ nChannel ].szName ) == 0 ) {

				/**     Corresponding entry found
				***     in the setup summary
				**/
				break;
			}
		}


		/**     Initialise the structure, defaulting unknown fields
		***
		**/
		lstrcpy ( pNewChanArray [ nConnection ].szChannel,
		                                     szConnection );

		pNewChanArray [ nConnection ].nChannelNumber =
		                              lpSummary [ nChannel ].nChannel;

		pNewChanArray [ nConnection ].nDeviceNumber =
		                              lpSummary [ nChannel ].nFrame;


		/**	Is this channel digital ?
		**/
		switch ( lpSummary [ nChannel ].cTypeCode ) {

			case 'B' :
			case 'R' :
			case 'T' : pNewChanArray [ nConnection ].bDigital = TRUE;
				break;

			default  : pNewChanArray [ nConnection ].bDigital = FALSE;
				break;
		}


		/**     The application must use ATKChannelUnits
		***     to obtain the units string even though this
		***     is available in the summary.
		**/
		pNewChanArray [ nConnection ].szUnits [ 0 ] = '\0';


		pNewChanArray [ nConnection ].nError        = 0;
		pNewChanArray [ nConnection ].szValue [ 0 ] = '\0';
		pNewChanArray [ nConnection ].ulTime        = 0L;
		pNewChanArray [ nConnection ].nMSecs        = 0;
		pNewChanArray [ nConnection ].szMin [ 0 ]   = '\0';
		pNewChanArray [ nConnection ].szMax [ 0 ]   = '\0';
		pNewChanArray [ nConnection ].szOn  [ 0 ]   = '\0';
		pNewChanArray [ nConnection ].szOff [ 0 ]   = '\0';



	}  /*  Next Connection */



	/**     Unlock the new array...
	**/
	LocalUnlock ( hNewArray );

	/** Return the new handle and the number of elements
	*** in the array by reference...
	**/
	*phNewArrayHandle = hNewArray;
	*pnNumElements    = (short) lNumConnected;
	return ( OK );

}
