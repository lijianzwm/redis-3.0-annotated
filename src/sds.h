/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * 简单动态字符串（simple dynamic string，SDS）
 * SDS是redis自己构建的一种数据类型，Redis没有直接使用C语言传统的字符串，而是默认使用SDS
 * 在Redis的数据库里面，包含字符串值的键值对在底层都是由SDS实现的（键和值都是SDS）
 *
 *   除了用来保存数据库中的字符串值之外，SDS还被用作缓冲区（buffer）：
 *      AOF模块中的AOF缓冲区
 *      客户端状态中的输入缓冲区
 */

#ifndef __SDS_H
#define __SDS_H

/*
 * 最大预分配长度
 */
#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

typedef char *sds;  //指向 sdshdr 的 buf 属性

/*
 * 保存字符串对象的结构
 */
struct sdshdr {

    int len;    // buf 中已占用空间的长度
    int free;   // buf 中剩余可用空间的长度

    /**
     * SDS遵循C字符串以空字符结尾的惯例，保存空字符的1字节空间不计算在SDS的len属性里面，
     * 并且为空字符分配额外的1字节空间，以及添加空字符到字符串末尾等操作，都是由SDS函数自动完成的，
     * 所以这个空字符对于SDS的使用者来说是完全透明的。
     * 遵循空字符结尾这一惯例的好处是，SDS可以直接重用一部分C字符串函数库里面的函数
     * buf[] -- “R'、'e'、'd'、'i'、's'、'\0'
     */
    char buf[]; //数据空间

};

/**
 * 返回sds实际保存的字符串的长度，T = O(1)
 * 原生C字符串获取字符串长度时，需要依次遍历字符串中的每个值，直到遇到\0为止 T = O(N)，
 * 这里性能上SDS优于原生C字符串
 */
static inline size_t sdslen(const sds s) {
    /**
     * sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)))
     * s是指向sdshdr中buf属性的指针，s的位置减去sizeof(struct sdshdr)就是sdshdr的位置
     * struct sdshdr 结构体中的最后一个 char buf[] 被称为 flexible array member ，
     * 在计算结构体大小的时候是不记入在内的，
     * 因此 sizeof(struct sdshdr) 实际上就是 sizeof(int) + sizeof(int)
     */
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr))); // 通过s计算sdshdr的指针
    return sh->len;
}

/**
 * 返回 sds 可用空间的长度，T = O(1)
 * 跟上面sdslen一个套路，就不解释了
 */
static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->free;
}

/**
 * 下面这些内容在sds.c中详细解释
 */
sds sdsnewlen(const void *init, size_t initlen);
sds sdsnew(const char *init);
sds sdsempty(void);
size_t sdslen(const sds s);
sds sdsdup(const sds s);
void sdsfree(sds s);
size_t sdsavail(const sds s);
sds sdsgrowzero(sds s, size_t len);
sds sdscatlen(sds s, const void *t, size_t len);
sds sdscat(sds s, const char *t);
sds sdscatsds(sds s, const sds t);
sds sdscpylen(sds s, const char *t, size_t len);
sds sdscpy(sds s, const char *t);

sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

sds sdscatfmt(sds s, char const *fmt, ...);
sds sdstrim(sds s, const char *cset);
void sdsrange(sds s, int start, int end);
void sdsupdatelen(sds s);
void sdsclear(sds s);
int sdscmp(const sds s1, const sds s2);
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);
void sdsfreesplitres(sds *tokens, int count);
void sdstolower(sds s);
void sdstoupper(sds s);
sds sdsfromlonglong(long long value);
sds sdscatrepr(sds s, const char *p, size_t len);
sds *sdssplitargs(const char *line, int *argc);
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);
sds sdsjoin(char **argv, int argc, char *sep);

/* Low level functions exposed to the user API */
sds sdsMakeRoomFor(sds s, size_t addlen);
void sdsIncrLen(sds s, int incr);
sds sdsRemoveFreeSpace(sds s);
size_t sdsAllocSize(sds s);

#endif
