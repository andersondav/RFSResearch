#!/bin/bash

# cmptree - walk a directory tree and compare each plain file
#		to a file of the same name in another directory
#		tree, reporting differences and missing files
#
# use:		cmptree  original_tree  new_tree
#

case $# in

  2)	OLDTREE=$1
	if test ! -d $OLDTREE; then
		echo "error: $OLDTREE is not a directory" >&2
		exit 1
	fi
	NEWTREE=$2
	if test ! -d $NEWTREE; then
		echo "error: $NEWTREE is not a directory" >&2
		exit 1
	fi
	;;

  *)	echo 'use: cmptree  original_tree  new_tree' >&2
	exit 1
	;;
esac

(cd $OLDTREE > /dev/null
find . -type f -print |
sed "s/^\.//;s@.*@OLDFILE=$OLDTREE&;NEWFILE=$NEWTREE&@") |
while read x; do
echo $x
echo 'if   test ! -r "$OLDFILE"; then echo "Unreadable: $OLDFILE" >&2;'
echo 'elif test ! -r "$NEWFILE"; then echo "Missing or unreadable: $NEWFILE" >&2;'
echo 'else if cmp -s "$OLDFILE" "$NEWFILE"; then true; else echo difference: "$OLDFILE" "$NEWFILE" >&2;fi;fi'
done | sh
