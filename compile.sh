#!/bin/sh

echo Remove current binaries...
rm ../bin/*

echo Compile THREADS...
cd threads/src/Linux_rls
make clean
make
cd ../../../

echo Compile ADAPTERS...
cd adapters/Linux_rls
make clean
make
cd ../../

echo Compile SOCKETS...
cd socket/Linux_rls
make clean
make
cd ../../

echo Compile UDP...
cd udp/Linux_rls
make clean
make
cd ../../

echo Compile TCPIP...
cd tcpip/Linux_rls
make clean
make
cd ../../

echo Compile MODBUS...
cd modbus/Linux_rls
make clean
make
cd ../../

echo Compile _TEST...
cd _test/Linux_dbg
make clean
make
cd ../../
