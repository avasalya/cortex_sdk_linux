Please be aware that this is a sample code that was written by a customer. The Labview-SDK2 is a sample program provided free of charge. 
Is it not supported in any way, it is made available in the hope that it might be of some use to you. No warranty is made or implied, it is provided strictly on an "as is" basis.

Please be aware that this is a sample code that was written by a customer of Motion Analysis. I have made some changes as per my purpose. 

feel free to use it and make changes.



Linux version of Cortex SDK --- Tested with Ubuntu 16.04/18.04
--------------------------------------------------------------

===========================
In the clienttest.cpp 
===========================
find line 

1) "retval = Cortex_Initialize("10.1.1.180", "10.1.1.100");"

and change to 

"retval = Cortex_Initialize("my local IP address", "cortex running PC IP address");"


===========================
In the cortex.cpp
===========================
find lines 

1) LOCAL unsigned short wMultiCastPort = 1001; // Cortex sends frames to this port and associated address
2) LOCAL unsigned short wCortexPort    = 1510; // Cortex is listening at this port
3) LOCAL in_addr MyNicCardAddress = { (10 << 24)  + (1 << 16)   + (1 << 8)   + 180 }; // My local IP address
4) LOCAL in_addr CortexNicCardAddress = { (10 << 24) + (1 << 16) + (1 << 8) + 100 };  // cortex running PC IP address


only if MultiCastAddress is not 255.255.255.1

// convert IP address to integer;
//http://www.aboutmyip.com/AboutMyXApp/IP2Integer.jsp?ipAddress=255.255.255.1

LOCAL in_addr MultiCastAddress = {  3774939393 }; // Cortex sends frames to this address and associated port


===========================
Inside Cortex
===========================
enable sdk2
change following

1) select "cortex running PC IP address" from the list
2) wMultiCastPort = 1001; // Cortex sends frames to this port and associated address
3) wCortexPort    = 1510; // Cortex is listening at this port



--------------------------------------------------------------
Compile this example code using the commands
-------------------------------------------------------------

mkdir build 
cd build 
cmake .
make

You will get a static library 'libcortex_sdk.a' and a test binary 'clienttest'.
As Cortex streams multi-cast data on port 1001, you need to be root to receive multi-cast data, e.g. by running

sudo ./clienttest



Linux version of Cortex SDK
===========================

Compile this example code using the commands

mkdir build 
cd build 
cmake ../
make

You will get a static library 'libcortex_sdk.a' and a test binary 'clienttest'.
As Cortex streams multi-cast data on port 1001, you need to be root to receive multi-cast data, e.g. by running

sudo ./clienttest

Tested with Ubuntu 10.04, 14.04, 16.04
