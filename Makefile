SRC_DIR=src

all: cton.o

cton.o: cton.c cton.h
	cc -c -fPIC -Wall -Wextra cton.c -o cton.o

cton.h: $(SRC_DIR)/cton.h
	@cp $(SRC_DIR)/cton.h ./

cton.c: $(SRC_DIR)/cton_alloc.c \
		$(SRC_DIR)/cton_array.c \
		$(SRC_DIR)/cton_common.c \
		$(SRC_DIR)/cton_declear.c \
		$(SRC_DIR)/cton_hash.c \
		$(SRC_DIR)/cton_numeric.c \
		$(SRC_DIR)/cton_object.c \
		$(SRC_DIR)/cton_string.c \
		$(SRC_DIR)/cton_util.c \
		utils/cton_json.c
	@cat $(SRC_DIR)/cton_declear.c > cton.c
	@cat $(SRC_DIR)/cton_common.c >> cton.c
	@cat $(SRC_DIR)/cton_alloc.c >> cton.c
	@cat $(SRC_DIR)/cton_util.c >> cton.c
	@cat $(SRC_DIR)/cton_array.c >> cton.c
	@cat $(SRC_DIR)/cton_hash.c >> cton.c
	@cat $(SRC_DIR)/cton_numeric.c >> cton.c
	@cat $(SRC_DIR)/cton_object.c >> cton.c
	@cat $(SRC_DIR)/cton_string.c >> cton.c
	@cat utils/cton_json.c >> cton.c

.PHONY: clean
clean:
	rm -f cton.c cton.h cton.o cton_json.o