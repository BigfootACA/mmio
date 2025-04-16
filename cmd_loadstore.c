#include "mmio.h"

void cmd_load(cmd_ctx*ctx){
	if(ctx->argc!=3)errx(1,"Bad usage for load (please see help)");
	mem_map_type_t map={
		.device=ctx->device,
		.arg_type=ctx->argv[1],
		.arg_addr=ctx->argv[2],
	};
	mem_map_type(&map);
	mem_value_t value=mem_load(map.target,map.type);
	mem_unmap_type(&map);
	mem_print_value(value,map.type);
}

void cmd_store(cmd_ctx*ctx){
	if(ctx->argc!=4)errx(1,"Bad usage for store (please see help)");
	mem_map_type_t map={
		.device=ctx->device,
		.arg_type=ctx->argv[1],
		.arg_addr=ctx->argv[2],
	};
	mem_map_type(&map);
	mem_value_t value=mem_parse_number(ctx->argv[3]);
	mem_store(map.target,map.type,value);
	mem_unmap_type(&map);
}
