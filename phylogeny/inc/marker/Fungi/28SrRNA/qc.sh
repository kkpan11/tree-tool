#!/bin/bash
source bash_common.sh
if [ $# -ne 1 ]; then
  echo "Quality control of distTree_inc_new.sh"
  echo "#1: go"
  echo "Requires: Time: O(n log^3(n))"
  exit 1
fi


INC=`dirname $0`


TMP=`mktemp`
#echo $TMP


grep '^>' $INC/seq.fa | sed 's/^>//1' | sed 's/ .*$//1' | sort > $TMP.seq-fa

sort -u $TMP.seq-fa > $TMP.seq-fa-uniq
diff $TMP.seq-fa $TMP.seq-fa-uniq

tree2obj.sh $INC/tree > $TMP.tree
diff $TMP.seq-fa $TMP.tree

sqsh-ms -S PROTEUS  -U anyone  -P allowed  -D uniColl << EOT | sed 's/|$//1' > $TMP.locus
  select id
    from Locus
    where     taxroot = 4751
          and gene = '28S'
          and in_tree = 1;
  go -m bcp  
EOT
diff $TMP.seq-fa $TMP.locus

ls $INC/new/ > $TMP.new
sqsh-ms -S PROTEUS  -D uniColl  -U anyone  -P allowed  << EOT | sed 's/|$//1' | sort > $TMP.locus-new
  select id
    from Locus
    where     taxroot = 4751
          and gene = '28S'
          and dead = 0
          and outlier is null
          and in_tree is null;
  go -m bcp  
EOT
#wc -l $TMP.genome-new 
#wc -l $TMP.new
diff $TMP.locus-new $TMP.new

#ls $INC/../seq/ > $TMP.seq
#diff $TMP.seq-fa $TMP.seq

echo ">aa" > $TMP.seq
echo "tttttttttttttttttttttttttt" >> $TMP.seq
blastn -query $TMP.seq  -db $INC/seq.fa | grep "Number of sequences in database:" | sed 's/,//1' | sed 's/^ *//1' > $TMP.blastn
N=`cat $TMP.seq-fa | wc -l`
M=(`cat $TMP.blastn`)
if [ $N -ne ${M[5]} ]; then
  echo "$N != ${M[5]}"
  exit 1
fi


rm $TMP*