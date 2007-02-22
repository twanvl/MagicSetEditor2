#run Doxygen

doxygen doxygen.cfg 1>&- 2>&-;

#fix it using the perl script

perl fix.pl 1>&- 2>&-;

#filter docwarnings.log and make a file called warnings.log to only contain warnings

grep "Warning" <docwarnings.log >warnings.log;
