#!/bin/bash --noprofile
source CPP_DIR/bash_common.sh
if [ $# -ne 2 ]; then
  echo "Set GenomeTaxroot.in_tree"
  echo "#1: List of Genome.id"
  echo "#2: GenomeTaxroot.in_tree (1/null)"
  exit 1
fi
OBJ_LIST=$1
IN_TREE=$2


INC=$( dirname $0 )
SERVER=$( cat $INC/server )
DATABASE=$( cat $INC/database )
BULK_REMOTE=$( cat $INC/bulk_remote )
TAXROOT=$( cat $INC/../taxroot )

CPP_DIR/bulk.sh $SERVER $INC/bulk $BULK_REMOTE $OBJ_LIST $DATABASE..List

sqsh-ms  -S $SERVER  -D $DATABASE << EOT 
  update GenomeTaxroot
    set in_tree = $IN_TREE
    from      List
         join GenomeTaxroot on     GenomeTaxroot.genome = List.id
                               and GenomeTaxroot.taxroot = $TAXROOT
  print @@rowcount
  go -m bcp
EOT

