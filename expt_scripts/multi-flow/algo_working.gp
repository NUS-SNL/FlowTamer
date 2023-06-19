set terminal pdf 

set output filename

set xrange [0:2]

unset xlabel

# For stacked multiplot
set tmargin 0
set bmargin 0
set lmargin 1
set rmargin 1

set multiplot layout 3,1 margins 0.10,0.98,.10,0.985 spacing 0,0 # title plottitle

# First Plot
set yrange [0:1500]
set ylabel 'KB' offset 1.8,0
set key top right
unset xtics

plot flowrwnd1dat using ($1-mints):($2 / 1000) with lines title flowrwnd1title lt rgb "red", \
    flowrwnd2dat using ($1-mints):($2 / 1000) with lines title flowrwnd2title lt rgb "blue", \
    algorwnddat using ($1-mints):($2 / 1000) with lines title algorwndtitle lt rgb "black" dashtype 2 lw 2

# Second plot
set yrange [0:700]
set ylabel 'KB' offset 0.8,0
set arrow from 0,250 to 2,250 nohead
set arrow from 0,25 to 2,25 nohead
set key top right
unset xtics

plot qdepthdat using ($1-mints):($2 / 1000) with lines title qdepthtitle lt rgb "orchid4"


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


