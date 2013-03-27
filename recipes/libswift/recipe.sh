#!/bin/bash

#TODO get a release version if possible
VERSION_libswift=
DEPS_libswift=()
#TODO get a version specific URL and update the md5sum
URL_libswift=https://github.com/whirm/tgs-android/archive/master.zip
MD5_libswift=23ce86e2bd4d213fdcf1d8c5c37a979a
BUILD_libswift=$BUILD_PATH/libswift/$(get_directory $URL_libswift)
RECIPE_libswift=$RECIPES_PATH/libswift

function prebuild_libswift() {
	true
}

function build_libswift() {
	cd $BUILD_libswift
	
	if [ -d "$BUILD_PATH/libs/libevent.so" ]; then
		#return
		true
	fi
	
	push_arm
	
	#FIXME get it so you don't have to download the jni module manually
	export $LDFLAGS=$LIBLINK
	try ndk-build -C $BUILD_libswift/jni
	unset LDFLAGS

	#TODO find out why it's libevent.so and not libswift.so
	try cp -a $BUILD_libswift/libs/$ARCH/*.so $LIBS_PATH

	pop_arm
}

function postbuild_libswift() {
	true
}

