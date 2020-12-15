_Request for Comments 20200723 short-circuits.org_

_Version 20200728 short-circuits.org_

# Toaru Object Notation

TON (Toaru Object Notation) is a lightweight data interchange and storage format. It is easy for humans to read and write. It is easy for machines to parse and generate. TON is a text format that is completely language independent but uses conventions that are formailiar to programs of the C-family of languages.

TON is built on two structures:

> A collection of name/value pairs. In various languages, this is realized as an struct, object, record, dictionary, hash table, keyed list, or associative array.

> An Ordered list of values. In most languages, this is realized as an array, vector, list, or sequence. 

These are universal data structures. Virtually all modern programing languages fupport them in one form or another. It makes sense that a data foramt that is Interchangeable with programing languages also be based on these structures.

The above is copied from Introducing JSON.

## Scope

There a plenty of data notation in the earth, but they all have any kinds of problems that prevent a using in some situation. Some does not support nesting conveniently, and some does not declare the data type clearly. Although you can do that in some way, but it also makes the notation redundancy. So we create this data notation format.

## TON Grammar

### Conformance

A conforming TON text is a sequence of Unicode code points that strictly conforms to the TON grammar defined by this specification.

A conforming processor of TON texts should not accept any inputs that are not conforming TON texts. A conforming processor may impose semantic restrictions that limit the set of conforming TON texts that it will process.

### TON text

A TON text is a sequence of tokens formatedfrom UTF-8 code points that conforms to the TON value grammer. The set of tokens include seven structural tokens, strings, numbers, types, and several liberal type tokens.

The seven structural tokens:

| Token | Unicode  | Comment              |
|:-----:|:---------|:---------------------|
| `,`   | `U+002C` | Comma                |
| `:`   | `U+003A` | Colon                |
| `=`   | `U+003D` | Equal sign           |
| `[`   | `U+005B` | Left square bracket  |
| `]`   | `U+005D` | Right square bracket |
| `{`   | `U+007B` | Left curly bracket   |
| `}`   | `U+007D` | Right curly bracket  |

These are the several literal type tokens:

| Token   | Unicode Sequence                   |
|:--------|:-----------------------------------|
| `true`  | U+0074 U+0072 U+0075 U+0065        |
| `false` | U+0066 U+0061 U+006C U+0073 U+0065 |
| `null`  | U+006E U+0075 U+006C U+006C        |


Insignificant whitespace is allowed before or after any token. Whitespaces is any sequence of one or more of the following code points: character tabulation (U+0009), line feed (U+000A), carriage return (U+000D), and space (U+0020). Whitespace is not allowed within any token, except the space is allowed in strings.

### TON Types, Values and Objects

TON type system is a superset of C language, but a subset of most language. A TON values can be any kind of a TON type, such as an hash table, array, number, string, binary-seqnence, true, false, or null.

The types:

| Token   | Type               | SubType   |
|:-------:|:-------------------|:----------|
|  `a`    | Array              |           |
|  `b`    | Binary Sequence    |           |
|  `f`    | Float-point number | `fN`      |
|  `h`    | Hash table         |           |
|  `i`    | Integer number     | `iN`,`uN` |
|  `s`    | String             |           |
| `true`  | Bool True          |           |
| `false` | Bool False         |           |
| `null`  | NULL               |           |

Type token appears before values and splited by a colom (U+003A). As some type has it's own notation, type token can be omitted if it will not cause confusion. The specific omission conditions will be introduced below

### Comment

TON allows writing comments in the file. Both C-style comment and sharp started comment is allowed but C plus plus style is forbidden. Comment is allowed every where except inside strings and binary sequencees.

C-style comment starts with `/*` and ended by `*/`, nesting comment is not allowed. Shell style comment starts with a single sharp `#` (U+0023) and end with a newline.

### Hash Table

A hash table structure is represented as a pair of curly bracket tokens surrounding zero or more key/value pairs. A key is a string. A single equal sign token follows each key, separating the name from the value. A single comma token separates a value from a following name. The TON syntax does not impose any restrictions on the strings used as names, but require that name strings be unique, and does not assign any significance to the ordering of key/value pairs. These are all semantic considerations that may be defined by TON processors or in specifications defining specific uses of TON for data interchange.

The type token of a hash table is always omittable.

###  Array

An array structure is a pair of square bracket tokens surrounding zero or more values. The values are separated by commas. Array stands for ordered data structure, but not force the values in the same type. If it will benefit from using the same type in an array, TON reserved a method of adding a type token after array type token and before square basket of an array to force all of the values in the array is in the same type.

The type token of an array is always omittable. 

###  Binary sequence

A binary sequence structure that can be used to storage any kind of data by filling the binary data directly. Data should be encoded by some method specified by type token. In default, TON select Base64 (Token `b64` or single `b`) for encoding binary sequence. Additional, using `b16` or `bh` token to specific using hexadecimal notation. Note that base 16 is processed by every 2 character as a stream data, without considering of endianness.

White space is allowed inside the binary sequence, and comment is allowed inside hexadecimal notation. Only comma will really stop the binary sequence.

The type token of a binary sequence is never omittable.

### String

A string is a sequence of C style UTF-8 code points wrapped with quotation marks (U+0022). Technologically, string use the same data structure with binary sequence, so all code points maybe placed within the quotation marks except for null character (U+0000) and the code points that must be escaped: quotation mark (U+0022), reverse solidus (U+005C), and the control characters U+0001 to U+001F. There are two-character escape sequence representations of some characters. 

| Sequence | Represents                              |
|:--------:|:----------------------------------------|
|   `\"`   | quotation mark character (U+0022)       |
|   `\\`   | reverse solidus character (U+005C)      |
|   `\/`   | solidus character (U+002F)              |
|   `\f`   | form feed character (U+000C)            |
|   `\n`   | line feed character (U+000A)            |
|   `\r`   | carriage return character (U+000D)      |
|   `\t`   | character tabulation character (U+0009) |

So, for example, a string containing only a single reverse solidus character may be represented as `"\\"`.

Any code point may be represented as a hexadecimal escape sequence. There are two kinds of hexadecimal escape sequence. One of the meaning of such a hexadecimal number is determined by ISO/IEC 10646. If the code point is in the Basic Multilingual Plane (U+0000 through U+FFFF), then it may be represented as a six-character sequence: a reverse solidus, followed by the lowercase letter u, followed by four hexadecimal digits that encode the code point. Hexadecimal digits can be digits (U+0030 through U+0039) or the hexadecimal letters A through F in uppercase (U+0041 through U+0046) or lowercase (U+0061 through U+0066). The code will be transformed into UTF-8 in the inner structure. So, for example, a string containing only a single reverse solidus character may be represented as "\u005C".

To escape a code point that is not in the Basic Multilingual Plane, the character may be represented as a twelve-character sequence, encoding the UTF-16 surrogate pair corresponding to the code point. So for example, a string containing only the G clef character (U+1D11E) may be represented as "\uD834\uDD1E".

Another of the meaning of the hexadecimal number is determined by ISO/IEC 9899. For any character except null character (U+0000), it cam be represented as a four-character sequence: a reverse solidus, followed by the lowcase letter x, followed by two hexadecimal digites that encode the character. Hexadecimal character limites is same with the escape sequence metioned before.

So, for another example, a string containing only a single reverse solidus character may be represented as "\x5C".

The following four cases all produce the same result:

```
"\u002F"
"\u002f"
"\x2F"
"\x2f"
"\/"
"/"
```

One thing make string is really defferent from binary sequence is that string type will add a null character after the sequence to confirm it is safe with any C-style string library.

The string is a subset of binary sequence theoretically, and same structure with binary sequence technologically. Although that means string can storage any kind of data even with `\0` character, TON limits the usage of string to make it human readable. So we strongly suggests only use string to storage a human readable string only.

Quotation marks will not stop the string immediately. Which means two strings without any split will be linked as a single string. So, for example, a string likes `"i""ro""ha"` equals to the string `"iroha"`. White spaces and comments are allowed between two string clips. So, for another example:

```
"i"  /* a */
"ro" /* b */
"ha" /* c */
```

is legal and will generate the string `"iroha"`

The type token of an array is always omittable.

### Integer number

A integer number is a sequence of decimal digits. The digits are the code
points U+0030 through U+0039.

Interger number has eight sub-classes by sign and bit width. Signed integer number has the type token leading by `i` and unsigned by `u`. After this token, it will be a number stands for bit width. The avaliable bit width set is: 8, 16, 32 and 64. It may have a preceding minus sign (U+002D) in signed interger.

Note that the limited is determied by the machine, so never challenge to storage the max or min value of a type unless you known what you are doing. (Parser reserved the rights to throw exception)

The type token of an interger can be omitted, and will be treated as `i32`.

### Float number

A float number is a sequence of decimal digits with no superfluous leading zero. It may have a preceding minus sign (U+002D). It may have a fractional part prefixed by a decimal point (U+002E). It may have an exponent, prefixed by e (U+0065) or E (U+0045) and optionally + (U+002B) or – (U+002D). The digits are the code points U+0030 through U+0039.

Numeric values that cannot be represented as sequences of digits (such as Infinity and NaN) are not permitted.

Float number also has sub-classes to specific the bit width, but only 16, 32 and 64 is allowed. Float will be storaged by IEEE754, and exponent in the notation may have effect to the exponent in the binary format.

The type token of a float number can be ommited only if it contains decimal point or exponent. A float without type token will be treated as `f32`

## Toaru Bionary Object Notation

TBON (Toaru Bionary Object Notation) is a binary format for TON designed for machines to parse and generate. It contains almost all information from TON format except comments and code formats. TBON is not designed for in-storage search or performace for disk I/O.

### Conformance

A conforming TBON data is a sequence of bytes strictly conforms to the TBON format defined by this specification.

A conforming processor of TBON should not accept any inputs that are not conforming TBON data. A conforming processor may impose semantic restrictions that limit the set of conforming TBON data that it will process.

### Header

A legal TBON data always starts with the magic header of `TBON` (`0x54` `0x42` `0x4E` `0x4C`)

After magic header, two bytes in big-ending stands for the version. This file is version 0.1, so this two byte should be `0x00` and `0x01`.

### Basic data formats

TBON datas has the following formats:

```
byte 0:   Type tag
byte 1~?: Extra information
byte ?~?: [payload]
```

The type tag is a single byte defining the contents of the payload of the tag. Extra informations and paylods varies by type tag.


| Tag | Type              |
|:---:|:------------------|
|   0 | Object            |
|   1 | NULL              |
|   2 | Bool true         |
|   3 | Bool false        |
|   4 | Binary Sequence   |
|   5 | String            |
|   6 | Array             |
|   7 | Hash table        |
|   8 | int8              |
|   9 | int16             |
|  10 | int32             |
|  11 | int64             |
|  12 | unsigned int8     |
|  13 | unsigned int16    |
|  14 | unsigned int32    |
|  15 | unsigned int64    |
|  16 | float8 (Reserved) |
|  17 | float16           |
|  18 | float32           |
|  19 | float64           |

### Object

An object stands for a struct with type tags and addtional payload. Type tags is a 8-bit width unsigned data, and payload is determided by type. The data structure of an object can be descripted as below:

```
struct object {
    uint8  type;
    struct payload;
}
```

Object is also a special type with the tag `0x00`, but never comes alone as all data comes in format of object. The only one place object type tag may come is in an array.

### NULL

NULL is a special type with no payload as no extra data is necessary for NULL type.

### Bool

Bool is also a special type with two type tags. As bool has only two value, no need to allocate another 8-bit space for value, so bool use the type tag for data.

A special condition is, when stands for array type, only true (0x02) stands for bool type array.

### Binary Sequence and String

Binary sequence and string shares the same payload struct as below:

```
struct bin_payload {
    uint64 length;
    uint8  data[length];
}
```

But note that string should have a `\0` at the end of data field, and the ending `\0` is also contained in the length.

### Array

Array stands for ordered datas in same type, payloads following this struct:

```
struct array_payload {
    uint8  payload_type
    uint64 count;
    struct payload_type data[count];
}
```

As object is a special type contains it's own type information, using object type inside an array means can have more than one types in an array.

### Hash table

Hash table stands for key/value pairs. Key is always a string, and value can be any kind of objects.

A single key/value pair is defined as:

```
struct key_value_pair {
    struct string_payload key;
    struct object         value;
}
```

Hash table is defined as:

```
struct hash_payload {
    uint64 count;
    struct key_value_pair data[count];
}
```

### Numberic

`iN`,`uN` and `fN` shares the same format. The payload is simple and stupid, the data only. Multi byte data comes in big-endian.

## Example

Here is an example for TON.

```
{
    "widget" = {
        "debug" = "on",
        "window" = {
            "title" = "Sample Konfabulator Widget",
            "name" = "main_window",
            "width" = 500,
            "height" = 500
        },
        "image" = { 
            "src" = "Images/Sun.png",
            "name" = "sun1",
            "hOffset" = 250,
            "vOffset" = 250,
            "alignment" = "center"
        },
        "text" = {
            "data" = "Click Here",
            "size" = 36,
            "style" = "bold",
            "name" = "text1",
            "hOffset" = 250,
            "vOffset" = 100,
            "alignment" = "center",
            "onMouseUp" = "sun1.opacity = (sun1.opacity / 100) * 90;"
        }
    }
}
```

If you want to keep type sign, it just like:

```
h:{
    "widget" = h:{
        "debug" = s:"on",
        "window" = h:{
            "title" = s:"Sample Konfabulator Widget",
            "name" = s:"main_window",
            "width" = i32:500,
            "height" = i32:500
        },
        "image" = h:{
            "src" = s:"Images/Sun.png",
            "name" = s:"sun1",
            "hOffset" = i32:250,
            "vOffset" = i32:250,
            "alignment" = s:"center"
        },
        "text" = h:{
            "data" = s:"Click Here",
            "size" = i32:36,
            "style" = s:"bold",
            "name" = s:"text1",
            "hOffset" = i32:250,
            "vOffset" = i32:100,
            "alignment" = s:"center",
            "onMouseUp" = s:"sun1.opacity = (sun1.opacity / 100) * 90;"
        }
    }
}
```

And this example can be transformed into TBON as below:

```
54 42 4F 4E                TBON
00 01                      Version 0.1
07                         Hash
00 00 00 00 00 00 00 01    length = 1
00 00 00 00 00 00 00 07    Key length = 7
77 69 64 67 65 74 00       Key = "widget"
07                         Value = Hash
00 00 00 00 00 00 00 04    length = 4
00 00 00 00 00 00 00 06    Key length = 6
64 65 62 75 67 00          Key = "debug"
05                         Value = String
00 00 00 00 00 00 00 03    Length = 3
6E 6E 00                   "on"
00 00 00 00 00 00 00 07    Key length = 7
77 69 6e 64 6f 77 00       Key = "window"
07                         Value = Hash
00 00 00 00 00 00 00 04    length = 4
...
```

## Require for Comments

1. Is it necessary to use equal sign for split key and value? is it okay to use colon? which will make JSON a subset of TON. Or make equal sign the same with colon?
2. How about limit max length to 32-bit unsigned int in TBON struct? 4294967296 may enough for most instance of array, hash and string?

<br><br><br><br><br><br><br><br><br><br><br><br>
© Short-circuits.org 2020