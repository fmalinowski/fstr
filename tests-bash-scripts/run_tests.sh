#!/bin/bash

# MOUNT_POINT="/tmp/fuse_bash_tests/"
MAKE_FOLDER_OF_FSTR="../src/"
BIN_FOLDER_OF_FSTR="../bin/"
MOUNT_POINT_RELATIVE_TO_BIN_FOLDER="./fuse_tests"

bash_tests_path=$(pwd)
mount_point_path="$(pwd)/$BIN_FOLDER_OF_FSTR$MOUNT_POINT_RELATIVE_TO_BIN_FOLDER"

cd $MAKE_FOLDER_OF_FSTR
make
cd $bash_tests_path

sudo fusermount -u $mount_point_path &> /dev/null
sudo rm -rf $mount_point_path

echo
echo
echo "RUNNING ALL BASH SCRIPT TESTS"
test_files=$(ls | grep test_)

for filename in $test_files
do
	mkdir $mount_point_path

	cd $BIN_FOLDER_OF_FSTR
	sudo ./fstr $MOUNT_POINT_RELATIVE_TO_BIN_FOLDER
	cd $bash_tests_path

	chmod u+x $filename
	echo -n "TEST - $filename "

	# go to mount point
	cd $mount_point_path

	path_of_test_file="$bash_tests_path/$filename"
	#Execute test script
	sudo $path_of_test_file $mount_point_path

	result_code=$?

	cd $bash_tests_path
	sudo fusermount -u $mount_point_path
	sudo rm -rf $mount_point_path

	if [ $result_code == 0 ]
	then
		echo "PASS"
	else
		echo " FAILED"
		exit 1
	fi
done

echo
echo "-----------------"
echo "ALL TESTS PASSED!"
exit 0
