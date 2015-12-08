# FSTR ![](https://magnum.travis-ci.com/fmalinowski/fstr.svg?token=vCQTxKRqTuz4r6h4qxQX&branch=master)
**File System That Rocks**


FSTR is a user space filesystem that uses the [FUSE library](http://fuse.sourceforge.net/).
It has been developed in the C language by Tanuj Mittal, Aviral Takkar and Francois Malinowski as part of the Graduate class CS270 (Operating Systems) project at UCSB. The CS270 class was taught by Professor Rich Wolski.

This project could be definitely extended to support more features but the file system in its current state is working and can support thousands of files and creation/copy/removal of files that have sizes greater than 2GB.
FSTR has been tested with a volume of 30GB.
A small presentation of our File System is available in the slides.

## How to set up the environment to use our File System?

You will need mainly the GCC compiler and the FUSE library.
On a CentOS 6.4 image, you can set up the environment with the following command:
```
sudo yum -y install gcc make fuse fuse-devel
```
To install fuse on Ubuntu, you can run the following command:
```
sudo apt-get install libfuse-dev
```


## How to configure the space needed by the File System and the location of the volume?

By default, FSTR assumes that the volume will be 30GB.
To change that value, you can open the common.h file in the src folder and modify the constant ``FS_SIZE`` to be the size of your volume (size in Bytes). For instance if your volume has 8GB, you will define the constant as follows: ``#define FS_SIZE ((big_int) 8 * 1024 * 1024 * 1024)``.

FSTR assumes that the attached volume will be at ``/dev/vdc``. This value can be changed by setting the correct path in the constant ``DISK_STORE_PATH`` of the file ``common.h``.


## How to compile the File System?

Different makefiles are present at different levels in this project.
There's a makefile in the src folder that compiles only the source files and produces a binary called ``fstr`` in the bin folder.
Another makefile is also present in the tests folder to compile the source files in src folder and to compile the unit tests.
A 3rd makefile is located in the tests-bash-scripts folder to execute the bash script tests against the File System.
At last, there's a top level makefile that executes all the makefiles in the src, tests and tests-bash-scripts folders.

In order to run FSTR, you can either run the ``sudo make`` command at the top level of this project or run that command from within the src folder.

**Please note that you need to be root to compile the project and to execute FSTR, hence the ``sudo``!**

## How to run FSTR?

After having compiled the project with one of the makefiles, you will get the ``fstr`` binary in the bin folder.
You need to create a folder somewhere on your system that will be the mount point.

If we consider that your mount point is located at ``/tmp/fstr``, then you can launch the file system by typing the following command in the bin folder:
```./fstr /tmp/fstr/```
Once the command is entered, FSTR will handle the file system mounted at ``/tmp/fstr`` in this case.

You can also get the debug statements provided by FUSE by inserting the ``-d`` option: ``./fstr /tmp/fstr/ -d``

**Please note that to run the file system you also need to be root. You can be root by executing the command: ``sudo -s``**
