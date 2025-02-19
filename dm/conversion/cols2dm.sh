#!/bin/bash --noprofile
THIS=$( dirname $0 )
source $THIS/../../bash_common.sh
if [ $# -ne 4 ]; then
  echo "Print a .dm-file"
  echo "#1: tab-delimited file with numeric columns (exept a 'name' column)"
  echo "#2: #1 is a tsv-file (0/1). For tsv-files spaces in column names are replaced by '_'"
  echo "#3: number precision"
  echo "#4: name column number (>= 1; 0 - no 'name' column)"
  exit 1
fi
F=$1
TSV=$2
PREC=$3
NAME_COL=$4


N=$( < $F wc -l )
if [ $N == 0 ]; then
  error "No data"
fi


TMP=$( mktemp )


head -1 $F | sed 's/^#//1' | tr '\t' '\n' > $TMP.header
mapfile -t H < $TMP.header

NAME="noname"
if [ $NAME_COL -ne 0 ]; then
  NAME="name"
fi

echo "ObjNum $N $NAME nomult" 
echo "Attributes"
i=1
while [ $i -le ${#H[@]} ]; do
  if [ $i -ne $NAME_COL ]; then
    V=V$i
    if [ $TSV == 1 ]; then
      j=$(( $i - 1 ))
      V=$( echo ${H[$j]} | sed 's/ /_/g' )
    fi
    echo "  $V real $PREC"
  fi
  i=$(( $i + 1 ))
done

echo "Data"
if [ $TSV == 1 ]; then
  if [ $NAME_COL == 0 ]; then
    tail -n +2 $F | sed 's/^\t/0\t/1' | sed 's/\t$/\t0/1' | sed 's/\t\t/\t0\t/g'
  else
    tail -n +2 $F | cut -f $NAME_COL > $TMP.1
    tail -n +2 $F | cut -f $NAME_COL --complement | sed 's/^\t/0\t/1' | sed 's/\t$/\t0/1' | sed 's/\t\t/\t0\t/g' > $TMP.2
    paste $TMP.1 $TMP.2
  fi
else
  cat $F
fi


rm $TMP*
