SRC_DIR=src
ZLIB_DIR=external/zlib
CC=cc
AR=ar
LD=ld
CFLAGS=-Wall -Wextra -std=c89 -Wpedantic -fPIC -O2 -I$(SRC_DIR) -I$(ZLIB_DIR)
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj

OBJECTS=$(OBJ_DIR)/cton_core.o $(OBJ_DIR)/cton_json.o $(OBJ_DIR)/cton_bmp.o $(OBJ_DIR)/cton_tbon.o

all: $(BUILD_DIR)/libcton.o $(BUILD_DIR)/libcton.so $(BUILD_DIR)/libcton.a $(BUILD_DIR)/test_json $(BUILD_DIR)/test_bmp

$(BUILD_DIR)/test_json: $(BUILD_DIR)/libcton.o test/json/json_test.c
	$(CC) $(CFLAGS) -I./ $(BUILD_DIR)/libcton.o test/json/json_test.c -o $(BUILD_DIR)/test_json

$(BUILD_DIR)/test_tbon: $(BUILD_DIR)/libcton.o test/tbon/tbon_test.c
	$(CC) $(CFLAGS) -I./ $(BUILD_DIR)/libcton.o test/tbon/tbon_test.c -o $(BUILD_DIR)/test_tbon

$(BUILD_DIR)/test_bmp: $(BUILD_DIR)/libcton.o test/bmp/bmp_test.c
	$(CC) $(CFLAGS) -I./ $(BUILD_DIR)/libcton.o test/bmp/bmp_test.c -o $(BUILD_DIR)/test_bmp

$(BUILD_DIR)/libcton.o: $(OBJECTS) $(BUILD_DIR)/libz.a
	$(LD) -r $(OBJECTS) -o $(BUILD_DIR)/libcton.o $(BUILD_DIR)/libz.a

$(BUILD_DIR)/libcton.so: $(OBJECTS)
	$(CC) $(OBJECTS) -shared -o $(BUILD_DIR)/libcton.so

$(BUILD_DIR)/libcton.a: $(OBJECTS)
	$(AR) rs $(BUILD_DIR)/libcton.a $(OBJECTS)

$(BUILD_DIR)/libz.a:
	cd build && ../external/zlib/configure --static && make libz.a

$(OBJ_DIR)/cton_tbon.o: $(SRC_DIR)/cton_tbon.c \
						$(SRC_DIR)/cton.h
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(SRC_DIR)/cton_tbon.c -o $(OBJ_DIR)/cton_tbon.o

$(OBJ_DIR)/cton_json.o: $(SRC_DIR)/cton_json.c \
						$(SRC_DIR)/cton_json.h \
						$(SRC_DIR)/cton.h
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(SRC_DIR)/cton_json.c -o $(OBJ_DIR)/cton_json.o


$(OBJ_DIR)/cton_bmp.o: $(SRC_DIR)/cton_bmp.c \
						$(SRC_DIR)/cton_bmp.h \
						$(SRC_DIR)/cton.h
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(SRC_DIR)/cton_bmp.c -o $(OBJ_DIR)/cton_bmp.o

$(OBJ_DIR)/cton_core.o: $(SRC_DIR)/cton.c $(SRC_DIR)/cton.h
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(SRC_DIR)/cton.c -o $(OBJ_DIR)/cton_core.o

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)