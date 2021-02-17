/*******************************************************************************
 * CTON Low level functions
 *
 *   These functions is used by cton as low level functions. some of them is
 * supposed to be offered by standard library, but for compatibily, we create
 * an abstruct of these functions.
 * 
 ******************************************************************************/

#ifndef CTON_LLIB
#define CTON_LLIB 1

/*
 * cton_llib_memcpy()
 *
 * DESCRIPTION
 *   挂名CTON的memcpy套娃，
 *   从src开始复制n个字符到dst.
 *
 * PARAMETER
 *   dst: 被粘贴的内存起始位置
 *   src: 复制源的内存起始位置
 *   n: 长度（字节数）
 *
 * RETURN
 *   void *dst.
*/
static void * cton_llib_memcpy(void *dst, const void *src, size_t n)
{
#ifdef CTON_HAVE_MEMCPY
    return memcpy(dst, src, n);
#else
    while (n > 0) {
    	*dst++ = *src++;
    	n -= 1;
    }
#endif
}

/*
 * cton_llib_memset()
 *
 * DESCRIPTION
 *   挂名CTON的memset套娃，
 *   从b开始的len个字符替换成c.
 *
 * PARAMETER
 *   b: 被粘贴的内存起始位置
 *   c: 一个整数，会被转换成unsigned char
 *   len: 长度（字节数）
 *
 * RETURN
 *   void *b.
 */
static void * cton_llib_memset(void *b, int c, size_t len)
{
    return memset(b, c, len);
}

/*
 * cton_llib_strncmp()
 *
 * DESCRIPTION
 *   挂名CTON的strncmp套娃，
 *   s1和s2前n个字符进行字符串比较
 *
 * PARAMETER
 *   s1: 一个字符串
 *   s2: 另一个字符串
 *   n: 需要比较的长度（字节数）
 *
 * RETURN
 *  当s1<s2时，返回一个负数；
 *	当s1=s2时，返回0；
 *	当s1>s2时，返回一个正数。
 */
static int cton_llib_strncmp(const char *s1, const char *s2, size_t n)
{
    return strncmp(s1, s2, n);
}

/*
 * cton_llib_align()
 *
 * DESCRIPTION
 *   好像是对齐用的东西，
 *   返回需要的空间大小数值
 *
 * PARAMETER
 *   size: 至少需要的空间大小
 *   align: 空间大小为这个参数的整数倍
 *
 * RETURN
 *  返回的数值不小于size，且能被align整除
 */
static size_t cton_llib_align(size_t size, size_t align)
{
    size_t remain;
    size_t aligned;

    remain = size % align;
    aligned = size ^ remain;
    return remain > 0 ? aligned + align : aligned ;
}

#endif /* CTON_LLIB */
