# C-lang Toaru Object Notation
_とあるC言語オブジェクトライブラリ・某C语言用对象操作库・Некий объектно библиотека языка Си_

[![BEER-WARE LICENSE](https://img.shields.io/badge/license-BEER--WARE%F0%9F%8D%9B-blue.svg)](https://github.com/short-circuits/cton/blob/current/LICENSE)
![Top Language](https://img.shields.io/github/languages/top/short-circuits/cton?color=00cc00)
![Total Lines](https://img.shields.io/tokei/lines/github/short-circuits/cton?color=00cc00)
![Github Workflow](https://img.shields.io/github/workflow/status/short-circuits/cton/C%20compile%20test/current)



## プロジェクト概要

このレポジトリは、C言語のため静的型付きオブジェクトライブラリを目指しているものです。設計の上はコンテキストごとスレッドセーフであり、メモリープールに統合可能です。連結グラフに基づいたガベージコレクション機能も計画中ですが、まだ実装していません。

データ型については、ハッシュと配列などが実装しましたので、ほとんどのデータ記述言語に適応できます。例としJSONの直列化と復元のプログラムを実装しました[[1]](https://github.com/yeonzi/cton/blob/current/src/cton_json.c)。さらに、このライブラリにぴったりするデータフォーマットも改めて定義して[[2]](https://github.com/yeonzi/cton/blob/current/docs/ton_specification.md)実装しました[[3]](https://github.com/yeonzi/cton/blob/current/src/cton_tbon.c)。

このライブラリは主にデータ交換を着目しているため、性能はそんなに良くないです。性能が大事なアプリなら、構造体を使うことがオススメですが、性能を向上するためのPRは大歓迎。


## 使い方

### コンパイルなど

まずはこのレポジトリをクローンして、サブモージュルも含んでいるので、`--recursive`フラグを追加するのはオススメです。

* `git clone --recursive`

configureスクリプトが用意したので、それを使ってください。

* `./configure`
* `make`

これでbuildというディレクトリが作ります。そのディレクトリの中にはlibcton.aとlibcton.soがあり、システム側にインストールしたい場合にこの二つのファイルを使ってください。また、このライブラリを利用するためのヘッダーファイルはsrcディレクトリにあります。

### 基本関数

全ての例は`#include <cton.h>`が必要となってます。

#### CTONコンテキスト

このライブラリ関数を使うため、まずコンテキストを一つ作っておきましょう。

```
cton_ctx *ctx;
ctx = cton_init(NULL);
```

これでCTONコンテキストというものが作りました。コンテキストごとスレッドセーフだけど、一つのコンテキストの中ではこいう保証がありません。また、コンテキストを作るの関数自身はスレッドセーフですが、メモリプールを作る関数はスレッドセーフではありませんので、コンテキストのマルチスレッド呼び出しはお控えでください。

#### CTONオブジェクト

コンテキストが作ったから、オブジェクトを作りましょう。

```
cton_obj *obj;
obj = cton_object_create(ctx, CTON_INT32);
```

このように`CTON_INT32`というデータ形が持ってるオブジェクトが作りました。すべて利用可能のデータ形は以下になります：

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

あるCTONオブジェクトのデータ型を調べるため，この関数を使ってください：

```
cton_type type;
type = cton_object_gettype(obj);
```

注意すべきなのは、CTONライブラリは、データ型の動的変換は**適応していません**、そういう計画もありません。どうしてというと、データ型を変換するとき精度の損が避けられないばあいに、するこのを決める権利を利用者に渡せたいからえです。どうしてもオブジェクトのデータ型を変更したいなら、オブジェクトを改めて作って、元オブジェクトを削除してください。

ところで、もし前作った`CTON_INT32`オブジェクトの値を編集したいなら、これで：

```
#include <stdint.h>

...

int32_t *dat;
dat = cton_object_getptr(obj);
*dat = 42;
```

データポインタをもらう方法で値を編集できます。ただし、この方法は一番乱暴なやり方で、`CTON_HASH`などのデータ型に適応ではないです。（それらのデータ型に対して定義がありますけど）詳しいやり方はAPIのドキュメントを参考することがオススメです。

#### JSONの入出力

入出力モージュルの例として、JSONのモージュルを実装しました。使い方は以下のようです。

```
cton_ctx *ctx;
cton_obj *data;
cton_obj *json;
cton_obj *out;

/* コンテキストを作る */
ctx = cton_init(NULL);

/* test.jsonというファイルから中身を読み込む */
data = cton_util_readfile(ctx, "test.json");

/* JSON文字列をデシリアライズする */
json = cton_json_parse(ctx, data);

/* ここでjsonオブジェクトにやましいことが何をしても構いません */

/* オブジェクトをJSONフォマットに直列化する */
out = cton_json_stringify(ctx, json);

/* 直列化したJSON文字列を出力する */
printf("%s\n", cton_string_getptr(out));
```

## 今後の計画

1. [NBTフォーマット](https://minecraft-ja.gamepedia.com/NBT%E3%83%95%E3%82%A9%E3%83%BC%E3%83%9E%E3%83%83%E3%83%88)に直列化と復元。([@Knighthana](https://github.com/Knighthana) に頼んだが、ドタキャンされるかも)
4. 文字列（xpathみたいなやつ）でオブジェクトを探す？

## コントリビュート

次の種類のPull Requestを受け付けています。Issueを立てずにPull Requestを送ってもらって問題ありません。

* ドキュメントの改善。誤字や表現の誤りの訂正など、小さな修正でも歓迎します。
* モジュールの追加。JSONのようなデータフォーマットを解析するモジュールは歓迎します。
* バグの修正。Issueにないバグでも受け付けます。
* 性能の向上。

Pull Requestはいつでも歓迎しています。

\<End>
___


## ライセンス・软件授权许可证・ЛИЦЕНЗИЯ・LICENSE

### 「ビールウェアライセンス」（改訂43）：

\<yeonji@ieee.org> がこのプロジェクトを書きました。この通知を保持している限り、
このプロジェクトでやりたいことが何でもできます。もし私たちはいつかの日に会い、
そしてあなたはこのプロジェクトはそれだけの価値があると思いましたら、お返しに、
甘口のカレーライスをごちそうさせればと思います（アルコールドリンク飲めないからね）。
Yeonji Lee

### “啤酒软件协议”（第四十三版）：

\<yeonji@ieee.org> 编写了此文件。只要你还保留本协议文本，你
可以使用此软件做任何事。如果咱们在某一天相遇了，而且你认为
此软件有价值，你可以给咱买一份甜味咖喱饭来回报咱（因为咱不能
饮用含酒精的饮料）。Yeonji Lee

### "ЛИЦЕНЗИЯ BEER-WARE" (Версия 43):
\<yeonji@ieee.org> написал этот файл. До тех пор, пока вы сохраняете 
это уведомление, вы можете делать с этими материалами все, что угодно. 
Если мы однажды встретимся, и вы будете считать, что оно того стоит, 
можете в ответную купить мне сладкий Японское карри Yeonji Lee
(потому что я не могу пить алкогольные напитки).

### "THE BEER-WARE LICENSE" (Revision 43):

\<yeonji@ieee.org> create this project. As long as you retain this notice you
can do whatever you want with this stuff. If we meet some day, and you think
this stuff is worth it, you can buy me a sweety curry rice in return (cuz
I cannot drink alcoholic beverages). Yeonji Lee
