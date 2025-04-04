#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <time.h>
#include <sys/socket.h>
#include <fcntl.h> 
#include <pthread.h> 

#include "decoder.h"
#include "stream.h"
#include "osal.h"
#include "vo.h"
#include "sys.h"
#include "vgs.h"
#include "alltasks.h"

#include "connet.h"
#include <sqlite3.h>
#undef  _DEBUG
//#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif 

void pkgProcess(u_char *rtpPkg,int);
FY_S32 decode_h264(int accessable);
void *_osalLoop(void  *arg);
static int got_sig_term = 0;
decEnv_t  decEnv;
#ifdef FILE_RECORD_EN
	FILE *fp;
#endif
int dec_type=0;//0--h264 1-- h265

static void on_sig_term(int sig)
{
	got_sig_term = 1;
	printf("term\n\r");
}

int server_read_handle(event_t ev)
{
	int r;
	
	conn_t c = (conn_t)ev->data;
	//threadPool_t tp = (threadPool_t)c->data;//lc->ls_arg
	buf_t buf=c->readBuf;
	do{
		buf_extend(buf, 4096);
		r = c->recv(c,buf->tail,4096);
		if(r<=0){
			if(r==0){
				close_conn(c);
				printf("peer conn closed:%s:%d\n",c->peer_ip,c->peer_port);
			}
			else handle_read_event(ev);
		}
		else{
			buf->size += r;
			buf->tail += r;
			
			//printf("recv>>%d\n",r);
			while(buf->size >= sizeof(struct rtpPkg_st)){
				if(strncmp((char *)buf->head,"nihaoya",7)!=0){
					printf("wrong head \r\n");
				}
				pkgProcess((u_char *)buf->head,dec_type);
				buf_consume(buf, sizeof(struct rtpPkg_st));
			}
			if(ev->ready){
				//printf("ready\n");
				//handle_read_event(ev);//此句注销后，必须用while循环将缓冲区的数据读完，这样好处就是  减少了event触发次数。
			}
		}
	}while(ev->ready);
	
	//handle_read_event(ev);//此句只有在缓存数据没读完才会触发，以上的循环已经把数据读完了
	return 0;
}

int server_write_handle(event_t ev)
{
	int n;
	conn_t c =(conn_t) ev->data;
	n = c->send(c,(u_char *)"send again",10);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	return 0;
}


int init_accepted_conn(conn_t c,void *arg)
{
	c->read->handler = server_read_handle;
	c->write->handler = server_write_handle;
	c->data = arg;//lc->ls_arg
	printf("Acceped,client=%s:%d peer=%s:%d\n", c->local_ip,c->local_port,c->peer_ip,c->peer_port);
	add_event(c->read,READ_EVENT);
	
	return 0;
}
int main()
{  
	conn_t lc ;
	msec64 t,delta=0;
	pthread_t osal_worker;
	sqlite3 * db = NULL; //声明sqlite关键结构指针
	char * errmsg = NULL;
	sqlite3_stmt *stmt = NULL;
	char sql[128];
	int result;
	
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
		
#ifdef FILE_RECORD_EN
	fp =fopen("./hwp.265","wb");
#endif
	init_conn_queue();
	init_timer();
	init_epoll();

	
	lc = create_listening_unix("/tmp/sockUnix");
	lc->ls_handler = init_accepted_conn;
	lc->ls_arg = NULL;//这里利用lc将参数最终传递给所有的c->data
	
	result = sqlite3_open( "/mnt/usr/ss.db",&db );

	if( result != SQLITE_OK )
	{
		printf("can not open the database %s\r\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	sprintf(sql,"select * from controller");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d h_type: %d, mutx4_type:%d,  \n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_int(stmt, 1),\
				sqlite3_column_int(stmt, 2));
		dec_type= sqlite3_column_int(stmt, 1);
	}
	sqlite3_finalize(stmt);
	sqlite3_close( db );

	decode_h264(dec_type);
	decEnv = create_dec_chns();

	osalInitEnv();
	pthread_create(&osal_worker, NULL, _osalLoop, NULL);
	osalAddTask(osal_printf,OSALSTART);
	osalAddTask(dec_reset_handler,OSALSTART);
	osalAddTask(check_left_freams,OSALSTART);
	
			
    while(!got_sig_term)
    {             		
		t = find_timer();
		process_events(t,1);
		if(get_current_ms() -delta) {
			expire_timers();
			delta = get_current_ms();
		}
    } 

	/*
		memset(&stStream, 0, sizeof(VDEC_STREAM_S) );
		stStream.bEndOfStream = HI_TRUE;
		HI_MPI_VDEC_SendStream(i, &stStream, -1);
	*/
#ifdef FILE_RECORD_EN
	if(fp){
		printf("close\r\n");
		fclose(fp);
	}
#endif
	close_conn(lc);
	free_all_conn();
	free_timer();
	free_epoll();
	pthread_join(osal_worker,NULL);
	free_dec_chns(decEnv);
	free(decEnv);

    vdec_stop(CHNS);
    vdec_vgs_unbind_vo_layer(CHNS,FY_VO_LAYER_VHD0,0);
    vdec_vgs_deinit(CHNS);
    vdec_vo_deinit_layer(VO_MODE_4MUX,FY_VO_LAYER_VHD0,0,VDEC_CHN_NUM_4);

    vo_deinit(FY_TRUE);
    FY_MPI_VB_ExitModCommPool(VB_UID_VDEC);

    //sample_aio_deinit();

    //SAMPLE_VDA_MdStopAll();
    //SAMPLE_VDA_CdStopAll();
    sys_exit();

	
    exit(0);
}

void *_osalLoop(void  *arg)
{
	while(!got_sig_term){
		osalRunSystem();
		usleep(100000);
	}
	return 0;
}
