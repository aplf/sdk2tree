#!/usr/bin/env gnuplot

#################################
################################# Dataset: DMGEN
#################################
set terminal pdfcairo mono font "sans, 16"
set output 'sdk2sdsl-TEST_CASE-STAT-dmgen.pdf'
set title 'TEST_NAME (DM)'

set key left top
set grid
set xlabel 'X_UNIT'
set ylabel 'Y_UNIT'
set format '%g'
set style data linespoints
set xrange [150:]
#set xtics 0,INCR,MAX

#set DMGEN-xtics 0,INCR,MAX
##### Black and white version.


plot 'sdk2sdsl-TEST_CASE-STAT-dmgen.tsv' using (X_FORMULA):($4*MULTIPLIER) pt 4 title 'sdk2sdsl'

##### Colored version.

set style increment user
# Color pallete parula: http://www.gnuplotting.org/data/parula.pal
set style line 1 lt 1 lc rgb '#0072bd' # blue
set style line 2 lt 1 lc rgb '#d95319' # orange
set style line 3 lt 1 lc rgb '#edb120' # yellow
set style line 4 lt 1 lc rgb '#7e2f8e' # purple
set style line 5 lt 1 lc rgb '#77ac30' # green
set style line 6 lt 1 lc rgb '#4dbeee' # light-blue
set style line 7 lt 1 lc rgb '#a2142f' # red
set terminal pdfcairo color font "sans, 16"
set output 'sdk2sdsl-TEST_CASE-STAT-dmgen_colorized.pdf'
replot
 
#################################
################################# Dataset: WEBGRAPH
#################################

set key left top
set grid
set xlabel 'X_UNIT'
set ylabel 'Y_UNIT'
set format '%g'
set style data linespoints
set xrange [150:]
#set xtics 0,INCR,MAX

set terminal pdfcairo mono font "sans, 16"
set output 'sdk2sdsl-TEST_CASE-STAT-webgraph.pdf'
set title  'TEST_NAME (REAL)'

#set WEBGRAPH-xtics 0,INCR,MAX

##### Black and white version.

plot 'sdk2sdsl-TEST_CASE-STAT-webgraph.tsv' using (X_FORMULA):($4*MULTIPLIER) pt 4 title 'sdk2sdsl'

##### Colored version.

set style increment user
# Color pallete parula: http://www.gnuplotting.org/data/parula.pal
set style line 1 lt 1 lc rgb '#0072bd' # blue
set style line 2 lt 1 lc rgb '#d95319' # orange
set style line 3 lt 1 lc rgb '#edb120' # yellow
set style line 4 lt 1 lc rgb '#7e2f8e' # purple
set style line 5 lt 1 lc rgb '#77ac30' # green
set style line 6 lt 1 lc rgb '#4dbeee' # light-blue
set style line 7 lt 1 lc rgb '#a2142f' # red
set terminal pdfcairo color font "sans, 16"
set output 'sdk2sdsl-TEST_CASE-STAT-webgraph_colorized.pdf'
replot

quit

