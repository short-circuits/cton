# CTON API解析

_源代码中亦存在相应说明，若本文说明与源代码相异，请以源代码为准并向本文提起PR_

本文档中提及的API均为公共API，私有API的说明请参考源代码文件中的说明。

1. [CTON上下文API](#CTON上下文 \(前缀: cton\_\))
    1. [cton\_init()](#cton\_init)
    2. [cton\_destroy()](#cton\_destroy) 
    3. [cton\_seterr()](#cton\_seterr)
    4. [cton\_geterr()](#cton\_geterr)
    5. [cton\_strerr()](#cton\_strerr)
    6. [cton\_err\_clear()](#cton\_err\_clear)

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

---