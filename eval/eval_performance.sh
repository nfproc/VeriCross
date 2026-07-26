#/bin/bash
# VeriCross Performance Evaluation for all benchmarks and cores
OUTFILE=eval/performance_logs.txt
SUMMARY=eval/performance_summary.txt

if [ ! -x ./vericross ]; then
  echo "!! please build the VeriCross binary first"
  echo "!! and make sure working directory is top of the VeriCross repository"
  exit 1
fi

if [ -e $OUTFILE ]; then
  rm -f ${OUTFILE}.old
  mv $OUTFILE ${OUTFILE}.old
fi

for CORE in kronos rvcorep
do
  echo $CORE
  ./vericross -c$CORE -i3g benchmark/binary/basicmath basicmath_small basicmath >> $OUTFILE
  ./vericross -c$CORE -i40m benchmark/binary/bitcount bitcnts bitcount >> $OUTFILE
  ./vericross -c$CORE -i700m benchmark/binary/CRC32 crc CRC32 >> $OUTFILE
  ./vericross -c$CORE -i40m benchmark/binary/dijkstra dijkstra_small dijkstra >> $OUTFILE
  ./vericross -c$CORE -i70m benchmark/binary/jpeg cjpeg jpeg_cjpeg >> $OUTFILE
  ./vericross -c$CORE -i30m benchmark/binary/jpeg djpeg jpeg_djpeg >> $OUTFILE
  ./vericross -c$CORE -i20g benchmark/binary/lame lame lame >> $OUTFILE
  ./vericross -c$CORE -i200m benchmark/binary/patricia patricia patricia >> $OUTFILE
  ./vericross -c$CORE -i20m benchmark/binary/qsort qsort_small qsort >> $OUTFILE
  ./vericross -c$CORE -i20m benchmark/binary/sha sha sha >> $OUTFILE
  ./vericross -c$CORE -i1m benchmark/binary/stringsearch search_small stringsearch >> $OUTFILE
  ./vericross -c$CORE -i10m benchmark/binary/susan susan susan_corners >> $OUTFILE
  ./vericross -c$CORE -i10m benchmark/binary/susan susan susan_edges >> $OUTFILE
  ./vericross -c$CORE -i200m benchmark/binary/susan susan susan_smooth >> $OUTFILE
done

grep '^##' $OUTFILE > $SUMMARY