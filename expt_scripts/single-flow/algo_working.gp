set terminal pdf 

set output 'cubic_020ms_250KB.pdf'

set xrange [0:2]

unset xlabel

# For stacked multiplot
set tmargin 0
set bmargin 0
set lmargin 1
set rmargin 1

set multiplot layout 2,1 margins 0.12,0.95,.15,0.92 spacing 0,0 title "CUBIC 20ms - 250KB"

# First Plot
set yrange [0:4000]
set ylabel 'KB' offset 1,0
set arrow from 0,250 to 2,250 nohead
set arrow from 0,75 to 2,75 nohead
set key top right

unset xtics

plot 'flow_rwnd_33676.dat' using ($1-1686211694.925996):($2 / 1000) with lines title "ReceiverRwnd" lt rgb "brown", \
     'algo_rwnd.dat' using ($1-1686211694.925996):($2 * 2/ 1000) with lines title "AlgoRwnd" lt rgb "brown" dashtype 2, \
     'flow_bif_33676.dat' using ($1-1686211694.925996):($2 / 1000) with lines title "flowPiF" lt rgb "red", \
     'qdepth.dat' using ($1-1686211694.925996):($2 / 1000) with lines title "qdepth" lt rgb "blue"

set xtics 0.2
set mxtics 20
set xlabel 'time (sec)' offset 0,0.5 # common x-label

# Second (and last) Plot
unset arrow
set ytics 0,20,100
set yrange [0:125]
set ylabel "Mbps" offset 1
set key top right

plot 'flow_th_33676.dat' using ($1-1686211694.925996):2 with lines title "Throughput" lt rgb "black"




