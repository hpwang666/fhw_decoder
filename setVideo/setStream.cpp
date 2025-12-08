#include "tinyxml2.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

#include "httpclient.h"
#include "sstr.h"

using namespace tinyxml2;


int setMainStream(char *camIp,char *passwd,char *encodeType,char *resolution)
{
	httpclient_t hc=NULL;
    XMLDocument doc;
    int res=0;
    hc=httpClientCreat(camIp,"admin",passwd);
	res=httpClientGet(hc,"/ISAPI/Streaming/channels/101");
    if(res<0){
        httpclientFree(hc);
        return res;
    }

	//printf(">>>%s\r\n",hc->recvContent);

	doc.Parse( hc->recvContent );
	httpClearConn(hc);
	doc.DeleteChild(doc.FirstChildElement()->FirstChildElement("Transport"));
	doc.DeleteChild(doc.FirstChildElement()->FirstChildElement("channelName"));
	XMLElement* titleElement = doc.FirstChildElement()->FirstChildElement("Video")->FirstChildElement();
	
	
	int cbr_vbr=-1;

	for (XMLElement* Video = titleElement; Video; Video = Video->NextSiblingElement()) {
		//person->SetAttribute("name", "UpdatedName");
		//person->SetText("UpdatedText");
		if(strcmp(Video->Name(),"SVC")==0){
			//printf( "%s %s\n",Video->FirstChildElement()->Name(),Video->FirstChildElement()->GetText());
			printf("set SVC from \t\t\t%s to false\r\n",Video->FirstChildElement()->GetText());
			Video->FirstChildElement()->SetText("false");
		}
		else if(strcmp(Video->Name(),"SmartCodec")==0){
			//printf( "%s %s\n",Video->FirstChildElement()->Name(),Video->FirstChildElement()->GetText());
			printf("set SmartCodec from \t\t%s to false\r\n",Video->FirstChildElement()->GetText());
			Video->FirstChildElement()->SetText("false");
		}
		else if(strcmp(Video->Name(),"videoCodecType")==0){
			printf("set videoCodecType from \t%s to %s\r\n",Video->GetText(),encodeType);
			Video->SetText(encodeType);
		}
		else if(strcmp(Video->Name(),"videoResolutionWidth")==0){
            if(strcmp(resolution,"1080p")==0){
                printf("set ResolutionWidth from \t%s to 1920\r\n",Video->GetText());
			    Video->SetText("1920");
            }
			else if(strcmp(resolution,"720p")==0){
                printf("set ResolutionWidth from \t%s to 1280\r\n",Video->GetText());
			    Video->SetText("1280");
            }
		}
		else if(strcmp(Video->Name(),"videoResolutionHeight")==0){
            if(strcmp(resolution,"1080p")==0){
                printf("set ResolutionHeight from \t%s to 1080\r\n",Video->GetText());
			    Video->SetText("1080");
            }
			else if(strcmp(resolution,"720p")==0){
                printf("set ResolutionHeight from \t%s to 720\r\n",Video->GetText());
			    Video->SetText("720");
            }
		}
		else if(strcmp(Video->Name(),"videoQualityControlType")==0){
			if(strcmp(Video->GetText(),"CBR")==0)//定码率
			cbr_vbr=0;
			if(strcmp(Video->GetText(),"VBR")==0)//变码率
			cbr_vbr=1;
		}
		else if(strcmp(Video->Name(),"constantBitRate")==0){
            if(strcmp(resolution,"1080p")==0){
                printf("set constantBitRate from \t%s to 4096\r\n",Video->GetText());
                Video->SetText("4096");
            }
            else if(strcmp(resolution,"720p")==0){
                printf("set constantBitRate from \t%s to 1024\r\n",Video->GetText());
			    Video->SetText("1024");
            }
		}
		else if(strcmp(Video->Name(),"vbrUpperCap")==0){
			 if(strcmp(resolution,"1080p")==0){
                printf("set vbrUpperCap from \t\t%s to 4096\r\n",Video->GetText());
                Video->SetText("4096");
            }
            else if(strcmp(resolution,"720p")==0){
                printf("set vbrUpperCap from \t\t%s to 1024\r\n",Video->GetText());
			    Video->SetText("1024");
            }
		}else if(strcmp(Video->Name(),"vbrAverageCap")==0){
			 if(strcmp(resolution,"1080p")==0){
                printf("set vbrAverageCap from \t\t%s to 4096\r\n",Video->GetText());
                Video->SetText("4096");
            }
            else if(strcmp(resolution,"720p")==0){
                printf("set vbrAverageCap from \t\t%s to 1024\r\n",Video->GetText());
			    Video->SetText("1024");
            }
		}
		
		
			//printf( "%s %s\n",Video->Name(),Video->GetText());
	}


	XMLPrinter printer1;
		
	doc.Print(&printer1);
	
	sprintf( hc->sendContent,"%s", printer1.CStr() );
	res=httpClientPut(hc,"/ISAPI/Streaming/channels/101",hc->sendContent);
	usleep(300*1000);
    if(res>0)
	res=httpClientPut(hc,"/ISAPI/Streaming/channels/101",hc->sendContent);


	//delete doc;
	if(hc) httpclientFree(hc);
    return res;
}


int setSubStream(char *camIp,char *passwd,char *encodeType)
{
    httpclient_t hc=NULL;
    XMLDocument doc;
    int res=0;
    hc=httpClientCreat(camIp,"admin",passwd);
	res=httpClientGet(hc,"/ISAPI/Streaming/channels/102");
    if(res<0){
        httpclientFree(hc);
        return res;
    }
	//printf(">>>%s\r\n",hc->recvContent);

	doc.Parse( hc->recvContent );
	httpClearConn(hc);
	doc.DeleteChild(doc.FirstChildElement()->FirstChildElement("Transport"));
	doc.DeleteChild(doc.FirstChildElement()->FirstChildElement("channelName"));
	XMLElement* titleElement = doc.FirstChildElement()->FirstChildElement("Video")->FirstChildElement();
	
	
	int cbr_vbr=-1;

	for (XMLElement* Video = titleElement; Video; Video = Video->NextSiblingElement()) {
		//person->SetAttribute("name", "UpdatedName");
		//person->SetText("UpdatedText");
		if(strcmp(Video->Name(),"SVC")==0){
			//printf( "%s %s\n",Video->FirstChildElement()->Name(),Video->FirstChildElement()->GetText());
			printf("set SVC from \t\t\t%s to false\r\n",Video->FirstChildElement()->GetText());
			Video->FirstChildElement()->SetText("false");
		}
		else if(strcmp(Video->Name(),"SmartCodec")==0){
			//printf( "%s %s\n",Video->FirstChildElement()->Name(),Video->FirstChildElement()->GetText());
			printf("set SmartCodec from \t\t%s to false\r\n",Video->FirstChildElement()->GetText());
			Video->FirstChildElement()->SetText("false");
		}
		else if(strcmp(Video->Name(),"videoCodecType")==0){
			printf("set videoCodecType from \t%s to %s\r\n",Video->GetText(),encodeType);
			Video->SetText(encodeType);
		}
		else if(strcmp(Video->Name(),"videoResolutionWidth")==0){
			printf("set ResolutionWidth from \t%s to 704\r\n",Video->GetText());
			Video->SetText("704");
		}
		else if(strcmp(Video->Name(),"videoResolutionHeight")==0){
			printf("set ResolutionHeight from \t%s to 576\r\n",Video->GetText());
			Video->SetText("576");
		}
		else if(strcmp(Video->Name(),"videoQualityControlType")==0){
			if(strcmp(Video->GetText(),"CBR")==0)//定码率
			cbr_vbr=0;
			if(strcmp(Video->GetText(),"VBR")==0)//变码率
			cbr_vbr=1;
		}
		else if(strcmp(Video->Name(),"constantBitRate")==0){
			printf("set constantBitRate from \t%s to 1024\r\n",Video->GetText());
			Video->SetText("1024");
		}
		else if(strcmp(Video->Name(),"vbrUpperCap")==0){
			printf("set vbrUpperCap from \t\t%s to 1024\r\n",Video->GetText());
			Video->SetText("1024");
		}
		
			//printf( "%s %s\n",Video->Name(),Video->GetText());
	}


	XMLPrinter printer1;
		
	doc.Print(&printer1);
	
	sprintf( hc->sendContent,"%s", printer1.CStr() );
	res=httpClientPut(hc,"/ISAPI/Streaming/channels/102",hc->sendContent);
	usleep(300*1000);
    if(res>0)
	res=httpClientPut(hc,"/ISAPI/Streaming/channels/102",hc->sendContent);


	//delete doc;
	if(hc) httpclientFree(hc);
    return res;
}