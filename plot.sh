#!/usr/bin/bash -x

PLOTSCRIPT=script.plot
DATA=data.txt
INPUTDIR=results

min () {
  head -n 1 $1
}

q1 () {
  head -n 24 $1 | tail -n 1
}

median () {
  head -n 50 $1 | tail -n 1
}

q3 () {
  head -n 75 $1 | tail -n 1
}

max () {
  tail -n 1 $1
}

buf_size () {
  echo $1 | cut -d "_" -f 4
}

x () {
  buf_size $1 | tr -cd '[[:digit:]]'
}

write_data () {
  FILE=$INPUTDIR/$1
  printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\n" $(x $FILE) $(min $FILE) $(q1 $FILE) \
    $(median $FILE) $(q3 $FILE) $(max $FILE) $(buf_size $FILE) >> $DATA
}

printf "# x\tmin\tq1\tmedian\tq3\tmax\tlabel\n" > $DATA
for file in $(ls $INPUTDIR)
do
  write_data $file
done

printf "set terminal png\n" > $PLOTSCRIPT
printf "set output 'plot.png'\n" >> $PLOTSCRIPT
printf "set title 'Read 1Go file from Unikraft+9pfs'\n" >> $PLOTSCRIPT
printf "set errorbars 10.0\n" >> $PLOTSCRIPT
printf "set style fill empty\n" >> $PLOTSCRIPT
printf "set ylabel 'Time in milliseconds'\n" >> $PLOTSCRIPT
printf "set xrange [0:11]\n" >> $PLOTSCRIPT
printf "plot '$DATA' using 1:3:2:6:5:xticlabels(7) with candlesticks title 'Quartiles' whiskerbars, \\" >> $PLOTSCRIPT
printf "\n" >> $PLOTSCRIPT
printf "     ''      using 1:4:4:4:4 with candlesticks lt -1 lw 2 notitle\n" >> $PLOTSCRIPT
# printf "plot '$DATA' using 1:3:2:6:5 with candlesticks whiskerbars 0.5\n" >> $PLOTSCRIPT

gnuplot $PLOTSCRIPT
rm $PLOTSCRIPT
