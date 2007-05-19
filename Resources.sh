#! /bin/bash

rm -r /usr/local/share/magicseteditor/resource

for DIR in /usr/local/share/magicseteditor /usr/local/share/magicseteditor/resource /usr/local/share/magicseteditor/resource/icon /usr/local/share/magicseteditor/resource/tool /usr/local/share/magicseteditor/resource/cursor
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

cp src/resource/common/* /usr/local/share/magicseteditor/resource;
cp src/resource/msw/tool/* /usr/local/share/magicseteditor/resource/tool;
cp src/resource/msw/icon/* /usr/local/share/magicseteditor/resource/icon;
cp src/resource/msw/cursor/* /usr/local/share/magicseteditor/resource/cursor;
cp src/resource/msw/other/* /usr/local/share/magicseteditor/resource;