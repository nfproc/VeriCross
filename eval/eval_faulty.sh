#/bin/bash
# VeriCross Error Detection Evaluation for all benchmarks
# (except stringsearch, susan_corners, and susan_edges because they take <10M cycles)
OUTFILE=eval/faulty_logs.txt
SUMMARY=eval/faulty_summary.txt

if [ ! -x ./vericross ]; then
  echo "!! please build the VeriCross binary first"
  echo "!! and make sure working directory is top of the VeriCross repository"
  exit 1
fi

if [ -e $OUTFILE ]; then
  rm -f ${OUTFILE}.old
  mv $OUTFILE ${OUTFILE}.old
fi

for i in {1..10}
do
  echo $i
  ./vericross -ckronos_faulty -i10m benchmark/binary/basicmath basicmath_small basicmath >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/bitcount bitcnts bitcount >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/CRC32 crc CRC32 >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/dijkstra dijkstra_small dijkstra >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/jpeg cjpeg jpeg_cjpeg >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/jpeg djpeg jpeg_djpeg >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/lame lame lame >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/patricia patricia patricia >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/qsort qsort_small qsort >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/sha sha sha >> $OUTFILE
  # ./vericross -ckronos_faulty -i10m benchmark/binary/stringsearch search_small stringsearch >> $OUTFILE
  # ./vericross -ckronos_faulty -i10m benchmark/binary/susan susan susan_corners >> $OUTFILE
  # ./vericross -ckronos_faulty -i10m benchmark/binary/susan susan susan_edges >> $OUTFILE
  ./vericross -ckronos_faulty -i10m benchmark/binary/susan susan susan_smooth >> $OUTFILE
done

grep '^##' $OUTFILE > $SUMMARY
