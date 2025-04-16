#include"mmio.h"

mem_ptr_t mem_map(const char*dev,mem_area_t area){
	int fd;
	void*map;
	fd=open(dev,O_RDWR|O_SYNC);
	if(fd<0)err(2,"open %s failed",dev);
	map=mmap(NULL,area.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,(off_t)area.address);
	if(map==MAP_FAILED)err(2,"map %s memory at 0x%"PRIXPTR"x size 0x%zx failed",dev,area.address,area.length);
	close(fd);
	return (mem_ptr_t){.ptr=map};
}

mem_value_t mem_load(const mem_ptr_t ptr,mem_type_t type){
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

void mem_store(const mem_ptr_t ptr,mem_type_t type,mem_value_t val){
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

void mem_print_value(mem_value_t val,mem_type_t type){
	switch(type){
		case MEM_8:printf("0x%02"PRIx8"\n",val.u8);break;
		case MEM_16:printf("0x%04"PRIx16"\n",val.u16);break;
		case MEM_32:printf("0x%08"PRIx32"\n",val.u32);break;
		case MEM_64:printf("0x%016"PRIx64"\n",val.u64);break;
		default:errx(1,"Unsupported memory type %d for print",(int)type);
	}
}

mem_type_t mem_size_to_type(size_t size){
	switch(size){
		case 1:return MEM_8;
		case 2:return MEM_16;
		case 4:return MEM_32;
		case 8:return MEM_64;
		default:return MEM_INV;
	}
}

void mem_map_type(mem_map_type_t*mm){
	assert(mm!=NULL);
	assert(mm->arg_type!=NULL);
	assert(mm->arg_addr!=NULL);
	if(!mm->device)mm->device=DEVICE;
	mm->type=mem_parse_type(mm->arg_type);
	mm->address=mem_parse_number(mm->arg_addr);
	mm->area.address=mm->address.un;
	mm->area.length=mm->type/8;
	mm->aligned=mem_parse_unaligned_size(mm->area);
	mm->mapped=mem_map(mm->device,mm->aligned.area);
	mm->target=ptr_off(mm->mapped,mm->aligned.offset);
}

void mem_unmap_type(mem_map_type_t*mm){
	assert(mm!=NULL);
	munmap(mm->mapped.ptr,mm->area.length);
}
