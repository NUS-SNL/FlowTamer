set terminal pdf 

set output filename

set xrange [0:2]

unset xlabel

# For stacked multiplot
set tmargin 0
set bmargin 0
set lmargin 1
set rmargin 1

set multiplot layout 2,1 margins 0.12,0.95,.15,0.92 spacing 0,0 title plottitle

# First Plot
#set yrange [0:4000]
set ylabel 'KB' offset 1,0
set arrow from 0,250 to 2,250 nohead
set arrow from 0,75 to 2,75 nohead
set key top right

unset xtics

plot 'flowrwnd2dat' using ($1-mints):($2 / 1000) with lines title flowrwnd2title lt rgb "brown", \
     'algorwnddat' using ($1-mints):($2 * 2/ 1000) with lines title algorwndtitle lt rgb "brown" dashtype 2, \
     flowbif2dat using ($1-mints):($2 / 1000) with lines title flowbif2title lt rgb "red", \
     flowbif1dat using ($1-mints):($2 / 1000) with lines title flowbif1title lt rgb "blue", \
     'qdepthdat' using ($1-mints):($2 / 1000) with lines title qdepthtitle lt rgb "blue"

set xtics 0.2
set mxtics 20
set xlabel 'time (sec)' offset 0,0.5 # common x-label

# Second (and last) Plot
unset arrow
set ytics 0,20,100
set yrange [0:125]
set ylabel "Mbps" offset 1
set key top right

plot flowth1dat using ($1-mints):2 with lines title flowth1title lt rgb "red", \
    flowth2dat using ($1-mints):2 with lines title flowth2title lt rgb "blue", \
