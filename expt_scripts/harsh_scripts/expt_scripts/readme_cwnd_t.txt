# About: Steps for running experiments for getting cwnd values every 2-4 ms using sstool

1. check the src ip inside mm-delay shell and accordingly change the ss_probe.sh script
2. execute receive script
3. execute send script
4. manually remove the parts from cwnd-values.txt
5. execute process_raw_cwnd_data.py script
6. copy needed columns from cwnd-values.csv to a copy of template-cwnd-values.csv and it will take care of applying formulas
7. renamve .csv file well. create a new .dat file and copy 3 columns to it