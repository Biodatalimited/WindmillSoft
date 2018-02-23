Attribute VB_Name = "UsbUtilities"
Option Explicit


'OpenUSBHandle
'This function opens a handle in the USB driver which allows you to talk to the specified device
'nDeviceType specifies the hardware type of the target device. For a Microlink frame this is 3304
'nIDCode is the number which distinguishes the device from others of the same type.
'In the case of a 3304 this is the primary address.
'The function will return a Handle which will be used in the other function calls.
'It will return -1 if it fails.
'eg  nHandle = OpenUSBHandle ( 3304, 7 ) opens a 3304 at primary address 7
Declare Function OpenUSBHandle Lib "mlusblib.dll" Alias "_OpenUSBHandle@8" (ByVal nDeviceType As Long, ByVal nIDCode As Integer) As Long

'CloseUSBHandle
'This function closes the handle obtained by OpenUSBHandle when it is no longer needed.
'It should be called when your program closes. The return value is not important
Declare Function CloseUSBHandle Lib "mlusblib.dll" Alias "_CloseUSBHandle@4" (ByVal nHandleNum As Long) As Long

'UsbSAListen
'This function sends data to a Microlink Module.
'nHandleNum is the handle obtained by OpenUSBHandle
'nSecAdd is the Secondary Address of the module
'cData is an array of Data Bytes to send
'nBytes is the number of bytes to send
'The return value is 0 if it succeeds
'See the 3010 example below
Declare Function UsbSAListen Lib "mlusblib.dll" Alias "_UsbSAListen@16" (ByVal nHandleNum As Long, ByVal nSecAdd As Long, ByRef cData As Byte, ByVal nBytes As Long) As Long

'UsbSATalk
'This function reads data from a Microlink Module.
'nHandleNum is the handle obtained by OpenUSBHandle
'nSecAdd is the Secondary Address of the module
'cData is an array of Data Bytes to return the data in
'nBytes is the number of bytes to read
'The return value is the number of bytes read
'See the 3010 example below
Declare Function UsbSATalk Lib "mlusblib.dll" Alias "_UsbSATalk@16" (ByVal nHandleNum As Long, ByVal nSecAdd As Long, ByRef cData As Byte, ByVal nBytes As Long) As Long


'High Speed Data
'The following functions are used to read data generated at high speed by 3070 / 3041 modules.
'The usb uses a mechanism known as isochronous streaming to get the data. It is necessary to
'start and stop this mechanism within the usb driver.

'UsbStartIsoStream
'When the Microlink is sampling data into the 3041 Fifo this function can be called to start
'the data being transfered across the Usb and into the computer.
'This uses a mechanism known as isochronous streaming
'nHandleNum is the handle obtained by OpenUSBHandle
'dScanPeriod is the Time between input scans in microseconds
'nChannels is the number of analog channels in the scan
'nNotify is the minimum amount of data which you wish to read in a single packet
'nBufferSA is the secondary address of the fifo element of the 3041 module
'These values are required so that the driver can determine data rates and packet sizes for
'the usb transfer
Declare Function UsbStartIsoStream Lib "mlusblib.dll" Alias "_UsbStartIsoStream@24" (ByVal nHandleNum As Long, ByVal dScanPeriod As Double, ByVal nChannels As Long, ByVal nNotify As Long, ByVal nBufferSA As Long) As Long

'UsbStopIsoStream
'This function terminates the streaming started above
'nHandleNum is the handle obtained by OpenUSBHandle
Declare Function UsbStopIsoStream Lib "mlusblib.dll" Alias "_UsbStopIsoStream@4" (ByVal nHandleNum As Long) As Long


'UsbReadIsoch
'This function reads the streamed data from the driver
'nHandleNum is the handle obtained by OpenUSBHandle
'cData is a data array to read the bytes into.
'nGetBytes is the maximum number of bytes to read
'nReadBytes is filled in as the number of bytes actually read.
'This will be some value between the notify level set in UsbStartIsostream and the maximum nGetBytes.
'It may be 0 if less than the notify level of data is available.
'The fuction returns 1 for OK. Any other value is an error
Declare Function UsbReadIsoch Lib "mlusblib.dll" Alias "_UsbReadIsoch@16" (ByVal nHandleNum As Long, ByRef cData As Byte, ByVal nGetBytes As Long, ByRef nReadBytes As Long) As Long



Dim n3304 As Long   'Handle returned by OpenUSBHandle

'Open the driver to the passed in Primary address
'return 0 for OK 1 else

Public Function OpenDriver(nPrimary As Long) As Long

    n3304 = OpenUSBHandle(3304, nPrimary)
    
    If n3304 = -1 Then
        Call MsgBox("Error opening the USB Driver", vbApplicationModal)
        OpenDriver = 1
    Else
        OpenDriver = 0
    End If
    

End Function


Public Sub CloseDriver()

    Call CloseUSBHandle(n3304)

End Sub


'This example sends the 2 byte message needed to program a 3010 port
'Port is 0 to 4
'Setting is 0 to 255

Public Function Write3010Port(Secondary As Integer, Port As Integer, Setting As Integer) As Integer
                                                                                               
    Dim cBuf(2) As Byte

    Write3010Port = 0
                                                            
    cBuf(0) = Port              'First byte is the port number
    cBuf(1) = Setting           'Second byte is the setting
    
    If UsbSAListen(n3304, Secondary, cBuf(0), 2) Then
        Call MsgBox("Error Writing to 3010", vbApplicationModal)
        Write3010Port = 1
    End If
    
       
End Function
    
' This example reads data from a 3010 port
' Secondary is the 3010 address.
' Port is the port to read
' Reading is the value read
' It returns the desired byte

Public Function Read3010Port(Secondary As Integer, Port As Integer) As Integer

       
    Dim cBuf(5) As Byte         'Always read 5 bytes
    Dim nBytesRead As Long
    Dim Temp As Integer

                                                        
    'Read all 5 ports
    If UsbSATalk(n3304, Secondary, cBuf(0), 5) <> 5 Then
        Call MsgBox("Error Reading 3010", vbApplicationModal)
        Read3010Port = 0
        Exit Function
    End If
     
    'Return the desired byte
    Read3010Port = cBuf(Port)
    
                
End Function


'This example reads takes a reading from a 3050 input
'Secondary is the 3050 address
'Channel is the input to read
'Range is the range selection code
'It returns the integer value

Public Function Read3050Input(Secondary As Integer, Channel As Integer, Range As Integer) As Integer

    Dim cBuf(8) As Byte
    Dim nReply As Long
    

    cBuf(0) = Channel       'Define the 3050 range
    cBuf(1) = 1
    cBuf(2) = Range
    cBuf(3) = 0
    
    cBuf(4) = Channel       'Select for AD reading
    cBuf(5) = 0
    cBuf(6) = 0
    cBuf(7) = 0
    
    If UsbSAListen(n3304, Secondary, cBuf(0), 8) Then
        Call MsgBox("Error Writing to 3050", vbApplicationModal)
        Read3050Input = 0
    End If
 
    If UsbSATalk(n3304, Secondary, cBuf(0), 2) <> 2 Then
         Call MsgBox("Error Reading from 3050", vbApplicationModal)
        Read3050Input = 0
    End If

    'Assume the reply is MSB first
    'Convert to an integer
    If cBuf(0) > 127 Then
        nReply = (cBuf(0) - 128) * 256 + cBuf(1)
        nReply = nReply - 32768
    Else
        nReply = 256 * cBuf(0) + cBuf(1)
    End If

    Read3050Input = nReply

End Function

'This example shows how to read data at high speed.
'We choose a 3010 as example so as not to clutter the code with 3070 details

Public Sub Read3010Fast(Secondary As Integer)

    Dim cBuf(51) As Byte
    Dim nReadBytes As Long
    Dim Delay As Variant
    
    'If this were a normal high speed analogue stream then the first action would be
    'to program the 3050 channles and trigger the 3070 so that a data stream is being
    'produced into the 3041 buffer. The 3010 in this example takes the place of a buffer
    'with data arriving.

    'Start the streaming mechanism in the driver.
    'We tell the driver that we are sampling every 100 microseconds from 1 channel.
    'The driver then arranges itself to read the resulting 20K bytes per second
    'We tell the driver not to supply us with data unless it has at least 10 bytes available.
    'At high data rates this prevents many small transactions occuring which would slow the process.
    'The Secondary address which in this case is that of a 3010 would normally be that of a 3041.

    If UsbStartIsoStream(n3304, 100, 1, 10, Secondary) <> 0 Then
        Call MsgBox("Error Starting Stream", vbApplicationModal)
    End If
   
   
    'Wait for the first data to arrive. Data packets become available in the driver every 50msec.
    Delay = Timer
    Delay = Delay + 0.1
   
    While Timer < Delay
    Wend
   
    'Read 50 bytes of data
    'In a normal stream you would repeat this call many times to obtain the full amount.
    'You would normally ask for rather more data than can arrive in each 50 msec packet.
    
    If UsbReadIsoch(n3304, cBuf(0), 50, nReadBytes) <> 1 Then
         Call MsgBox("Error Reading Stream", vbApplicationModal)
    End If

    'Stop the streaming mechanism. You cannot use the listen and talk calls whilst streaming
    Call UsbStopIsoStream(n3304)
    

End Sub

