#!/bin/sh

dir=$1
#print error message, red words
#@param string message
#@return void
function error_message() {
  local message=${@}
  echo -e "\e[0;31;1merror: ${message}\e[0m"
  exit 1
}

#print warning message, yellow words
#@param message
#@return string void
function warning_message() {
  local message=${@}
  echo -e "\e[0;33;1mwarning: ${message}\e[0m"
}

[[ "" == $dir ]] && error_message "please set save dir"

files=`git status -s | awk '{print $2}' | grep -v "cscope.*" | grep -v "tags"`

for file in $files; do
  path=`dirname $file`
  mkdir -p $dir/$path
  echo $file
  cp $file $dir/$path
done
