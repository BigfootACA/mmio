#include"mmio.h"

void cmd_show(cmd_ctx*ctx){
	if(ctx->argc!=3)errx(1,"Bad usage for show (please see help)");
	mem_value_t address=mem_parse_number(ctx->argv[1]);
	mem_value_t len=mem_parse_number(ctx->argv[2]);
	mem_area_t area={.address=address.un,.length=len.size};
	mem_aligned_area_t aligned=mem_parse_unaligned_size(area);
	mem_ptr_t mapped=mem_map(ctx->device,aligned.area);
	mem_ptr_t target=ptr_off(mapped,aligned.offset);
	mem_dump_with(target.ptr,len.size,&ctx->dump);
	munmap(mapped.ptr,len.size);
}
