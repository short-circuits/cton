# C-lang Toaru Object Notation
_とあるC言語オブジェクトライブラリ・某C语言用对象操作库・Некий объектно библиотека языка Си_

[![BEER-WARE LICENSE](https://img.shields.io/badge/license-BEER--WARE%F0%9F%8D%9B-blue.svg)](https://github.com/yeonzi/cton/blob/current/LICENSE)
![Top Language](https://img.shields.io/github/languages/top/yeonzi/cton?color=00cc00)
![Total Lines](https://img.shields.io/tokei/lines/github/yeonzi/cton?color=00cc00)


### 目次・目录・Оглавление・T.O.C

1. 日本語
	1. [ライセンス](#ライセンス)
2. 简体中文
	1. [软件授权许可证](#软件授权许可证)
	2. [使用方法](#使用方法)
3. Русский
	1. [ЛИЦЕНЗИЯ](#ЛИЦЕНЗИЯ)
4. English (CA)
	1. [LICENSE](#LICENSE)


## ライセンス

### 「ビールウェアライセンス」（改訂43）：

\<yeonji@ieee.org> がこのプロジェクトを書きました。この通知を保持している限り、
このプロジェクトでやりたいことが何でもできます。もし私たちはいつかの日に会い、
そしてあなたはこのプロジェクトはそれだけの価値があると思いましたら、お返しに、
甘口のカレーライスをごちそうさせればと思います（アルコールドリンク飲めないからね）。
Yeonji Lee


## 软件授权许可证

### “啤酒软件协议”（第四十三版）：

\<yeonji@ieee.org> 编写了此文件。只要你还保留本协议文本，你
可以使用此软件做任何事。如果咱们在某一天相遇了，而且你认为
此软件有价值，你可以给咱买一份甜味咖喱饭来回报咱（因为咱不能
饮用含酒精的饮料）。Yeonji Lee

## 使用方法

### 欢迎来到CTON

CTON项目希望创建一个统一的固定类型C语言对象操作库，并且你可以轻松地把这个C语言完成的程序链接进你自己的程序。

CTON项目提供了简单的内存池集成，设计中同时包含了一个简单的垃圾回收器(TODO)。同时以CTON上下文为操作单位，保证了线程安全。这允许在服务器程序中可以放心安全地使用这个库，而不必过分担心内存泄漏(TODO)或线程竞争导致的服务器错误。

CTON提供了两种自带的数据格式(TODO)，同时也允许了如JSON等的其他格式的读写。

JSON格式的定义在[http://www.json.org/](http://www.json.org/)，它是一个类似XML却不那么冗余的格式。你可以用它来传送，存储数据，或仅用来简单地反应自己程序的运行状态。本项目中使用到的JSON测试样例来自于[http://www.json.org/example.html](http://www.json.org/example.html)

CTON具备了解析诸如XML, YAML, NBT等其他数据格式的能力，但是这部分代码还在编写中。

### 编译代码

Makefile文件中提供了简单的编译案例，你可以尝试编译cton库为一个单独的.o文件或一个动态链接库。

目前为止没有过多的编译说明，具体内容尚处于TODO阶段。

### 基础调用

#### CTON上下文

在使用CTON库进行操作前，首先需要创建一个CTON上下文。

```
cton_ctx *ctx;
ctx = cton_init(NULL);
```

通过此步骤即创建一个CTON上下文。CTON的一个上下文对象并非线程安全，请避免对其进行多线程操作。

#### CTON对象

当拥有了CTON的上下文，即可开始创建CTON对象。

```
cton_obj *obj;
obj = cton_object_create(ctx, CTON_INT32);
```

即可完成一个类型为`CTON_INT32`类型的对象的创建。全部可用的CTON类型包括：

```
CTON_NULL
CTON_BOOL
CTON_BINARY
CTON_STRING
CTON_ARRAY
CTON_HASH
CTON_INT8
CTON_INT16
CTON_INT32
CTON_INT64
CTON_UINT8
CTON_UINT16
CTON_UINT32
CTON_UINT64
CTON_FLOAT32
CTON_FLOAT64
```

如果想要知道一个CTON对象的类型，可以通过调用：

```
cton_type type;
type = cton_object_gettype(ctx, obj);
```

来获得obj对象的类型。

注意：CTON目前**不支持**对象的类型转换，也暂无对象类型转换的计划。这样设计的原因是因为类型转换过程中难以避免的各种精度变化问题。我们希望将决定如何转换的权利交给使用者而不是强制规定。因此一旦完成对象的创建，你将无法更改对象的类型。如果有必要更改对象的类型，请重新创建一个对象并销毁源对象。

对于上文创建的`CTON_INT32`对象，如果想要编辑它的数值，可以通过：

```
#include <stdint.h>

...

int32_t *dat;
dat = cton_object_getptr(ctx, obj);
*dat = 42;
```

这样的方式，通过获得对象指针的方法来修改对象的内容。当然这只是一个最简单的操作方法，甚至不能适用于`CTON_HASH`等复杂的类型（尽管对于这些类型来说这个调用仍然是有定义的）。因此我们墙裂建议您进一步阅读API详解来获得不同类型对应的方法。

#### JSON的输入与输出

作为输入输出组件的案例，我们实装了一个JSON的输入输出组件。其主要API及操作如下：

```
cton_ctx *ctx;
cton_obj *data;
cton_obj *json;
cton_obj *out;

/* 初始化上下文并从文件读取JSON字符串到一个名为data的字符串对象 */
ctx = cton_init(NULL);
data = cton_util_readfile(ctx, "test.json");

/* 解析JSON字符串 */
json = cton_json_parse(ctx, data);
/* 对root对象做一些你想做的事情 */

/* 获得处理后的JSON字符串 */
out = cton_json_stringify(ctx, json);

printf("%s\n", cton_str_getptr(ctx, out));
```

### API详解

TODO


## ЛИЦЕНЗИЯ

### "ЛИЦЕНЗИЯ BEER-WARE" (Версия 43):
\<yeonji@ieee.org> написал этот файл. До тех пор, пока вы сохраняете 
это уведомление, вы можете делать с этими материалами все, что угодно. 
Если мы однажды встретимся, и вы будете считать, что оно того стоит, 
можете в ответную купить мне сладкий Японское карри Yeonji Lee Kamp (потому что я не могу пить алкогольные напитки).


## LICENSE

### "THE BEER-WARE LICENSE" (Revision 43):

\<yeonji@ieee.org> create this project. As long as you retain this notice you
can do whatever you want with this stuff. If we meet some day, and you think
this stuff is worth it, you can buy me a sweety curry rice in return (cuz
I cannot drink alcoholic beverages). Yeonji Lee
