#!/bin/bash --noprofile
source bash_common.sh
if [ $# -ne 1 ]; then
  echo "Record genogroup outliers in a database"
  echo "#1: file genogroup_table"
  exit 1
fi
IN=$1


error "$0 is not implemented"
