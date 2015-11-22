#!/bin/bash

FILES_DIRECTORY=5000
MOUNT_POINT=$1

# WE DON'T NEED TO GO TO THE MOUNT POINT FOLDER AS THE RUN_TESTS SCRIPT DOES THAT FOR US
mkdir -p folder1
cd folder1

# Create multiple files
for ((i=1; i<=$FILES_DIRECTORY; i++))
do
	touch $i.txt
	echo $i >> $i.txt
done


function readFiles
{
	for ((i=1; i<=$FILES_DIRECTORY; i++))
	do
		content_file=$(cat $i.txt)
		if [ $content_file != $i ]
		then
			echo "ERROR: The content of a file doesn't match the expected one." >&2
			exit 2
		fi
	done
}

# Count the number of files and make sure it's what we expected
count_of_files=$(ls | wc -l)

if [ $count_of_files == $FILES_DIRECTORY ]
then
	# Read all the files and make sure the content matches the id of the file in its filename
	readFiles
else
	echo "ERROR: Number of files in folder doesn't match the expected one. Expected: $FILES_DIRECTORY but we got: $count_of_files" >&2
	exit 1
fi

rm -rf *
exit 0
