import java.io.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

import it.unimi.dsi.webgraph.*;
import it.unimi.dsi.fastutil.ints.*;

/**
 * Transforms WebGraph compressed datasets into the format expected by the
 * k2-tree implementation of this project which is located in directory src/
 * 
 */
class BinarizeAdjacencyTSV {

  public static void main(String[] args) throws Exception {
    if (args.length != 1) {
      System.out.println("Uso: java BinarizeAdjacencyTSV <basename>");
      return;
    }

    // TODO: 1 convert from regular 'a 5 1' commands to .
    final FileInputStream fstream = new FileInputStream(args[0]);
    final BufferedReader br = new BufferedReader(new InputStreamReader(fstream));

    String strLine;

    // Read File Line By Line
    final Map<Integer, ArrayList<Integer>> adj = new TreeMap<Integer, ArrayList<Integer>>();

    long edgeCtr = 0;
    while ((strLine = br.readLine()) != null) {

      if (strLine.startsWith("a")) {

        edgeCtr = edgeCtr + 1;

        final String[] tokens = strLine.split("\\s+");
        final int src = Integer.parseInt(tokens[1]);
        final int dst = Integer.parseInt(tokens[2]);

        if (!adj.containsKey(src)) {
          final ArrayList<Integer> l = new ArrayList<>();
          l.add(dst);
          adj.put(src, l);
        } else {
          adj.get(src).add(dst);
        }
      }
    }

    // Close the input stream
    fstream.close();

    // BVGraph gr = BVGraph.load(args[0]);
    final int termIndex = args[0].lastIndexOf(".");
    final RandomAccessFile out = new RandomAccessFile(args[0].substring(0, termIndex > -1 ? termIndex : args[0].length()) + ".adj", "rw");

    

    out.writeInt(Integer.reverseBytes(adj.size()));
    out.writeLong(Long.reverseBytes(edgeCtr));

//    adj.keySet().s

    for (Integer i : adj.keySet()) {

      out.writeInt(Integer.reverseBytes(-(i+1)));

      final ArrayList<Integer> neighbors = adj.get(i);
      Collections.sort(neighbors);
      for(int n : neighbors) {
        out.writeInt(Integer.reverseBytes(1+n));
      }

      if(i%1000000==0) System.out.println("Nodes "+i);
    }

    //out.writeInt(Integer.reverseBytes(gr.numNodes()));
    //out.writeLong(Long.reverseBytes(gr.numArcs()));
    /*
    for(int i=0;i<gr.numNodes();i++) {
      int[] array = gr.successorArray(i);
      out.writeInt(Integer.reverseBytes(-(i+1)));
      for(int j=0;j<gr.outdegree(i);j++) {
        out.writeInt(Integer.reverseBytes(1+array[j]));
      }
      if(i%1000000==0) System.out.println("Nodes "+i);
    }

    */

    out.close();
  }
}
