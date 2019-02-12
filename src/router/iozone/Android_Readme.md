# compiling iozone

Assuming you have compiler toolchain installed, just type  `make` to get a list of targets

then to generate for your iozone for your `<OS/target>` simply type:
`make <target>`

**Android specifics**

building iozone for Android means doing a cross compilation

There is two options then.

* cross compiling using Android NDK
* cross compiling from android source tree. Due to the disk space
  required to get an android source tree, this option is valuable
  only if you are already working with android source tree.

**_cross-compiling using Android NDK_**

The prerequiste is to download SDK and NDK.
SDK cannot easilly dissociated from Android Studio those days.

Follow the following steps to get SDK and NDK and
cross compile iozone.

1. Prerequesite : Ubuntu LTS release 16.04 or 18.04 or Mac OS
2. Download Android Studio for your platform from https://developer.android.com/studio/
3. Extract the archive to some place, e.g. your home directory
4. Start android studio
    1. on linux: Android Studio needs to be started from a terminal so open a Terminal
    2. in terminal type `<install_dir>/android-studio/bin/studio.sh`
    3. on Mac OS Android Studio can be launch from lauchpad. if you wish to start it from terminal type `open -a Android\ Studio`
5. keep all the defaults proposed Android Studio.
6. Wait for the download to be ready
7. Download the NDK
    1. Select Configure in Android Studio Splash Screen
    2. Select SDK Manager
    3. Go to system settings Android SDK on the left
    4. Select tab SDK Tools
    5. Select NDK and apply to install NDK
8. Set NDK environment variable to the ndk-bundle directory 
    1.  e.g. `export NDK=/home/parallels/Android/Sdk/ndk-bundle/`
9. download and extract iozone archive
    1. you can opt for extacting only source directory. 
        1. `mkdir iozone`
        2. `cd iozone`
        3. `tar -xvf <path to tar archive>/iozoneX_XXX.tgz -C ./iozone --strip-components 3`
    2. or extracting the full archive
        1. `tar xvf iozoneX_XXX.tgz`
        2. `cd iozoneX_XXX/src/current`
10. build iozone
    1. `make android` This will generate 4 version of iozone for different architecture: arm7, arm64, x86, x86_64
    2. push the iozone excutable to your Android target e.g. `adb push libs/arm64-v8a/iozone /data/iozone`

**_cross-compiling iozone within android source tree_**

The Prerequisite is to have an Android source tree availavle.
If you do not have it, download it android from Google

1. `cd <android root>`
2. `source ./build/envsetup.sh`
3. `lunch <target>`
4. `cd <android root>/external`
5. `mkdir iozone`
6. `curl http://www.iozone.org/src/current/iozoneX_XXX.tar -o iozoneX_XXX.tar` # assuming this version no proxy, etc`
7. `tar -xvf ./iozoneX_XXX.tar -C ./iozone --strip-components 3`
8. `cd iozone`
9. `mm`


