#!/bin/bash

if [ $# != 1 ]; then
	echo "Usage: $0 OUTPUT_DIR"
	exit 1
fi

if [ -d $1 ]; then
	echo "Directory $1 already exists, any existing files will be overwritten."
else
	echo "Creating directory $1..."
	mkdir $1
fi

echo "Finding and copying vcruntime files..."

vcruntime_files_copied=`find "$VCINSTALLDIR/Redist/MSVC/14.29"*/x64 -iname "vcruntime*" -print -exec cp {} $1 \;`
vcruntime_count=`echo "$vcruntime_files_copied" | wc -l`

echo "$vcruntime_files_copied"

if [ $vcruntime_count == 0 ]; then
	echo "No vcruntime files were found!"
	exit 1
fi

if [ $vcruntime_count -gt 2 ]; then
	echo "More than 2 vcruntime files were found (found $vcruntime_count), expected exactly 2, so finishing with an error."
	exit 1
fi

echo "Done finding and copying vcruntime files"

echo "Finding and copying msvcp file..."

msvcp_files_copied=`find "$VCINSTALLDIR/Redist/MSVC/14.29"*/x64 -iname "msvcp140.dll" -print -exec cp {} $1 \;`
msvcp_count=`echo "$msvcp_files_copied" | wc -l`

echo "$msvcp_files_copied"

if [ $msvcp_count == 0 ]; then
	echo "No msvcp file was found!"
	exit 1
fi

if [ $msvcp_count -gt 1 ]; then
	echo "More than 1 msvcp file was found (found $msvcp_count), expected exactly 1, so finishing with an error."
	exit 1
fi

echo "Done finding and copying msvcp file"
