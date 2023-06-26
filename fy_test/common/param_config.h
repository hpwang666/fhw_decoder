#ifndef __HYSA_FILE_COMM_H__
#define __HYSA_FILE_COMM_H__

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define  MAX_VALUE  64 /* max length of section,key,value*/
typedef struct _option {
  char    key[MAX_VALUE];   /* ��Ӧ�� */
  char    value[MAX_VALUE]; /* ��Ӧֵ */
  struct  _option *next;    /* �������ӱ�ʶ */
}Option;

typedef struct _data {
  char    section[MAX_VALUE]; /* ����sectionֵ          */
  Option  *option;            /* option����ͷ           */
  struct  _data *next;        /* �������ӱ�ʶ           */
}Data;

typedef struct {
  char    comment;              /* ��ʾע�͵ķ���    */
  char    separator;            /* ��ʾ�ָ���        */
  char    re_string[MAX_VALUE]; /* ����ֵ�ַ�����ֵ  */
  int     re_int;               /* ����int��ֵ       */
  bool    re_bool;              /* ����bool��ֵ      */
  double  re_double ;           /* ����double����    */
  Data    *data;                /* �������ݵ�ͷ      */
  unsigned int num;
}Config;

Config *cnf_load_config(const char *filename);
bool cnf_get_value(Config *cnf, const char *section, const char *key);
void print_config_info(Config *cnf);
int cnf_release(Config *cnf);
bool cnf_remove_section(Config *cnf, const char *section);
bool cnf_add_option(Config *cnf, const char *section, const char *key, const char *value);
Data *cnf_has_section(Config *cnf, const char *section);
unsigned int cnf_get_total_num(Config *cnf);


#endif
