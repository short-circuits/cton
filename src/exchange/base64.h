
typedef enum {
    CTON_BASE64 = 0,
    CTON_BASE64URL,
    CTON_BASE64_RFC1421,
    CTON_BASE64_RFC2045,
    CTON_BASE64_RFC2152,
    CTON_BASE64_RFC3501,
    CTON_BASE64_RFC3548,
    CTON_BASE64_RFC4868S4,
    CTON_BASE64_RFC4868S5,
    CTON_BASE64_RFC4880
} cton_base64_std;

cton_obj *cton_base64_encode(cton_ctx *ctx, cton_obj* obj, cton_base64_std std);
cton_obj *cton_base64_decode(cton_ctx *ctx, cton_obj* obj);
