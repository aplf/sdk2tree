all: dmgen java k2tree

dmgen:
	cd dmgenerator && make clean all

java:
	./util.sh | xargs -I{} javac -cp {} Transforma.java Transforma2.java BinarizeAdjacencyTSV.java

k2tree:
	cd ../implementations/k2tree && make all

clean:
	rm -f *.class
	cd ../implementations/k2tree && make clean
	cd dmgenerator && make clean