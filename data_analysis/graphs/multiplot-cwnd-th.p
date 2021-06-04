set terminal postscript color portrait dashed enhanced "Times-Roman, 9" size 6,6
set output 'rwnd-cwnd-th.eps'
set grid lc rgb "#abafae"

set multiplot layout 2,1 title "     RTT 10ms | Link Capacity 100Mbps | Approx. Buffer Size 16MB"

set ylabel "Average Sender Throughput [Mbps]" offset 2,0,0
set xlabel "Receiver's Receive Window [KB]" offset 0,0.4,0
set yrange[0:130]
set ytics 10 scale 0.5
set xtics 50 scale 0.5
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'summary_bbr_10ms_100Mbps.dat' using 1:2 with lines title 'BBR' lt rgb 'blue', \
'summary_reno_10ms_100Mbps.dat' using 1:2 with lines title 'Reno', \
'summary_htcp_10ms_100Mbps.dat' using 1:2 with lines title 'HTCP', \
'summary_yeah_10ms_100Mbps.dat' using 1:2 with lines title 'YeAH' lt rgb 'red',\
'summary_cubic_10ms_100Mbps.dat' using 1:2 with lines title 'CUBIC'

set ylabel "Average Sender CWND [KB]" offset 2.5,0,0
set xlabel "Receiver's Receive Window [KB]" offset 0,0.4,0
set yrange[0:1300]
set ytics 100 scale 0.5
set xtics 50 scale 0.5
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'summary_bbr_10ms_100Mbps.dat' using 1:3 with lines title 'BBR' lt rgb 'blue', \
'summary_reno_10ms_100Mbps.dat' using 1:3 with lines title 'Reno', \
'summary_htcp_10ms_100Mbps.dat' using 1:3 with lines title 'HTCP', \
'summary_yeah_10ms_100Mbps.dat' using 1:3 with lines title 'YeAH' lt rgb 'red',\
'summary_cubic_10ms_100Mbps.dat' using 1:3 with lines title 'CUBIC'


unset multiplot