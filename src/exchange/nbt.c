
#define NBT_TAG_END        0
#define NBT_TAG_BYTE       1
#define NBT_TAG_SHORT      2
#define NBT_TAG_INT        3
#define NBT_TAG_LONG       4
#define NBT_TAG_FLOAT      5
#define NBT_TAG_DOUBLE     6
#define NBT_TAG_BTYE_ARRAY 7
#define NBT_TAG_STRING     8
#define NBT_TAG_LIST       9
#define NBT_TAG_COMPOUND   10
#define NBT_TAG_INT_ARRAY  11
#define NBT_TAG_LONG_ARRAY 12


cton_obj * cton_nbt_parse(cton_ctx *ctx, cton_obj *nbt);
cton_obj * cton_nbt_serialize(cton_ctx *ctx, cton_obj *obj);
