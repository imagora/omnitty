# OMNITTY

Copyright (c) 2017 shanhui@agora.io
Licensed under the GNU General Public License
See the COPYING file for more details on the license terms.

## 1 About omnitty 

OMNITTY MULTIPLE-MACHINE SSH MULTIPLEXER, forked from `http://omnitty.sourceforge.net`.

This program was written by Bruno Takahashi C. de Oliveira in C language. I rewrite it to C++ and add some new feature to it, such as build for macOS and so on.

Now it support **MacOS**, **Ubuntu 14.04 64bit** and above.

### 1.1 What is it?

Omnitty is a curses-based program that allows one to log into several machines simultaneously and interact with them, selectively directing input to individual machines or groups of selected machines. You can run both line-oriented and screen oriented in the target machines, because Omnitty has built-in terminal emulation capability. When the terminal is large enough, Omnitty also displays a "summary area" for each machine, in which it shows what the latest output from the machine was.

### 1.2 More details

Multiple-host network administration usually involves running the same set of commands on several different machines. An administrator might ssh into each of the machines in his network individually to perform the required tasks, but the process soon becomes repetitive and prone to errors. Scripts might help in the case of noninteractive programs and when the administrator knows exactly what commands are to be given. Error handling in these scripts is also difficult to code, and the process becomes especially tedious if these tasks have to be done regularly.

Omnitty tries to present a different approach to manipulating several machines remotely. It simultaneously logs you into all the machines you specify and then presents a screen in which you navigate through the list of machines. When you select a machine, its "terminal" is shown onscreen and they keypresses you type are sent to that machine while it is selected. The user may freely navigate the list, interacting with the machines in any order.

Another feature is that you can 'tag' machines on the list and enter a mode where the input you provide is directed to ALL the machines you tagged, simultaneously. Thus you might tag all the machines in which you need to run a particular command and then type the command once to have all machines execute it.

Omnitty not only works with regular commands, but also with visual programs. For example, you might run 'vi' remotely on several machines simultaneously, and every keystroke you supply will be reproduced in every machine you tagged. Thus you might interactively edit files in several machines at once.

## 2 How to build

The third partys:

* ncurses
* jsoncpp
* log4cplus
* rote
* sshpass

### 2.1 ncurses

For MacOS, you can ignore this step, because the ncurses is system provided.

You can get `ncurses` by `apt-get install` on Ubuntu:

``` sh
sudo apt-get install libncurses5-dev
```

### 2.2 Build jsoncpp

Under the omnitty's root directory, get the `jsoncpp` submodule.

``` sh
git submodule init
git submodule update
```

Then build and install the jsoncpp.

``` sh
cd 3rdparty/jsoncpp
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ../..
make
sudo make install
cd ../../../..
```

By defalut, this will install the libjsoncpp into `/usr/local/lib/` and copy the include files into `/usr/local/include/`.

### 2.3 Build log4cplus

Run command:

``` sh
cd 3rdparty/log4cplus-2.0.0-rc1
./configure
make
sudo make install
cd ../..
```

By defalut, this will install the libjsoncpp into `/usr/local/lib/` and copy the include files into `/usr/local/include/`.

### 2.4 Build rote

Open your terminal, and `cd` to the omnitty.

``` sh
cd 3rdparty/rote-0.2.8
./configure
make
sudo make install
cd ../..
```

By defalut, this will install the libROTE into `/usr/local/lib/` and copy the include files into `/usr/local/include/`.

### 2.5 Build sshpass

Command `ssh` cannot pass password in plaintext, so there are troubles in multi-ssh login.

We'd better not to use this command because it makes it too easy for novice SSH users to ruin SSH's security, and it's better to use ssh identity file instead of password. If there is a reason that must to use password, then you can install the `sshpass` command.

``` sh
cd 3rdparty/sshpass-1.06
./configure
make
sudo make install
cd ../..
```

By defalut, this will install the openssh command into `/usr/local/bin/` directory.

### 2.6 Build omnitty

You can build omnitty by `Qt Creator`, `XCode` or `CMake`.

IDE | Path | Mac | Linux
--- | --- | --- | ---
Qt Creator | ide/qmake/omnitty.pro | ✅ | ✅
XCode | ide/xcode/omnitty.xcodeproj | ✅ | ❎
CMake | ide/cmake/CMakeLists.txt | ✅ | ❎

**Qt Creator**

``` sh
cd ide/qmake
qmake *.pro
make
```

**XCode**

Open `ide/omnitty.xcodeproj` and build.

**Cmake**

``` sh
cd ide/cmake
camek CMakeLists.txt
make
```
