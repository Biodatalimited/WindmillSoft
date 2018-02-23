/**
***	WPDO_DDE.c		ATK-PANELS - Toolkit Module
***				DDE Support Fns for
***				Windmill Display and Control Panels
***     Andrew Thomson
***
***	This module provides the Data Panels with DDE support.
***	It is implemented using the Windows 3.1 DDE Manager Library.
***
***	The Data Panel is a DDE Server and must be accessed by a
***	DDE Client.
***
***
***	This server will respond to :
***
***	  Service name ( formally appl name )	   : <AppName>_<IMSSetup>
***
***
***	  Topics				   : 'System'
***						   : 'Units'
***						   : 'Data'
***						   : 'Range'
***						   : 'Error'
***						   : 'Alarm'
***
***
***	  System Topic Items			   : 'ItemList'
***						   : 'Channels'
***						   : 'Topics'
***						   : 'Formats'
***						   : 'SysItems'
***						   : 'Help'
***						   : 'Status'
***						   : 'ExecuteList'
***
***	  Nonsystem Topic Item Names		   : <Active Channel Name>
***						   : 'AllChannels'
***						   : 'Commands'
***
***	  Executes			   : 'Minimize'
***						   : 'Maximize'
***						   : 'Restore'
***						   : 'Destroy'
***						   : 'LoadSetup'
***						   : 'Move'
***
***
***
***
***
***
***
***	Associated Files : 	
***
***	Version History :
***	WMILL4-PANELS-4.01	-Redundant code removed from
***				 DDEStartConverstaion; which caused each item
***				 to register its itemname string twice.
***				-Addition of DDE executes
***				-Addition of Alarm topic
***
***
**/

#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT
#define  CTLAPIS
#define  IML_APPLICATION

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <DDEML.h>	/* DDE Manager Library				 */

#include "\Windmill 32\Compile Libs\IMLinter.h"	/* ...using the COMMSLIB Dynalink		 */
#include "\Windmill 32\Compile Libs\imslib.h"     /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATkit\ATKit.h"      /* Application Toolkit Header/Prototypes         */


#include "WPDO_def.h"	/* Manifest Constants and Structure Declarations */
			/* Functions Prototypes				 */

#define  NDDE_TOPICS		7 /* No supported Topics including System */
#define  NDDE_SYSTEMITEMS	8 /* No of Items in System Topic	  */

#define  SZDDE_SERVICENAME	"DigitalOut"
#define  SZDDE_SUPPORTED_FORMAT	"CF_TEXT"

#define  SZDDE_TOPIC_DATA	"Data"		/* Topic Strings	*/
#define  SZDDE_TOPIC_UNITS	"Units"
#define  SZDDE_TOPIC_RANGE	"Range"
#define  SZDDE_TOPIC_ERROR	"Error"
#define  SZDDE_TOPIC_ALARM	"Alarm"
#define  SZDDE_TOPIC_COMMANDS	"Commands"


/**	System Item strings are defined in DDEML.h
***	we will add two addition system items.
**/
#define  SZDDE_ITEM_CHANNELS	"Channels"	/* Addt Sysitem strings */
#define  SZDDESYS_ITEM_EXECUTELIST  "ExecuteList"

/**	Data item names are extracted from a array of connected
***	CHANNEL structures. We will add one additional item ' AllChannels'
**/
#define  SZDATA_ITEM_ALLCHANNELS "AllChannels"	/* Addt dataitem string */
#define  MAX_SERVICE_NAME_LEN	 20

/**	The following identifiers are stored along with the
***	the topic/data handles in TOPICSTRUCTs and ITEMSTRUCTs
***
***	Most transactions pass the DdeCallback function two
***	string handles identifying the topic/data item pair.
***	We search for these in the relavent topic and items tables.
***	When a string handle match is found in a structure the
***	identifier may be used to determine what action is
***	required.
***
***	An alternative method would be to store function pointers
***	This functions would then be used to process the topic or
***	item. This is demonstrated in the Microsoft example.
***	However I believe this would be inefficient for our
***	application were the code for all except the System Topics
***	is virtually identical.
**/
#define  TOPIC_ID_SYSTEM	0	/* Topic Identifiers */
#define  TOPIC_ID_ALARM 	1
#define  TOPIC_ID_COMMANDS	2
#define  TOPIC_ID_DATA		3
#define  TOPIC_ID_ERROR		4
#define  TOPIC_ID_RANGE		5
#define  TOPIC_ID_UNITS		6

#define  SYSITEM_ID_CHANNELS	0	/* System Topic Item Identifiers */
#define  SYSITEM_ID_FORMATS	1
#define  SYSITEM_ID_HELP	2
#define  SYSITEM_ID_STATUS	3
#define  SYSITEM_ID_SYSITEMS	4
#define  SYSITEM_TOPIC_IDS	5
#define  SYSITEM_ID_ITEMLIST	6
#define  SYSITEM_ID_EXECUTELIST 7

#define  DDESTATUS_OK		0	/* DDE Status codes (for ver 3 )    */
#define  DDESTATUS_NOT_TOPIC	1
#define  DDESTATUS_NOT_SYSITEM	2
#define  DDESTATUS_NOT_ITEM	3
#define  DDESTATUS_NO_MEMORY	4
#define  SZSTATUS_MESSAGE	"No Server Errors"  /* Stock V2 status Mess */




/**	A data handle to the following text is returned to DDEML
***	on a request for the System Help item.
***
**/
char szDdeHelp [] = "WINDMILL DISPLAY AND CONTROL PANEL - DDE HELP\r\n\r\n"\
	"The following Topics are supported:\r\n\r\n"\
	"  'Alarm; - The channels alarm status;0-No alarm, 1-Warning, 2-Critical\r\n"\
	"  'Data' - The items data value\r\n"\
	"  'Error' - An error code for the last item operation\r\n"\
	"  'Range' - The minimum and maximum possible data values\r\n"\
	"  'System'- Details DDE support offered by this application\r\n"\
	"  'Units' - The units of the item's data values\r\n\r\n"\
	"The following Execute commands are supported:\r\n\r\n"\
	"  'Load <setup filename>'- Loads a new DigitalOut setup file\r\n"\
	"  'Move <x> <y> <width> <height>'- Repositions and resizes DigitalOut\r\n"\
	"  'Maximize'- Resizes the application to full screen size\r\n"\
	"  'Minimize' -Iconizes the application\r\n"\
	"  'Restore'- Restores the normal Window size if minimized or maximized\r\n"\
	"  'Destroy' - Closes the application down\r\n\r\n"\
	"The System topic supports the following items:\r\n\r\n"\
	"  'Channels' - List of supported items for non-System topics\r\n"\
	"  'ExecuteList' - A list of supported EXECUTE commands\r\n"\
	"  'Formats' - A list of supported clipboard formats\r\n"\
	"  'Help' - General help information\r\n"\
	"  'Status' - The current status of the server\r\n"\
	"  'SysItems' - A list of supported System Items\r\n"\
	"  'Topics' - A list of supported Topic names\r\n"\
	"  'TopicItemList' - List of Supported Items for non-System topics\r\n\r\n"\
	"The Commands topic should be used when sending DDE Execute commands\r\n"
	"All other topics take a currently connected channel name\r\n"\
	"as an item or the special item 'AllChannels' which will\r\n"\
	"return data on all currently connected channels.\r\n";


/**	The following tables contain the topic names and
***	the system item strings. Items strings for the other topics
***	have to be extracted from the pciConnections array when the
***	first convesation begins. Only at this point are the active
***	channel names known.
***
**/
typedef struct _STRINGIDPAIR {

	short	nIdentifier;
	char	*szName;

} STRINGIDPAIR;




STRINGIDPAIR asipTopicTable [ NDDE_TOPICS ] = {

	{ TOPIC_ID_SYSTEM,	SZDDESYS_TOPIC	  },
	{ TOPIC_ID_ALARM,	SZDDE_TOPIC_ALARM },
	{ TOPIC_ID_COMMANDS,	SZDDE_TOPIC_COMMANDS },
	{ TOPIC_ID_DATA,	SZDDE_TOPIC_DATA  },
	{ TOPIC_ID_ERROR,	SZDDE_TOPIC_ERROR },
	{ TOPIC_ID_RANGE,	SZDDE_TOPIC_RANGE },
	{ TOPIC_ID_UNITS,	SZDDE_TOPIC_UNITS }

};


STRINGIDPAIR asipSystemItemTable [ NDDE_SYSTEMITEMS ] = {

	{ SYSITEM_ID_CHANNELS,		SZDDE_ITEM_CHANNELS	},
	{ SYSITEM_ID_EXECUTELIST,	SZDDESYS_ITEM_EXECUTELIST },
	{ SYSITEM_ID_FORMATS,		SZDDESYS_ITEM_FORMATS	},
	{ SYSITEM_ID_HELP,		SZDDESYS_ITEM_HELP	},
	{ SYSITEM_ID_STATUS,		SZDDESYS_ITEM_STATUS	},
	{ SYSITEM_ID_SYSITEMS,		SZDDESYS_ITEM_SYSITEMS	},
	{ SYSITEM_TOPIC_IDS,		SZDDESYS_ITEM_TOPICS	},
	{ SYSITEM_ID_ITEMLIST,		SZDDE_ITEM_ITEMLIST	}


};


/**	The following structures are basically used to
***	store topic and item string handles used to determine
***	what data is being requested by the client.
***	Arrays of these structures will be created dynamically
***	when conversations begin. At these point we know what
***	data items ( channels ) are connected and may be accessed
***	through DDE.
**/
typedef struct _TOPICSTRUCT {

	short	 nIdentifier;	/* Topic Identifier	*/
	HSZ	 hszTopic;	/* Topic String Handle	*/

} TOPICSTRUCT,  *LPTOPICSTRUCT;



typedef struct _ITEMSTRUCT {

	short	 nIdentifier;	/* Item Identifer			  */
	HSZ	 hszItem;	/* Item String Handle			  */
	short	 nNumAdvising;	/* Number of advise loops using this item */

} ITEMSTRUCT, *LPITEMSTRUCT;




/**	The following table details supported Execute commands.
***
***	An execute command will have the following format:
***
***	Command <Optional Parameters>
***
***	examples: Load Mysetup.WAI
***		  Destroy
***		  Move 10 10
***
***	The function GetExecuteId ( ) will return, by reference,
***	the execute Id and a copy of the parameter list when
***	given the string handle of the DDE execute command.
**/

#define NDDE_EXECUTES	6

#define SZDDE_EXEC_DESTROY	"Destroy"
#define SZDDE_EXEC_LOADSETUP	"Load"
#define SZDDE_EXEC_MAXIMIZE	"Maximize"
#define SZDDE_EXEC_MINIMIZE	"Minimize"
#define SZDDE_EXEC_MOVE 	"Move"
#define SZDDE_EXEC_RESTORE	"Restore"


#define EXEC_ID_DESTROY 	0
#define EXEC_ID_LOADSETUP	1
#define EXEC_ID_MAXIMIZE	2
#define EXEC_ID_MINIMIZE	3
#define EXEC_ID_MOVE		4
#define EXEC_ID_RESTORE 	5


#define MAX_EXECUTE_LEN 	_MAX_PATH /* Length of Execute data string */

STRINGIDPAIR asipExecuteTable [ NDDE_EXECUTES ] = {

	{ EXEC_ID_DESTROY,	SZDDE_EXEC_DESTROY	},
	{ EXEC_ID_LOADSETUP,	SZDDE_EXEC_LOADSETUP	},
	{ EXEC_ID_MAXIMIZE,	SZDDE_EXEC_MAXIMIZE	},
	{ EXEC_ID_MINIMIZE,	SZDDE_EXEC_MINIMIZE	},
	{ EXEC_ID_MOVE,		SZDDE_EXEC_MOVE		},
	{ EXEC_ID_RESTORE,	SZDDE_EXEC_RESTORE	}

};










/**	Extern Variables required by this module
**/
extern	PANELINFO	panDigitalOut;	/* General application Information */
extern	CONNECTIONS	conninfo;	/* Connection Details		   */
extern  int nDDECloseCommand;



/**	Static variables and structures used by DDE module
**/
FARPROC lpDdeProc = (FARPROC) NULL;	/* Procedure Instance Address */



/**	ddeInfo : The following structure is declared statically.
***		  It stores general information used in the implementation
***		  of DDE conversations, which is used throughout this module.
***		  The three lists are used to store string handles for all
***		  supported topics and items.
***
***
***		  It holds the following :
***
***		  hwndAppl	: The Main Applications Window Handle.
***		  szServiceName : The DDE Service Name e.g 'DigitalOut'
***		  atsTopics	:  Pointer to supported topic list.
***		  atsDataItems	:  Pointer to supported data items list.
***		  atsSystemItems:  Pointer to supported system item list.
***
**/
struct _DDEINFO {

	HWND		hwndAppl;
	DWORD		idInst;

	char		szServiceName [ MAX_SERVICE_NAME_LEN ];
	HSZ		hszServiceName;

	short		nNumDataItems;	/* Items in aisDataItems table */

	LPTOPICSTRUCT	atsTopics;
	LPITEMSTRUCT	aisDataItems;
	LPITEMSTRUCT	aisSystemItems;

	BOOL		bServiceRegistered;


} ddeInfo;



/**	Static & Callback function prototypes. Dont put these in the
***	WPDO_DEF.h header because we would then need to include 'DDEML.h'
***	in all modules.
**/
HDDEDATA EXPENTRY DdeCallback ( WORD wType, WORD wFmt, HCONV hConv,
	HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2 );

BOOL  GetDataItemIndex ( HSZ hszItem, short *pnIndex );
BOOL  GetSystemItemId  ( HSZ hszSysItem, short *pnIdentifier );
BOOL  GetTopicId ( HSZ hszTopic, short *pnIdentifier );
BOOL  GetTopicStrHandle ( short nIdentifier, HSZ *phszTopic );
BOOL  GetSystemItemStrHandle ( short nSysItemId, HSZ *phszSysItem );

short DdeStartConversation ( DWORD idInst );
short DdeTerminateConversation ( DWORD idInst );

short DdeCreateTopicList     ( VOID );
short DdeDestroyTopicList    ( VOID );
short DdeCreateSysItemList   ( VOID );
short DdeDestroySysItemList  ( VOID );
short DdeCreateDataItemList  ( VOID );
short DdeDestroyDataItemList ( VOID );

HDDEDATA DdeGetSysItemData ( short nSysItemID );
HDDEDATA DdeGetItemData ( short nTopicID, short nItemID );

static short ExtractPokeData ( HDDEDATA hData, short nItemId );


BOOL GetExecuteId ( HDDEDATA hExecute, short *pnIndex, char *szParams );
BOOL IsServiceNameFree ( LPSTR lpszService );





/**	LoadDDECallBackProc
***
***	This function actually installs the callback loop required
***	to capture the message sent from DDEML. A static pointer to
***	callbackfunction is stored in 'lpDdeProc'. This function should
***	only be called once after which the callback function remains
***	in-place.
**/
short LoadDDECallBackProc ( VOID )
{
	HINSTANCE	hInst;


	/**	Obtain the instance identifier by calling the
	***	DDEML's DdeInitialise function. This opens the
	***	link to the DDEML library and allows us to obtain
	***	string handles for all our strings.
	***
	**/


	/**	DdeInitialize will register our CallBack fn
	***	for DDE transaction messages.
	**/
	hInst = (HINSTANCE) GetWindowLong ( ddeInfo.hwndAppl, GWL_HINSTANCE );


	lpDdeProc = MakeProcInstance ( (FARPROC) DdeCallback, hInst );



	ddeInfo.idInst	 = 0L;		/* Instance identifier - Init to 0L */

	if ( DdeInitialize (
		(LPDWORD) &ddeInfo.idInst,  /* Recieves Instance Identifier */
		(PFNCALLBACK) lpDdeProc,    /* points to callback fn	    */
		APPCLASS_STANDARD ,	    /* execs supported		    */
		0L ) != DMLERR_NO_ERROR ) {


		/**	Failed to Initialize DDE. Take relevent action...
		**/
		return ( ERR_DDE_INIT_FAIL );
	}



	/**	Since we have only just installed teh callback
	***	we can assume no service name has yet been registered.
	***
	***	Until we have successfully registered with DDEML
	***	set our 'registered flag' FALSE; This will prevent
	***	further calls to this module having to assume whether
	***	we are, or are not, supporting a service.
	**/
	ddeInfo.bServiceRegistered = FALSE;


	return ( OK );

}




/**	FreeDDECallBackProc ( )
***
***
**/
short FreeDDECallBackProc ( )
{

	DdeUninitialize  ( ddeInfo.idInst );	/* Free DDEML Resources   */
	FreeProcInstance ( lpDdeProc ); 	/* Finished with callback */
	return ( OK );
}












/**	Function:	PTKInitializeDDEService
***
***	Description	This function is one of the predefined entry
***			points into the PTK_DDE module. The application
***			calls this function, once, on creation to register
***			DDE support.
***
***			The function creates necessary tables, registers
***			the DDE callback fn and opens a link with the
***			DDEML library.
***
***
***	Return		OK, ERR_NO_GLOBAL_MEMORY, ERR_NOT_PROTECT_MODE
***			ERR_DDE_INIT_FAIL, ERR_DUPLICATE_SERVICE_NAME
**/
short PTKInitializeDDEService ( HWND hwndApplication, char *szServiceName )
{
	short		nReturn;



	/**	Until successful completeion of this function
	***	assume no registered service exists
	***
	**/
	ddeInfo.bServiceRegistered = FALSE;





	/**	Register the application as owning
	***	this dde server.
	**/
	ddeInfo.hwndAppl = hwndApplication;


	/**	make a copy of the service name in ddeInfo.
	**/
	strcpy ( ddeInfo.szServiceName, szServiceName );


	/**	Has the Callback function been registered
	***	with the DDEML library
	**/
	if ( lpDdeProc == (FARPROC) NULL ) {

		nReturn = LoadDDECallBackProc ( );

		if ( nReturn != OK ) {

			return ( nReturn );
		}
	}



	if ( !IsServiceNameFree ( ddeInfo.szServiceName ) ) {

		/**	Another application has already registered
		***	this service name. Do not continue and do not set
		***	our 'bServiceRegistered' flag TRUE thereby preventing
		***	this instance from processing any messages from
		***	the DDEML.
		**/


		return ( ERR_DUPLICATE_SERVICE_NAME );


	}


	/**	Create Topic List
	**/
	nReturn = DdeCreateTopicList ( );

	if ( nReturn != OK ) {

		  return ( nReturn );
	}





	/**
	***	Create System Item List
	**/
	nReturn = DdeCreateSysItemList ( );

	if ( nReturn != OK ) {


		/**	Destroy topic list before failing
		**/
		DdeDestroyTopicList ( );

		return ( nReturn );
	}




	/**	At this point we do NOT know what the data topics
	***	items will be so that list will be created when starting
	***	a conversation.
	**/






	/**	Create and Store other useful String Handles
	***	including the service name..
	**/
	ddeInfo.hszServiceName = DdeCreateStringHandle ( ddeInfo.idInst,
					  ddeInfo.szServiceName, CP_WINANSI );



	DdeNameService ( ddeInfo.idInst, ddeInfo.hszServiceName,
							NULL, DNS_REGISTER );


	/**	We now have a successfully register DDE service.
	***
	**/
	ddeInfo.bServiceRegistered = TRUE;

	return ( OK );

}







/**	PTKTerminateDDEService
***
***	Frees all memory and string handles created by PTKInitializeDDE fn
***	and terminates the link with the DDEML library.  This function
***	should be called on receipt of a WM_DESTROY message.
**/
short PTKTerminateDDEService ( void )
{


	if ( !ddeInfo.bServiceRegistered ) {

		/**	No service to terminate
		**/
		return ( OK );
	}


	/**	Free all String Handles and associated memory
	***	created by PTKInitializeDDE Function.
	**/
	DdeFreeStringHandle ( ddeInfo.idInst, ddeInfo.hszServiceName );


	/**	Destroy the string handle lists
	**/
	DdeDestroyTopicList   ( );

	DdeDestroySysItemList ( );




	/**	Unregister all services
	**/
	DdeNameService ( ddeInfo.idInst, NULL, NULL, DNS_UNREGISTER );



	/**	The service has gone.
	**/
	ddeInfo.bServiceRegistered = FALSE;

	return ( OK );

}










/**	DdeStartConversation
***
***	This function should be called whenever a new conversation
***	is started. If this is the first conversation the function will
***	create the string-handle tables of supported topics, items etc.
***	The following ddeInfo fields are initialized :
***
***		hszServiceName	   - The Supported Service
***		atsTopic	   - An array of supported topics
***		aisDataItems	   - An array of Data Topic items
***		aisSysItems	   - An array of System Topic items
***
**/
short DdeStartConversation ( DWORD idInst )
{



	short	nReturn;


	if ( ++panDigitalOut.nNumDDELinks == 1 ) {

		/**	This is the first converstaion.
		***	At this point we can assume the
		***	connections list will not change until
		***	all conversations have terminated.
		***	Before proceding, we will therefore
		***	fill-in the data item list with the
		***	current channel names and create and
		***	store string handles for each string
		***	we will be using.
		***
		**/

		/**	Check if setup has changed, and prevent other
		***	applications getting their fingers in...
		**/
		nReturn = PreventSetupConfiguration ( ddeInfo.hwndAppl,
							&conninfo.imsSetup );

		if ( nReturn != OK ) {

			/**     Cannot continue without setup.
			**/
			return ( nReturn );
		}




		/**
		***	Create Data Item List for use by the following
		***	topics : Data, Range, Units, Alarm and Error
		***
		***	We need an item for each connected channel
		***	plus one for the AllChannels items.
		**/
		nReturn = DdeCreateDataItemList ( );
		if ( nReturn != OK ) {

			return ( nReturn );
		}







	}
	/**    Update status and menu bars to inform the user
	***    how many conversations are inplace and prevent
	***    him from changing the setup and connections.
	**/
	UpdateStatusBar ( ddeInfo.hwndAppl );
	UpdateMenuBar	( ddeInfo.hwndAppl );
	UpdateButtonBar ( ddeInfo.hwndAppl );

	return ( OK );
}









/**	DdeTerminateConversation
***
***	This function should be called whenever an existing conversation
***	is terminated by the DDEML i.e. the server receives the a
***	XTYP_DISCONNECT transaction. If it is the last conversation that
***	is being terminated, the function tidies up, freeing all memory
***	allocated for DDE and destroying the all topic/item string handles.
***
**/
short DdeTerminateConversation ( DWORD idInst )
{



	/**	Decrement the conversation count and if no more exist
	***	tidy up the string tables and memory used during the
	***	conversations.
	**/
	if ( --panDigitalOut.nNumDDELinks == 0 ) {



		DdeDestroyDataItemList ( );

		/**	Allow the Setup to be
		***	edited now we have finished..
		**/
		AllowSetupConfiguration ( ddeInfo.hwndAppl,
							&conninfo.imsSetup );


	}

	/**    Update status and menu bars to inform the user
	***    how many conversations are inplace and allow
	***    him to change the setup and connections if no
	***    conversations are inplace.
	**/
	UpdateStatusBar ( ddeInfo.hwndAppl );
	UpdateMenuBar	( ddeInfo.hwndAppl );
	UpdateButtonBar ( ddeInfo.hwndAppl );

	return ( OK );
}






/**
***    DdeCallBack     DDE Message Handler
***
***    Processes all DDE messages
***
***
***    Parameters :
***	wType : 	transaction type
***	wFmt :		clipboard data format
***	hConv : 	handle of the conversation
***	hsz1 :		handle of a string
***	hsz2 :		handle of a string
***	hData : 	handle of global memory object
***	dwData1 :	transaction specific data
***	dwData2 :	transaction specific data
***
***	Return Value
***		Transaction type dependent. ( See Transactions )
***
**/
HDDEDATA EXPENTRY DdeCallback ( WORD wType, WORD wFmt, HCONV hConv,
	HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2 )
{


	HDDEDATA hRequestedDataData;
	short	 nTopic;
	short	 nTopicId, nItemId;
	BOOL	 bTopicSupported, bItemSupported;


	RECT	 rectApplc;		/* Vars used in Execute section */
	long	 lXPos, lYPos, lHeight, lWidth;
	short	 nExecuteId;
	char	 szParams [ MAX_EXECUTE_LEN ];
	char	 *szArg;





	/**	Implement Blocking here if required.
	***
	**/
	if ( !ddeInfo.bServiceRegistered ) {

		/**	No service name is currently registered
		***	for this instance..
		**/
		return ( (HDDEDATA) NULL );
	}




	/**	Only allow transactions on the formats we support
	***	if they have a format.
	**/
	if ( wFmt ) {

		if ( wFmt != CF_TEXT ) {

		    /** Illegal format
		    **/
		    return ( (HDDEDATA) NULL );

		}
	}



	switch ( wType ) {

		case XTYP_CONNECT :

			/**	hsz1 - Topic Name
			***	hsz2 - Service Name
			***
			***
			***	return TRUE if we support
			***	the Service/Topic name pair
			**/


			if ( DdeCmpStringHandles ( hsz2,
						ddeInfo.hszServiceName ) == 0 ) {


				/** Service  supported
				**/


				/**	Now check we also support the topic
				**/
				bTopicSupported = GetTopicId ( hsz1, &nTopicId );

				if ( bTopicSupported ) {

					/**	Topic also supported
					**/
					return ( (HDDEDATA) TRUE );

				}
			}

			/** Not supported, wash our hands of it.
			**/

			return ( (HDDEDATA) FALSE );



		case XTYP_CONNECT_CONFIRM :

			/**	hConv - The conversation handle.
			***
			***	Now we have a confirmed converstaion
			***	create an item table for the data topics.
			**/
			DdeStartConversation ( ddeInfo.idInst );

			break;



		case XTYP_WILDCONNECT :


			/**	The Client has requested a hsz list of all
			***	our available app/topic pairs conforming to
			***	hszTopic and hszItem;
			***
			***	hsz1 - Topic Name
			***	hsz2 - Service Name
			***
			**/
			{
			HSZ	ahszPairTable [ ( NDDE_TOPICS + 1 ) * 2 ];
			short	nPairTableIndex;




			if ( ( hsz2 != NULL ) && ( DdeCmpStringHandles
					( hsz2, ddeInfo.hszServiceName ) != 0 ) ) {

				/**	The requested service was neither NULL
				***	or our supported service name, so we
				***	dont support it.
				**/
				return ( ( HDDEDATA) NULL );

			}


			/**	Scan the topic list and craete the hsz pairs
			**/
			nPairTableIndex = 0;
			for ( nTopic = 0; nTopic < NDDE_TOPICS; nTopic++ ) {

				if ( ( hsz1 == NULL ) ||
				 ( DdeCmpStringHandles ( hsz1,
					ddeInfo.atsTopics [ nTopic ].hszTopic ) == 0 ) ) {


					/**	Topic name is NULL or matches
					***	our current topic so return
					***	the service topic pair
					**/


					/**	First the service name
					**/
					ahszPairTable [ nPairTableIndex++ ] =
								 ddeInfo.hszServiceName;


					/**	Secondily the topic name to form the pair
					**/
					ahszPairTable [ nPairTableIndex++ ] =
						ddeInfo.atsTopics [ nTopic ].hszTopic;
				}

			}

			/**	Terminate the list with to NULL handles
			**/
			ahszPairTable [ nPairTableIndex++ ] = NULL;
			ahszPairTable [ nPairTableIndex ]   = NULL;



			return ( DdeCreateDataHandle ( ddeInfo.idInst,
					(LPBYTE) &ahszPairTable [ 0 ],
					( sizeof ( HSZ)	* ( NDDE_TOPICS + 1 ) * 2 ),
					0L,
					0,
					wFmt,
					0 ) );

			}
			break;

		case XTYP_DISCONNECT :

			/**    Use DdeQueryConvInfo to obtain information about
			***    conversation
			**/
			DdeTerminateConversation ( ddeInfo.idInst );

			break;


		case XTYP_REQUEST :

			/**
			***	hsz1  - topic name
			***	hsz2  - item name
			***	hConv - identifies the conversation
			***	wFmt  - Specifies the format in which to
			***		submit data.
			***
			***	Check item and format supported ( if not return
			***	NULL ). Return a data handle to the data.
			**/

			/**	What topic first...
			**/
			bTopicSupported = GetTopicId ( hsz1, &nTopicId );

			if ( !bTopicSupported ) {

				return ( (HDDEDATA) NULL );
			}


			if ( nTopicId == TOPIC_ID_SYSTEM ) {

				/**	Process system topic
				**/
				bItemSupported = GetSystemItemId ( hsz2, &nItemId );

				if ( !bItemSupported ) {

					/** Non-supported System Topic item
					**/
					return ( (HDDEDATA) NULL );
				}
				/**	Process system item
				***		:
				***		:
				**/
				hRequestedDataData = DdeGetSysItemData ( nItemId );


				return ( hRequestedDataData );

			}


			/**	All other topics use the same data item table
			**/
			bItemSupported = GetDataItemIndex ( hsz2, &nItemId );

			if ( !bItemSupported ) {

				/**	Non-supported Data Topic item
				**/
				return ( (HDDEDATA) NULL );
			}

			/**	Process data item
			***		:
			***		:
			**/


			/**	Acquire new channel data if needed.
			***	Send this message because we will need
			***	the data before proceeding.
			**/
			if ( nTopicId == TOPIC_ID_DATA ) {


				/**	Force Hardware Read on selected Channel
				***
				**/

				FORWARD_WM_READCHAN ( ddeInfo.hwndAppl, nItemId,
									SendMessage );
			}


			/**	Fill the data buffer with data of the correct topic
			**/
			hRequestedDataData = DdeGetItemData ( nTopicId, nItemId );



			/**	Allow the DDEML to copy the data for the client.
			**/

			return ( hRequestedDataData );



		case XTYP_POKE :

			/**	wFmt  - Format of data being sent from client
			***	hConv - Identifies the conversation
			***	hsz1  - Topic name
			***	hsz2  - not used
			***	hData - Identifies the data that the client is
			***		sending to us, the server.
			***
			***	return DDE_FACK if processed or
			***	       DDE_FBUSY or DDE_FNOTPROCESSED
			***
			***	The DDEML will destroy the data on return from
			***	the callback. If we need to keep a copy use
			***	DdeGetData fn.
			**/
			/**	What topic first...
			**/
			bTopicSupported = GetTopicId ( hsz1, &nTopicId );

			if ( !bTopicSupported ) {

				   return ( (HDDEDATA) DDE_FNOTPROCESSED );
			}

			/**	Support only DATA topic for
			***	POKE transactions.
			**/
			if ( nTopicId == TOPIC_ID_DATA ) {


				bItemSupported = GetDataItemIndex ( hsz2, &nItemId );

				if ( !bItemSupported ) {

					return ( (HDDEDATA) DDE_FNOTPROCESSED );

				}


				/**	Extract the data from the global memory
				***	object and transfer it to the array of
				***	DATAOUTPUT structures:
				***		 connect.lpodOutputData
				**/
				if ( ExtractPokeData ( hData, nItemId ) != OK ) {

					return ( (HDDEDATA) DDE_FNOTPROCESSED );

				}
				/**	Force any new data in the OUTPUTDATA
				***	structures to be transfered to the
				***	hardware.
				**/
				FORWARD_WM_WRITECHAN ( ddeInfo.hwndAppl, 0,
								SendMessage );


				/**	Inform the client that the transaction
				***	has been successfull ( to the best of
				***	our knowledge ). There isn't really the
				***	facility here to inform the client if
				***	the write operation failed. The client
				***	must check this himself, using the
				***	'Error' topic.
				**/
				return ( (HDDEDATA) DDE_FACK );

			} else {

				return ( (HDDEDATA) DDE_FNOTPROCESSED );
			}







		case XTYP_EXECUTE :

			/**	hsz1  - Topic name
			***	hsz2  - not used
			***	hData - Identifies command string
			***
			***	return DDE_FACK, DDE_FBUSY or DDE_FNOTPROCESSED
			***
			***	The DDEML will destroy the data on return from
			***	the callback. If we need to keep a copy use
			***	DdeGetData fn.
			**/

			/**	Implement 'Load', 'Size', 'Start', 'Interval' etc
			***	and possibly 'File.Hardware' type thingies.
			**/
			if ( GetExecuteId ( hData, &nExecuteId, szParams ) ) {


				switch ( nExecuteId ) {

					case EXEC_ID_MINIMIZE :

						ShowWindow ( ddeInfo.hwndAppl, SW_SHOWMINIMIZED	);
						return ( (HDDEDATA) DDE_FACK );

					case EXEC_ID_MAXIMIZE :

						ShowWindow ( ddeInfo.hwndAppl, SW_SHOWMAXIMIZED	);
						return ( (HDDEDATA) DDE_FACK );


					case EXEC_ID_RESTORE  :


						ShowWindow ( ddeInfo.hwndAppl, SW_RESTORE );
						return ( (HDDEDATA) DDE_FACK );


					case EXEC_ID_DESTROY  :

						/** DDE is closing us down **/
						nDDECloseCommand = TRUE;
						/**	Post a close message or all
						***	hell we be let loose if we
						***	close the application before
						***	acknowledging the DDE message.
						**/
						FORWARD_WM_CLOSE ( ddeInfo.hwndAppl,
									 PostMessage );

						return ( (HDDEDATA) DDE_FACK );

					case EXEC_ID_LOADSETUP:



						/** Maximise the application if
						*** minimised
						**/
						if ( IsIconic ( ddeInfo.hwndAppl ) ) {

							OpenIcon ( ddeInfo.hwndAppl );
						}

						if ( strlen ( szParams ) == 0 ) {

							/**	No filespec specified
							**/
							return	( (HDDEDATA) DDE_FACK );
						}


						/**	Load the file.
						**/
						LoadAppSetup ( szParams );

						FORWARD_WM_COMMAND ( ddeInfo.hwndAppl,
							IDM_INIT_FOR_NEW_SETUP,
							(HWND) NULL,
							0,
							SendMessage );



						/**	Bring this window to top
						***
						***	It is assumed that if another
						***	application has loaded a setup
						***	in this application they assume it
						***	will be visible.
						**/
						//InvalidateRect ( ddeInfo.hwndAppl,
						//				NULL, TRUE );

						BringWindowToTop ( ddeInfo.hwndAppl );

						/** Any errors will already have been
						*** reported by the LoadAppSetup fn.
						**/


						return	( (HDDEDATA) DDE_FACK );






					case EXEC_ID_MOVE	  :

						/** Restore the application if
						*** minimised
						**/
						if ( IsIconic ( ddeInfo.hwndAppl ) ) {

							OpenIcon ( ddeInfo.hwndAppl );
						}


						/**	Modify the window origin and
						***	call MoveWindow. Do not alter
						***	the position of the Window.
						**/

						GetWindowRect ( ddeInfo.hwndAppl, &rectApplc );


						lWidth	= 0;	/* Default parameters */
						lHeight = 0;
						lXPos	= 0;
						lYPos	= 0;

						/**	Extract upto 4 params these are :
						***	xpos, ypos, width and height
						***
						***	The last two are optional
						**/
						if ( szParams != (char *) NULL ) {

							szArg = strtok ( szParams, " ," );
							if ( szArg != (char *) NULL ) {

							lXPos = atoi ( szArg );
							}

							szArg = strtok ( (char *) NULL, " ," );
							if ( szArg != (char *) NULL ) {

							lYPos = atoi ( szArg );
							}

							szArg = strtok ( (char *) NULL, " ," );
							if ( szArg != (char *) NULL ) {

							lWidth = atoi ( szArg );
							}

							szArg = strtok ( (char *) NULL, " ," );
							if ( szArg != (char *) NULL ) {

							lHeight = atoi ( szArg );
							}


						}


						/**	A few checks on the arguments
						***	If width/height arguments have
						***	not been specified OR are zero
						***	the use the originals.
						**/
						if ( lWidth <= 0 ) {
							lWidth = rectApplc.right -
									rectApplc.left;

						}
						lWidth = min ( lWidth,
							GetSystemMetrics ( SM_CXSCREEN ) );



						if ( lHeight <= 0 ) {
							lHeight = rectApplc.bottom -
									rectApplc.top;

						}
						lHeight = min ( lHeight,
							GetSystemMetrics ( SM_CYSCREEN) ),



						lXPos = min ( lXPos,
							GetSystemMetrics ( SM_CXSCREEN ) );

						lYPos = min ( lYPos,
							GetSystemMetrics ( SM_CYSCREEN ) );

						lXPos = max ( 0, lXPos );
						lYPos = max ( 0, lYPos );

						MoveWindow (
							ddeInfo.hwndAppl,
							lXPos,
							lYPos,
							lWidth,
							lHeight,
							TRUE );


						/**	Bring this window to top
						***
						***	It is assumed that if another
						***	application has moved
						***	this applicattion they assume it
						***	will be visible.
						**/
						BringWindowToTop ( ddeInfo.hwndAppl );

						return	( (HDDEDATA) DDE_FACK );

					default:

						return ( (HDDEDATA) DDE_FNOTPROCESSED );

				}
			}
			return ( (HDDEDATA) DDE_FNOTPROCESSED );

		default :

			return ( (HDDEDATA) NULL );

	}
	return ( (HDDEDATA) NULL );

}






/**
***	Function	PTKAdviseDDE
***
***	Description	This function informs the DDE Manager Library that
***			new 'Data' topic data exists for the specified item.
***			The item may be the AllChannels item.
**/
VOID PTKAdviseDDE ( short nItem )
{

	HSZ	hszDataTopic, hszAlarmTopic;

	/**	Inform the manager that the specified item
	***	has changed in the Data topic. Check first
	***	that conversations are in-place. If not the
	***	data item array will not yet exist.
	**/
	if ( panDigitalOut.nNumDDELinks == 0 ) {

	    /** No conversations to advise upon.
	    **/
	    return;
	}


	/**	Get a string handle to the data and alarm topics
	**/
	GetTopicStrHandle ( TOPIC_ID_DATA,  &hszDataTopic  );
	GetTopicStrHandle ( TOPIC_ID_ALARM, &hszAlarmTopic );



	if ( nItem == conninfo.nNumConnected  ) {



		/**	'AllChannels';	Advise on all data items...
		**/
		DdePostAdvise ( ddeInfo.idInst, hszDataTopic,  (HSZ) NULL );
		DdePostAdvise ( ddeInfo.idInst, hszAlarmTopic, (HSZ) NULL );


	} else {

		/**	Advise on single data item.....
		**/
		DdePostAdvise ( ddeInfo.idInst, hszDataTopic,
				   ddeInfo.aisDataItems [ nItem ].hszItem );

		DdePostAdvise ( ddeInfo.idInst, hszAlarmTopic,
				   ddeInfo.aisDataItems [ nItem ].hszItem );

		/**	...and the AllChannels ( since one value in
		***	this data item will have changed as well )
		**/
		DdePostAdvise ( ddeInfo.idInst, hszDataTopic,
		   ddeInfo.aisDataItems [ conninfo.nNumConnected ].hszItem );

		DdePostAdvise ( ddeInfo.idInst, hszAlarmTopic,
		   ddeInfo.aisDataItems [ conninfo.nNumConnected ].hszItem );
	}
	return;


}






/**	Function	GetDataItemChan
***
***	Description	This function searches the array of data item
***			structures for the specified string handle. If
***			found the index into data item array ( and also
***			therefore into the connections array ) is returned
***			by reference.
***
***	Note		The return value MUST be checked to ensure a match
***			exists and a valid index returned.
***
***	Return Value	TRUE if data item supported and Chan returned.
***			FALSE if data item not supported.
***
**/
BOOL GetDataItemIndex ( HSZ hszItem, short *pnIndex )
{
	short	nItem;

	for ( nItem = 0; nItem <= conninfo.nNumConnected; nItem++ ) {

		if ( DdeCmpStringHandles ( hszItem,
			    ddeInfo.aisDataItems [ nItem ].hszItem ) == 0 ) {

			/**	Item found
			***
			***	The indexes into both the connections
			***	array and the data items array are the same.
			***	(Since the data item array is created
			***	 from the connections array )
			**/
			*pnIndex = nItem;

			return ( TRUE );
		}
	}
	return ( FALSE );  /* Item not in table */
}









/**	Function	GetSystemItemId
***
***	Description	This function searches the array of system item
***			structures for the specified string handle. If
***			found the System Item's Identifier is returned by
***			reference.
***
***	Note		The return value MUST be checked to ensure a match
***			exists and a valid identifier returned.
***
***	Return Value	TRUE if system item supported and identifier returned.
***			FALSE if sytem item not supported.
***
**/
BOOL GetSystemItemId ( HSZ hszSysItem, short *pnIdentifier )
{
	short	nSysItem;

	for ( nSysItem = 0; nSysItem < NDDE_SYSTEMITEMS; nSysItem++ ) {

		if ( DdeCmpStringHandles ( hszSysItem,
			ddeInfo.aisSystemItems [ nSysItem ].hszItem ) == 0 ) {

			/**	Item found
			**/
			*pnIdentifier =
			   ddeInfo.aisSystemItems [ nSysItem ].nIdentifier;

			return ( TRUE );
		}
	}
	return ( FALSE );  /* Item not in table */
}





/**	Function	GetTopicId
***
***	Description	This function searches the array of topic
***			structures for the specified string handle. If
***			found the Topic's Identifier is returned by
***			reference.
***
***	Note		The return value MUST be checked to ensure a match
***			exists and a valid identifier returned.
***
***	Return Value	TRUE if topic supported and identifier returned.
***			FALSE if topic not supported.
***
**/
BOOL GetTopicId ( HSZ hszTopic, short *pnIdentifier )
{
	short	nTopic;

	for ( nTopic = 0; nTopic < NDDE_TOPICS; nTopic++ ) {

		if ( DdeCmpStringHandles ( hszTopic,
			    ddeInfo.atsTopics [ nTopic ].hszTopic ) == 0 ) {

			/**	Item found
			**/
			*pnIdentifier = ddeInfo.atsTopics [ nTopic ].nIdentifier;

			return ( TRUE );
		}
	}
	return ( FALSE );  /* Item not in table */
}










/**	Function	GetTopicStrHandle
***
***	Description	This function searches the array of topic
***			structures for the specified Topic Identifier. If
***			found the Topic's String Handle is returned by
***			reference.
***
***	Note		The return value MUST be checked to ensure a match
***			exists and a valid identifier returned.
***
***	Return Value	TRUE if topic supported and string handle returned.
***			FALSE if topic not supported.
***
**/
BOOL GetTopicStrHandle ( short nTopicId, HSZ *phszTopic )
{
	short	nTopic;

	for ( nTopic = 0; nTopic < NDDE_TOPICS; nTopic++ ) {

		if ( nTopicId == ddeInfo.atsTopics [ nTopic ].nIdentifier ) {

			/**	Item found
			**/
			*phszTopic = ddeInfo.atsTopics [ nTopic ].hszTopic;

			return ( TRUE );
		}
	}
	return ( FALSE );  /* Item not in table */
}



/**	Function	GetSysItemStrHandle
***
***	Description	This function searches the array of sysItem
***			structures for the specified Topic Identifier. If
***			found the SysItems' String Handle is returned by
***			reference.
***
***	Note		The return value MUST be checked to ensure a match
***			exists and a valid identifier returned.
***
***	Return Value	TRUE if topic supported and string handle returned.
***			FALSE if topic not supported.
***
**/
BOOL GetSystemItemStrHandle ( short nSysItemId, HSZ *phszSysItem )
{
	short	nSysItem;

	for ( nSysItem = 0; nSysItem < NDDE_SYSTEMITEMS; nSysItem++ ) {

		if ( nSysItemId ==
			ddeInfo.aisSystemItems [ nSysItem ].nIdentifier ) {

			/**	Item found
			**/
			*phszSysItem =
			   ddeInfo.aisSystemItems [ nSysItem ].hszItem;

			return ( TRUE );
		}
	}
	return ( FALSE );  /* Item not in table */
}




/**
***	DdeCreateTopicList	Allocates global memory to hold a list
***				of TOPICSTRUCTS and fills the TOPICSTRUCT'S
***				with string handles and identifiers for
***				each supported Topic.
***				If successful, the pointer to the list is
***				stored in the static 'ddeInfo' structure.
***
***	Returns 		OK or ERR_NO_GLOBAL_MEMORY
***
***
**/
short DdeCreateTopicList ( VOID )
{

	short	nTopic;



	/**
	***	Create Topic List
	**/
	if ( ( ddeInfo.atsTopics = (LPTOPICSTRUCT)
		 GlobalAllocPtr ( GMEM_MOVEABLE,
			sizeof ( TOPICSTRUCT ) * NDDE_TOPICS ) ) == NULL ) {


		return ( ERR_NO_GLOBAL_MEMORY );

	}


	if ( !GlobalLockPtr ( ddeInfo.atsTopics ) ) {

		GlobalFreePtr ( ddeInfo.atsTopics );

		return ( ERR_NO_GLOBAL_MEMORY );
	}



	/**	Now create and store the topic string handles
	***	and topic identifiers. These are stored statically
	***	at the beginning of this module in table asipTopicTable.
	**/
	for ( nTopic = 0; nTopic < NDDE_TOPICS; nTopic++ ) {

		/** Copy Identifier from static Topic Table
		**/
		ddeInfo.atsTopics [ nTopic ].nIdentifier =
			asipTopicTable [ nTopic ].nIdentifier;

		/** Create String Handle using string in Topic Table
		**/
		ddeInfo.atsTopics [ nTopic ].hszTopic =
			DdeCreateStringHandle (	ddeInfo.idInst,
			   asipTopicTable [ nTopic ].szName, CP_WINANSI );



	}


	return (OK );
}


/**
***	DdeDestroyTopicList	Destroys all string handles and frees the
***				global memory used by the TOPIC structure
***				list pointed to by 'ddeInfo.atsTopics'.
***
***
***	Returns 		OK
***
***
**/
short DdeDestroyTopicList (  )
{
	short	nTopic;


	for ( nTopic = 0; nTopic < NDDE_TOPICS; nTopic++ ) {

		DdeFreeStringHandle ( ddeInfo.idInst,
				ddeInfo.atsTopics [ nTopic ].hszTopic );

	}
	GlobalUnlockPtr ( ddeInfo.atsTopics );
	GlobalFreePtr	( ddeInfo.atsTopics );

	return ( OK );
}











/**
***	DdeCreateSysItemList	Allocates global memory to hold a list
***				of ITEMSTRUCTS and fills the STRUCT'S
***				with string handles and identifiers for
***				each supported system item.
***				If successful, the pointer to the list is
***				stored in the static 'ddeInfo' structure.
***
***	Returns 		OK or ERR_NO_GLOBAL_MEMORY
***
***
**/
short DdeCreateSysItemList ( VOID )
{
	short	nSysItem;


	if ( ( ddeInfo.aisSystemItems = (LPITEMSTRUCT)
		GlobalAllocPtr ( GMEM_MOVEABLE,
		    sizeof ( ITEMSTRUCT ) * NDDE_SYSTEMITEMS ) ) == NULL ) {


		return ( ERR_NO_GLOBAL_MEMORY );

	}


	if ( !GlobalLockPtr ( ddeInfo.aisSystemItems ) ) {


		GlobalFreePtr ( ddeInfo.aisSystemItems );

		return ( ERR_NO_GLOBAL_MEMORY );
	}


	/**	Fill the String Handle array with the appropraite handles.
	***	The strings are stored in the table: aszSystemItems.
	**/
	for ( nSysItem = 0; nSysItem < NDDE_SYSTEMITEMS; nSysItem++ ) {

		/** Copy Identifier from static Item Table
		**/
		ddeInfo.aisSystemItems [ nSysItem ].nIdentifier =
			asipSystemItemTable [ nSysItem ].nIdentifier;

		/** Create String Handle using string in SystemItem Table
		**/
		ddeInfo.aisSystemItems [ nSysItem ].hszItem =
		     DdeCreateStringHandle ( ddeInfo.idInst,
			asipSystemItemTable [ nSysItem ].szName, CP_WINANSI );

		/**	Finally initialize the advise count for this item.
		***	This is the number of advise loops currently
		***	accessing this item.
		***
		**/
		ddeInfo.aisSystemItems [ nSysItem ].nNumAdvising = 0;

	}

	return ( OK );
}






/**
***	DdeDestroySysItemList	Destroys all string handles and frees the
***				global memory used by the ITEM structure
***				list pointed to by 'ddeInfo.atsSystemItems'.
***
***
***	Returns 		OK
***
***
**/
short DdeDestroySysItemList ( VOID )
{
	short	nSysItem;



	for ( nSysItem = 0; nSysItem < NDDE_SYSTEMITEMS; nSysItem++ ) {

		DdeFreeStringHandle ( ddeInfo.idInst,
			ddeInfo.aisSystemItems [ nSysItem ].hszItem );

	}

	GlobalUnlockPtr ( ddeInfo.aisSystemItems );
	GlobalFreePtr	( ddeInfo.aisSystemItems );


	return ( OK );
}






/**
***	DdeCreateDataItemList	Allocates global memory to hold a list
***				of ITEMSTRUCTS and fills the STRUCT'S
***				with string handles and identifiers for
***				each supported data item.
***				If successful, the pointer to the list is
***				stored in the static 'ddeInfo' structure.
***
***	Returns 		OK or ERR_NO_GLOBAL_MEMORY
***
***
**/
short DdeCreateDataItemList ( VOID )
{
	short		nItem;

	/**	Exit if no channels available to use as data items
	***
	**/
	if ( conninfo.nNumConnected == 0 ) {

		ddeInfo.nNumDataItems = 0;

		return ( OK );
	}


	if ( ( ddeInfo.aisDataItems = (LPITEMSTRUCT) GlobalAllocPtr (
			  GMEM_MOVEABLE, sizeof ( ITEMSTRUCT )
			     * ( conninfo.nNumConnected + 1 ) ) ) == NULL ) {

		ddeInfo.nNumDataItems = 0;

		return ( ERR_NO_GLOBAL_MEMORY );

	}


	if ( !GlobalLockPtr ( ddeInfo.aisDataItems ) ) {


		GlobalFreePtr ( ddeInfo.aisDataItems );
		ddeInfo.nNumDataItems = 0;


		return ( ERR_NO_GLOBAL_MEMORY );
	}
	/**	Now fill in the data list with string handles
	***	and data identifiers.
	**/


	/**	Fill in the dde items strings
	**/
	for ( nItem = 0; nItem < conninfo.nNumConnected; nItem++ ) {

		/**	Assign the item a unique identifier.
		**/
		ddeInfo.aisDataItems [ nItem ].nIdentifier = nItem;

		/**	Create the items string handle.
		**/
		ddeInfo.aisDataItems [ nItem ].hszItem =
			DdeCreateStringHandle (	ddeInfo.idInst,
				  conninfo.pciConnections [ nItem ].szChannel,
								 CP_WINANSI );

		/**	Initialize the items advise loop count
		**/
		ddeInfo.aisDataItems [ nItem ].nNumAdvising = 0;


	}

	/**	Repeat above the special 'AllChannels' item
	**/
	ddeInfo.aisDataItems [ nItem ].nIdentifier = nItem;

	ddeInfo.aisDataItems [ nItem ].hszItem = DdeCreateStringHandle (
			ddeInfo.idInst, SZDATA_ITEM_ALLCHANNELS, CP_WINANSI );

	ddeInfo.aisDataItems [ nItem ].nNumAdvising = 0;

	/**	Store the number of items we have created
	***	in the DDEINFO structure. We can then use this when
	***	we need to destroy the items at the end of the
	***	converstaion
	**/
	ddeInfo.nNumDataItems = conninfo.nNumConnected + 1;

	return ( OK );
}







/**
***	DdeDestroyDataItemList	Destroys all string handles and frees the
***				global memory used by the ITEM structure
***				list pointed to by 'ddeInfo.atsSystemItems'.
***
***
***	Returns 		OK
***
***
**/
short DdeDestroyDataItemList ( VOID )
{
	short	nItem;

	/**	If no items exist i.e no connections have been made
	***	then exit now.
	**/
	if ( ddeInfo.nNumDataItems == 0 ) {

		return ( OK );
	}


	for ( nItem = 0; nItem < ddeInfo.nNumDataItems; nItem++ ) {

		DdeFreeStringHandle ( ddeInfo.idInst,
				ddeInfo.aisDataItems [ nItem ].hszItem );


	}
	GlobalUnlockPtr ( ddeInfo.aisDataItems );
	GlobalFreePtr	( ddeInfo.aisDataItems );


	ddeInfo.nNumDataItems = 0;


	return ( OK );
}









/**	Function	DdeGetItemData
***
***	Description	Transfers data from the conninfo.pciConnections array
***			into a DDE transfer buffer. Data of the correct
***			topic is copied into the buffer and correctly
***			delimited.
***
***	Returns		Handle to the Data
**/
HDDEDATA DdeGetItemData ( short nTopicID, short nItemID )
{
	char		szErrorCode [ MAX_VALUE + 1 ];
	char		szAlarmCode [ 2 ]; /* Single digit alarm state code */
	short		nChan;
	DWORD		dwBytesReq;
	WORD		wFmt;
	short		nNumChansInItem;
	short		nFirstChanInItem;
	HSZ		hszItem;
	HDDEDATA	hRequestedData;
	LPBYTE		lpszDdeData;

	/**	Assign common memory object attributes.
	**/
	wFmt	= CF_TEXT;
	hszItem = ddeInfo.aisDataItems [ nItemID ].hszItem;



	/**	Determine how many channels we are using.
	***	This is either one or all connected channels
	**/
	if ( nItemID == conninfo.nNumConnected ) {

		nFirstChanInItem = 0;
		nNumChansInItem	 = conninfo.nNumConnected;

	} else {

		nFirstChanInItem = nItemID;
		nNumChansInItem	 = 1;
	}




	/** Create an empty buffer of the sufficient size
	*** to hold the data for our channel(s).
	*** Include space for Tab delimitors
	**/
	switch ( nTopicID ) {

	case TOPIC_ID_DATA : dwBytesReq = (DWORD)
				( nNumChansInItem * ( MAX_VALUE + 1 ) );

			     break;

	case TOPIC_ID_UNITS :dwBytesReq = (DWORD)
				( nNumChansInItem * ( MAX_UNITS + 1 ) );

			     break;

	case TOPIC_ID_RANGE :/* Two range strings per Connection */

			     dwBytesReq = (DWORD) ( 2 *
				 nNumChansInItem * ( MAX_LIMIT + 1 ) );

			     break;


	case TOPIC_ID_ERROR :dwBytesReq = (DWORD)
				( nNumChansInItem * ( MAX_VALUE + 1 ) );

			     break;

	case TOPIC_ID_ALARM : dwBytesReq = (DWORD)
				( nNumChansInItem * 2 );

			     break;

	}




	/**	Ask the DDEML for a data handle to a memory object
	***	of the correct size. At this point dont copy data
	***	into the memory.
	**/
	hRequestedData = DdeCreateDataHandle (
			  ddeInfo.idInst,		/* Instance handle */
			  NULL,				/* Buffer	   */
			  dwBytesReq,			/* bytes required  */
			  0L,				/* Offset into buf */
			  hszItem,			/* Item String Hdl */
			  wFmt, 			/* Clipboard Format*/
			  0 );				/* Flags	   */



	if ( hRequestedData == (HDDEDATA) NULL ) {

		/**	Failed to create the handle
		***	DdeGetLastError will tell us why
		***	but its probably lack of memory
		***	Simply return the NULL handle.
		***	The caller will return this to
		***	DDEML indicating no data available.
		**/
		return ( (HDDEDATA) NULL );

	}

	/**	Access the memory object
	**/
	lpszDdeData = DdeAccessData ( hRequestedData, NULL );

	if ( lpszDdeData == NULL ) {

		/**	Were really in trouble now...
		***	The memory object exists but we cant
		***	fill it. Safest bet is to return the
		***	handle minus the data.
		**/
		return ( hRequestedData );
	}


	/**	Initialize the memory object with a null-terminator
	**/
	lpszDdeData [ 0 ] = '\0';


	/**	Now copy the data for each channel in the item
	***	into the memory object
	**/
	for ( nChan = nFirstChanInItem;
		nChan < ( nFirstChanInItem + nNumChansInItem ); nChan++ ) {

		switch ( nTopicID ) {

		default :
		case TOPIC_ID_DATA :

			lstrcat ( lpszDdeData,
				(LPSTR) conninfo.pciConnections
						[ nChan ].szValue );
			break;

		case TOPIC_ID_UNITS :

			lstrcat ( lpszDdeData,
				(LPSTR) conninfo.pciConnections
						[ nChan ].szUnits );
			break;

		case TOPIC_ID_RANGE :

			lstrcat ( lpszDdeData,
				(LPSTR) conninfo.pciConnections
						[ nChan ].szMin );

			lstrcat ( lpszDdeData,
					(LPSTR) "\t" );

			lstrcat ( lpszDdeData,
				(LPSTR) conninfo.pciConnections
						[ nChan ].szMax );

			break;




		case TOPIC_ID_ERROR :

			sprintf ( szErrorCode,	"%d",
			    conninfo.pciConnections [ nChan ].nError );

			lstrcat ( lpszDdeData,
					(LPSTR) szErrorCode );
			break;


		case TOPIC_ID_ALARM :

			sprintf ( szAlarmCode,	"%1d",
			    conninfo.pciConnections [ nChan ].nAlarmState );

			lstrcat ( lpszDdeData,
					(LPSTR) szAlarmCode );

			break;
		}



		/**	Tab delimit the field unless last.
		***
		**/
		if ( nChan !=
		    ( nFirstChanInItem + nNumChansInItem - 1 ) ) {

			/**	Tab Field delimitor
			**/
			lstrcat ( lpszDdeData, (LPSTR) "\t" );
		}

	}  /* Next Channel */


	/**	Finished with the memory object
	**/
	DdeUnaccessData ( hRequestedData );

	return ( hRequestedData );
}









/**	Function	DdeGetSysItemData
***
***	Description	Transfers system topic data
***			into a DDE memory object. Data for the
***			correct System item is copied into the object
***			and correctly delimited.
***
***	Returns		Handle to the Memory Object
**/
HDDEDATA DdeGetSysItemData ( short nSysItemId )
{
	DWORD		dwBytesReq;
	WORD		wFmt;
	HSZ		hszItem;
	HDDEDATA	hRequestedData;
	LPBYTE		lpszDdeData;
	short		nTopic, nItem, nSysItem, nExecute;

	/**	Assign common memory object attributes.
	**/
	wFmt = CF_TEXT;
	GetSystemItemStrHandle ( nSysItemId, &hszItem );


	/** Create an empty buffer of the sufficient size
	*** to hold the data for our channel(s).
	*** Include space for Tab delimitors
	**/


	switch ( nSysItemId ) {

		case SYSITEM_ID_FORMATS :

			/**	Only a single clipborad format supported
			**/
			dwBytesReq = (DWORD) strlen ( SZDDE_SUPPORTED_FORMAT ) + 1;
			break;

		case SYSITEM_ID_SYSITEMS :

			/**	Return a list of System Item names, get these from
			***	the static string table 'asipSystemItemTable'
			***
			**/
			dwBytesReq = 0;
			for ( nSysItem = 0; nSysItem < NDDE_SYSTEMITEMS; nSysItem++ ) {

				dwBytesReq += (DWORD) ( strlen
				  ( asipSystemItemTable [ nSysItem ].szName ) + 1 );


			}
			break;

		default :
		case SYSITEM_ID_STATUS :

			dwBytesReq = (DWORD) strlen ( SZSTATUS_MESSAGE );
			break;

		case SYSITEM_TOPIC_IDS :

			/**	Return a list of Topic name, get these from
			***	the static string table 'asipTopicTable'
			***
			**/
			dwBytesReq = 0;
			for ( nTopic = 0; nTopic < NDDE_TOPICS; nTopic++ ) {

				dwBytesReq += (DWORD) (
				  strlen ( asipTopicTable [ nTopic ].szName ) + 1 );
			}
			break;

		case SYSITEM_ID_ITEMLIST :
		case SYSITEM_ID_CHANNELS :

			/** Data Items i.e. Channel names as stored in the
			*** connections array ( see conninfo STRUCT )
			**/
			dwBytesReq = 0;
			for ( nItem = 0; nItem < conninfo.nNumConnected; nItem++ ) {


				dwBytesReq += (DWORD) ( strlen
				  ( conninfo.pciConnections [ nItem ].szChannel ) + 1 );


			}

			/**	Dont forget the addition data item 'AllChannels'
			***	...allocate space for this..
			**/
			dwBytesReq += (DWORD)
				( strlen ( SZDATA_ITEM_ALLCHANNELS ) + 1 );
			break;

		case SYSITEM_ID_EXECUTELIST :

			/**	Return a list of supported Executes, get these from
			***	the static string table 'asipExecuteTable'
			***
			**/
			dwBytesReq = 0;
			for ( nExecute = 0; nExecute < NDDE_EXECUTES; nExecute++ ) {

				dwBytesReq += (DWORD) (
				 strlen ( asipExecuteTable [ nExecute ].szName ) + 1 );


			}
			break;



		case SYSITEM_ID_HELP :

			dwBytesReq = strlen ( szDdeHelp ) + 1;
			break;

	} /* End Case */



	/**	Ask the DDEML for a data handle to a memory object
	***	of the correct size. At this point dont copy data
	***	into the memory.
	**/
	hRequestedData = DdeCreateDataHandle (
			  ddeInfo.idInst,		/* Instance handle */
			  NULL,				/* Buffer	   */
			  dwBytesReq,			/* bytes required  */
			  0L,				/* Offset into buf */
			  hszItem,			/* Item String Hdl */
			  wFmt, 			/* Clipboard Format*/
			  0 );				/* Flags	   */



	if ( hRequestedData == NULL ) {

		/**	Failed to create the handle
		***	DdeGetLastError will tell us why
		***	but its probably lack of memory
		***	Simply return the NULL handle.
		***	The caller will return this to
		***	DDEML indicating no data available.
		**/
		return ( (HDDEDATA) NULL );

	}
	/**	Access the memory object
	**/
	lpszDdeData = DdeAccessData ( hRequestedData, NULL );

	if ( lpszDdeData == NULL ) {

		/**	Were really in trouble now...
		***	The memory object exists but we cant
		***	fill it. Safest bet is to return the
		***	handle minus the data.
		**/
		return ( hRequestedData );
	}



	/**	Initialize the memory object with a null-terminator
	**/
	lpszDdeData [ 0 ] = '\0';



	/**	Now copy the data for the system item
	***	into the memory object
	**/
	switch ( nSysItemId ) {

		case SYSITEM_ID_FORMATS :

			/**	Only a single clipborad format supported
			**/
			lstrcpy ( lpszDdeData, SZDDE_SUPPORTED_FORMAT );
			break;

		case SYSITEM_ID_SYSITEMS :

			/**	Return a list of System Item names, get these from
			***	the static string table 'asipSystemItemTable'
			***
			**/
			for ( nSysItem = 0; nSysItem < NDDE_SYSTEMITEMS; nSysItem++ ) {

				lstrcat ( lpszDdeData,
					asipSystemItemTable [ nSysItem ].szName );


				/**	Tab delimit the field unless last.
				***
				**/
				if ( nSysItem != ( NDDE_SYSTEMITEMS - 1 ) ) {

					/**	Tab Field delimitor
					**/
					lstrcat ( lpszDdeData, (LPSTR) "\t" );
				}
			}
			break;


		case SYSITEM_ID_STATUS :

			lstrcpy ( lpszDdeData, SZSTATUS_MESSAGE	);
			break;

		default :
		case SYSITEM_TOPIC_IDS :

			/**	Return a list of Topic name, get these from
			***	the static string table 'asipTopicTable'
			***
			**/
			for ( nTopic = 0; nTopic < NDDE_TOPICS; nTopic++ ) {

				lstrcat ( lpszDdeData,
					asipTopicTable [ nTopic ].szName );


				/**	Tab delimit the field unless last.
				***
				**/
				if ( nTopic != ( NDDE_TOPICS - 1 ) ) {

					/**	Tab Field delimitor
					**/
					lstrcat ( lpszDdeData, (LPSTR) "\t" );
				}
			}
			break;

		case SYSITEM_ID_ITEMLIST :
		case SYSITEM_ID_CHANNELS :

			/** Data Items i.e. Channel names as stored in the
			*** connections array ( see conninfo STRUCT )
			**/
			for ( nItem = 0; nItem < conninfo.nNumConnected; nItem++ ) {

				lstrcat ( lpszDdeData,
				  (LPSTR) conninfo.pciConnections [ nItem ].szChannel );

				/**	Tab delimit the field.
				***	The 'AllChannels' field will be the
				***	last item name therefore ALL the channel
				***	names should be appended with a TAB char.
				***
				**/

				/**	Field terminator
				**/
				lstrcat ( lpszDdeData,	(LPSTR) "\t" );


			}
			/**	Finally append the special channel name 'AllChannels'
			***	This is the last field and should be null terminated
			***	It does NOT need a tab serperator...
			**/
			lstrcat ( lpszDdeData, (LPSTR) SZDATA_ITEM_ALLCHANNELS );
			break;


		case SYSITEM_ID_EXECUTELIST :

			/**	Return a list of Execute commands, get these from
			***	the static string table 'asipExecuteTable'
			***
			**/
			for ( nExecute = 0; nExecute < NDDE_EXECUTES; nExecute++ ) {

				lstrcat ( lpszDdeData,
					asipExecuteTable [ nExecute ].szName );


				/**	Tab delimit the field unless last.
				***
				**/
				if ( nExecute != ( NDDE_EXECUTES - 1 ) ) {

					/**	Tab Field delimitor
					**/
					lstrcat ( lpszDdeData, (LPSTR) "\t" );
				}
			}
			break;




		case SYSITEM_ID_HELP :

			lstrcat ( lpszDdeData, (LPSTR) szDdeHelp );

			break;

	} /* End Case */


	/**	Finished with the memory object
	**/
	DdeUnaccessData ( hRequestedData );

	return ( hRequestedData );
}





/**	Function	ExtractPokeData
***
***	Description	This function is passed a global memory object
***			handle containing poke data for the specified
***			data item. The data item may be an any channel
***			index or the special item; 'AllChannels'. The
***			channel data is accessed and stored in the
***			channels OUTPUTDATA structure. This will then be
***			transfered to the hardware on receipt of a
***			WM_WRITEDATA message.
***	Parameters
***	  hData		Is the global memory object handle containing
***			the poke data.
***
***	  nItemId	Is the 'Data' topic item identifier, this may be
***			the 'AllChannels' identifier. The identifier may
***			be used to index the conninfo.pciConnections and
***			the conninfo.lpodOutputData arrays ( unless
***			it is the 'AllChannels' identifier )
***
***	Return		OK, or
***			ERR_DDE_ITEM_NOT_SUPPORTED
***
***
***			( NOTE The static 'connect' structure may be
***			modified by this function. Inparticular the
***			array of DATAOUTPUT structures may contain new
***			channel data requiring processing by a WM_WRITECHAN
***			message	)
**/
static short ExtractPokeData ( HDDEDATA hData, short nItemId )
{


	short	nChannel;
	int  	nState;  // Changed from short to fix sscanf bug
	DWORD	dwBytesRecieved;



	/**	The following are for the DDE data buffer which
	***	contains the poke data. If the DDE data item is
	***	'AllChannels' this should contain one value for each
	***	connected channel seperated by a Tab delimiter.
	**/
	LPBYTE	lpszDdeData;	/* Pointer to the DDE buffer */
	DWORD	ulDataLength;	/* Length of the buffer      */
	DWORD	ulDataIndex;	/* Index into the buffer     */


	/**	 The following local string is used to hold ONE
	***	 data value. We will index this string using 'nChar'
	**/
	char	szChanValue [ MAX_VALUE + 1 ];
	short	nValueIndex;

	short	nConverted;



	/**	We have two conditions here which determine how we
	***	process the data in the global memory object.
	***	If the data item has the 'AllChannels' item identifier
	***	we must extract a value for each connected channel
	***	otherwise a single value should exist which belongs
	***	to the channel indexed by the item identifier.
	**/
	if ( nItemId == conninfo.nNumConnected ) {


		/**	Obtain access to the Data and
		***	extract the data for each channel.
		**/
		lpszDdeData = DdeAccessData ( hData, &ulDataLength );

		/**	POKE data is for 'AllChannels'
		***
		**/
		nChannel	= 0;	 /* Start with first channel	*/
		nValueIndex	= 0;	 /* Set local buffer to start	*/

		for ( ulDataIndex = 0L; ulDataIndex < ulDataLength;
							ulDataIndex++ ) {

			/**
			***	Remove the next character
			***	from the global Dde buffer
			***	and copy it into a local
			***	string buffer.
			**/
			szChanValue [ nValueIndex ] =
			       lpszDdeData [ ulDataIndex ];


			/**
			***	Check if the character is a field
			***	terminator or if it would overflow
			***	our local string buffer. Remember
			***	we need room to null terminate the
			***	local string.
			***
			***	In either case, copy upto and including the
			***	delimiter, null terminate the local string
			***	(overwriting the delimitor which is not
			***	needed ) convert to binary format and store in
			***	the current channels channel's DATAOUTPUT structure.
			***	From here it will be transfered to the hardware on
			***	the next WM_WRITECHAN message.
			***
			***	NOTE Tabs will normally be expected as field
			***	seperators but also accept spaces or
			***	commas. Additionally the CF_TEXT format
			***	defines the data terminator as being
			***	a carriage return\linefeed. This will
			***	therefore terminate the data for the last
			***	channel.
			***
			***
			**/

			if ( ( szChanValue [ nValueIndex ] == '\t' ) ||
			     ( szChanValue [ nValueIndex ] == ' ' )  ||
			     ( szChanValue [ nValueIndex ] == ',' )  ||
			     ( szChanValue [ nValueIndex ] == '\r' ) ||
			     ( nValueIndex == ( MAX_VALUE - 1 ) ) ) {

				/**	Null terminate the
				***	buffer segment that
				***	has accumulated in
				***	szChanValue. Overwrite
				***	the delimiter or if o\flow
				***	the last character in the
				***	string.
				**/
				szChanValue [ nValueIndex ] = '\0';


				/**	Convert to binary format. When nState was specified
				*** as short this could cause an error.
				**/
				nConverted =
				  sscanf ( szChanValue,"%d", &nState );

				if ( nConverted != 0 ) {


				    /** String successfully translated
				    *** to a valid short.
				    **/


				    /** Copy the new data into the channel's
				    *** OUTPUTDATA structure
				    **/
				    conninfo.lpodOutputData
					[ nChannel ].nNewState =
						 ( nState == 0 ) ? 0 : 1;

				    /**	Flag the data as being new and
				    ***	requiring attention.
				    **/
				    conninfo.lpodOutputData
					[ nChannel ].bWriteData = TRUE;
				}


				/** Move onto next Channel
				**/
				if ( ++nChannel ==
				    conninfo.nNumConnected ) {

					/** No more channels
					*** to process, abort
					*** the loop witout
					*** concerning
					*** ourselves about
					*** what remains in
					*** the ddeData buffer.
					*** Hopefully the buffer
					*** should now be empty.
					**/
					break;
				}

				nValueIndex = 0 ;/* Reset local buffer */

			} else {
				/** Move onto next Char in local buffer
				**/
				nValueIndex++;
			}
		}

		/**	Invalidate the lpszChanValue pointer.
		**/
		DdeUnaccessData ( hData );


	} else {

		/**	The buffer should contain a single data value
		***	Rather than access the shared DDE memory we will
		***	get the DDEML to copy the data directly into
		***	our local buffer.
		**/


		dwBytesRecieved = DdeGetData ( hData,
				(LPBYTE) szChanValue,
				(DWORD) sizeof ( szChanValue ),
				0L );



		/**	POKE data is for single channel
		**/
		nConverted = sscanf ( szChanValue,
					"%d", &nState );

		if ( nConverted == 0 ) {


			/** string could not be translated
			*** to a valid short.
			**/
			return ( ERR_DDE_ITEM_NOT_SUPPORTED );



		}

		/**	Make the data accessible to the
		***	WPDO_MSS module ready to be processed
		***	by a WM_WRITEDATA message.
		***
		***	Each channel has an OUTPUTDATA
		***	structure for storage of new data.
		***	A pointer to the array of OUTPUTDATA
		***	structures is stored in :
		***
		***		'conninfo.lpodOutputData'
		***
		***
		***	The output value should be digital
		**/
		conninfo.lpodOutputData [ nItemId ].nNewState
					= ( nState == 0 ) ? 0 : 1;


		/**	Flag the data as being new and
		***	requiring attention.
		**/
		conninfo.lpodOutputData [ nItemId ].bWriteData
				= TRUE;




	}

	return ( OK );


}




/**	Function	GetExecuteId
***
***	Description	This function searches the array of Execute
***			structures for the specified string handle. If
***			found the Execute's Identifier is returned by
***			reference together with a copy of any execute
***			command parameters appended on to the string.
***
***	Note		The return value MUST be checked to ensure a match
***			exists and a valid identifier returned.
***
***	Return Value	TRUE if topic supported and identifier returned.
***			FALSE if topic not supported.
***
**/
BOOL GetExecuteId ( HDDEDATA hExecute, short *pnIdentifier, char *szParams )
{
	short	nExecute;
	char	szExecute [ MAX_EXECUTE_LEN + 1 ];
	char	*szEndCmd;
	LPSTR	lpszData;
	DWORD	dwLength;

	lpszData = (LPSTR) DdeAccessData ( hExecute, (LPDWORD) &dwLength );
	if ( lpszData == NULL ) {


		DdeUnaccessData ( hExecute );
		return ( FALSE );

	}

	/**	Copy the data to local memory and relaese the far copy
	**/
	_fstrcpy ( (LPSTR) szExecute,  lpszData );
	DdeUnaccessData ( hExecute );


	/**	Strip of Parameters and store in szParam
	***	It is assumed the command is seperated from any
	***	parameters by a space character.
	**/
	szEndCmd = strchr ( szExecute, ' ' );
	if ( szEndCmd != NULL ) {

		/** Terminate command portion of string
		*** Replace the field terminator ( space char ) with a
		*** end	of string terminator
		**/
		szEndCmd [ 0 ] = '\0';
		strcpy ( szParams, ++szEndCmd );

	} else {

		szParams [ 0 ] = '\0'; /* Empty Parameter string	*/
	}




	for ( nExecute = 0; nExecute < NDDE_EXECUTES; nExecute++ ) {

		if ( stricmp ( szExecute,
		       asipExecuteTable	[ nExecute ].szName ) == 0 ) {

			/**	Item found
			**/
			*pnIdentifier =
				asipExecuteTable [ nExecute ].nIdentifier;

			return ( TRUE );
		}
	}
	return ( FALSE );  /* Item not in table */
}





/**
***	Is this service name available
***
***	Establishes what Services are currently loaded and
***	compares them with the requested service name.
***
***	Returns TRUE if no other service is registered with
***	that service name or FALSE if the service is already in use.
**/
BOOL IsServiceNameFree ( LPSTR lpszServiceName )
{
	HSZ	hszService, hszTopic;
	HCONV	hConv;



	hszService = DdeCreateStringHandle ( ddeInfo.idInst,
					lpszServiceName, CP_WINANSI );

	hszTopic = DdeCreateStringHandle ( ddeInfo.idInst,
						"System", CP_WINANSI );



	hConv = DdeConnect ( ddeInfo.idInst, hszService, hszTopic,
							(LPVOID) NULL );


	DdeFreeStringHandle ( ddeInfo.idInst, hszService );
	DdeFreeStringHandle ( ddeInfo.idInst, hszTopic );

	if ( hConv == (HCONV) NULL ) {


		/**	No conversation is using this name
		**/

		return ( TRUE );


	} else {



		DdeDisconnect ( hConv );
		return ( FALSE );

	}

}
