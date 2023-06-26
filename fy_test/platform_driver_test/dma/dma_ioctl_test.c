#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
 
#define DRIVER_NAME 			"/dev/dma64"
 
#define MC_DMA_IOC_MAGIC 		'A'
#define MC_DMA_IOCGETCHN		_IO(MC_DMA_IOC_MAGIC, 0)
#define MC_DMA_IOCCFGANDSTART 		_IO(MC_DMA_IOC_MAGIC, 1)
#define MC_DMA_IOCGETSTATUS 		_IO(MC_DMA_IOC_MAGIC, 2)
#define MC_DMA_IOCRELEASECHN 		_IO(MC_DMA_IOC_MAGIC, 3) 
 
#define DMA_STATUS_UNFINISHED		0
#define DMA_STATUS_FINISHED		1
#define DMA_MMAP_LENGTH	  		0x4000
#define DMA_MEMCPY_LEN			0x1000
#define SRC_PATTERN			0x70
#define DST_PATTERN			0x00

#define SRC_ADDR     			 1
#define DST_ADDR     			 2

#define ARRAY_COUNT			 512

struct mc_dma_chncfg {
	/*va 2 pa in user space*/
	unsigned int src_addr[ARRAY_COUNT];
	unsigned int dst_addr[ARRAY_COUNT];
	unsigned int src_len[ARRAY_COUNT];
	unsigned int dst_len[ARRAY_COUNT];
	/*common items*/
	unsigned int scnt;
	unsigned int dcnt;
	unsigned int chn_num;
	unsigned int status;
};
/**
 * mem_addr- tanslate vaddr to paddr
 * @vaddr: virtual address
 * @paddr: physical address
 *
 */
void mem_addr(unsigned int vaddr, unsigned int *paddr)
{
    int pageSize = getpagesize();
    unsigned int v_pageIndex = vaddr / pageSize;
    unsigned int v_offset = v_pageIndex * sizeof(uint64_t);
    unsigned int page_offset = vaddr % pageSize;
    uint64_t item = 0;
    int fd = open("/proc/self/pagemap", O_RDONLY);
	  
    if(fd == -1)
    {
        printf("open /proc/self/pagemap error\n");
        return;
    }

    if(lseek(fd, v_offset, SEEK_SET) == -1)
    {
        printf("sleek error\n");
        return; 
    }

    if(read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t))
    {
        printf("read item error\n");
        return;
    }
    if((((uint64_t)1 << 63) & item) == 0)
    {
        printf("page present is 0\n");
	return ;
    }
    uint64_t phy_pageIndex = (((uint64_t)1 << 55) - 1) & item;
    *paddr = (phy_pageIndex * pageSize) + page_offset;
    close(fd);
}

/**
 *dma_va_tp_phy: to convert a contiguous virtual address
 *		 into a discrete physical address
 * @chncfg: used to store discrete physical addresses and length info
 * @va_addr:virtual address
 * @len: buffer length
 * @flags: source address or dest address
 *
 * return:0 :success
 * 	 -1 :failed  ARRAY_SIZE is not bit enough
 *
 */
int dma_va_to_phy(struct mc_dma_chncfg *chncfg, unsigned int va_addr,unsigned int len,unsigned int flags)
{
	unsigned int i,ret;
	unsigned int seg_len[ARRAY_COUNT];
	unsigned int tmp_len = 0;
	unsigned int addr[ARRAY_COUNT];
	unsigned int pagesize;
	unsigned int first_seg_len = 0;
	unsigned int map_len = 0;
	unsigned int cnt = 0;
	unsigned int tmp_addr = 0;
	unsigned int buf = va_addr;
	pagesize = getpagesize();
	unsigned int pg_off = buf & (pagesize-1);

	while(map_len != len)
	{
		/*caculate translen in every page*/
		if(!first_seg_len)
		{
			tmp_len = pagesize - pg_off;
			if(tmp_len >= len){
				tmp_len = len;	
				first_seg_len = len;
			}
			else
				first_seg_len = tmp_len;
		}else
		{
			if(len - map_len < pagesize)
				tmp_len = len-map_len;
			else
				tmp_len = pagesize;
		}

		/*get the phy addr */
		mem_addr(buf, &tmp_addr);

		if(cnt == 0)
		{	
			addr[cnt] = tmp_addr;
			seg_len[cnt] = tmp_len;
			cnt++;
		}else
		{
			/*caculate the src and seg_len according phy consistence*/
			if((tmp_addr == addr[cnt-1] + first_seg_len) || (tmp_addr == addr[cnt-1] + pagesize))
			{
				seg_len[cnt-1] += tmp_len;			
			}
			else
			{
				if(cnt == ARRAY_COUNT){
					printf("error:The DMA_SG DEMO 's ARRAY SIZE is Too small! ");
					return -1;
				}
				addr[cnt] = tmp_addr;
				seg_len[cnt] = tmp_len;
				cnt++;
			}
		}
		/*find the next va*/	
		va_addr = va_addr + tmp_len;
		buf =va_addr;
		/*update the finish condition*/
		map_len += tmp_len;
	
	}

	if(flags == SRC_ADDR){
		for (i = 0; i < cnt; i++) {
			chncfg->src_addr[i] = addr[i];
			chncfg->src_len[i] = seg_len[i];
		}
			chncfg->scnt = cnt;
	}
	else
	{
		for (i = 0; i < cnt; i++) {
			chncfg->dst_addr[i] = addr[i];
			chncfg->dst_len[i] = seg_len[i];
		}
		chncfg->dcnt = cnt;
	}
	return 0;
}
/*va to pa in user space*/
int main(int argc,char *argv[])
{
	struct mc_dma_chncfg chncfg;
	int fd = -1;
	int ret;
	unsigned int  *src_addr = NULL;
	unsigned int  *dst_addr = NULL;
	unsigned int  *fsrc_addr; 
	unsigned int  *fdst_addr;
	unsigned char  *tmp_addr = NULL;
	unsigned long  paddr_src;
	unsigned long  paddr_dst;
	unsigned int   test_length;
	unsigned int i;
	/*get the test length*/
	test_length = atoi(argv[1]);
	printf("test length %d\r\n",test_length);
	/*malloc src buffer*/
	src_addr =(unsigned int*)malloc(sizeof(char)*test_length);
	if(!src_addr)
		printf("src molloc failed\r\n");
	/*malloc dest buffer*/
	dst_addr = (unsigned int*)malloc(sizeof(char)*test_length);
	if(!dst_addr)
		printf("dst molloc failed\r\n");
	/*prepare for free buffer*/
	fsrc_addr = src_addr;	
	fdst_addr = dst_addr;
	/*init and check src buffer with a pattern*/
	memset(src_addr,0x89,test_length);
	tmp_addr = (unsigned char*)src_addr;	
	for(i = 0;i<test_length;i++)	
	{
		if(*tmp_addr != 0x89)
			printf("src_addr = %s\r\n != 0x89",tmp_addr);
		tmp_addr++;
	}
	/*init and check dest buffer with a pattern*/
	memset(dst_addr,0x0,test_length);
	tmp_addr = (unsigned char*)dst_addr;
	for(i = 0;i<test_length;i++)	
	{
		if(*tmp_addr != 0x00)
			printf("dst_addr = 0x%s\r\n != 0x00",tmp_addr);
		tmp_addr++;
	}

	/* open /dev/dma64 */
	fd = open(DRIVER_NAME, O_RDWR);
	if(fd < 0){
		printf("open %s failed\n", DRIVER_NAME);
		return -1;
	}
	
	/* get a channel */
	ret = ioctl(fd, MC_DMA_IOCGETCHN, &chncfg);
	if(ret){
		printf("ioctl: get channel failed\n");
		goto error;
	}
 
	/* translate src address from virtual address to phsical address*/
       ret =  dma_va_to_phy(&chncfg,(unsigned int)src_addr,test_length,SRC_ADDR);
       if(ret < 0)
		goto error;
	/* translate dest address from virtual address to phsical address*/
       ret =  dma_va_to_phy(&chncfg,(unsigned int)dst_addr,test_length,DST_ADDR);
	if(ret < 0)
		goto error;
	/*start dma transfer */
	ret = ioctl(fd, MC_DMA_IOCCFGANDSTART, &chncfg);
	if(ret){
		printf("ioctl: config and start dma failed\n");
		goto error;
	}
 
	/* wait dma finish */
	while(1){
		ret = ioctl(fd,MC_DMA_IOCGETSTATUS, &chncfg);
		if(ret){
			printf("ioctl: get status failed\n");
			goto error;
		}
		if (DMA_STATUS_FINISHED == chncfg.status){
			break;
		}
		sleep(1);
	}
	/*check the dma transfer result is right or not */
	for(i = 0;i<test_length>>2;i++)	
	{
		if(*src_addr != *dst_addr)
		{
			printf("dst_addr : 0x%x != src_addr=0x%x\r\n",(unsigned int)src_addr,(unsigned int)dst_addr);
			ret = 1;		
		}
		src_addr++;
		dst_addr++;	
	}
//	ret = memcmp(src_addr,dst_addr,DMA_MEMCPY_LEN); 
	if(ret == 0)
		printf("dma copy right\r\n");
	else
		printf("dma copy error\r\n");
	/* release dma channel */
	ret = ioctl(fd, MC_DMA_IOCRELEASECHN, &chncfg);
	if(ret){
		printf("ioctl: release channel failed\n");
		goto error;
	}
	free(fsrc_addr);
	free(fdst_addr);
 
	close(fd);
 
	return 0;
error:
	free(fsrc_addr);
	free(fdst_addr);
	close(fd);
	return -1;
}
