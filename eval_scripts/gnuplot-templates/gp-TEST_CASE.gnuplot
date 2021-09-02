#!/usr/bin/env gnuplot

#################################
################################# Dataset: DMGEN
#################################

set terminal pdfcairo mono font "sans, 16"
set output 'all-impls-TEST_CASE-STAT-dmgen.pdf'
set title  'TEST_NAME (DM)'

set key left top
set grid
set xlabel 'log_k(n) log(m)'
set ylabel 'Y_UNIT'
set format '%g'
set style data linespoints
#set xrange [150:]

#set xrange [210:350]

#set xrange [X_MIN_DMGEN:X_MAX_DMGEN]

##### Black and white version.

#set DMGEN-xtics 0,INCR,MAX
#set xtics 0,1.5e12,8e12

plot \
  'all-impls-TEST_CASE-STAT-dmgen.tsv' using (log($2)/log(2)*log($3)):($4*MULTIPLIER) pt 4 title 'dk2tree', \
  'all-impls-TEST_CASE-STAT-dmgen.tsv' using (log($2)/log(2)*log($3)):($5*MULTIPLIER) pt 8 title 'sdk2sdsl', \
  'all-impls-TEST_CASE-STAT-dmgen.tsv' using (log($2)/log(2)*log($3)):($6*MULTIPLIER) pt 10 title 'sdk2tree', \
  'all-impls-TEST_CASE-STAT-dmgen.tsv' using (log($2)/log(2)*log($3)):($7*MULTIPLIER) pt 16 title 'k2trie1', \
  'all-impls-TEST_CASE-STAT-dmgen.tsv' using (log($2)/log(2)*log($3)):($8*MULTIPLIER) pt 12 title 'k2trie2', \
  'all-impls-TEST_CASE-STAT-dmgen.tsv' using (log($2)/log(2)*log($3)):($9*MULTIPLIER) dt '_' pt 6 title 'k2tree'

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
set output 'all-impls-TEST_CASE-STAT-dmgen_colorized.pdf'
replot

reset

#################################
################################# Dataset: WEBGRAPH
#################################

set key left top
set grid
set xlabel 'log_k(n) log(m)'
set ylabel 'Y_UNIT'
set format '%g'
set style data linespoints

set terminal pdfcairo mono font "sans, 16"
set output 'all-impls-TEST_CASE-STAT-webgraph.pdf'
set title  'TEST_NAME (REAL)'


#set WEBGRAPH-xtics 0,INCR,MAX
#set xrange [X_MIN_WEBGRAPH:X_MAX_WEBGRAPH]
#set xrange [240:475]




##### Black and white version.

plot \
  'all-impls-TEST_CASE-STAT-webgraph.tsv' using (log($2)/log(2)*log($3)):($4*MULTIPLIER) pt 4 title 'dk2tree', \
  'all-impls-TEST_CASE-STAT-webgraph.tsv' using (log($2)/log(2)*log($3)):($5*MULTIPLIER) pt 8 title 'sdk2sdsl', \
  'all-impls-TEST_CASE-STAT-webgraph.tsv' using (log($2)/log(2)*log($3)):($6*MULTIPLIER) pt 10 title 'sdk2tree', \
  'all-impls-TEST_CASE-STAT-webgraph.tsv' using (log($2)/log(2)*log($3)):($7*MULTIPLIER) pt 16 title 'k2trie1', \
  'all-impls-TEST_CASE-STAT-webgraph.tsv' using (log($2)/log(2)*log($3)):($8*MULTIPLIER) pt 12 title 'k2trie2', \
  'all-impls-TEST_CASE-STAT-webgraph.tsv' using (log($2)/log(2)*log($3)):($9*MULTIPLIER) dt '_' pt 6 title 'k2tree'

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
set output 'all-impls-TEST_CASE-STAT-webgraph_colorized.pdf'
replot

quit

