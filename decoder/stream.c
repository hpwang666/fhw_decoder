#include "stream.h"
#include "decoder.h"
#include "common.h"
#include <time.h>


#ifdef FILE_RECORD_EN
	extern FILE *fp;
	int fileCnt=0;
#endif
int process_rtp(u_char *bufIN, size_t inLen, int chn,decoder_t dec,int dec_type)
{
	int rtpType;	
	make_seqc_right( bufIN, inLen,  chn,dec);
	if(dec_type==1)
		rtpType = UnpackRTPH265(bufIN,inLen,dec);
	else 
		rtpType = UnpackRTPH264(bufIN,inLen,dec);
	if(rtpType >0)
		do_decode(dec,chn, rtpType);
	return 0;
}

#define DEC_TIMESTAMP  (dec->time40ms)
int do_decode(decoder_t dec,int chn,int rtpType)
{
	int ret;
	if(rtpType == RTP_ERR){
		dec_buf_init(dec->buf);
		dec->PKG_STARTED = 0;
		return 0;
	}
	//	printf("size:%d\r\n",dec->buf->size);
#ifdef FILE_RECORD_EN
	if(fileCnt>100){
		if(fp){
			printf("close\r\n");
			fclose(fp);
		}
		fp=NULL;
	}
	else{
		if(dec->EN_SPS==0 ||rtpType != RTP_SPS)
			fwrite(dec->buf->head,1,dec->buf->size,fp);
		fflush(fp);
	}
#endif
	//以下实际上第二次来PPP的时候并没有删除，而是和后面的I帧一起送给了解码器
	//如果分开送，就会出现解码延时等问题，很奇怪
	if((rtpType &RTP_SPS) && (dec->EN_SPS == 0)){//SPS PPS		
		dec->decStream.u64PTS   =DEC_TIMESTAMP;
		dec->decStream.pu8Addr =  dec->buf->head;
		dec->decStream.u32Len  =  dec->buf->size; 
		dec->decStream.bEndOfFrame  = FY_TRUE;
		dec->decStream.bEndOfStream = FY_FALSE;  
		
		if(dec->refused == 0)	{
			ret=FY_MPI_VDEC_SendStream(chn, &(dec->decStream), 0);
			if(ret !=FY_SUCCESS) printf("%d:may be dec is busy：%x\r\n",chn,ret);
			dec->EN_SPS =1;	
		}
		
		dec_buf_init(dec->buf);
		dec->PKG_STARTED = 0;
	}
	else if(RTP_P & rtpType){	
		dec->decStream.u64PTS =DEC_TIMESTAMP;//由VO模块进行帧率控制
		dec->decStream.pu8Addr = dec->buf->head;
		dec->decStream.u32Len  = dec->buf->size; 
		dec->decStream.bEndOfFrame  = FY_TRUE;
		dec->decStream.bEndOfStream = FY_FALSE;  

		//下面必须保证重新开始接收解码之前要有I帧先解码，这样就不会出现花屏
		if(dec->refused == 0 && dec->waitIfream==0)	{
			ret=FY_MPI_VDEC_SendStream(chn, &(dec->decStream), 0);
			if(ret !=FY_SUCCESS) printf("%d:may be dec is busy：%x\r\n",chn,ret);
		}
		if(dec->refused){
			dec->waitIfream = 1;
		}		
		dec_buf_init(dec->buf);
		dec->PKG_STARTED = 0;
	}
	else if(RTP_I & rtpType){	
		dec->decStream.u64PTS  =DEC_TIMESTAMP;//由VO模块进行帧率控制
		dec->decStream.pu8Addr = dec->buf->head;
		dec->decStream.u32Len  = dec->buf->size; 
		dec->decStream.bEndOfFrame  = FY_TRUE;
		dec->decStream.bEndOfStream = FY_FALSE;  
			
		if(dec->refused == 0)	{
			ret=FY_MPI_VDEC_SendStream(chn, &(dec->decStream), 0);
			if(ret !=FY_SUCCESS) printf("%d:may be dec is busy：%x\r\n",chn,ret);
			dec->waitIfream =0;
		}
		else{
			dec->waitIfream = 1;
		} 
		
		dec_buf_init(dec->buf);
		dec->PKG_STARTED = 0;
	}	
	else if(RTP_PKG & rtpType){	
		dec->decStream.u64PTS  =DEC_TIMESTAMP;//由VO模块进行帧率控制
		dec->decStream.pu8Addr = dec->buf->head;
		dec->decStream.u32Len  = dec->buf->size; 
		dec->decStream.bEndOfFrame  = FY_TRUE;
		dec->decStream.bEndOfStream = FY_FALSE;  
			
		if(dec->refused == 0)	{
			ret=FY_MPI_VDEC_SendStream(chn, &(dec->decStream), 0);
			if(ret !=FY_SUCCESS) printf("%d:may be dec is busy：%x\r\n",chn,ret);
		}
		dec_buf_init(dec->buf);
		dec->PKG_STARTED = 0;
	}	
	
	return 0;
}
/*
   The FU indicator octet has the following format:

      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |F|NRI|  Type   |
      +---------------+

   The FU header has the following format:

      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |S|E|R|  Type   |
      +---------------+
*/	  
#define RTP_HEADLEN 16
int UnpackRTPH264( u_char *bufIN, size_t len, decoder_t dec)
{
	size_t outLen = 0 ;
	dec_buf_t bufOUT;
	if (len < RTP_HEADLEN) return -1 ;
	
	//static time_t now,old;
	u_char *pBufTmp = NULL;
	u_char * src = bufIN + RTP_HEADLEN;
	u_char head1 = * src; // 获取第一个字节
	u_char head2 = * (src + 1); // 获取第二个字节
	u_char nal = head1 & 0x1f ; // 获取FU indicator的类型域，
	u_char flag = head2 & 0xe0 ; // 获取FU header的前三位，判断当前是分包的开始、中间或结束
	u_char nal_fua = (head1 & 0xe0 ) | (head2 & 0x1f ); // FU_A nal  海康的只有I帧  和  P帧
	
	int bFinishFrame = 0 ;

	
	bufOUT = dec->buf;
	//printf("%x ",nal);
	if (nal == 0x1c ){  // 判断NAL的类型为0x1c=28，说明是FU-A分片
		if (flag == 0x80 ){ // 开始
			pBufTmp = src - 3 ;
			*(( int * )(  pBufTmp)) = 0x01000000 ;  
			* (( u_char * )( pBufTmp) + 4 ) = nal_fua;
			//printf(">>%x\n",nal_fua);
			//+---------------+
			//|0|1|2|3|4|5|6|7|
			//+-+-+-+-+-+-+-+-+
			//|F|NRI| Type    |
			//+---------------+
			//00 00 00 01 67    (SPS)
			//00 00 00 01 68    (PPS)
			//00 00 00 01 65    ( IDR 帧)
			//00 00 00 01 61    (P帧)
			//00 00 00 01 41    (P帧) 只是重要级别NRI和61不一样
			//正常的是应该只读Type来判断帧类型，此处和NRI合为一体了
			outLen = len - RTP_HEADLEN + 3 ;
			dec->PKG_STARTED =1;
			//printf("%x:%d \n",flag,seqc);
		}
		else if (flag == 0x40){ // 结束
			pBufTmp = src + 2 ;
			outLen = len - RTP_HEADLEN - 2 ;
			if(nal_fua == 0x65) bFinishFrame =RTP_I;
			else  bFinishFrame = RTP_P;
			if(dec->PKG_STARTED == 0){
				bFinishFrame=0;
				dec_buf_init(dec->buf);
				printf("bad buf \n");
				return bFinishFrame;
			} 
		}
		else{ // 中间
			pBufTmp = src + 2 ;
			outLen = len - RTP_HEADLEN - 2 ;
		}
		dec_buf_append(bufOUT,pBufTmp,outLen);
	}
	else /*if(nal <=23 && nal >=1)*/ { // 单包数据  // 所有的包都会拆包，除了 NALU 7,8,6 序列结束，码流结束，分界符
		 pBufTmp = src - 4 ;
		*(( int * )(pBufTmp)) = 0x01000000 ; 
		outLen = len - RTP_HEADLEN + 4 ;
		dec_buf_append(bufOUT,pBufTmp,outLen);
		switch(nal){
			case 1: //signel pkg
			case 7: // sequence parameter set (SPS)
			case 8: // picture parameter set (PPS)
				bFinishFrame=RTP_PKG;//单包数据
				break;
			default: 
				bFinishFrame=RTP_PKG;//单包数据
		}
	}
		
	return bFinishFrame;
}


int UnpackRTPH265( u_char *bufIN, size_t len, decoder_t dec)
{
	int outLen = 0 ;
	u_char startCode[4]={0x00,0x00,0x00,0x01};
	dec_buf_t bufOUT;
	if (len < RTP_HEADLEN+1) {
		printf("len is too small \r\n");

		
		return 0;
	}
	
	u_char *pBufTmp = NULL;
	u_char * src = bufIN + RTP_HEADLEN;
	u_char head1 = * src; // 获取第一个字节
	u_char head2 = * (src + 2); // 获取第3个字节
	u_char nal = ((head1>>1) & 0x3f)	; // 
	u_char flag = head2  ; // 获取FU header的前三位，判断当前是分包的开始、中间或结束
	u_char nal_fua =  (head2 & 0x3f ); // FU_A nal  海康的只有I帧  和  P帧
	

	u_char nalu_header0 = (u_char) (nal_fua<<1)|(*src &0x81);
	u_char nalu_header1 = *(src+1);	  
	int bFinishFrame = 0 ;

	
	bufOUT = dec->buf;
	//printf("%x %x\r\n",nal,nal_fua);
	if(((head1 << 5) & 0x20) | ((*(src+1) >> 3) & 0x1f)){
		printf("skip \r\n");
		return 0;
	}
	if((*(src +1)&(0x07))==0){
		printf("not good \r\n");
		return 0;
	}
	if (nal == 48) printf("this is 48\r\n");
	if (nal == 49 ){  // 判断NAL的类型为0x1c=28，说明是FU-A分片
		if (flag & 0x80 ){ // 开始
			pBufTmp = src - 3 ;
			memcpy(pBufTmp,startCode,4);
			* (( u_char * )( pBufTmp) + 4 ) = nalu_header0;
			* (( u_char * )( pBufTmp) + 5 ) = nalu_header1;

			outLen = len - RTP_HEADLEN +2+4-3 ;
			dec->PKG_STARTED =1;
			//printf("%x:%d \n",flag,seqc);
		}
		else if (flag & 0x40){ // 结束
			pBufTmp = src + 3 ;
			outLen = len - RTP_HEADLEN - 3 ;
			if(nal_fua == 0x13) bFinishFrame =RTP_I;
			else  {
				bFinishFrame = RTP_P;
			}
			if(dec->PKG_STARTED == 0){
				bFinishFrame=0;
				dec_buf_init(dec->buf);
				printf("bad buf \n");
				return bFinishFrame;
			} 
#ifdef FILE_RECORD_EN
			fileCnt++;
#endif
		}
		else{ // 中间
			pBufTmp = src + 3;
			outLen = len - RTP_HEADLEN - 3 ;
		}
		dec_buf_append(bufOUT,pBufTmp,outLen);
	}
	else  { // 单包数据  
		//printf("nal == %d\r\n",nal);
		pBufTmp = src - 4 ;
		memcpy(pBufTmp,startCode,4);
		outLen = len - RTP_HEADLEN + 4 ;
		dec_buf_append(bufOUT,pBufTmp,outLen);
		switch(nal){
			case 1:
				bFinishFrame=RTP_PKG;//单包数据
				break;
			case 32: // video parameter set (VPS)
			case 33: // sequence parameter set (SPS)
			case 34: // picture parameter set (PPS)
			case 39: // supplemental enhancement information (SEI)
				bFinishFrame=RTP_SPS;
				break;
			default: // 4.4.1. Single NAL Unit Packets (p24)
				bFinishFrame=RTP_PKG;//单包数据
		}
	}
		
	return bFinishFrame;
}



int make_seqc_right(u_char *bufIN, size_t len, int chn ,decoder_t dec)
{
	unsigned short seqc=0;
	unsigned int timestamp;

	int seqcRight = 1;
	seqc = 0x00ff & (*(bufIN+6));
	seqc<<=8;
	seqc |=(*(bufIN+7));
	

	timestamp =  bufIN[11] & 0x00ff;
	timestamp |=((bufIN[10]& 0x000000FF)<<8);
	timestamp |=((bufIN[9]& 0x000000FF)<<16);
	timestamp |=((bufIN[8]& 0x000000FF)<<24);
	
	
	if((seqc !=(unsigned short) (dec->last_seqc+0x0001))){
		printf("dec>>>[%d] err [%d : %d]\n",chn,dec->last_seqc,seqc);
	}
	dec->last_seqc = seqc;
	
	if(dec->timestamp != timestamp){
		dec->timestamp = timestamp;
		if(dec->refused == 0)
		    dec->time40ms+=39800;
	}
	
	return seqcRight;
}
