#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include "queue.h"
#include <hikisapi.h>
#include <ptz.h>

#undef _DEBUG
//#define _DEBUG
#ifdef _DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

//ISAPI/PTZCtrl/channels/1/presets/1/goto
int transCmd(loop_ev env, custom_t cmdCmd)
{
	int channel = cmdCmd->ch;
	camConnection camConn = &(env->camConn[channel]);
	int len = 0;
	int cam0407 = 0;
	static char h_con[1024];
	static char cmd_xml[256];
	const char *_camName0 = "DS-2ZMN0407(C)";
	const char *_camName1 = "DS-2ZMN0407(D)";
	len = strlen(camConn->address);
	if (len == 0)
		return 0;
	len = strlen(camConn->camName);
	if (len == 0)
	{
		if (0 == getCamName(env,camConn))
			return 0;
	}
	if(strncmp(camConn->address,"0.0.0.0",7)==0){
		return 0;
	}

	if ((0 == strncmp(camConn->camName, "DS-", 3))||(0 == strncmp(camConn->camName, "iDS", 3))||(0 == strncmp(camConn->camName, "UV-", 3)))
	{
		if (0 == strncmp(camConn->camName, _camName0, 14))
			cam0407 = 0;
		if (0 == strncmp(camConn->camName, _camName1, 14))
			cam0407 = 0;
		if (cmdCmd->cmd == 0xaa)
		{ //union ctrl
			printf("ch0 auto zoom ctrl \r\n");
			snprintf(h_con, 1024, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\
					<PTZData><AbsoluteHigh><elevation>%d</elevation><azimuth>%d</azimuth> \
					<absoluteZoom>%d</absoluteZoom></AbsoluteHigh></PTZData>",env->ch0_elevation,env->ch0_azimuth,cmdCmd->stop);
			httpClientPut(camConn->ct,"/ISAPI/PTZCtrl/channels/1/absolute" , h_con);
			httpClearConn(camConn->ct);
			return 0;
		}

		snprintf(h_con, 1024, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
		switch (cmdCmd->cmd)
		{
		case 0x90:
			snprintf(cmd_xml, 128, "<PTZAux><id>1</id><type>WIPER</type><status>on</status></PTZAux>");
			break; //WIPER ON
		case 0x91:
			snprintf(cmd_xml, 128, "<PTZAux><id>1</id><type>WIPER</type><status>off</status></PTZAux>");
			break; //WIPER OFF
		case 0x80:
			if (cam0407)
				snprintf(cmd_xml, 128, "<PTZData><pan>0</pan><tilt>0</tilt><zoom>0</zoom></PTZData>");
			else
				snprintf(cmd_xml, 128, "<PTZData><pan>0</pan><tilt>0</tilt><zoom>100</zoom></PTZData>");
			break; //ZOOM_IN
		case 0x81:
			if (cam0407)
				snprintf(cmd_xml, 128, "<PTZData><pan>0</pan><tilt>0</tilt><zoom>0</zoom></PTZData>");
			else
				snprintf(cmd_xml, 128, "<PTZData><pan>0</pan><tilt>0</tilt><zoom>-100</zoom></PTZData>");
			break; //ZOOM_OUT
		case 0x8C:
			snprintf(cmd_xml, 128, "<PTZData><pan>0</pan><tilt>-80</tilt><zoom>0</zoom></PTZData>");
			break; //TILT_UP
		case 0x8D:
			snprintf(cmd_xml, 128, "<PTZData><pan>0</pan><tilt>80</tilt><zoom>0</zoom></PTZData>");
			break; //TILT_DOWN
		case 0x8E:
			snprintf(cmd_xml, 128, "<PTZData><pan>-80</pan><tilt>0</tilt><zoom>0</zoom></PTZData>");
			break; //PAN_LEFT
		case 0x8F:
			snprintf(cmd_xml, 128, "<PTZData><pan>80</pan><tilt>0</tilt><zoom>0</zoom></PTZData>");
			break; //PAN_RIGHT
		case 0xA4:
			snprintf(cmd_xml, 200, "<RegionalExposure><StartPoint><positionX>760</positionX><positionY>400</positionY></StartPoint><EndPoint><positionX>900</positionX><positionY>540</positionY></EndPoint></RegionalExposure>");
			break; //RegionalExposure
		case 0xA5:
			snprintf(cmd_xml, 200, "<RegionalExposure><StartPoint><positionX> 0 </positionX><positionY>0</positionY></StartPoint><EndPoint><positionX>100</positionX><positionY> 80</positionY></EndPoint></RegionalExposure>");
			break; //RegionalExposure
		default:
			snprintf(cmd_xml, 128, "<PTZData><pan>0</pan><tilt>0</tilt><zoom>0</zoom></PTZData>");
			break; //LIGHT_PWRON
		}
		if (cmdCmd->stop){
			snprintf(cmd_xml, 128, "<PTZData><pan>0</pan><tilt>0</tilt><zoom>0</zoom></PTZData>");
		}

		strcat(h_con, cmd_xml);
		if ((cmdCmd->cmd) == 0x90 || (cmdCmd->cmd) == 0x91)//雨刷指令
		{
			if (httpClientPut(camConn->ct, "/ISAPI/PTZCtrl/channels/1/auxcontrols/1", h_con))
			{
				httpClearConn(camConn->ct);
			}
		}
		else if((cmdCmd->cmd) == 0xA4||(cmdCmd->cmd) == 0xA5)
		{
			if (httpClientPut(camConn->ct, "/ISAPI/Image/channels/1/regionalExposure", h_con))
			{
				httpClearConn(camConn->ct);
			}
		}
		else//PTZ停止指令
		{
			if (httpClientPut(camConn->ct, "/ISAPI/PTZCtrl/channels/1/continuous", h_con))
			{
				httpClearConn(camConn->ct);
			}
		}
	}
	else if ((0 == strncmp(camConn->camName, "DH-", 3))||(0 == strncmp(camConn->camName, "CA-", 3)))
	{
		if (cmdCmd->cmd == 0xaa)
		{ //union ctrl
			snprintf(h_con, 1024, "/cgi-bin/ptz.cgi?action=start&channel=1&code=GotoPreset&arg1=0&arg2=%d&arg3=0", cmdCmd->stop);
			httpClientGet(camConn->ct, h_con);
			httpClearConn(camConn->ct);
			return 0;
		}
		if (cmdCmd->stop)
			snprintf(cmd_xml, 128, "stop");
		else
			snprintf(cmd_xml, 128, "start");
		switch (cmdCmd->cmd)
		{
		case 0x36:
			if (cmdCmd->stop)
				snprintf(h_con, 1024, "/cgi-bin/rainBrush.cgi?action=stopMove");
			else
				snprintf(h_con, 1024, "/cgi-bin/rainBrush.cgi?action=moveOnce");
			break; //WIPER
		case 0x0a:
			if (cam0407)
				;
			else
				snprintf(h_con, 1024, "/cgi-bin/ptz.cgi?action=%s&channel=1&code=ZoomTele&arg1=0&arg2=0&arg3=0", cmd_xml);
			break; //ZOOM_IN
		case 0x09:
			if (cam0407)
				;
			else
				snprintf(h_con, 1024, "/cgi-bin/ptz.cgi?action=%s&channel=1&code=ZoomWide&arg1=0&arg2=0&arg3=0", cmd_xml);
			break; //ZOOM_OUT
		case 0x02:
			snprintf(h_con, 1024, "/cgi-bin/ptz.cgi?action=%s&channel=1&code=Up&arg1=0&arg2=1&arg3=0", cmd_xml);
			break; //TILT_UP
		case 0x07:
			snprintf(h_con, 1024, "/cgi-bin/ptz.cgi?action=%s&channel=1&code=Down&arg1=0&arg2=1&arg3=0", cmd_xml);
			break; //TILT_DOWN
		case 0x04:
			snprintf(h_con, 1024, "/cgi-bin/ptz.cgi?action=%s&channel=1&code=Left&arg1=0&arg2=1&arg3=0", cmd_xml);
			break; //PAN_LEFT
		case 0x05:
			snprintf(h_con, 1024, "/cgi-bin/ptz.cgi?action=%s&channel=1&code=Right&arg1=0&arg2=1&arg3=0", cmd_xml);
			break; //PAN_RIGHT
		default:
			snprintf(h_con, 1024, "/cgi-bin/ptz.cgi?action=getStatus&channel=1");
			break; //LIGHT_PWRON
		}

		if (httpClientGet(camConn->ct, h_con))
		{
			httpClearConn(camConn->ct);
		}
	}else
		printf("##warning:No support device type for %s!",camConn->camName);

	return 0;
}
