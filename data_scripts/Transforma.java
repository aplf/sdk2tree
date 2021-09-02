import java.io.*;
import it.unimi.dsi.webgraph.*;
import it.unimi.dsi.fastutil.ints.*;

/**
 * Transforms WebGraph compressed datasets into the (.adj) format expected by the k2-tree implementation of:
 * 
 *  Brisaboa, Nieves R., Susana Ladra, and Gonzalo Navarro.
 *  "Compact representation of web graphs with extended functionality."
 *  Information Systems 39 (2014): 152-174."
 */
class Transforma {

  public static void main(String [] args) throws Exception {
    if(args.length!=1) {
      System.out.println("Uso: java Transforma <basename>");
      return;
    }
    BVGraph gr = BVGraph.load(args[0]);
    RandomAccessFile out = new RandomAccessFile(args[0]+".adj","rw");
    out.writeInt(Integer.reverseBytes(gr.numNodes()));
    out.writeLong(Long.reverseBytes(gr.numArcs()));
    for(int i=0;i<gr.numNodes();i++) {
      int[] array = gr.successorArray(i);
      out.writeInt(Integer.reverseBytes(-(i+1)));
      for(int j=0;j<gr.outdegree(i);j++) {
        out.writeInt(Integer.reverseBytes(1+array[j]));
      }
      if(i%1000000==0) System.out.println("Nodes "+i);
    }
    out.close();
  }
}
