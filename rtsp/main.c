/*added in 2020/4/8*/


#include "connet.h"
#include "timer.h"
#include "event.h"
#include "rtsp.h"

#include "shmring.h"
#include "media.h"


#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif 


poolList_t list;

conn_t unixFd;
buf_t unixBuf;
rtpPkg_t rtpPkg;
int search_pkg_head(buf_t buf);
int config_pkg(buf_t buf, size_t *PkgLen);

int unix_write_handle(event_t ev);
//这两个handle可以放在模块层，由具体应用来实现
int server_read_handle(event_t ev)
{
	int r;
	int ret;
	int pkg_type;
	size_t pkg_len;
	netConfig_t netCfg;
	static int currentSunwin = 4;
	struct chnStatus_st chnStatus;
	conn_t c = (conn_t)ev->data;
	buf_t buf=c->readBuf;
	buf_extend(buf, 4096);
	r = c->recv(c,buf->tail,4096);
	if(r<=0){
		if(r==0){
			close_conn(c);
			debug("peer conn closed:%s:%d\n",c->peer_ip,c->peer_port);
		}
		else handle_read_event(ev);
	}
	else{
		buf->size += r;
		buf->tail += r;
		
		//printf("%x,%x,%x \n",buf->head[0],buf->head[1],buf->head[2]);
		
		pkg_type = config_pkg(buf,&pkg_len);
		while(pkg_type>0){
			//printf("%d\n",pkg_len);
			netCfg = (netConfig_t)(buf->head);
			//printf("%d,%d, %s, %s \n",netCfg->magic,netCfg->chn,netCfg->mediaInfo.camAddress,netCfg->mediaInfo.camUrl);
			
			free_old_subwin(netCfg->subwin,currentSunwin);
			currentSunwin = netCfg->subwin;
			//全部重新释放，重新获取流
				
			chnStatus.id = netCfg->chn;
			chnStatus.err = 0;
			chnStatus.update =set_cam_chn(netCfg->chn,netCfg);
			chnStatus.subwin =netCfg->subwin;
			
			//如果配置4画面，那么以下就会配置4次
			send_pkg(unixFd,CFG_SUBWIN,(u_char *)&chnStatus,sizeof(struct chnStatus_st));
			//ring_push(ring,CFG_SUBWIN,(u_char *)&chnStatus,sizeof(struct chnStatus_st));
			buf_consume(buf, pkg_len);
		
			if(1) {
				r = c->send(c,(u_char *)"sucess config",13);
				//if(c->write->ready) printf("write ok \n");
				//else add_event(c->write,WRITE_EVENT);
			}
			pkg_type = config_pkg(buf,&pkg_len);
		}
		
		if( pkg_type == -1){
			printf(">>>>>>>bad config package\r\n");
			ret = search_pkg_head(buf);
			buf_consume(buf, ret);	
		}
		
		if(ev->ready){
			handle_read_event(ev);
		}
	}
	return 0;
}

int server_write_handle(event_t ev)
{
	int n;
	conn_t c = ev->data;
	n = c->send(c,(u_char *)"send again",10);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	return 0;
}
int unix_write_handle(event_t ev)
{
	int n=-1;
	conn_t c = ev->data;
	while(c->write->ready && unixBuf->size){
		n=-1;
		if(unixBuf->size>=4096)	
			n = c->send(c,(u_char *)unixBuf->head,4096);
		else {
			n = c->send(c,(u_char *)unixBuf->head,unixBuf->size);
		}
		if(n>0){
			buf_consume(unixBuf,n);
		}
	}
#if 0
	if(!c->write->ready){
		printf("#");
		fflush(stdout);
	} 
#endif
	return 0;
}


int init_accepted_conn(conn_t c,void *arg)
{
	c->read->handler = server_read_handle;
	c->write->handler = server_write_handle;
	c->data = arg;
	printf("Acceped,client=%s:%d peer=%s:%d\n", c->local_ip,c->local_port,c->peer_ip,c->peer_port);
	add_event(c->read,READ_EVENT);
	
	return 0;
}

static int got_sig_term = 0;
static void on_sig_term(int sig)
{
	got_sig_term = 1;
	printf("term\n\r");
}

int main()
{
	msec64 t,delta;
	int ret;	
	conn_t lc;
//	rtspClient_t rc[16];
	
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
	
	init_conn_queue();
	init_timer();
	init_epoll();
	list = create_pool_list();

	unixBuf=buf_new(1024*1024);
	init_media();
	lc = create_listening(10000);
	lc->ls_handler = init_accepted_conn;
	lc->ls_arg = NULL;//这里利用lc将参数最终传递给所有的c->data


	ret = connect_peer_unix("/tmp/sockUnix",&unixFd);
	unixFd->data = NULL;//传入参数
	if(ret != AIO_OK) goto end;
	unixFd->read->handler = server_read_handle;
	unixFd->write->handler = unix_write_handle;
	add_event(unixFd->read,READ_EVENT);

	rtpPkg = malloc(sizeof(struct rtpPkg_st));
	
#if 0
	rc[0] = init_rtsp_clients(list,"172.16.10.44",554,"admin","fhjt12345","/h264/ch1/main/av_stream");	
	rc[0]->chn = 0;
	rc[1] = init_rtsp_clients(list,"172.16.10.43",554,"admin","fhjt12345","/h264/ch1/main/av_stream");	
	rc[1]->chn = 1;
	rc[2] = init_rtsp_clients(list,"172.16.10.42",554,"admin","fhjt12345","/h264/ch1/main/av_stream");	
	rc[2]->chn = 2;
	rc[3] = init_rtsp_clients(list,"172.16.10.44",554,"admin","fhjt12345","/h264/ch1/main/av_stream");	
	rc[3]->chn = 3;

	rc[1] = init_rtsp_clients(list,"192.168.0.5",554,"admin","fhjt123456","/h264/ch1/main/av_stream");	
	rc[1]->chn = 1;
	rc[2] = init_rtsp_clients(list,"192.168.0.6",554,"admin","fhjt123456","/h264/ch1/main/av_stream");	
	rc[2]->chn = 2;
	rc[3] = init_rtsp_clients(list,"192.168.0.7",554,"admin","fhjt123456","/h264/ch1/main/av_stream");	
	rc[3]->chn = 3;


	rc[0] = init_rtsp_clients(list,"172.16.10.41",554,"root","fhjt12345","/axis-media/media.amp?videocodec=h264&resolution=1280x720&fps=25");	
	rc[0]->chn=0;
	rc[1] = init_rtsp_clients(list,"172.16.10.44",554,"admin","fhjt12345","/h264/ch1/main/av_stream");	
	rc[1]->chn = 1;

	rc[2] = init_rtsp_clients(list,"172.16.10.40",554,"root","fhjt12345","/axis-media/media.amp?videocodec=h264&resolution=1280x720&fps=25");	
	rc[2]->chn=2;
#endif 
	while(!got_sig_term)
	{
		t = find_timer();
		process_events(t,1);
		if(get_current_ms() -delta) {
			expire_timers();
			delta = get_current_ms();
		}
	}
end:
	free(rtpPkg);
	free_media();
	free_all_conn();
	free_pool_list(list);
	free_timer();
	free_epoll();
	buf_free(unixBuf);
	return 0;
}


int config_pkg(buf_t buf, size_t *PkgLen)
{
	size_t size = buf->size;
	
	*PkgLen =0;
	netConfig_t netCfg = (netConfig_t)(buf->head);
	
	if(size < sizeof(struct netConfig_st))
		return 0;
	 
	if(netCfg->magic == PKG_MAGIC){
		// this is an rtp packet.
		*PkgLen =  sizeof(struct netConfig_st);
		return 1;
	}
	else return -1;

}

int search_pkg_head(buf_t buf)
{
	u_char *data = buf->head;
	size_t size = buf->size;
	size_t i =0;
	
	for(i=0;i<size-2;i++)
	{
		if((data[i] == 0x56) && (data[i+1] == 0x34) && (data[i+2] == 0x12))
		return i;
	}
	
	return i;
}



