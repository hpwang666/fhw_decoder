#ifndef _PTZ_H
#define _PTZ_H

#define WIPER 0x36

/**********************������̨�������� begin*************************/    
#define LIGHT_PWRON        2    /* ��ͨ�ƹ��Դ */
#define WIPER_PWRON        3    /* ��ͨ��ˢ���� */
#define FAN_PWRON        4    /* ��ͨ���ȿ��� */
#define HEATER_PWRON    5    /* ��ͨ���������� */
#define AUX_PWRON1        6    /* ��ͨ�����豸���� */
#define AUX_PWRON2        7    /* ��ͨ�����豸���� */
#define SET_PRESET        8    /* ����Ԥ�õ� */
#define CLE_PRESET        9    /* ���Ԥ�õ� */

#define ZOOM_IN            11    /* �������ٶ�SS���(���ʱ��) */
#define ZOOM_OUT        12    /* �������ٶ�SS��С(���ʱ�С) */
#define FOCUS_NEAR      13  /* �������ٶ�SSǰ�� */
#define FOCUS_FAR       14  /* �������ٶ�SS��� */
#define IRIS_OPEN       15  /* ��Ȧ���ٶ�SS���� */
#define IRIS_CLOSE      16  /* ��Ȧ���ٶ�SS��С */

#define TILT_UP            21    /* ��̨��SS���ٶ����� */
#define TILT_DOWN        22    /* ��̨��SS���ٶ��¸� */
#define PAN_LEFT        23    /* ��̨��SS���ٶ���ת */
#define PAN_RIGHT        24    /* ��̨��SS���ٶ���ת */

int transCmd(loop_ev ev,custom_t cmdCmd);
#endif