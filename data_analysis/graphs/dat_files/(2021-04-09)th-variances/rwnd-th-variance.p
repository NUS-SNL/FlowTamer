set ylabel "Standard Deviation of Sender Throughput [Mbps]" offset 1.5,0,0
set xlabel "Receiver's Receive Window [KB]" offset 0,0,0
set yrange[0:13]
set ytics 1 scale 0.5
set xtics 50 scale 0.5
set border lw 0.3
set key box lw 0.5 reverse samplen 1 width 0.2 height 0.3 Left horizontal maxcolumns 2
plot 'bbr_avg_th_stddev.dat' using 1:2 with linespoints pt 7 ps 0.7 title 'BBR', \
'reno_avg_th_stddev.dat' using 1:2 with linespoints pt 7 ps 0.7 title 'Reno', \
'htcp_avg_th_stddev.dat' using 1:2 with linespoints pt 7 ps 0.7 title 'HTCP', \
'yeah_avg_th_stddev.dat' using 1:2 with linespoints lc rgb 'red' pt 7 ps 0.7 title 'YeAH', \
'cubic_avg_th_stddev.dat' using 1:2 with linespoints pt 7 ps 0.7 title 'CUBIC'


set title "RTT 10ms | Link Capacity 100Mbps | Approx. Buffer Size 16MB"
set terminal postscript color portrait dashed enhanced "Times-Roman, 9" size 6,4
set output 'rwnd-th-stddev.eps'
set grid lc rgb "#abafae"
replot