#include"mmio.h"

static void cmd_readwrite(cmd_ctx*ctx,bool do_write){
	char*action=do_write?"write":"read";
	char*file_action=do_write?"read":"write";
	if(ctx->argc<3||ctx->argc>4)
		errx(1,"Bad usage for %s (please see help)",action);
	mem_value_t address=mem_parse_number(ctx->argv[1]);
	mem_value_t len=mem_parse_number(ctx->argv[2]);
	char*file=ctx->argc==3?"-":ctx->argv[3];
	mem_area_t area={
		.address=address.un,
		.length=len.size,
	};
	mem_aligned_area_t aligned=mem_parse_unaligned_size(area);
	mem_ptr_t mapped=mem_map(ctx->device,aligned.area);
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
	}else{
		fd=do_write?STDIN_FILENO:STDOUT_FILENO;
		if(!do_write&&!ctx->force&&isatty(fd)){
			bool fail=len.size>PAGE;
			fprintf(
				stderr,"%s memory to tty, please use -f to force it\n",
				fail?"ERROR: Refusing to write":"WARNING: Try write"
			);
			if(fail)exit(1);
		}
	}
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

void cmd_read(cmd_ctx*ctx){
	cmd_readwrite(ctx,false);
}

void cmd_write(cmd_ctx*ctx){
	cmd_readwrite(ctx,true);
}
