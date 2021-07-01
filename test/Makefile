all: output/data0.tbon output/data1.tbon output/data2.tbon output/data3.tbon

output/data0.tbon: obj/json2tbon.o data/json/data0.json
	@mkdir -p output
	./obj/json2tbon.o data/json/data0.json output/data0.tbon

output/data1.tbon: obj/json2tbon.o data/json/data1.json
	@mkdir -p output
	./obj/json2tbon.o data/json/data1.json output/data1.tbon

output/data2.tbon: obj/json2tbon.o data/json/data2.json
	@mkdir -p output
	./obj/json2tbon.o data/json/data2.json output/data2.tbon

output/data3.tbon: obj/json2tbon.o data/json/data3.json
	@mkdir -p output
	./obj/json2tbon.o data/json/data3.json output/data3.tbon

output/data4.tbon: obj/json2tbon.o data/json/data4.json
	@mkdir -p output
	./obj/json2tbon.o data/json/data4.json output/data4.tbon

obj/json2tbon.o: src/json2tbonv2.c
	mkdir -p obj
	cc -I../build ../build/libcton.o src/json2tbonv2.c -o obj/json2tbon.o