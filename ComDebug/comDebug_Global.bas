Attribute VB_Name = "comDebug_Global"
Option Explicit

'Modes on entering Main Menu

Public Const MODE_NEW_FILE = 0
Public Const MODE_LOAD_FILE = 1
Public Const MODE_NEW_EDIT = 2


'Constants for nEditMode of ChannelDiag

Public Const MODE_ADD_UNALLOCATED_CHANNELS = 0
Public Const MODE_READ_VALUE_CHANNEL = 1
Public Const MODE_WRITE_VALUE_CHANNEL = 2
Public Const MODE_READ_BIT_CHANNEL = 3
Public Const MODE_WRITE_BIT_CHANNEL = 4
Public Const MODE_READ_RECNUM_CHANNEL = 5
