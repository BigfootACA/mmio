#include"mmio.h"

void cmd_watch(cmd_ctx*ctx){
	if(ctx->argc<3||ctx->argc>4)errx(1,"Bad usage for watch (please see help)");
	size_t hz=ctx->argc==4?parse_number(ctx->argv[3],10):1000;
	mem_value_t address=mem_parse_number(ctx->argv[1]);
	mem_value_t len=mem_parse_number(ctx->argv[2]);
	mem_area_t area={.address=address.un,.length=len.size};
	mem_aligned_area_t aligned=mem_parse_unaligned_size(area);
	mem_ptr_t mapped=mem_map(ctx->device,aligned.area);
	mem_ptr_t target=ptr_off(mapped,aligned.offset);
	mem_type_t type=mem_size_to_type(len.size);
	void*curr=malloc(len.size);
	if(!curr)err(2,"malloc %zu bytes failed",len.size);
	memcpy(curr,target.ptr,len.size);
	printf("Watch 0x%llx length 0x%llx with %zuHz\n",address.ull,len.ull,hz);
	size_t period=1000000/hz;
	while(true){
		if(memcmp(curr,target.ptr,len.size)!=0){
			if(type!=MEM_INV)mem_print_value(mem_load(target,type),type);
			else mem_dump_with(target.ptr,len.size,&ctx->dump);
			memcpy(curr,target.ptr,len.size);
		}
		if(period>0)usleep(period);
	}
	free(curr);
	munmap(mapped.ptr,len.size);
}
