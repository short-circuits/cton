SRC_DIR=src
CC=cc
AR=ar
LD=ld
CFLAGS=-Wall -Wextra -fPIC -O2 -I$(SRC_DIR)
OBJ_DIR=obj

all: cton.o

cton_json.o: cton.o test/json/json_test.c
	$(CC) $(CFLAGS) -I./ cton.o test/json/json_test.c -o cton_json.o


$(OBJ_DIR)/cton_json.o: $(SRC_DIR)/cton_json.c \
						$(SRC_DIR)/cton_json.h \
						$(SRC_DIR)/cton.h
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(SRC_DIR)/cton_json.c -o $(OBJ_DIR)/cton_json.o


$(OBJ_DIR)/cton_bmp.o: $(SRC_DIR)/cton_bmp.c \
						$(SRC_DIR)/cton_bmp.h \
						$(SRC_DIR)/cton.h
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(SRC_DIR)/cton_bmp.c -o $(OBJ_DIR)/cton_bmp.o

$(OBJ_DIR)/cton.o: $(SRC_DIR)/cton.c $(SRC_DIR)/cton.h
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(SRC_DIR)/cton.c -o $(OBJ_DIR)/cton.o

cton.o: $(OBJ_DIR)/cton.o $(OBJ_DIR)/cton_json.o $(OBJ_DIR)/cton_bmp.o
	$(LD) -r \
	$(OBJ_DIR)/cton_json.o \
	$(OBJ_DIR)/cton_bmp.o \
	$(OBJ_DIR)/cton.o -o cton.o

libcton.so: $(OBJ_DIR)/cton.o $(OBJ_DIR)/cton_json.o
	$(CC) $(OBJ_DIR)/cton.o $(OBJ_DIR)/cton_json.o -shared -o libcton.so


libcton.a: $(OBJ_DIR)/cton.o $(OBJ_DIR)/cton_json.o
	$(AR) rvs libcton.a $(OBJ_DIR)/cton.o $(OBJ_DIR)/cton_json.o

.PHONY: clean
clean:
	@rm -rf cton.o libcton.so libcton.a cton_json.o $(OBJ_DIR)