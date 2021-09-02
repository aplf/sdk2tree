import java.io.*;
import it.unimi.dsi.webgraph.*;
import it.unimi.dsi.fastutil.ints.*;


/**
 * Transforms WebGraph compressed datasets into the (.tsv) format expected by the k2-tree implementation of this project
 * which is located in directory src/
 * 
 */
class Transforma2 {

  public static void main(String [] args) throws Exception {
    if(args.length!=1) {
      System.out.println("Uso: java Transforma2 <basename>");
      return;
    }
    BVGraph gr = BVGraph.load(args[0]);
    for(int i=0;i<gr.numNodes();i++) {
      int[] array = gr.successorArray(i);
      for(int j=0;j<gr.outdegree(i);j++) {
        System.out.println("a " + i + " " + array[j]);
      }
    }
  }
}
