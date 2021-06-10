# TBON v0.2 Specification

TBON is an object serialization specification like JSON.

This document stands for TBON v0.2

## TBON Types

* NULL
* BOOL (TRUE/FALSE)
* NUMERIC
    * Signed Integer (INT8/INT16/INT32/INT64)
    * Unsigned Integer (UINT8/UINT16/UINT32/UINT64)
    * Float (FLOAT16/FLOAT32/FLOAT64/FLOAT128)
* Array
    * Array of object
    * Array of value
* Map (aka "Associative Array" or "Dictionary")
* Raw
    * C-Style String
    * Binary

## Format

### Header

Magic header always be `54 42 4F 4E`, Stands for TBON.

Version field follows the magic header. in this version, it always be `00 02`.

### Type Tags:

| Type       |  Binary  | Hex         |
|:-----------|:--------:|:------------|
| RESERVED   | 00000000 | 0x00        |
| NULL       | 00000001 | 0x01        |
| Bool False | 00000010 | 0x02        |
| Bool True  | 00000011 | 0x03        |
| RESERVED   | 000001xx | 0x04 - 0x07 |
| Float      | 00001bbb | 0x08 - 0x0F |
| Integer    | 00010bbb | 0x10 - 0x17 |
| Unsigned   | 00011bbb | 0x18 - 0x1F |
| Map        | 001sssss | 0x20 - 0x3F |
| Array      | 010sssss | 0x40 - 0x5F |
| ArrayCx    | 011sssss | 0x60 - 0x7F |
| Binary     | 100sssss | 0x80 - 0x9F |
| String     | 101sssss | 0xA0 - 0xBF |
| RESERVED   | 11xxxxxx | 0xC0 - 0xFF |

### Base 128 Varints Encoding

TBONv2 use Base128 to encode variable length integers, which is same with Google
ProtoBuf. The below instruments comes from google website.

> To understand your simple protocol buffer encoding, you first need to
> understand varints. Varints are a method of serializing integers using
> one or more bytes. Smaller numbers take a smaller number of bytes.
> 
> Each byte in a varint, except the last byte, has the most significant
> bit (msb) set – this indicates that there are further bytes to come. The
> lower 7 bits of each byte are used to store the two's complement
> representation of the number in groups of 7 bits, least significant group
> first.
> 
> So, for example, here is the number 1 – it's a single byte, so the msb is
> not set:
> 
> ```
> 0000 0001
> ```
> 
> And here is 300 – this is a bit more complicated:
> 
> ```
> 1010 1100 0000 0010
> ```
>
> How do you figure out that this is 300? First you drop the msb from each
> byte, as this is just there to tell us whether we've reached the end of
> the number (as you can see, it's set in the first byte as there is more
> than one byte in the varint):
> 
> ```
> 1010 1100 0000 0010
> → 010 1100  000 0010
> ```
>
> You reverse the two groups of 7 bits because, as you remember, varints
> store numbers with the least significant group first. Then you
> concatenate them to get your final value:
>
> ``` 
> 000 0010  010 1100
> →  000 0010 ++ 010 1100
> →  100101100
> →  256 + 32 + 8 + 4 = 300
> ```
>

### NULL object

As single byte of `0x01`

```
+--------+
|  0x01  |
+--------+
```

### BOOL object

Single byte of `0x02` for false

```
+--------+
|  0x02  |
+--------+
```

Or `0x03` for true.

```
+--------+
|  0x03  |
+--------+
```

### NUMERIC object

Numeric object type contains a `bbb` area, which stands for bits as the table below:

| bbb |  bits    |
|:----|:--------:|
| 000 | 8        |
| 001 | 16       |
| 010 | 32       |
| 011 | 64       |
| 100 | 128      |
| 101 | RESERVED |
| 110 | RESERVED |
| 111 | RESERVED |

For example, `0x12 (00010010, xxx=010)` stands for 32-bit signed int

FLOAT8 is not offered as it is not an IEEE754 standard float.

FLOAT16 stores a 16-bit big-endian signed float point number

```
+--------+--------+--------+
|  0x09  |XXXXXXXX|XXXXXXXX|
+--------+--------+--------+
```

FLOAT32 stores a 32-bit big-endian signed float point number

```
+--------+--------+--------+--------+--------+
|  0x0A  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
+--------+--------+--------+--------+--------+
```

FLOAT64 stores a 64-bit big-endian signed float point number

```
+--------+--------+--------+--------+--------+--------+--------+--------+
|  0x0B  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
+--------+--------+--------+--------+--------+--------+--------+--------+
|XXXXXXXX|
+--------+
```

FLOAT128 stores a 128-bit big-endian signed float point number

```
+--------+--------+--------+--------+--------+--------+--------+--------+
|  0x0C  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
+--------+--------+--------+--------+--------+--------+--------+--------+
|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
+--------+--------+--------+--------+--------+--------+--------+--------+
|XXXXXXXX|
+--------+
```

INT8 stores a 8-bit signed integer

```
+--------+--------+
|  0x10  |XXXXXXXX|
+--------+--------+
```

INT16 stores a 16-bit big-endian signed integer

```
+--------+--------+--------+
|  0x11  |XXXXXXXX|XXXXXXXX|
+--------+--------+--------+
```

INT32 stores a 32-bit big-endian signed integer

```
+--------+--------+--------+--------+--------+
|  0x12  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
+--------+--------+--------+--------+--------+
```

INT64 stores a 64-bit big-endian signed integer

```
+--------+--------+--------+--------+--------+--------+--------+--------+
|  0x13  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
+--------+--------+--------+--------+--------+--------+--------+--------+
|XXXXXXXX|
+--------+
```

UINT8 stores a 8-bit unsigned integer

```
+--------+--------+
|  0x18  |XXXXXXXX|
+--------+--------+
```

UINT16 stores a 16-bit big-endian unsigned integer

```
+--------+--------+--------+
|  0x19  |XXXXXXXX|XXXXXXXX|
+--------+--------+--------+
```

UINT32 stores a 32-bit big-endian unsigned integer

```
+--------+--------+--------+--------+--------+
|  0x1A  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
+--------+--------+--------+--------+--------+
```

UINT64 stores a 64-bit big-endian unsigned integer

```
+--------+--------+--------+--------+--------+--------+--------+--------+
|  0x1B  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|
+--------+--------+--------+--------+--------+--------+--------+--------+
|XXXXXXXX|
+--------+
```

128-bit integer and unsigned integer is reseved as almost archtecture not
implement it.

### MAP object

Map object, as known as "Associative Array" or "Dictionary" stores a sequence of
key-value pairs in 1 ~ 11 bytes of extra bytes in addition to it.

To stores a map whose length is upto 30 elements

```
+--------+~~~~~~~~~~~~~~~~~+
|001SSSSS|   N*2 objects   |
+--------+~~~~~~~~~~~~~~~~~+
```

*SSSSS is a 5-bit unsigned integer which represents N*

For a map whose length is upto (2^64)-1 elements

```
+--------+=================+~~~~~~~~~~~~~~~~~+
|  0x3F  |   Base128(N)    |   N*2 objects   |
+--------+=================+~~~~~~~~~~~~~~~~~+
```

### ARRAY object

Array object store a sequence of N elements. 

To stores a sequence whose length is upto 30 elements

```
+--------+~~~~~~~~~~~~~~~~~+
|011SSSSS|    N objects    |
+--------+~~~~~~~~~~~~~~~~~+
```

*SSSSS is a 5-bit unsigned integer which represents N*

For a map whose length is upto (2^64)-1 elements

```
+--------+=================+~~~~~~~~~~~~~~~~~+
|  0x7F  |   Base128(N)    |    N objects    |
+--------+=================+~~~~~~~~~~~~~~~~~+
```

---

For Array to save elements in same type, There are an shortcuts to omit the type
contains in "N objects area"

To stores a map whose length is upto 30 elements with the same type

```
+--------+--------+~~~~~~~~~~~~~~~~~+
|010SSSSS|  type  |    N objects    |
+--------+--------+~~~~~~~~~~~~~~~~~+
```

*SSSSS is a 5-bit unsigned integer which represents N*

For a map whose length is upto (2^64)-1 elements with the same type

```
+--------+=================+--------+~~~~~~~~~~~~~~~~~+
|  0x5F  |   Base128(N)    |  type  |    N objects    |
+--------+=================+--------+~~~~~~~~~~~~~~~~~+
```

* When type is NULL(0x01), Bool(0x02), or Numeric(0x08 ~ 0x1F), the "N objects"
  area only saves the data without extra attachments.
  * For example, when type is `0x02`, data area will just save a sequence of
    `0x02` and `0x03` stands FALSE or TRUE.
  * For another example, when type is `0x10`, the data area will just contains
    a sequence of INT8 data (the XXXXXXXX parts in NUMERIC object chapter).
* When type is MAP, ARRAY, STRING or BINARY, the type should be set to the tag
  with the max length, and "N objects" area stores the data with type.
  * For example, ARRAY of HASH should be:

    ```
+--------+=================+--------+~~~~~~~~~~~~~~~~~+
|  0x5F  |   Base128(N)    |  0x3F  |   N Map Objs    |
+--------+=================+--------+~~~~~~~~~~~~~~~~~+
    ```
    Which N Map Objs area is like
    ```
    +=================+~~~~~~~~~~~~~~~~~+
    |   Base128(N)    |    N objects    |
    +=================+~~~~~~~~~~~~~~~~~+
   ```
   
ARRAY of ARRAY is supported but should never be used.
As ARRAY is an special object, ARRAY of ARRAY just shares the same format
as ARRAY of TBON OBJECT.

### STRING and BINARY object
    
STRING and BINARY format family stores a byte array in 1 ~ 11 bytes of
extra bytes in addition to the byte array.

To stores a byte array whose length is upto 30 bytes
```
+--------+~~~~~~~~+
|100SSSSS|  data  |
+--------+~~~~~~~~+
```
Or
```
+--------+~~~~~~~~+
|101SSSSS|  data  |
+--------+~~~~~~~~+
```
Where SSSSS is a 5-bit unsigned integer which represents N
 
For a byte array whose length is upto (2^64)-1 bytes

```
+--------+=================+~~~~~~~~+
|  0x9F  |   Base128(N)    |  data  |
+--------+=================+~~~~~~~~+
```
    Or
```
+--------+=================+~~~~~~~~+
|  0xBF  |   Base128(N)    |  data  |
+--------+=================+~~~~~~~~+
```

* STRING object stands for the byte array with an ending '\0'.
  The ending '\0' is not counted in N. Which means the really memory
  usage of STRING object is `N + 1`, and the parser should add an ending
  `\0` manually.
¡   