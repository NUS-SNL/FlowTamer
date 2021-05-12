set terminal postscript color portrait dashed enhanced "Times-Roman, 9" size 10,20
set output 'cc-cwnd-time.eps'
set grid lc rgb "#abafae"

set multiplot layout 5,1 title "RTT 10ms | Link Capacity 100Mbps | Approx. Buffer Size 16MB"

set ylabel "Sender CWND [KB]" offset 2,0,0
set xlabel "Time [seconds]" offset 0,0,0
set yrange[0:7500]
set xrange[0:10]
set ytics 500 scale 0.5
set xtics 0.25 scale 0.5
set title "CUBIC"
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'cubic_cwnd_t_baseline.dat' using 1:2 with lines title 'baseline' lt rgb 'red', \
'cubic_cwnd_t_1.dat' using 1:2 with lines title '0.128 KB RWND' lt rgb 'blue', \
'cubic_cwnd_t_1001.dat' using 1:2 with lines title '128.128 KB RWND' lt rgb '#009900', \
'cubic_cwnd_t_2001.dat' using 1:2 with lines title '256.128 KB RWND' lt rgb '#6E12CA', \
'cubic_cwnd_t_3001.dat' using 1:2 with lines title '384.128 KB RWND' lt rgb 'orange', \
'cubic_cwnd_t_4001.dat' using 1:2 with lines title '512.128 KB RWND' lt rgb "dark-orange"

set ylabel "Sender CWND [KB]" offset 2,0,0
set xlabel "Time [seconds]" offset 0,0,0
set yrange[0:7500]
set xrange[0:10]
set ytics 500 scale 0.5
set xtics 0.25 scale 0.5
set title "BBR"
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'bbr_cwnd_t_baseline.dat' using 1:2 with lines title 'baseline' lt rgb 'red', \
'bbr_cwnd_t_1.dat' using 1:2 with lines title '0.128 KB RWND' lt rgb 'blue', \
'bbr_cwnd_t_1001.dat' using 1:2 with lines title '128.128 KB RWND' lt rgb '#009900', \
'bbr_cwnd_t_2001.dat' using 1:2 with lines title '256.128 KB RWND' lt rgb '#6E12CA', \
'bbr_cwnd_t_3001.dat' using 1:2 with lines title '384.128 KB RWND' lt rgb 'orange', \
'bbr_cwnd_t_4001.dat' using 1:2 with lines title '512.128 KB RWND' lt rgb "dark-orange"

set ylabel "Sender CWND [KB]" offset 2,0,0
set xlabel "Time [seconds]" offset 0,0,0
set yrange[0:7500]
set xrange[0:10]
set ytics 500 scale 0.5
set xtics 0.25 scale 0.5
set title "Reno"
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'reno_cwnd_t_baseline.dat' using 1:2 with lines title 'baseline' lt rgb 'red', \
'reno_cwnd_t_1.dat' using 1:2 with lines title '0.128 KB RWND' lt rgb 'blue', \
'reno_cwnd_t_1001.dat' using 1:2 with lines title '128.128 KB RWND' lt rgb '#009900', \
'reno_cwnd_t_2001.dat' using 1:2 with lines title '256.128 KB RWND' lt rgb '#6E12CA', \
'reno_cwnd_t_3001.dat' using 1:2 with lines title '384.128 KB RWND' lt rgb 'orange', \
'reno_cwnd_t_4001.dat' using 1:2 with lines title '512.128 KB RWND' lt rgb "dark-orange"

set ylabel "Sender CWND [KB]" offset 2,0,0
set xlabel "Time [seconds]" offset 0,0,0
set yrange[0:7500]
set xrange[0:10]
set ytics 500 scale 0.5
set xtics 0.25 scale 0.5
set title "HTCP"
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'htcp_cwnd_t_baseline.dat' using 1:2 with lines title 'baseline' lt rgb 'red', \
'htcp_cwnd_t_1.dat' using 1:2 with lines title '0.128 KB RWND' lt rgb 'blue', \
'htcp_cwnd_t_1001.dat' using 1:2 with lines title '128.128 KB RWND' lt rgb '#009900', \
'htcp_cwnd_t_2001.dat' using 1:2 with lines title '256.128 KB RWND' lt rgb '#6E12CA', \
'htcp_cwnd_t_3001.dat' using 1:2 with lines title '384.128 KB RWND' lt rgb 'orange', \
'htcp_cwnd_t_4001.dat' using 1:2 with lines title '512.128 KB RWND' lt rgb "dark-orange"

set ylabel "Sender CWND [KB]" offset 2,0,0
set xlabel "Time [seconds]" offset 0,0,0
set yrange[0:7500]
set xrange[0:10]
set ytics 500 scale 0.5
set xtics 0.25 scale 0.5
set title "YeAH"
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'yeah_cwnd_t_baseline.dat' using 1:2 with lines title 'baseline' lt rgb 'red', \
'yeah_cwnd_t_1.dat' using 1:2 with lines title '0.128 KB RWND' lt rgb 'blue', \
'yeah_cwnd_t_1001.dat' using 1:2 with lines title '128.128 KB RWND' lt rgb '#009900', \
'yeah_cwnd_t_2001.dat' using 1:2 with lines title '256.128 KB RWND' lt rgb '#6E12CA', \
'yeah_cwnd_t_3001.dat' using 1:2 with lines title '384.128 KB RWND' lt rgb 'orange', \
'yeah_cwnd_t_4001.dat' using 1:2 with lines title '512.128 KB RWND' lt rgb "dark-orange"

unset multiplot