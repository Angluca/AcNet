all: 
	make  -C./build || exit 1
create:
	cd build && cmake .. && cd .. || exit 1
clean:
	make clean -C./build  || exit 1
.PHONY: clean cleanall create
