#!/bin/bash

VERSION_netifaces=0.8
DEPS_netifaces=(hostpython python setuptools)
URL_netifaces=http://alastairs-place.net/projects/netifaces/netifaces-0.8.tar.gz
MD5_netifaces=e57e5983f4c286fac5f8068fbfc5c873
BUILD_netifaces=$BUILD_PATH/netifaces/$(get_directory $URL_netifaces)
RECIPE_netifaces=$RECIPES_PATH/netifaces

function prebuild_netifaces() {
	true
}

function build_netifaces() {
	cd $BUILD_netifaces

	if [ -d "$BUILD_PATH/python-install/lib/python2.7/site-packages/netifaces" ]; then
		#return
		true
	fi
	
	push_arm
	
	# build python extension
	export CFLAGS="$CFLAGS -I$BUILD_PATH/python-install/include/python2.7"
	export LDSHARED=$LIBLINK
	export PYTHONPATH=$BUILD_PATH/python-install/lib/python2.7/site-packages
	
	# resulting .so is empty but .o will be collected into libpymodules.so in final distribute.sh step
	try $BUILD_hostpython/hostpython setup.py build_ext -v
	
	unset LDSHARED
	
	try $BUILD_hostpython/hostpython setup.py install -O2 --prefix $BUILD_PATH/python-install

	pop_arm
}

function postbuild_netifaces() {
	true
}

