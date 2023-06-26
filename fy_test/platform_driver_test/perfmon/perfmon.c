#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>

#define DDR_MAX_CH 5

struct perf_info_t {
	unsigned int rtrans_cnt;
	unsigned int rbw_cnt;
	unsigned int rlatency_cnt;
	unsigned int wtrans_cnt;
	unsigned int wbw_cnt;
	unsigned int wlatency_cnt;
	unsigned int rbw_cnt_hig;
	unsigned int wbw_cnt_hig;
	unsigned long long rbw;
	unsigned long long wbw;
};

int main(int argc, char **argv)
{
	struct timeval *stv,*etv;
	unsigned long long start_time, end_time;
	unsigned int sample_it = 0, ms = 1000, ddr_clk;
	double d_time, res_rbw[DDR_MAX_CH],res_wbw[DDR_MAX_CH],res_tbw = 0.0;
	int fd = 0, i;
	char *s = (char*)malloc(256);
	char *e = (char*)malloc(256);
	struct perf_info_t *spi = (struct perf_info_t *)malloc(DDR_MAX_CH * sizeof(struct perf_info_t));
	struct perf_info_t *epi = (struct perf_info_t *)malloc(DDR_MAX_CH * sizeof(struct perf_info_t));

	memset(s,0,256);
	memset(e,0,256);

	if (argc > 1) {
		sample_it = atoi(argv[1]);
		ddr_clk = atoi(argv[2]);
	} else {
		sample_it = 1;
		ddr_clk = 1866;
		printf("Usage:\r\n");
		printf("./perfmon sample_interval_msecs ddr_data_rate\r\n");
		printf("Default:\r\n");
		printf("./perfmon 1 1866\r\n");
	}

	fd = open("/dev/perf",O_RDWR);
	if(fd < 0) {
		printf("open fail!\n");
		return -1;
	}

	read(fd,s,DDR_MAX_CH * 4 * 8 + 8);

	for(i=0;i<DDR_MAX_CH;i++) {
		#if 1
			spi[i].rtrans_cnt   = *(unsigned int*)(s+(i<<5)+0x00);
			spi[i].rbw_cnt      = *(unsigned int*)(s+(i<<5)+0x04);
			spi[i].rlatency_cnt = *(unsigned int*)(s+(i<<5)+0x08);
			spi[i].wtrans_cnt   = *(unsigned int*)(s+(i<<5)+0x0c);
			spi[i].wbw_cnt      = *(unsigned int*)(s+(i<<5)+0x10);
			spi[i].wlatency_cnt = *(unsigned int*)(s+(i<<5)+0x14);
			spi[i].rbw_cnt_hig  = *(unsigned int*)(s+(i<<5)+0x18);
			spi[i].wbw_cnt_hig  = *(unsigned int*)(s+(i<<5)+0x1C);
			spi[i].rbw          = ((unsigned long long)spi[i].rbw_cnt_hig << 32) | spi[i].rbw_cnt;
			spi[i].wbw          = ((unsigned long long)spi[i].wbw_cnt_hig << 32) | spi[i].wbw_cnt;
		#else
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(s+(i<<5)+0x00),spi[i].rtrans_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(s+(i<<5)+0x04),spi[i].rbw_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(s+(i<<5)+0x08),spi[i].rlatency_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(s+(i<<5)+0x0c),spi[i].wtrans_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(s+(i<<5)+0x10),spi[i].wbw_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(s+(i<<5)+0x14),spi[i].wlatency_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(s+(i<<5)+0x18),spi[i].rbw_cnt_hig);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(s+(i<<5)+0x1C),spi[i].wbw_cnt_hig);
			printf("CH[%d] 0x%016llx 0x%016llx\n",i,spi[i].rbw,spi[i].wbw);
		#endif
	}
	stv = (struct timeval*)(s+(6<<5)+0x20);

	usleep(sample_it * ms);

	read(fd,e,DDR_MAX_CH * 4 * 8 + 8);

	for(i=0;i<DDR_MAX_CH;i++) {
		#if 1
			epi[i].rtrans_cnt   = *(unsigned int*)(e+(i<<5)+0x00);
			epi[i].rbw_cnt      = *(unsigned int*)(e+(i<<5)+0x04);
			epi[i].rlatency_cnt = *(unsigned int*)(e+(i<<5)+0x08);
			epi[i].wtrans_cnt   = *(unsigned int*)(e+(i<<5)+0x0c);
			epi[i].wbw_cnt      = *(unsigned int*)(e+(i<<5)+0x10);
			epi[i].wlatency_cnt = *(unsigned int*)(e+(i<<5)+0x14);
			epi[i].rbw_cnt_hig  = *(unsigned int*)(e+(i<<5)+0x18);
			epi[i].wbw_cnt_hig  = *(unsigned int*)(e+(i<<5)+0x1C);
			epi[i].rbw          = ((unsigned long long)epi[i].rbw_cnt_hig << 32) | epi[i].rbw_cnt;
			epi[i].wbw          = ((unsigned long long)epi[i].wbw_cnt_hig << 32) | epi[i].wbw_cnt;
		#else
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(e+(i<<5)+0x00),epi[i].rtrans_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(e+(i<<5)+0x04),epi[i].rbw_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(e+(i<<5)+0x08),epi[i].rlatency_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(e+(i<<5)+0x0c),epi[i].wtrans_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(e+(i<<5)+0x10),epi[i].wbw_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(e+(i<<5)+0x14),epi[i].wlatency_cnt);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(e+(i<<5)+0x18),epi[i].rbw_cnt_hig);
			printf("CH[%d] 0x%08x 0x%08x\n",i,*(unsigned int*)(e+(i<<5)+0x1C),epi[i].wbw_cnt_hig);
			printf("CH[%d] 0x%016llx 0x%016llx\n",i,epi[i].rbw,epi[i].wbw);
		#endif
	}
	etv = (struct timeval*)(e+(6<<5)+0x20);
	//printf("%08x %08x\n",etv->tv_sec,etv->tv_usec);

	start_time = (unsigned long long)stv->tv_sec * 1000000 + stv->tv_usec;
	end_time   = (unsigned long long)etv->tv_sec * 1000000 + etv->tv_usec;
	printf("the start_time is %d \n",start_time);
	printf("the end_time is %d \n",end_time);
	d_time = (end_time - start_time) / 1000000.0;
	printf("interval time is %fs\n",d_time);

	for(i=0;i<DDR_MAX_CH;i++) {
		res_rbw[i] = (epi[i].rbw - spi[i].rbw)/(d_time * 1000000.0);
		res_wbw[i] = (epi[i].wbw - spi[i].wbw)/(d_time * 1000000.0);
		res_tbw += (res_rbw[i] + res_wbw[i]);
		printf("CH[%d] R : %0.2fMB/s\n",i,res_rbw[i]);
		printf("CH[%d] W : %0.2fMB/s\n",i,res_wbw[i]);
		if((epi[i].rlatency_cnt <= spi[i].rlatency_cnt) || (epi[i].rtrans_cnt <= spi[i].rtrans_cnt)) {
			printf("CH[%d] RL: Monitor OF   \n",i);
		} else {
			printf("CH[%d] RL: %0.8fus   \n",i,
							1.0*(epi[i].rlatency_cnt-spi[i].rlatency_cnt)/(epi[i].rtrans_cnt-spi[i].rtrans_cnt)/(1.0*(ddr_clk>>2)));
		}
		if((epi[i].wlatency_cnt <= spi[i].wlatency_cnt) || (epi[i].wtrans_cnt <= spi[i].wtrans_cnt)) {
			printf("CH[%d] WL: Monitor OF   \n",i);
		} else {
			printf("CH[%d] WL: %0.8fus   \n",i,
							1.0*(epi[i].wlatency_cnt-spi[i].wlatency_cnt)/(epi[i].wtrans_cnt-spi[i].wtrans_cnt)/(1.0*(ddr_clk>>2)));
		}
	}

	printf("Total usage   : %0.2fMB/s\n",res_tbw);
	printf("DDR efficiency: %0.2f%%\n",100 * res_tbw/(ddr_clk<<1));

	free(spi);
	free(epi);
	free(s);
	free(e);
	close(fd);
	return 0;
}

