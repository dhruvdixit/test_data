#!/bin/bash

cd $ALICE_PHYSICS
cd ../build
echo "I am in Alice Physics build"
make install
#./hello.sh
cd ~/test_data
echo "I am in test_data"
ar runAnalysisConv.C
