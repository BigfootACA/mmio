#include"mmio.h"

mem_type_t mem_parse_type(const char*type){
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

unsigned long long parse_number(const char*value,int base){
	char*end=NULL;
	unsigned long long ret=0;
	assert(value!=NULL);
	if(!value[0])errx(1,"Bad number value");
	errno=0;
	ret=strtoull(value,&end,base);
	if(errno!=0||!end||*end)
		errx(1,"Bad number value %s",value);
	return ret;
}

mem_value_t mem_parse_number(const char*value){
	mem_value_t ret;
	ret.ull=parse_number(value,0);
	return ret;
}

mem_aligned_t mem_parse_unaligned(uintptr_t address){
	mem_aligned_t ret={.address=address,.offset=0};
	if(address%PAGE!=0){
		ret.address=round_down(address,PAGE);
		ret.offset=address-ret.address;
	}
	return ret;
}

mem_aligned_area_t mem_parse_unaligned_size(mem_area_t area){
	mem_aligned_t alg=mem_parse_unaligned(area.address);
	return (mem_aligned_area_t){
		.area.address=alg.address,
		.area.length=round_up((area.length+alg.offset),PAGE),
		.offset=alg.offset,
	};
}
