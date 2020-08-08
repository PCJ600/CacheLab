#include "cachelab.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#define BUFFER_SIZE 256
#define INVALID_TAG (-1)
#define VALID 0
#define INVALID 1

// 命令行参数
struct Args {
    int s;
    int E;
    int b;
    char traceFile[BUFFER_SIZE];
};

// 保存结果
struct Result {
    int hits;
    int misses;
    int evictions;
};

// 缓存
struct CacheLine {
    int valid;              // 有效位
    int tag;                // 标记位
    int timestamp;          // 时间戳
};

struct CacheSet {
    struct CacheLine *lines;
};

struct Cache {
    struct CacheSet *sets;
    int E;                  // 每个Set的行数
    int S;                  // Set个数
    int s;                  // 2^s = S;
    int b;                  // 块地址位数
};

// -h选项, 打印提示信息
void printHelp()
{
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("  -h         Print this help message.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("Examples:\n");
    printf("    linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("    linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
    exit(0);
}

// getopt解析命令行输入
void parseArgs(int argc, char **argv, struct Args *args) 
{
    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        if (opt == 'h') {
            printHelp();
        } else if (opt == 'v') {
            ;
        } else if (opt == 's') {
            args->s = atoi(optarg);
        } else if (opt == 'E') {
            args->E = atoi(optarg);
        } else if (opt == 'b') {
            args->b = atoi(optarg);
        } else if (opt == 't') {
            strcpy(args->traceFile, optarg);
        } else {
            ;
        }
    }
    printf("[DEBUG][parseArgs]: s = %d, E = %d, b = %d, %s\n", args->s, args->E, args->b, args->traceFile);
}

// 初始化Cache结构
void initCache(struct Cache *cache, struct Args *args) {
    cache->s = args->s;
    cache->S = 1 << cache->s;
    cache->E = args->E;
    cache->b = args->b;
    cache->sets = (struct CacheSet *)malloc(sizeof(struct CacheSet) * cache->S);
    for(int i = 0; i < cache->S; ++i) {
        cache->sets[i].lines = (struct CacheLine *)malloc(sizeof(struct CacheLine) * cache->E);
        for(int j = 0; j < cache->E; ++j) {
            cache->sets[i].lines[j].valid = INVALID;
            cache->sets[i].lines[j].tag = INVALID_TAG;
        }
    }
}

// 释放Cache
void recycleCache(struct Cache *cache) {
    for(int i = 0; i < cache->S; ++i) {
        free(cache->sets[i].lines);
    }
    free(cache->sets);
}

void updateTimeStamp(struct Cache *cache) {
    int numSets = cache->S;
    int numLines = cache->E;
    for(int i = 0; i < numSets; ++i) {
        for(int j = 0; j < numLines; ++j) {
            if(cache->sets[i].lines[j].valid == VALID) {
                ++cache->sets[i].lines[j].timestamp;
            }
        }
    }
}

void update(struct Cache *cache, int64_t addr, struct Result *result) {
    int numLines = cache->E;
    // 标记 + 组索引(s位) + 块偏移(b位)
    int setIndex = (addr >> cache->b) & (cache->S - 1); // 先求s位组索引, 得到addr位于第几组
    int tag = addr >> (cache->s + cache->b);            // 求出tag, 方法为地址右移s+b位

    struct CacheSet *curSet = &cache->sets[setIndex];
    for(int i = 0; i < numLines; ++i) {
        if(curSet->lines[i].tag == tag) {  // tag相等表示命中
            ++result->hits;
            curSet->lines[i].timestamp = 0;
            return;
        }
    }

    // 不命中
    ++result->misses;        
    for(int i = 0; i < numLines; ++i) {
        if(curSet->lines[i].valid == INVALID) { // 找到一个空行
            curSet->lines[i].tag = tag;
            curSet->lines[i].valid = VALID;
            curSet->lines[i].timestamp = 0;
            return;
        }
    }
    // 没有命中又没有空行, 表示冲突不命中
    ++result->evictions;
    int maxIdx = 0;
    int maxTime = curSet->lines[0].timestamp;
    for(int i = 1; i < numLines; ++i) {
        if(curSet->lines[i].timestamp > maxTime) {
            maxTime = curSet->lines[i].timestamp;
            maxIdx = i;
        }
    }
    curSet->lines[maxIdx].tag = tag;
    curSet->lines[maxIdx].timestamp = 0;
}

void updateCache(struct Cache *cache, char ch, int64_t addr, struct Result *result) 
{
    update(cache, addr, result);
    if(ch == 'M') {                 // 相当于一次读取加上一次写入, update两次
        update(cache, addr, result);
    }
    updateTimeStamp(cache);         // 每条指令执行后更新Cache的时间戳
}

void DoParse(struct Cache *cache, char *traceFile, struct Result *result) {
    FILE *fp;
    char ch;
    int size;
    int64_t addr;
    char buf[BUFFER_SIZE];

    fp = fopen(traceFile, "r");
    if(fp == NULL) {
        printf("open traceFile: %s failed!\n", traceFile);
        return;
    }

    while(fgets(buf, BUFFER_SIZE, fp) != NULL) {
        if(buf[0] == 'I') { // 指令cache不做处理
            continue;
        }
        sscanf(buf, " %c %lx,%d\n", &ch, &addr, &size);
        updateCache(cache, ch, addr, result);
    }
    fclose(fp);
}

int main(int argc, char **argv)
{
    struct Args args;
    struct Cache cache;
    struct Result res = {0, 0, 0};

    parseArgs(argc, argv, &args);
    initCache(&cache, &args);
    DoParse(&cache, args.traceFile, &res);
    recycleCache(&cache);
    printSummary(res.hits, res.misses, res.evictions);

    return 0;
}









