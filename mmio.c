#include<err.h>
#include<errno.h>
#include<fcntl.h>
#include<assert.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
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

typedef union mem_value_t{
	uintptr_t un;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;
	size_t size;
	unsigned long long ull;
}mem_value_t;

typedef union mem_ptr_t{
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
}mem_ptr_t;

typedef enum mem_type_t{
	MEM_8  = 8,
	MEM_16 = 16,
	MEM_32 = 32,
	MEM_64 = 64,
}mem_type_t;

typedef struct mem_area_t{
	uintptr_t address;
	size_t length;
}mem_area_t;

#define value_oper(type,left,oper,right) do{switch(type){\
	case MEM_8:(left).u8 oper (right).u8;break;\
	case MEM_16:(left).u16 oper (right).u16;break;\
	case MEM_32:(left).u32 oper (right).u32;break;\
	case MEM_64:(left).u64 oper (right).u64;break;\
}}while(0)

static int usage(int ret){
	fprintf(stderr,
		"Usage: mmio <ACTION>\n\n"
		"Actions:\n"
		"   load <TYPE> <ADDR>              Read data and print hex\n"
		"   store <TYPE> <ADDR> <VAL>       Write data with hex\n"
		"   set <TYPE> <ADDR> <FLAG>        Set bits flag\n"
		"   clr <TYPE> <ADDR> <FLAG>        Clear bits flag\n"
		"   fill <TYPE> <ADDR> <LEN> <VAL>  Fill memory with hex\n"
		"   zero <ADDR> <LEN>               Zero memory\n"
		"   read <ADDR> <LEN> [FILE]        Read data to file or stdout\n"
		"   write <ADDR> <LEN> [FILE]       Write data from file or stdin\n"
		"Arguments:\n"
		"   TYPE:       Data size\n"
		"      b/8       8-bits\n"
		"      s/16      16-bits\n"
		"      l/32      32-bits\n"
		"      q/64      64-bits\n"
		"   ADDR:      Memory address in hex\n"
		"   VAL:       Memory value in hex\n"
		"   FLAG:      Flag value in hex\n"
		"   LEN:       Memory length in bytes\n"
		"   FILE:      Target file path (default stdio)\n"
	);
	return ret;
}

static mem_type_t parse_type(const char*type){
	assert(type!=NULL);
	if(!type[0])errx(1,"Bad operation type");
	if(strcmp(type,"8")==0)return MEM_8;
	if(strcmp(type,"16")==0)return MEM_16;
	if(strcmp(type,"32")==0)return MEM_32;
	if(strcmp(type,"64")==0)return MEM_64;
	if(strcasecmp(type,"b")==0)return MEM_8;
	if(strcasecmp(type,"s")==0)return MEM_16;
	if(strcasecmp(type,"l")==0)return MEM_32;
	if(strcasecmp(type,"q")==0)return MEM_64;
	errx(1,"Unknown operation type: %s",type);
}

static mem_value_t parse_number(const char*value){
	char*end=NULL;
	mem_value_t ret={};
	assert(value!=NULL);
	if(!value[0])errx(1,"Bad number value");
	errno=0;
	ret.ull=strtoull(value,&end,16);
	if(errno!=0||!end||*end)
		errx(1,"Bad number value %s",value);
	return ret;
}

typedef struct mem_aligned_t{
	uintptr_t address;
	uintptr_t offset;
}mem_aligned_t;

static inline mem_aligned_t parse_unaligned(uintptr_t address){
	mem_aligned_t ret={.address=address,.offset=0};
	if(address%PAGE!=0){
		ret.address=round_down(address,PAGE);
		ret.offset=address-ret.address;
	}
	return ret;
}

typedef struct mem_aligned_area_t{
	mem_area_t area;
	uintptr_t offset;
}mem_aligned_area_t;

static inline mem_aligned_area_t parse_unaligned_size(mem_area_t area){
	mem_aligned_t alg=parse_unaligned(area.address);
	return (mem_aligned_area_t){
		.area.address=alg.address,
		.area.length=round_up((area.length+alg.offset),PAGE),
		.offset=alg.offset,
	};
}

static inline mem_ptr_t ptr_off(mem_ptr_t ptr,uintptr_t diff){
	return (mem_ptr_t){.ptr=ptr.ptr+diff};
}

static mem_ptr_t mem_map(mem_area_t area){
	int fd;
	void*map;
	fd=open(DEVICE,O_RDWR|O_SYNC);
	if(fd<0)err(2,"open %s failed",DEVICE);
	map=mmap(NULL,area.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,(off_t)area.address);
	if(map==MAP_FAILED)err(2,"map memory at 0x%"PRIXPTR"x size 0x%zx failed",area.address,area.length);
	close(fd);
	return (mem_ptr_t){.ptr=map};
}

static inline mem_value_t mem_load(const mem_ptr_t ptr,mem_type_t type){
	if(type!=0&&(ptr.val.ull%(type/8))!=0)
		errx(1,"Load address 0x%"PRIxPTR" unaligned to %d",ptr.val.un,(int)type);
	switch(type){
		case MEM_8:return (mem_value_t){.u8=*ptr.u8};
		case MEM_16:return (mem_value_t){.u16=*ptr.u16};
		case MEM_32:return (mem_value_t){.u32=*ptr.u32};
		case MEM_64:return (mem_value_t){.u64=*ptr.u64};
		default:errx(1,"Unsupported memory type %d for load",(int)type);
	}
}

static inline void mem_store(const mem_ptr_t ptr,mem_type_t type,mem_value_t val){
	if(type!=0&&(ptr.val.ull%(type/8))!=0)
		errx(1,"Store address 0x%"PRIxPTR" unaligned to %d",ptr.val.un,(int)type);
	switch(type){
		case MEM_8:*ptr.vu8=val.u8;break;
		case MEM_16:*ptr.vu16=val.u16;break;
		case MEM_32:*ptr.vu32=val.u32;break;
		case MEM_64:*ptr.vu64=val.u64;break;
		default:errx(1,"Unsupported memory type %d for store",(int)type);
	}
}

static inline void print_value(mem_value_t val,mem_type_t type){
	switch(type){
		case MEM_8:printf("0x%02"PRIx8"\n",val.u8);break;
		case MEM_16:printf("0x%04"PRIx16"\n",val.u16);break;
		case MEM_32:printf("0x%08"PRIx32"\n",val.u32);break;
		case MEM_64:printf("0x%016"PRIx64"\n",val.u64);break;
		default:errx(1,"Unsupported memory type %d for print",(int)type);
	}
}

typedef struct mem_map_type_t{
	const char*arg_type;
	const char*arg_addr;
	mem_value_t address;
	mem_type_t type;
	mem_area_t area;
	mem_aligned_area_t aligned;
	mem_ptr_t mapped;
	mem_ptr_t target;
}mem_map_type_t;


static void mem_map_type(mem_map_type_t*mm){
	assert(mm!=NULL);
	assert(mm->arg_type!=NULL);
	assert(mm->arg_addr!=NULL);
	mm->type=parse_type(mm->arg_type);
	mm->address=parse_number(mm->arg_addr);
	mm->area.address=mm->address.un;
	mm->area.length=mm->type/8;
	mm->aligned=parse_unaligned_size(mm->area);
	mm->mapped=mem_map(mm->aligned.area);
	mm->target=ptr_off(mm->mapped,mm->aligned.offset);
}

static inline void mem_unmap_type(mem_map_type_t*mm){
	assert(mm!=NULL);
	munmap(mm->mapped.ptr,mm->area.length);
}

static void cmd_load(int argc,char**argv){
	if(argc!=3)errx(1,"Bad usage for load (please see help)");
	mem_map_type_t map={.arg_type=argv[1],.arg_addr=argv[2]};
	mem_map_type(&map);
	mem_value_t value=mem_load(map.target,map.type);
	mem_unmap_type(&map);
	print_value(value,map.type);
}

static void cmd_store(int argc,char**argv){
	if(argc!=4)errx(1,"Bad usage for store (please see help)");
	mem_map_type_t map={.arg_type=argv[1],.arg_addr=argv[2]};
	mem_map_type(&map);
	mem_value_t value=parse_number(argv[3]);
	mem_store(map.target,map.type,value);
	mem_unmap_type(&map);
}

static void cmd_set(int argc,char**argv){
	if(argc!=4)errx(1,"Bad usage for set (please see help)");
	mem_map_type_t map={.arg_type=argv[1],.arg_addr=argv[2]};
	mem_map_type(&map);
	mem_value_t flag=parse_number(argv[3]);
	mem_value_t value=mem_load(map.target,map.type);
	value_oper(map.type,value,|=,flag);
	mem_store(map.target,map.type,value);
	mem_unmap_type(&map);
}

static void cmd_clr(int argc,char**argv){
	if(argc!=4)errx(1,"Bad usage for clr (please see help)");
	mem_map_type_t map={.arg_type=argv[1],.arg_addr=argv[2]};
	mem_map_type(&map);
	mem_value_t flag=parse_number(argv[3]);
	mem_value_t value=mem_load(map.target,map.type);
	value_oper(map.type,value,&=~,flag);
	mem_store(map.target,map.type,value);
	mem_unmap_type(&map);
}

static void cmd_fill(int argc,char**argv){
	if(argc!=5)errx(1,"Bad usage for fill (please see help)");
	mem_type_t type=parse_type(argv[1]);
	mem_value_t address=parse_number(argv[2]);
	mem_value_t len=parse_number(argv[3]);
	mem_value_t value=parse_number(argv[4]);
	mem_area_t area={.address=address.un,.length=len.size};
	mem_aligned_area_t aligned=parse_unaligned_size(area);
	if((len.ull%(type/8))!=0)errx(1,"Length must aligned to %d",(int)type);
	mem_ptr_t mapped=mem_map(aligned.area);
	mem_ptr_t target=ptr_off(mapped,aligned.offset);
	for(uintptr_t off=0;off<len.un;off+=type/8)
		mem_store(ptr_off(target,off),type,value);
	munmap(mapped.ptr,len.size);
}

static void cmd_zero(int argc,char**argv){
	if(argc!=3)errx(1,"Bad usage for zero (please see help)");
	mem_value_t address=parse_number(argv[1]);
	mem_value_t len=parse_number(argv[2]);
	mem_area_t area={.address=address.un,.length=len.size};
	mem_aligned_area_t aligned=parse_unaligned_size(area);
	mem_ptr_t mapped=mem_map(aligned.area);
	mem_ptr_t target=ptr_off(mapped,aligned.offset);
	memset(target.ptr,0,len.size);
	munmap(mapped.ptr,len.size);
}

static void cmd_readwrite(int argc,char**argv,bool do_write){
	char*action=do_write?"write":"read";
	char*file_action=do_write?"read":"write";
	if(argc<3||argc>4)errx(1,"Bad usage for %s (please see help)",action);
	mem_value_t address=parse_number(argv[1]);
	mem_value_t len=parse_number(argv[2]);
	char*file=argc==3?"-":argv[3];
	mem_area_t area={.address=address.un,.length=len.size};
	mem_aligned_area_t aligned=parse_unaligned_size(area);
	mem_ptr_t mapped=mem_map(aligned.area);
	mem_ptr_t target=ptr_off(mapped,aligned.offset);
	int fd=-1;
	bool want_close=false;
	if(strcmp(file,"-")!=0){
		int flags=do_write?
			O_RDONLY:
			O_WRONLY|O_CREAT|O_TRUNC;
		fd=open(file,flags,0600);
		if(fd<0)err(1,"failed to open %s",file);
		want_close=true;
	}else fd=do_write?STDOUT_FILENO:STDIN_FILENO;
	ssize_t ret=do_write?
		read(fd,target.ptr,len.size):
		write(fd,target.ptr,len.size);
	if(ret<0)err(2,"%s failed",file_action);
	if((size_t)ret!=len.size)errx(
		2,"%s reach EOF (wants %zu bytes, got %zd bytes)",
		file_action,len.size,ret
	);
	if(want_close)close(want_close);
	munmap(mapped.ptr,len.size);
}

static inline void cmd_read(int argc,char**argv){
	cmd_readwrite(argc,argv,false);
}

static inline void cmd_write(int argc,char**argv){
	cmd_readwrite(argc,argv,true);
}

int main(int argc,char**argv){
	if(argc<2)return usage(1);
	#define do_action(act) do{if(strcasecmp(argv[1],#act)==0){\
		cmd_##act(argc-1,argv+1);\
		return 0;\
	}}while(0)
	do_action(load);
	do_action(store);
	do_action(set);
	do_action(clr);
	do_action(fill);
	do_action(zero);
	do_action(read);
	do_action(write);
	if(strcasecmp(argv[1],"help")==0)return usage(0);
	errx(1,"Unknown action %s (please see help)",argv[1]);
}
