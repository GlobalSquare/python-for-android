#!/bin/bash

VERSION_minimal=0.1
URL_minimal=
MD5_minimal=
DEPS_minimal=(python)
BUILD_minimal=$BUILD_PATH/minimal/minimal-$VERSION_minimal
RECIPE_minimal=$RECIPES_PATH/minimal

function prebuild_minimal() {
	true
}

function build_minimal() {
	cd $SRC_PATH/minimal/jni

	push_arm
	try ndk-build V=1
	pop_arm

	try cp -a $SRC_PATH/minimal/libs/$ARCH/*.so $LIBS_PATH
}

function postbuild_minimal() {
	true
}
