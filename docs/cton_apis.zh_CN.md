# CTON API解析

_源代码中亦存在相应说明，若本文说明与源代码相异，请以源代码为准并向本文提起PR_

本文档中提及的API均为公共API，私有API的说明请参考源代码文件中的说明。

1. [CTON上下文API](#CTON上下文\ \(前缀: cton\_\))
    1. [cton\_init()](#cton\_init)
    2. [cton\_destroy()](#cton\_destroy) 
    3. [cton\_seterr()](#cton\_seterr)
    4. [cton\_geterr()](#cton\_geterr)
    5. [cton\_strerr()](#cton\_strerr)
    6. [cton\_err\_clear()](#cton\_err\_clear)
2. [CTON内存钩子](#内存钩子)
    1. [cton\_memhook\_init()](#cton\_memhook\_init)
3. [CTON对象](#CTON对象\ (前缀:\ cton\_object\_))
    1. [cton\_object\_create()](#cton\_object\_create)
    2. [cton\_object\_delete()](#cton\_object\_delete)
    3. [cton\_object\_gettype()](#cton\_object\_gettype)
    4. [cton\_object\_getval()](#cton\_object\_getval)

## CTON上下文（前缀: cton_)

### cton_init

`cton_ctx *cton_init(cton_memhook *hook);`

- 创建一个新的CTON上下文

#### 参数

- hook: 此上下文中将使用的内存钩子。

#### 返回值

若成功则返回新创建的CTON上下文句柄，否则返回NULL。

#### 功能描述

`cton_init()` 函数会创建一个使用传入内存钩子的新上下文。

若传入参数为NULL则会使用内置的内存钩子（代理C语言标准库提供的malloc/free函数）来初始化新创建的上下文。

新创建的上下文所需要的存储空间也会使用此内存钩子来分配，因此如果使用了内存池来创建内存钩子，可以通过重置内存池的方法来安全地清空全部CTON数据结构。

传入的内存钩子中，palloc调用是必要的。如果此钩子没有被正确设定，那么此函数会返回NULL。

尽管本调用是多线程安全的，但是因为创建内存钩子的调用非线程安全，**将通过创建内存钩子的函数调用创建的钩子，传入此函数来创建上下文的操作是非线程安全的**。

#### 参见

[cton\_memhook\_init()](#cton_memhook_init)

---

### cton_destroy

`int cton_destory(cton_ctx *ctx);`

- 删除（解构）一个CTON上下文。

#### 参数

- ctx: 需要解构的上下文句柄。

#### 功能描述

本API会根据传入的上下文句柄有两种略不同的行为。

如果传入的上下文句柄被设置了内存钩子中的destroy方法，则此API会直接调用destroy方法销毁内存池。

如果传入的上下文句柄中没有设置destroy方法（设置为NULL），则此API会尝试对所有对象调用内存钩子的free方法来释放所有对象。

如果free方法也没有被设置，这个函数将什么也不做（等待进程结束后或内存池生命周期结束时回收）。函数将返回-1，并设置ctx上下文的错误代码为CTON_EMHOOK。

#### 使用提示

- 尽管使用此调用后原始句柄不会被改变，但除非本API返回值不为0，原始句柄应当视为不可用，不应当继续尝试对原始句柄进行任何调用。

#### TODO

本API尚未完成

---

### cton_seterr

`void cton_seterr(cton_ctx *ctx, cton_err err);`

- 为此CTON上下文设置错误代码。
- 警告：在绝大多数情况下，没有必要也不应当直接调用此API。

#### 参数

- ctx: 将被设置错误代码的上下文句柄。
- err: 将设置的错误代码。

#### 功能描述

本程序库提供了一个类似于C标准库的errno功能。但不同于标准库中的errno，此错误代码是每个上下文独立而非线程独立。因此即使在同一个线程中操作了两个不同的上下文，它们的错误代码是可以独立保持的。

本调用将用来设置传输上下文的错误代码。此API设计上是用来给函数库内部的程序设置错误代码使用，调用者理论上没有直接调用这个API的必要。但因为“没有发生错误”也被实现成了一种特殊的错误代码，因此本API同时可以用来清空当前的错误代码。但用户不应当尝试直接调用这个函数，请使用专门的清空错误代码API。

#### 参见

cton\_geterr cton\_strerr cton\_err\_clear

---

### cton_geterr

`cton_err cton_geterr(cton_ctx *ctx);`

- 查看传入的上下文中发生的错误代码。

#### 参数

- ctx: 欲查看错误代码的上下文句柄。

#### 使用提示

- 可以在判断语句中使用如`cton_geterr(ctx) == CTON_OK`的语句来确认此上下文没有发生错误。

---

### cton_strerr

`char * cton_strerr(cton_err err);`

- 以可打印字符串的形式返回一个错误代码对应的内容。

#### 参数

- err: 欲查看可打印字符串说明的错误代码。

#### TODO

本方法尚未实现。
暂无i18n计划。

---

### cton\_err\_clear

`#define cton_err_clear(ctx) {cton_seterr((ctx), CTON_OK)}`

- 覆盖掉上下文中发生的错误代码，并强制设置为没有错误。

#### 参数

- ctx: 欲清除错误代码的上下文句柄。


## 内存钩子

### cton\_memhook\_init

```
cton_memhook* cton_memhook_init (void * pool,
    void * (*palloc)(void *pool, size_t size),
    void * (*prealloc)(void *pool, void *ptr, size_t size),
    void   (*pfree)(void *pool, void *ptr),
    void   (*pdestroy)(void *pool))
```

- 创建一个内存钩子。

#### 参数

- pool: 需要传递给内存钩子的结构体，如内存池等。
- palloc: 类似于malloc
- prealloc: 类似于realloc
- pfree: 类似于free
- pdestroy: 可以用来一次性清空所有内存使用的函数。

#### 内存钩子简介

按照设计，CTON程序库会在一定程度上接管对于内存的管理。而内存钩子允许在一定程度上修改内存申请及释放的行为。当完成一个CTON上下文的创建后，通过该上下文创建的所有新对象都会使用这个内存钩子来申请内存。

与标准库中的malloc/free函数不同，CTON中定义的palloc/pfree系列函数需要一个任意类型指针的传入参数，并且在调用对应的函数时会传入这个参数。因此使用者可以尝试通过自行实现一个内存分配器来回避系统内存分配导致的性能问题。也可以通过设置一个可以整体销毁的内存池来保证不会有内存泄漏的出现。

#### 使用提示

- 本函数及返回的结构体并非线程安全，请不要尝试并行创建多个拥有不同参数的内存钩子。
- 本调用创建的内存钩子在部署到CTON上下文的时候会被复制一份，因此在完成了上下文创建之前请不要尝试创建第二个内存钩子。

#### 使用案例

- 通过传入一个可整体free的内存池来保证不会有内存泄漏。
- 通过传入一个支持内存分配统计的中间件来统计你的CTON总共使用了多少内存。
- 通过设置一个可以直接修改页表的prealloc钩子来加速realloc操作。
- etc.

## CTON对象 (前缀: cton\_object\_)

CTON对象是CTON函数库中的基本单位之一。

CTON对象保存一个静态类型的变量，变量类型在初始化对象的时候被决定，在对象生命周期中不会被任何开放API所更改。

可用于CTON对象的变量类型包括：

| 名称         | 类型            | 前缀        |
|:-------------|:----------------|:------------|
| CTON_NULL    | 空对象          |             |
| CTON_BOOL    | 布尔对象        | cton_bool   |
| CTON_BINARY  | 二进制对象      | cton_binary |
| CTON_STRING  | C风格字符串对象 | cton_string |
| CTON_ARRAY   | 阵列对象        | cton_array  |
| CTON_HASH    | 散列对象        | cton_hash   |
| CTON_INT8    | 8位有符号整数   |             |
| CTON_INT16   | 16位有符号整数  |             |
| CTON_INT32   | 32位有符号整数  |             |
| CTON_INT64   | 64位有符号整数  |             |
| CTON_UINT8   | 8位无符号整数   |             |
| CTON_UINT16  | 16位无符号整数  |             |
| CTON_UINT32  | 32位无符号整数  |             |
| CTON_UINT64  | 64位无符号整数  |             |
| CTON_FLOAT8  | 8位浮点数       |             |
| CTON_FLOAT16 | 16位浮点数      |             |
| CTON_FLOAT32 | 32位浮点数      |             |
| CTON_FLOAT64 | 64位浮点数      |             |

---

### cton\_object\_create

`cton_obj * cton_object_create(cton_ctx *ctx, cton_type type);`

- 创建一个新的CTON对象。

#### 参数

- ctx: 新创建的对象所属的上下文。
- type: 新创建的对象的类型。

#### 返回值

若成功则返回新创建的对象句柄，否则返回NULL并设置错误代码。

---

### cton\_object\_delete

`void cton_object_delete(cton_ctx *ctx, cton_obj *obj);`

- 删除一个CTON对象。

#### 参数

- ctx: 要删除的对象所属的上下文。
- obj: 要删除的对象。

#### 功能描述

本调用将调用ctx中的内存钩子对obj对象进行回收。此函数不会检查传入的obj对象是否属于ctx上下文。如果传入的obj对象不属于传入的ctx上下文，其行为是未定义的（取决于设置的内存池）。

本调用不会对obj对象进行解构，即如果obj对象是数组或散列类型，不会释放其子元素。但会导致其子对象成为不可到达的对象，如果长时间使用一个ctx上下文，可能会因此导致内存泄露问题。（TODO：添加基于连通性的自动对象回收调用。）

---

### cton\_object\_gettype

`cton_type cton_object_gettype(cton_ctx *ctx, cton_obj *obj);`

- 获得一个对象的类型。

#### 参数

- ctx: 要获取类型的对象所属的上下文。
- obj: 要获取类型的对象。

#### 使用提示

- CTON是一个静态类型的库，因此不提供修改对象类型的调用。如果有必要修改一个对象的类型，请重新创建一个对象。

---

### cton\_object\_getvalue

`void * cton_object_getvalue(cton_ctx *ctx, cton_obj *obj);`

- 获得当前对象的数据指针。（不推荐）

#### 参数

- ctx: 要获取数据指针的对象所属的上下文。
- obj: 要获取数据指针的对象。

#### 返回值

如果成功则返回对象中表示数据的指针。否则返回NULL。特别地，对于CTON_NULL类型变量，此调用将永远返回NULL指针。

下表说明了对于不同类型的对象，调用此方法会返回的内容。

| 类型         | 返回值              | 类型        |
|:-------------|:--------------------|:------------|
| CTON_NULL    | 空指针              | void *      |
| CTON_BOOL    | 布尔对象指针        | cton_bool * |
| CTON_BINARY  | 二进制数据指针      | void *      |
| CTON_STRING  | C风格字符串指针     | char *      |
| CTON_ARRAY   | 阵列对象指针        | cton_obj *  |
| CTON_HASH    | 散列私有结构体指针  | cton_hash * |
| CTON_INT8    | 8位有符号整数指针   | int8_t *    |
| CTON_INT16   | 16位有符号整数指针  | int16_t *   |
| CTON_INT32   | 32位有符号整数指针  | int32_t *   |
| CTON_INT64   | 64位有符号整数指针  | int64_t *   |
| CTON_UINT8   | 8位无符号整数指针   | uint8_t *   |
| CTON_UINT16  | 16位无符号整数指针  | uint16_t *  |
| CTON_UINT32  | 32位无符号整数指针  | uint32_t *  |
| CTON_UINT64  | 64位无符号整数指针  | uint64_t *  |
| CTON_FLOAT8  | 8位浮点数指针       | float8_t *  |
| CTON_FLOAT16 | 16位浮点数指针      | float16_t * |
| CTON_FLOAT32 | 32位浮点数指针      | float32_t * |
| CTON_FLOAT64 | 64位浮点数指针      | float64_t * |

#### 使用提示

这个方法可以用来方便的处理数值类型和字符串类型等，返回的指针可供直接访问。虽然对于数组和散列，此方法依旧可以返回对应的指针，但是因为返回的是内部的数据结构指针，对这个指针进行解引用并进行操作的行为是未定义的。

## CTON布尔对象 (前缀: cton\_bool\_)

布尔对象是一类只有真(`true`)和假(`false`)两种取值的类型。

cton中没有明确规定使用0还是1代表真，但是这个取值需要能满足机器或者语言提供的布尔运算规则。例如在x86体系中1被视为真，因此如果将一个布尔真赋值给一个8位有符号整数，将会获得数值1。

### cton\_bool\_set

`int cton_bool_set(cton_ctx *ctx, cton_obj *obj, cton_bool val);`

- 设置一个布尔对象的值。

#### 参数

- ctx: 要设置布尔值的对象所属的上下文。
- obj: 要设置布尔值的对象。
- val: 要设置的布尔值。

#### 返回值

成功时返回0，若发生了任何错误则返回非零值。

#### 功能描述

本API会为传入对象设置一个布尔值。

若传入对象非布尔对象，则本调用会返回`-1`，并不会对传入对象进行任何操作。
若传入对象是布尔对象，则本调用会检查`val`参数，只有传入`CTON_TRUE`时，布尔变量才会被设置为真值。传入任何非`CTON_TRUE`的值均会设置布尔变量为假。但如果传入参数为`CTON_TRUE`或`CTON_FALSE`以外的数值，调用会返回`1`。

### cton\_bool\_get

`cton_bool cton_bool_get(cton_ctx *ctx, cton_obj *obj);`

- 获取一个布尔对象的值。

#### 参数

- ctx: 要获取布尔值的对象所属的上下文。
- obj: 要获取布尔值的对象。

#### 返回值

返回要获取的布尔对象的布尔值。如果传入对象非布尔类型将永远获得`CTON_FALSE`。

## CTON字符串对象 & CTON二进制对象

CTON的字符串对象和二进制对象共享了绝大多数的API。
设置这两种类型的主要目的，仅仅是为了区分这个字符串是否是简单的可打印字符串。
同时也

### cton\_string\_getlen

`size_t cton_string_getlen(cton_ctx *ctx, cton_obj *obj);`

- 获得字符串存储空间长度

#### 参数

- ctx: 要获取长度的字符串对象所属的上下文。
- obj: 要获取长度的字符串对象。

#### 返回值

字符串的存储空间长度。

#### 功能描述

本API设计用于查询字符串的可用内存长度。这个长度只是调用`cton_string_setlen()`时申请的内存长度，**不一定等于字符串长度**。如果需要获得字符串长度，请考虑获得字符串指针后使用如标准库的strlen等方法来计算。

此方法虽然不能用来获得字符串长度，但是可以作为参数传递给`strnlen()`等调用以保证不会出现内存越界访问。

---

### cton\_string\_setlen

`int cton_string_setlen(cton_ctx *ctx, cton_obj *obj, size_t len);`

- 设置字符串存储空间长度。

#### 参数

- ctx: 要设置长度的字符串对象所属的上下文。
- obj: 要设置长度的字符串对象。
- len: 要设置的长度值

#### 返回值

操作后字符串的存储空间长度。这个长度可能不等于len（例如发生了错误）

#### 功能说明

本API会更改字符串的存储空间长度。调用本API后字符串的指针可能会发生改变，因此**调用此API后想要继续访问时，请重新使用cton\_[string\|binary]_getptr()调用获得指针**。使用本调用后仍然继续使用原始指针的行为是未定义的。本API没有释放内存的要求，因此使用本API后依旧访问超过申请长度的内存的行为同样是未定义的。本调用可能会失败，如果调用失败（返回值小于传入参数len），访问超过返回值的内存的行为同样是未定义的。

---

### cton\_string\_getptr

`char * cton_string_getptr(cton_ctx *ctx, cton_obj *obj);`

- 获得字符串对象的数据指针。

#### 参数

- ctx: 要获得指针的字符串对象所属的上下文。
- obj: 要获得指针的字符串对象。

#### 返回值

字符串指针。

#### 使用提示

- 对于一个刚创建，还没有被分配内存的字符串，这个调用可能会返回NULL指针。

---

### cton\_binary\_getptr

`void * cton_binary_getptr(cton_ctx *ctx, cton_obj *obj);`

- 获得字符串对象的数据指针。

#### 参数

- ctx: 要获得指针的二进制对象所属的上下文。
- obj: 要获得指针的二进制对象。

#### 返回值

任意类型指针。

#### 使用提示

- 对于一个刚创建，还没有被分配内存的二进制类型，这个调用可能会返回NULL指针。
- 这是唯一一个二进制类型独立于字符串类型的调用，虽然实际上对于字符串的调用同样可以使用。设置这个调用的意义是可以避免一次类型转换。（如果你使用了`-Werror`编译选项会很方便）

## CTON阵列对象 (前缀: cton\_array\_)

在CTON中，阵列被定义为若干个同一类型的数据组成的有序且支持随机访问的数据结构。
CTON没有强制阵列必须是稠密的。因此一个阵列中可能在不确定的位置有任意个空缺元素。

### cton\_array\_getlen

`size_t cton_array_getlen(cton_ctx *ctx, cton_obj *arr);`

- 获得阵列对象可以容纳的元素数量。

---

`size_t cton_array_setlen(cton_ctx *ctx, cton_obj *arr, size_t len);`

- 设置阵列对象可以容纳的元素数量

---

`int cton_array_settype(cton_ctx *ctx, cton_obj *arr, cton_type type);`

- 设置阵列对象允许容纳的元素类型

---

`cton_type cton_array_gettype(cton_ctx *ctx, cton_obj *arr);`

- 获得阵列对象允许容纳的元素类型

---

`cton_obj * cton_array_get(cton_ctx *ctx, cton_obj *arr, size_t index);`

- 获得阵列对象中指定位置的元素。

---

`int cton_array_set(cton_ctx *ctx, cton_obj *arr, cton_obj *obj, size_t index);`

- 设置阵列对象中指定位置的元素。

---

```
int cton_array_foreach(cton_ctx *ctx, cton_obj *arr, void *rctx,
    int (*func)(cton_ctx *, cton_obj *, size_t, void*));
```

- 对阵列对象中每个元素进行循环操作。

## CTON散列对象 (前缀: cton\_hash\_)

在CTON中，散列被定义为通过键值对的方式存储的，非有序数据结构。
目前的实现中限制了key必须是一个字符串对象，但这不应当是一个硬性限制，未来的版本可能会取消这一限制以实现更灵活的功能。

`cton_obj * cton_hash_set(cton_ctx *ctx, cton_obj *h, cton_obj *k, cton_obj *v);`

- 在散列对象中设置一个键值对

---

`cton_obj * cton_hash_get(cton_ctx *ctx, cton_obj *h, cton_obj *k);`

- 在散列对象中根据key获得其value

---

`cton_hash_foreach`

- TODO




