#! /bin/bash

BASEDIR=~/.magicseteditor/resource

rm -rf $BASEDIR

for DIR in $BASEDIR $BASEDIR/icon $BASEDIR/tool $BASEDIR/cursor
do
if [ -d $DIR ]; then
: ;
elif [ -a $DIR ]; then
echo $DIR "exists and is not a directory!";
exit 1 ;
else
mkdir $DIR;
fi
done

cp src/resource/common/* $BASEDIR;
cp src/resource/msw/tool/* $BASEDIR/tool;
cp src/resource/msw/icon/* $BASEDIR/icon;
cp src/resource/msw/cursor/* $BASEDIR/cursor;
cp src/resource/msw/other/* $BASEDIR;
