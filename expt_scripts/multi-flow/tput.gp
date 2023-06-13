set terminal pdf 

set output filename

set xrange [0:2]

unset xlabel

# For stacked multiplot
set tmargin 0
set bmargin 0
set lmargin 1
set rmargin 1

set multiplot layout 1,1 margins 0.12,0.95,.15,0.92 spacing 0,0 title plottitle

# Third (and last) Plot
set xtics 0.2
set mxtics 20
set xlabel 'time (sec)' offset 0,0.5 # common x-label
unset arrow
set ytics 0,20,100
set yrange [0:125]
set ylabel "Mbps" offset 1
set key top right

plot flowth1dat using ($1-mints):2 with lines title flowth1title lt rgb "red", \
    flowth2dat using ($1-mints):2 with lines title flowth2title lt rgb "blue", \


