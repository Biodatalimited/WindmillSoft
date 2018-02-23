/*++

Copyright (c) 1997-1998  Microsoft Corporation

Module Name:

    GuidIso.h

Abstract:

 The below GUID is used to generate symbolic links to
  driver instances created from user mode

Environment:

    Kernel & user mode

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

  Copyright (c) 1997-1998 Microsoft Corporation.  All Rights Reserved.

Revision History:

    2/11/98 : created

--*/
#ifndef GUIDISOH_INC
#define GUIDISOH_INC

#include <initguid.h>



// {D6B744D1-48E9-11d4-9848-525400DDF830}
DEFINE_GUID(GUID_ML751, 
0xd6b744d1, 0x48e9, 0x11d4, 0x98, 0x48, 0x52, 0x54, 0x0, 0xdd, 0xf8, 0x30);


#endif // end, #ifndef GUIDISOH_INC

