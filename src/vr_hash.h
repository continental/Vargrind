#ifndef __HASH_H
#define __HASH_H

typedef enum{
	GLOBAL,
	LOCAL,
    DYNAMIC,
    UNKNOWN
}var_type;


struct _varinfo{
	var_type type;
	char* variable_name;
	unsigned long int address;
	char* declared_at;
	unsigned long int counter;
	unsigned long size;
};

typedef struct _varinfo var_info;

unsigned long hash_then_xor(var_info *data);

#endif