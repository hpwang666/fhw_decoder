/*
 * Demo on how to use /dev/crypto device for ciphering.
 *
 * Placed under public domain.
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#define EFUSE_MAX_LEN   64
#define fpath0  "/sys/bus/nvmem/devices/nvmem0/nvmem"
#define fpath1  "/sys/bus/nvmem/devices/nvmem1/nvmem"

int main(int argc, char **argv)
{
	FILE *cfd = NULL;
    char * p;
    unsigned int n , i;
    char epath[256] = {0};
    char edata[EFUSE_MAX_LEN] = {0};
    int  elen, olen;

    if((argc != 5) && (argc != 6)) {
        printf("usage:read/write efuse0/efuse1 offset length hex:data\n" \
                "\nexample: read 0 4 \n" \
                "\n write 0 4 hex:00010203 \n");
        goto exit;
    }
    if(strcmp(argv[2],"efuse0") == 0)
        strcpy(epath, fpath0);
    else
        strcpy(epath, fpath1);
	/* Open the crypto device */
	cfd = fopen(epath, "rb+");
	if (cfd == NULL) {
		printf("open error\n");
		goto exit;
	}

	if( fseek( cfd,atoi(argv[3]), SEEK_SET ) < 0 )
    {
        printf( "fseek(0,SEEK_SET) failed\n" );
        goto exit;
    }

    olen = atoi(argv[4]);
    printf("olen=%d\n", olen);
    if(strcmp(argv[1], "read") == 0) {
        if(fread(edata, 1, olen, cfd) != olen ){
            printf("read error\n");
            goto exit;
        }
        for(i = 0; i < olen ; i++){
            printf("%02x", edata[i]);

            if(((i + 1) % 16) == 0)
                printf("\r\n");
            else
                printf(" ");
        }
    } else  if(strcmp(argv[1], "write") == 0) {
        if(argc != 6) {
            printf("argment error\n");
            goto exit;
        }

        if( memcmp( argv[5], "hex:", 4 ) == 0 )
        {
            p = &argv[5][4];
            elen = 0;
            while( sscanf( p, "%02X", &n ) > 0 &&
                 elen < EFUSE_MAX_LEN)
            {
                edata[elen++] = (unsigned char) n;
                p += 2;
            }

            if(fwrite(edata, 1, olen, cfd) != olen) {
                printf("write error\n");
                goto exit;
            }
        }

    }


exit:
	/* Close the original descriptor */
	if (cfd)
        fclose(cfd);

	return 0;
}



