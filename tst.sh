#!/bin/bash
# tst.sh - build the programs on Linux.

. ./lbm.sh

if echo "$OSTYPE" | egrep -i darwin >/dev/null; then :
  LIBS="-pthread -l m -L $LBM/lib -llbm"
else :
  LIBS="-pthread -l m -l rt -L $LBM/lib -llbm"
fi

gcc -DUM_SSRC -Wall -g -o um_tgen -I $LBM/include cprt.c tgen.c um_tgen.c $LIBS
if [ $? -ne 0 ]; then echo error in tgen.c; exit 1; fi

./um_tgen -a 2 -g -x um.xml -t topic1 -s "
  delay 200 msec # let topic resolution happen.
  sendc 700 bytes 2 persec 10 msgs
  delay 2 sec	 # linger to allow NAK/retransmits to complete." 
