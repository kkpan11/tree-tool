#!/bin/bash
source CPP_DIR/bash_common.sh
if [ $# -ne 2 ]; then
  echo "Set Genome.in_tree"
  echo "#1: List of uniColl..Genome.id"
  echo "#2: Genome.in_tree (1/null)"
  exit 1
fi
OBJ_LIST=$1
IN_TREE=$2


INC=`dirname $0`
SERVER=`cat $INC/server`
DATABASE=`cat $INC/database`
BULK_REMOTE=`cat $INC/bulk_remote`

CPP_DIR/database/bulk.sh $SERVER $INC/bulk $BULK_REMOTE $OBJ_LIST $DATABASE..LIST

sqsh-ms  -S $SERVER  -D $DATABASE << EOT 
  update Genome
    set in_tree = $IN_TREE
    from      LIST
         join Genome on Genome.id = LIST.id;
  print @@rowcount;
  go -m bcp
EOT
