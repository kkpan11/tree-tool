#!/bin/bash --noprofile
THIS=$( dirname $0 )
source $THIS/bash_common.sh
if [ $# -ne 1 ]; then
  echo "Try to fix symbolic links/executable permissions. Idempotent"
  echo "#1: file"
  exit 1
fi
F="$1"


if file $F | grep -q ": ASCII text"; then
  W=$( < $F wc -w )
  C=$( < $F wc -c )
  if [ $W != 1 ]; then
    exit 0
  fi
  if [ $C -gt 1000 ]; then   # PAR
    exit 0
  fi
  
  LINK=$( cat $F )
  D=$( realpath $( dirname $F ) )
  NEW_F=$D/$LINK
  
  $THIS/check_file.sh $NEW_F 1
  
  rm $F
  set -x
  ln -s $NEW_F $F
elif file $F | grep -q ": ELF 64-bit LSB executable"; then
  set -x
  chmod a+x $F
elif file $F | grep -q ": directory"; then
  $THIS/trav $F "$THIS/fix_link.sh %Q%d/%f%Q"
fi
