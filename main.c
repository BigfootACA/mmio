#include"mmio.h"
#include <unistd.h>

static int usage(int ret){
	fprintf(stderr,
		"Usage: mmio <ACTION>\n\n"
		"Options:\n"
		"   -d, --device <DEV>              Device to read and write (default to "DEVICE")\n"
		"   -f, --force                     Force write to stdout even if stdout is a tty\n"
		"   -s, --step <STEP>               Bytes length per line in show memory\n"
		"   -F, --dump-unicode-fat          Show memory with fat unicode table\n"
		"   -U, --dump-unicode              Show memory with unicode table (default if stdout is tty)\n"
		"   -A, --dump-ascii                Show memory with ascii table (default if stdout is not tty)\n"
		"   -D, --no-decode-ascii           Do not try decode printable chars in show memory\n"
		"   -H, --no-show-header            Do not print header in show memory\n"
		"   -h, --help                      Show this help\n"
		"Actions:\n"
		"   load <TYPE> <ADDR>              Read data and print hex\n"
		"   store <TYPE> <ADDR> <VAL>       Write data with hex\n"
		"   set <TYPE> <ADDR> <FLAG>        Set bits flag\n"
		"   clr <TYPE> <ADDR> <FLAG>        Clear bits flag\n"
		"   fill <TYPE> <ADDR> <LEN> <VAL>  Fill memory with hex\n"
		"   zero <ADDR> <LEN>               Zero memory\n"
		"   read <ADDR> <LEN> [FILE]        Read data to file or stdout\n"
		"   write <ADDR> <LEN> [FILE]       Write data from file or stdin\n"
		"   show <ADDR> <LEN>               Read data and print\n"
		"   watch <ADDR> <LEN> [HZ]         Watch memory changes\n"
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
		"   HZ:        Watch frequency (default 1000Hz)\n"
	);
	return ret;
}

static int cmd_help(cmd_ctx*ctx){
	return usage(ctx->argc==1?0:1);
}

static char sopts[]="d:f:s:hFUADH";
static struct option lopts[]={
	{"device",            required_argument, NULL, 'd'},
	{"force",             no_argument,       NULL, 'f'},
	{"step",              required_argument, NULL, 's'},
	{"dump-unicode-fat",  no_argument,       NULL, 'F'},
	{"dump-unicode",      no_argument,       NULL, 'U'},
	{"dump-ascii",        no_argument,       NULL, 'A'},
	{"no-decode-ascii",   no_argument,       NULL, 'D'},
	{"no-show-header",    no_argument,       NULL, 'H'},
	{"help",              no_argument,       NULL, 'h'},
	{NULL,0,NULL,0},
};

int main(int argc,char**argv){
	int o;
	cmd_ctx ctx={.device=DEVICE};
	ctx.dump=mem_dump_def;
	ctx.dump.table=isatty(STDOUT_FILENO)?unicode_table_char:ascii_table_char;
	while((o=getopt_long(argc,argv,sopts,lopts,NULL))!=-1)switch(o){
		case 'd':ctx.device=optarg;break;
		case 'f':ctx.force=true;break;
		case 's':ctx.dump.step=parse_number(optarg,0);break;
		case 'F':ctx.dump.table=unicode_fat_table_char;break;
		case 'U':ctx.dump.table=unicode_table_char;break;
		case 'A':ctx.dump.table=ascii_table_char;break;
		case 'D':ctx.dump.print_ascii=false;break;
		case 'H':ctx.dump.show_header=false;break;
		case 'h':return usage(0);
		default: return usage(1);
	}
	if(argc<optind+1)return usage(1);
	ctx.argc=argc-optind;
	ctx.argv=argv+optind;
	#define do_action(act) do{if(strcasecmp(argv[1],#act)==0){\
		cmd_##act(&ctx);\
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
	do_action(show);
	do_action(watch);
	do_action(help);
	errx(1,"Unknown action %s (please see help)",argv[1]);
}
