#!/bin/bash

MOUNT_POINT="/tmp/fuse_tests"
MAKE_FOLDER_OF_FSTR="../src/"
BIN_FOLDER_OF_FSTR="../bin/"

bash_tests_path=$(pwd)

# Compile FSTR and MKFS
cd $MAKE_FOLDER_OF_FSTR
sudo make

echo
echo
echo "RUNNING ALL BASH SCRIPT TESTS"

# Get test files
cd $bash_tests_path
test_files=$(ls | grep test_)

for filename in $test_files
do
	# MKFS
	cd $BIN_FOLDER_OF_FSTR

	# Create the mount point
	sudo fusermount -u $MOUNT_POINT &> /dev/null
	sudo rm -rf $MOUNT_POINT
	sudo mkdir $MOUNT_POINT

	# Mount the disk
	sudo ./fstr $MOUNT_POINT
	
	# Execute permissions for the test script
	cd $bash_tests_path
	chmod u+x $filename
	echo -n "TEST - $filename "

	# go to mount point
	cd $MOUNT_POINT

	path_of_test_file="$bash_tests_path/$filename"
	#Execute test script
	sudo $path_of_test_file $MOUNT_POINT

	result_code=$?

	cd $bash_tests_path

	# Unmount FSTR and delete mount point
	#sudo fusermount -u $MOUNT_POINT
	#sudo rm -rf $MOUNT_POINT

	if [ $result_code == 0 ]
	then
		echo "PASS"
	else
		echo " FAILED"
		exit $result_code
	fi
done
# Unmount FSTR and delete mount point
sudo fusermount -u $MOUNT_POINT
sudo rm -rf $MOUNT_POINT


echo
echo "-----------------"
echo "ALL TESTS PASSED!"
exit 0
