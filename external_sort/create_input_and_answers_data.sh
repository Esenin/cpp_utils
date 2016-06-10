#!/bin/bash

BUILD_DIR=bin

mkdir $BUILD_DIR

TESTCASES=testcases.txt

export LC_ALL=C # for correct sorting using gnu sort


while read -r SIZE_VAL
do
	INPUT_DATA=$BUILD_DIR/$SIZE_VAL.txt
	ANSWERS=$BUILD_DIR/ans$SIZE_VAL.txt
	echo "Creating $SIZE_VAL input.."
	python input_data_generator.py $INPUT_DATA $SIZE_VAL
	echo "Building relevant answers.."
	sort -o $ANSWERS -T . $INPUT_DATA 
done < "$TESTCASES"

echo "Input data for tests has been created"

