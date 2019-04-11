Please be aware that this is a sample code that was written by a customer. The Labview-SDK2 is a sample program provided free of charge. 
Is it not supported in any way, it is made available in the hope that it might be of some use to you. No warranty is made or implied, it is provided strictly on an "as is" basis.



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

