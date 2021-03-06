#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

BUILD_DIR=bin
mkdir $BUILD_DIR  > /dev/null 2>&1 && ./build.sh

TEST_SIZE="20M"
DATA_FILE=$BUILD_DIR/data.txt
ANSWER_FILE=$BUILD_DIR/refer.txt
TEST_WORD="ABRAcadABRA"
echo "Functional test begins.."
echo "Creating $TEST_SIZE input.."
python input_data_generator.py $DATA_FILE $TEST_SIZE $TEST_WORD

echo "Building relevant answer.."
export LC_ALL=C # for correct sorting using gnu sort
sed "s/\s\(\<$TEST_WORD\>\s\)*/ /g" $DATA_FILE |   \
	sed "s/\s\<$TEST_WORD\>$//g" |  \
	sed -r "s/^\<$TEST_WORD\>(\s|$)//g"  |  \
	sort --ignore-case --output=$ANSWER_FILE

echo "Start testing.."
APP_RESULT_FILE=$BUILD_DIR/output.txt

for MODE in "--regex" ""
do
	echo "Start TC $TEST_SIZE @ $(date)"  
	echo "./$BUILD_DIR/filter_n_sort $TEST_WORD $DATA_FILE $APP_RESULT_FILE $MODE"
	time ./$BUILD_DIR/filter_n_sort $TEST_WORD $DATA_FILE $APP_RESULT_FILE $MODE
	if diff -i $ANSWER_FILE $APP_RESULT_FILE  > /dev/null
	then
	    echo "${green}TESTCASE $TEST_SIZE PASSED ${reset}"
	else
	    echo "${red}TESTCASE $TEST_SIZE FAILED ${reset}"
	    exit 1
	fi
	rm -f $APP_RESULT_FILE
done
echo "All tests passed ($(date))"