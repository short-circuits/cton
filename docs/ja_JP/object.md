# CTONオブジェクト

[docs](../).[ja_JP](./README.md).[CTON coreAPI](./coreAPI.md).CTONオブジェクト

CTONオブジェクトはCTONのデータ型の一つを表します。
これはより複雑な実態を格納するために使用されます。
CTONオブジェクトは`cton_object_create()`コンストラクターを使用して生成することができます。

CTONのほぼすべてのオブジェクトがCTONオブジェクトのインスタンスです。
CTONのオブジェクトは必ず以下のデータ型の何れかを持ちます。

| データ型名   | データ型          | 前缀        |
|:-------------|:------------------|:------------|
| CTON_NULL    | 空                |             |
| CTON_BOOL    | ブーリアン        | cton_bool   |
| CTON_BINARY  | バイナリ          | cton_binary |
| CTON_STRING  | Cスタイル文字列   | cton_string |
| CTON_ARRAY   | 配列              | cton_array  |
| CTON_HASH    | 連想配列          | cton_hash   |
| CTON_INT8    | 8bit符号付整数    |             |
| CTON_INT16   | 16bit符号付整数   |             |
| CTON_INT32   | 32bit符号付整数   |             |
| CTON_INT64   | 64bit符号付整数   |             |
| CTON_UINT8   | 8bit符号無し整数  |             |
| CTON_UINT16  | 16bit符号無し整数 |             |
| CTON_UINT32  | 32bit符号無し整数 |             |
| CTON_UINT64  | 64bit符号無し整数 |             |
| CTON_FLOAT8  | 8bit浮動小数点数  |             |
| CTON_FLOAT16 | 16bit浮動小数点数 |             |
| CTON_FLOAT32 | 32bit浮動小数点数 |             |
| CTON_FLOAT64 | 64bit浮動小数点数 |             |

このデータ型はオブジェクト生成する際に指定されます。
CTONは静的型付けライブラリなので、ほぼのオブジェクトは型変換ができません。
しかし、バイナリとCスタイル文字列は例外であり、
一定の前提で変換することができます。

## cton\_object\_create

- CTONオブジェクトを生成する

### 書式

```C
#include <cton.h>

cton_obj * cton_object_create(cton_ctx *ctx, cton_type type);
```

### 説明

`cton_object_create`はCTONオブジェクトを作成して初期化する関数です。
作ったオブジェクトはパラメータによる指定されたCTONコンテキストに所属します。

作成したオブジェクトに対する初期化は、データ構造の初期化のみです。
すなわち、配列や文字列などのメモリ確保とかは行いません。

### 返り値

成功すると作成したオブジェクトのポインターを返します。
失敗すると`NULL`が返されます。

