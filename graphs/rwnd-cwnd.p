set ylabel "Sender CWND [KB]" offset 2,0,0
set xlabel "Receiver's Receive Window [KB]" offset 0,0,0
set yrange[0:1300]
set ytics 100 scale 0.5
set xtics 50 scale 0.5
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'summary_cubic_10ms_100Mbps.dat' using 1:3 with lines title 'cubic', \
'summary_reno_10ms_100Mbps.dat' using 1:3 with lines title 'reno', \
'summary_bbr_10ms_100Mbps.dat' using 1:3 with lines title 'bbr', \
'summary_htcp_10ms_100Mbps.dat' using 1:3 with lines title 'htcp', \
'summary_yeah_10ms_100Mbps.dat' using 1:3 with lines title 'yeah' lt rgb 'red'

set title "RTT 10ms | Link Capacity 100Mbps | Approx. Buffer Size 16MB"
set terminal postscript color portrait dashed enhanced "Times-Roman, 9" size 6,4
set output 'rwnd-cwnd.eps'
set grid lc rgb "#abafae"
replot