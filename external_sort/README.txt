To compile program run:
chmod +x build.sh; 
./build.sh


You can find compiled program at bin/external_sort

Usage:
external_sort <input file> <output-file> <memory limit>[G|M|K|B(default)]

Usage example:
./bin/external_sort input.txt output.txt 4G


Description:
The program may save temporary files in /tmp  or in other directory by
invoking Linux tmpfile() function. If it fails to create file, program will
try to store file in the output file's directory


Further upgrades:
It's possible to gain using parallel I/O






