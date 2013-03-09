#!/bin/bash

VERSION_libswift=
DEPS_libswift=()
URL_libswift=https://github.com/whirm/tgs-android/archive/master.zip
MD5_libswift=23ce86e2bd4d213fdcf1d8c5c37a979a
BUILD_libswift=$BUILD_PATH/libswift/$(get_directory $URL_libswift)
RECIPE_libswift=$RECIPES_PATH/libswift

function prebuild_libswift() {
	true
}

function build_libswift() {
	cd $BUILD_libswift
	
	#FIXME check for .so
	if [ -d "$BUILD_PATH/python-install/lib/python2.7/site-packages/libswift" ]; then
		#return
		true
	fi
	
	push_arm
	
	#FIXME get it so you don't have to download the jni module manually
	export $LDFLAGS=$LIBLINK
	try ndk-build -C $BUILD_libswift/jni
	unset LDFLAGS

	echo "Copying libs:"
	try cp -a $BUILD_libswift/libs/$ARCH/*.so $LIBS_PATH

	pop_arm
}

function postbuild_libswift() {
	true
}

