# Microsoft Developer Studio Project File - Name="Logger" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Logger - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Logger.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Logger.mak" CFG="Logger - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Logger - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Logger - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Logger - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Logger - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Logger - Win32 Release"
# Name "Logger - Win32 Debug"
# Begin Group "Kit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ATkit\Atappset.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atdde.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Aterror.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atimlcom.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atkini.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atlimits.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atlodset.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atmonit.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atreawri.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atselect.c
# End Source File
# Begin Source File

SOURCE=..\ATkit\Atunits.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\lg.rc
# End Source File
# Begin Source File

SOURCE=.\Lg_dde.c
# End Source File
# Begin Source File

SOURCE=.\Lg_def.h
# End Source File
# Begin Source File

SOURCE=.\Lg_dlgs.c
# End Source File
# Begin Source File

SOURCE=.\Lg_dlgs.h
# End Source File
# Begin Source File

SOURCE=.\Lg_fdlgs.c
# End Source File
# Begin Source File

SOURCE=.\Lg_file.c
# End Source File
# Begin Source File

SOURCE=.\Lg_hist.c
# End Source File
# Begin Source File

SOURCE=.\Lg_log.c
# End Source File
# Begin Source File

SOURCE=.\Lg_main.c
# End Source File
# Begin Source File

SOURCE=.\Lg_paint.c
# End Source File
# Begin Source File

SOURCE=.\Lg_prot.h
# End Source File
# Begin Source File

SOURCE=.\Lg_setup.c
# End Source File
# Begin Source File

SOURCE=.\Lg_utils.c
# End Source File
# Begin Source File

SOURCE=.\livelog.ico
# End Source File
# Begin Source File

SOURCE=.\Logger.ico
# End Source File
# Begin Source File

SOURCE=.\logger.txt
# End Source File
# Begin Source File

SOURCE="..\Compile Libs\Commslib.lib"
# End Source File
# Begin Source File

SOURCE="..\Compile Libs\Imslib.lib"
# End Source File
# Begin Source File

SOURCE="..\..\Program Files\Microsoft Visual Studio\VC98\Lib\htmlhelp.lib"
# End Source File
# End Target
# End Project
