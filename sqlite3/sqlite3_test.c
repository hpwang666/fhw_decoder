#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>




int callback(void *param, int f_num, char **f_value, char **f_name);
int main()
{
	sqlite3 * db = NULL; //声明sqlite关键结构指针
	int result;
	int i=0;
	char * errmsg = NULL;
	sqlite3_stmt *stmt = NULL;
	char sql[128];
	//需要传入 db 这个指针的指针，因为 sqlite3_open 函数要为这个指针分配内存，还要让db指针指向这个内存区
	result = sqlite3_open( "/mnt/usr/ss.db",&db );

	if( result != SQLITE_OK )
	{
		printf("can not open the database %s\r\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
#if 0
	result = sqlite3_exec( db, "create table camera( ID integer primary key autoincrement, address varchar(128), url varchar(128),h_type int)", NULL, NULL, &errmsg );

	if(result != SQLITE_OK )

	{
		printf( "创建表失败，错误码:%d，错误原因:%s\r\n", result, errmsg );
	}

	for(i=4;i<17;i++){
		
		//插入一些记录
		result = sqlite3_exec( db, "insert into camera(address, url,h_type) values ('172.16.10.1','/chn/0/h264/main',0)", NULL, 0, &errmsg );
		if(result != SQLITE_OK )

		{
			printf( "插入记录失败，错误码:%d，错误原因:%s\r\n", result, errmsg );

		}
	}

#endif
#if 0
	result = sqlite3_exec( db, "create table plc_ctrl( ID integer primary key autoincrement, cmd varchar(32),vo_mutx int,cameras varchar(64) )", NULL, NULL, &errmsg );

	if(result != SQLITE_OK )

	{
		printf( "创建表失败，错误码:%d，错误原因:%s\r\n", result, errmsg );
	}

	for(i=0;i<4;i++){
		
		//插入一些记录
		result = sqlite3_exec( db, "insert into plc_ctrl(cmd,vo_mutx,cameras) values ('LEFT',4,'1,2,3,4')", NULL, 0, &errmsg );
		if(result != SQLITE_OK )

		{
			printf( "插入记录失败，错误码:%d，错误原因:%s\r\n", result, errmsg );

		}
	}

#endif


#if 0
	result = sqlite3_exec( db, "update camera set url='/chn/1/h264/sub' where id = 2", callback, NULL, &errmsg );
	if(result != SQLITE_OK )

	{
		printf( "更新失败，错误码:%d，错误原因:%s\r\n", result, errmsg );

	}
	result = sqlite3_exec( db, "select * from camera", callback, NULL, &errmsg );
	if(result != SQLITE_OK )

	{
		printf( "插入记录失败，错误码:%d，错误原因:%s\r\n", result, errmsg );

	}
#endif
	sprintf(sql,"select * from plc_ctrl");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d cmd: %s,vo:%d , cameras:%s  \n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_text(stmt, 1),\
				sqlite3_column_int(stmt, 2),\
				sqlite3_column_text(stmt, 3));\
	}
	sqlite3_finalize(stmt);
	sqlite3_close( db );

	return 0;
}


int callback(void *param, int f_num, char **f_value, char **f_name)

{
	int i=0;
	for(i=0;i<f_num;i++){
		printf("%s = %s \r\n",f_name[i],f_value[i]?f_value[i]:NULL);
	}
	//printf("%s:这是回调函数!\n", __FUNCTION__);
	return 0;

}
