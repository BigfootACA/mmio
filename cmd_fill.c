#include"mmio.h"

void cmd_fill(cmd_ctx*ctx){
	if(ctx->argc!=5)errx(1,"Bad usage for fill (please see help)");
	mem_type_t type=mem_parse_type(ctx->argv[1]);
	mem_value_t address=mem_parse_number(ctx->argv[2]);
	mem_value_t len=mem_parse_number(ctx->argv[3]);
	mem_value_t value=mem_parse_number(ctx->argv[4]);
	mem_area_t area={.address=address.un,.length=len.size};
	mem_aligned_area_t aligned=mem_parse_unaligned_size(area);
	if((len.ull%(type/8))!=0)errx(1,"Length must aligned to %d",(int)type);
	mem_ptr_t mapped=mem_map(ctx->device,aligned.area);
	mem_ptr_t target=ptr_off(mapped,aligned.offset);
	for(uintptr_t off=0;off<len.un;off+=type/8)
		mem_store(ptr_off(target,off),type,value);
	munmap(mapped.ptr,len.size);
}

void cmd_zero(cmd_ctx*ctx){
	if(ctx->argc!=3)errx(1,"Bad usage for zero (please see help)");
	mem_value_t address=mem_parse_number(ctx->argv[1]);
	mem_value_t len=mem_parse_number(ctx->argv[2]);
	mem_area_t area={.address=address.un,.length=len.size};
	mem_aligned_area_t aligned=mem_parse_unaligned_size(area);
	mem_ptr_t mapped=mem_map(ctx->device,aligned.area);
	mem_ptr_t target=ptr_off(mapped,aligned.offset);
	memset(target.ptr,0,len.size);
	munmap(mapped.ptr,len.size);
}
