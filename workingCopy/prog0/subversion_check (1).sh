#! /bin/bash

# subversion_check.sh 
#
# Script to check the status of your repository
# It's not necessary, but if you find it convenient you are welcome to use it
#

# check if both required arguments are present
if [ "3" != "$#" ]; then
	echo >&2
	echo "Error: Wrong number of arguments" >&2
	echo "usage:" >&2
	echo "./subversion_check.sh course_num quarter_id progx" >&2
	echo >&2
	echo "where" >&2
	echo "   course_num is the three digit course number (e.g. 367)" >&2
	echo "   quarter_id is a six digit identifier of the quarter" >&2
	echo "      the first four digits are the year (e.g. 2015)" >&2
	echo "      the last two digits are the quarter (Winter=10, Spring=20, Summer=30, Fall=40)" >&2
	echo "   progx is either prog0, prog1, prog2, or prog3" >&2
	echo >&2
	echo "For example:" >&2
	echo "./subversion_check.sh 367 201510 prog0" >&2
	echo >&2
	exit
fi;

COURSE_NUM=$1
QUARTER_ID=$2
PROGRAM_NUM=$3

# sanity check COURSE_NUM argument
if [ 0 = $(echo $COURSE_NUM | grep "^[0-9][0-9][0-9]$" | wc -l) ]; then
	echo >&2
	echo "Error: course number \"$COURSE_NUM\" appears to be invalid (should be three digits)" >&2
	echo >&2
	exit;
fi;

# sanity check QUARTER_ID argument
if [ 0 = $(echo $QUARTER_ID | grep "^201[0-9][1-4]0$" | wc -l) ]; then
	echo >&2
	echo "Error: Quarter ID \"$QUARTER_ID\" appears to be invalid (call with no arguments to see format)" >&2
	echo >&2
	exit;
fi;

# sanity check COURSE_NUM argument
if [ 0 = $(echo $PROGRAM_NUM | grep "^prog[0-3]$" | wc -l) ]; then
	echo >&2
	echo "Error: program number \"$PROGRAM_NUM\" appears to be invalid (should be prog0, prog1, prog2 or prog3)" >&2
	echo >&2
	exit;
fi;

echo >&2
echo "The goal of this program is to assist you in confirming that the files that" >&2
echo "you think you have added and committed are actually in your repository" >&2
echo >&2
echo "It does this by checking out a fresh copy of your working copy to a" >&2
echo "temporary location and seeing which files are there." >&2
echo "In theory, what you see through this check program is exactly what I " >&2
echo "would see if I were to check out your working copy right now." >&2
echo >&2
read -p "Press enter to continue"
	
REPO_DIR=~/$QUARTER_ID/$COURSE_NUM

TEMP_WORKING_COPY=~/.tmp.workingCopy.$(date +%s)

svn co file://$REPO_DIR $TEMP_WORKING_COPY

if [ ! -d $TEMP_WORKING_COPY/$PROGRAM_NUM ]; then
	echo >&2
	echo "Error: No directory found named $PROGRAM_NUM" >&2
	echo "You can investigate by changing directory (cd) to $TEMP_WORKING_COPY and looking for $PROGRAM_NUM" >&2
	echo >&2
	echo "Note: if you files are in the wrong location or directory is named incorrectly," >&2
	echo "      be sure to use \"svn mv\" instead of \"mv\" when you fix it" >&2
	echo >&2
	echo "Exiting now..." >&2
	exit;
fi

echo >&2
read -p "Hit enter to see the list of files in $TEMP_WORKING_COPY/$PROGRAM_NUM (then hit q to quit viewing them)"
find $TEMP_WORKING_COPY/$PROGRAM_NUM | less

read -p "Hit enter to see your log of commits (then hit q to quit viewing them)"
svn log $TEMP_WORKING_COPY/$PROGRAM_NUM | less

echo >&2
echo "You can look at the files in your repository by changing directory (cd) to $TEMP_WORKING_COPY/$PROGRAM_NUM" >&2
echo "Do not work out of $TEMP_WORKING_COPY/$PROGRAM_NUM -- it is for sanity checking your commits only" >&2
echo >&2
