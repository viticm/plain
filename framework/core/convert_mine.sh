#!/bin/sh

files=`find ./ -name "*.h" -o -name "*.cc"`

for file in $files; do
  echo $file
  sed -i 's;char\* ;char \*;g' $file
  sed -i 's;CHAR\* ;CHAR \*;g' $file
  sed -i 's;string\* ;string \*;g' $file
  sed -i 's;string\& ;string \&;g' $file
  sed -i 's;string_view\& ;string_view \&;g' $file
  sed -i 's;T\* ;T \*;g' $file
  sed -i 's;T\& ;T \&;g' $file
  sed -i 's;string_view \& ;string_view \&;g' $file
  sed -i 's;T \* ;T \*;g' $file
  sed -i 's;T \& ;T \&;g' $file
done
