#/bin/bash
# VeriCross Performance Evaluation for all benchmarks (simulation only)
OUTFILE=eval/performance_sim_logs.txt
SUMMARY=eval/performance_sim_summary.txt

if [ ! -x ./vericross ]; then
  echo "!! please build the VeriCross binary first"
  echo "!! and make sure working directory is top of the VeriCross repository"
  exit 1
fi

if [ -e $OUTFILE ]; then
  rm -f ${OUTFILE}.old
  mv $OUTFILE ${OUTFILE}.old
fi

./vericross -i3g benchmark/binary/basicmath basicmath_small basicmath >> $OUTFILE
./vericross -i40m benchmark/binary/bitcount bitcnts bitcount >> $OUTFILE
./vericross -i700m benchmark/binary/CRC32 crc CRC32 >> $OUTFILE
./vericross -i40m benchmark/binary/dijkstra dijkstra_small dijkstra >> $OUTFILE
./vericross -i70m benchmark/binary/jpeg cjpeg jpeg_cjpeg >> $OUTFILE
./vericross -i30m benchmark/binary/jpeg djpeg jpeg_djpeg >> $OUTFILE
./vericross -i20g benchmark/binary/lame lame lame >> $OUTFILE
./vericross -i200m benchmark/binary/patricia patricia patricia >> $OUTFILE
./vericross -i20m benchmark/binary/qsort qsort_small qsort >> $OUTFILE
./vericross -i20m benchmark/binary/sha sha sha >> $OUTFILE
./vericross -i1m benchmark/binary/stringsearch search_small stringsearch >> $OUTFILE
./vericross -i10m benchmark/binary/susan susan susan_corners >> $OUTFILE
./vericross -i10m benchmark/binary/susan susan susan_edges >> $OUTFILE
./vericross -i200m benchmark/binary/susan susan susan_smooth >> $OUTFILE

grep '^##' $OUTFILE > $SUMMARY