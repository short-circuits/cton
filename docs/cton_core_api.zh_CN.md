### CTON HASH表（散列表）

#### 定义

CTOM HASH表是CTON提供的一种数据结构。

是由一些键(key)和值(value)组成的表，每个键对应一个值，就像

{
	key:value
}

#### 需要注意的地方

1. 不允许多个一样的key
2. 不允许某个value没有key对应
3. 不允许某个key没有对应value
4. key单向映射value
5. 不允许通过value查找key

----

### cton_hash_set

`cton_obj * cton_hash_set(cton_obj *h, cton_obj *k, cton_obj *v);`

- 在hash表里设定值

#### 参数

- h: CTON哈希表
- k: CTON HASH key
- v: CTON HASH value.

#### 返回值

成功返回v，失败返回NULL.

#### 功能描述

若全部参数类型有一个不正确将直接返回失败

若表里有相应的key，若v为空将会删除这个key并释放内存，反之则把v赋值给key对应的值

若表里没有相应的key，则会申请内存并在表里插入项目，若申请失败则直接返回失败，反之成功