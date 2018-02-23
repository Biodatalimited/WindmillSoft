/** Header file for the Common parts of the copy control software
*** These are shared between Confiml and Imslib.
**/



/** struct to hold details obtained from the SOFTWARE section
*** of IML.INI.
**/
typedef struct SoftwareTag 

{	char szName [20]; // Program Name
	int  nCode;      // Authorisation Code
	int  nDate;		 // Install Date in Months
} SOFTWARE;


/** Works out the computer code **/
int GetComputerCode ( void );
/** Works out the authorisation code for a piece of software **/
int CalculateSoftwareCode ( char* cProgramTitle );
/** Gets details of all software from an INI or INF file **/
int GetSoftwareDetailsFromINI ( char * cFile );
/** Returns the code stored in IML.INI for a piece of software **/
int ReadSoftwareCode ( char* cProgramTitle );
/** Reads the nember of releases from iml.ini **/
int GetNumberofReleases (void);
/** Calculate and store the correct codes in iml.ini **/
int AuthoriseAllPrograms ( int nReleases );
/** Calculate the release key **/
void CalculateKey ( char* cBuffer, int nPrograms, int nInvalidKeys,
				   int nDate, int nComputerCode, int nReleases);

int InstallSoftwareFromINF ( char* cFile );


