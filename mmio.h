#ifndef MMIO_H
#define MMIO_H
#include<err.h>
#include<errno.h>
#include<ctype.h>
#include<fcntl.h>
#include<getopt.h>
#include<assert.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdarg.h>
#include<stdbool.h>
#include<inttypes.h>
#include<sys/mman.h>

#ifndef PAGE
#define PAGE 4096
#endif
#ifndef DEVICE
#define DEVICE "/dev/mem"
#endif
#ifndef MIN
#define MIN(a,b)((b)>(a)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b)((b)<(a)?(a):(b))
#endif
#define round_up(val,alg) (((val)+(alg)-1)&~((alg)-1))
#define round_down(val,alg) ((val)&~((alg)-1))

typedef union mem_value_t mem_value_t;
typedef union mem_ptr_t mem_ptr_t;
typedef struct mem_area_t mem_area_t;
typedef struct mem_aligned_t mem_aligned_t;
typedef struct mem_aligned_area_t mem_aligned_area_t;
typedef struct mem_map_type_t mem_map_type_t;
typedef struct mem_dump mem_dump;
typedef struct cmd_ctx cmd_ctx;

typedef enum mem_type_t{
	MEM_INV = 0,
	MEM_8   = 8,
	MEM_16  = 16,
	MEM_32  = 32,
	MEM_64  = 64,
}mem_type_t;

union mem_value_t{
	uintptr_t un;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;
	size_t size;
	unsigned long long ull;
};

union mem_ptr_t{
	volatile uint8_t*vu8;
	volatile uint16_t*vu16;
	volatile uint32_t*vu32;
	volatile uint64_t*vu64;
	volatile void*vptr;
	uint8_t*u8;
	uint16_t*u16;
	uint32_t*u32;
	uint64_t*u64;
	void*ptr;
	mem_value_t val;
};

struct mem_area_t{
	uintptr_t address;
	size_t length;
};

struct mem_aligned_t{
	uintptr_t address;
	uintptr_t offset;
};

struct mem_aligned_area_t{
	mem_area_t area;
	uintptr_t offset;
};

struct mem_map_type_t{
	const char*device;
	const char*arg_type;
	const char*arg_addr;
	mem_value_t address;
	mem_type_t type;
	mem_area_t area;
	mem_aligned_area_t aligned;
	mem_ptr_t mapped;
	mem_ptr_t target;
};

struct mem_dump{
	uint8_t step;
	uint8_t addr_len;
	bool show_header;
	bool print_ascii;
	bool real_address;
	bool buffered;
	const char**table;
	void*data;
	char*buffer;
	size_t buf_size,buf_pos;
	int(*printf)(mem_dump*d,const char*fmt,...);
	int(*putchar)(mem_dump*d,char ch);
	int(*print)(mem_dump*d,const char*str);
	int(*write)(mem_dump*d,const char*str,size_t len);
	void(*finish)(mem_dump*d);
};

struct cmd_ctx{
	int argc;
	char**argv;
	const char*device;
	bool force;
	mem_dump dump;
};

#define value_oper(type,left,oper,right) do{switch(type){\
	case MEM_8:(left).u8 oper (right).u8;break;\
	case MEM_16:(left).u16 oper (right).u16;break;\
	case MEM_32:(left).u32 oper (right).u32;break;\
	case MEM_64:(left).u64 oper (right).u64;break;\
	default:assert(false);\
}}while(0)
extern const char*unicode_fat_table_char[];
extern const char*unicode_table_char[];
extern const char*ascii_table_char[];
extern mem_dump mem_dump_def;
extern void mem_dump_with(void*addr,size_t len,mem_dump*dump);
extern unsigned long long parse_number(const char*value,int base);
extern mem_type_t mem_parse_type(const char*type);
extern mem_value_t mem_parse_number(const char*value);
extern mem_aligned_t mem_parse_unaligned(uintptr_t address);
extern mem_aligned_area_t mem_parse_unaligned_size(mem_area_t area);
extern mem_ptr_t mem_map(const char*dev,mem_area_t area);
extern mem_value_t mem_load(const mem_ptr_t ptr,mem_type_t type);
extern void mem_store(const mem_ptr_t ptr,mem_type_t type,mem_value_t val);
extern void mem_print_value(mem_value_t val,mem_type_t type);
extern mem_type_t mem_size_to_type(size_t size);
extern void mem_map_type(mem_map_type_t*mm);
extern void mem_unmap_type(mem_map_type_t*mm);
extern void cmd_load(cmd_ctx*ctx);
extern void cmd_store(cmd_ctx*ctx);
extern void cmd_set(cmd_ctx*ctx);
extern void cmd_clr(cmd_ctx*ctx);
extern void cmd_fill(cmd_ctx*ctx);
extern void cmd_zero(cmd_ctx*ctx);
extern void cmd_read(cmd_ctx*ctx);
extern void cmd_write(cmd_ctx*ctx);
extern void cmd_show(cmd_ctx*ctx);
extern void cmd_watch(cmd_ctx*ctx);
static inline mem_ptr_t ptr_off(mem_ptr_t ptr,uintptr_t diff){
	return (mem_ptr_t){.val={.un=ptr.val.un+diff}};
}
#endif
