#include"mmio.h"

void cmd_set(cmd_ctx*ctx){
	if(ctx->argc!=4)errx(1,"Bad usage for set (please see help)");
	mem_map_type_t map={.arg_type=ctx->argv[1],.arg_addr=ctx->argv[2]};
	mem_map_type(&map);
	mem_value_t flag=mem_parse_number(ctx->argv[3]);
	mem_value_t value=mem_load(map.target,map.type);
	value_oper(map.type,value,|=,flag);
	mem_store(map.target,map.type,value);
	mem_unmap_type(&map);
}

void cmd_clr(cmd_ctx*ctx){
	if(ctx->argc!=4)errx(1,"Bad usage for clr (please see help)");
	mem_map_type_t map={.arg_type=ctx->argv[1],.arg_addr=ctx->argv[2]};
	mem_map_type(&map);
	mem_value_t flag=mem_parse_number(ctx->argv[3]);
	mem_value_t value=mem_load(map.target,map.type);
	value_oper(map.type,value,&=~,flag);
	mem_store(map.target,map.type,value);
	mem_unmap_type(&map);
}
