#!/bin/bash

TOPDIR=`pwd`
BINDIR=${TOPDIR}/build/demo

xterm -T "Dec"  -e "/bin/bash -c 'cd ${BINDIR}/dec   && ./dec;        exec /bin/bash -i'"&
xterm -T "User" -e "/bin/bash -c 'cd ${BINDIR}/user  && ./user 12 3;  exec /bin/bash -i'"&
xterm -T "Cs"   -e "/bin/bash -c 'cd ${BINDIR}/cs    && ./cs;         exec /bin/bash -i'"&
