#!/bin/bash

# [w] generate input and relevant data first !
USAGE="usage ex. (1) test all cases with specific memory limit: ./functional_test.sh 1G
\nusage ex. (2) test single testcase w/ memory limit:  ./functional_test.sh bin/100M.txt 300M"

BUILD_DIR=bin
TESTCASES=testcases.txt

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

if (($# == 0 || $# >= 3));
    then 
    echo -e $USAGE
    exit 1
fi


echo "Start testing "

# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
# sleep 1

if (($# == 1));
    then
        declare -a TC_LIST
        readarray -t TC_LIST < $TESTCASES

        MEM_LIM=$1
    else    
        TC_LIST=$1
        MEM_LIM=$2
fi

CORRECT_ANS="cmp: EOF on $BUILD_DIR/output.txt"
OUTPUT=$BUILD_DIR/output.txt

echo "TC_LIST=${TC_LIST[*]}  MEM_LIM=$MEM_LIM"

for TEST_SIZE in ${TC_LIST[*]}
do
    if [ -z $TEST_SIZE ]
    then
        continue
    fi
    TC_INPUT=$BUILD_DIR/$TEST_SIZE.txt
    TC_ANSWES=$BUILD_DIR/ans$TEST_SIZE.txt

    echo "Start TC $TEST_SIZE sorting @ $(date)"  
    ./$BUILD_DIR/external_sort $TC_INPUT $OUTPUT $MEM_LIM

    if cmp $OUTPUT $TC_ANSWES > /dev/null
    then
        echo "${green}TESTCASE $TEST_SIZE PASSED ${reset}"
    else
        echo "${red}TESTCASE $TEST_SIZE FAILED ${reset}"
        exit 1
    fi

done

echo "All tests passed ($(date))"





