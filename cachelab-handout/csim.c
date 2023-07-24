#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "cachelab.h"

typedef struct
{
    int valid_bit;
    int tag_bit;
    int time_stamp;
} cache_line, *cache_set, **_cache;

int v = -1, t = -1, s = -1, E = -1, b = -1, S;
_cache cache;

void init_cache()
{
    cache = (_cache) malloc(sizeof(cache_set) * S);
    for (int i = 0; i < S; i++)
    {
        cache[i] = (cache_set) malloc(sizeof(cache_line) * E);
        for (int j = 0; j < E; j++)
        {
            cache[i][j].valid_bit = 0;
            cache[i][j].tag_bit = -1;
            cache[i][j].time_stamp = 0;
        }
    }
}

void destroy_cache()
{
    for (int i = 0; i < S; i++)
        free(cache[i]);
    free(cache);
}

void print_help()
{
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
           "Options:\n"
           "  -h         Print this help message.\n"
           "  -v         Optional verbose flag.\n"
           "  -s <num>   Number of set index bits.\n"
           "  -E <num>   Number of lines per set.\n"
           "  -b <num>   Number of block offset bits.\n"
           "  -t <file>  Trace file.\n"
           "\n"
           "Examples:\n"
           "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
           "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int hit_cnt, miss_cnt, eviction_cnt;

#define HIT 1
#define MISS 2

int update(unsigned int address)
{
    unsigned int set_index = (address >> b) & ((1 << s) - 1);
    int tag = address >> (b + s);
    int max_stamp = INT_MIN;
    int max_stamp_index = -1;

    for (int i = 0; i < E; i++)
    {
        if (cache[set_index][i].tag_bit == tag)
        {
            cache[set_index][i].time_stamp = -1;
            ++hit_cnt;
            return HIT;
        }
    }

    for (int i = 0; i < E; i++)
    {
        if (cache[set_index][i].valid_bit == 0)
        {
            cache[set_index][i].valid_bit = 1;
            cache[set_index][i].tag_bit = tag;
            cache[set_index][i].time_stamp = -1;
            ++miss_cnt;
            return MISS;
        }
    }

    ++miss_cnt;
    ++eviction_cnt;

    for (int i = 0; i < E; i++)
    {
        if (cache[set_index][i].time_stamp > max_stamp)
        {
            max_stamp = cache[set_index][i].time_stamp;
            max_stamp_index = i;
        }
    }

    cache[set_index][max_stamp_index].tag_bit = tag;
    cache[set_index][max_stamp_index].time_stamp = -1;
    return MISS;
}

void update_stamp()
{
    for (int i = 0; i < S; i++)
        for (int j = 0; j < E; j++)
            if (cache[i][j].valid_bit == 1)
                cache[i][j].time_stamp++;
}

void print_of_v(int hit_or_miss)
{
    if (v == -1)return;
    if (hit_or_miss == HIT)
        printf(" hit");
    else
        printf(" miss");
}

void parse_trace(FILE *fp)
{
    char operation[2];
    unsigned int address;
    int size;

    while (fscanf(fp, "%s %x,%d", operation, &address, &size) != EOF)
    {
        if (operation[0] == 'I')
        {
            update_stamp();
            continue;
        }
        if (v == 1)
            printf("%c %x,%d", operation[0], address, size);
        print_of_v(update(address));
        if (operation[0] == 'M')
            print_of_v(update(address));
        if (v == 1)
            putchar('\n');
        update_stamp();
    }
    fclose(fp);
}


int main(int argc, char *argv[])
{
    char t_name[128];

    int opt;
    while ((opt = getopt(argc, argv, "hvt:s:E:b:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help();
                exit(0);
            case 'v':
                v = 1;
                break;
            case 't':
                t = 1;
                strcpy(t_name, optarg);
                break;
            case 's':
                s = atoi(optarg);
                S = 1 << s;
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            default:
                print_help();
        }
    }
    if (t == -1 || s == -1 || E == -1 || b == -1)
    {
        printf("Missing required command line argument\n");
        exit(-1);
    }

    FILE *fp = fopen(t_name, "r");
    if (fp == NULL)
    {
        printf("open file error");
        exit(-1);
    }

    init_cache();
    parse_trace(fp);
    destroy_cache();

    printSummary(hit_cnt, miss_cnt, eviction_cnt);
    return 0;
}
