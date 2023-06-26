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
#include "crypto/cryptodev.h"
#include "tcrypt.h"
#include "aes.h"

unsigned char * ckey = "\x96\x15\xa0\x41\x7c\xb7\x9d\xcc"
			"\x04\x6d\xd0\x3a\x82\x38\x00\xa4";
static const struct cipher_testvec cipher_key = {
	.key    = "\xc2\x86\x69\x6d\x88\x7c\x9a\xa0"
		  "\x61\x1b\xbb\x3e\x20\x25\xa4\x5a",
	.klen   = 16,
	.iv     = "\x56\x2e\x17\x99\x6d\x09\x3d\x28"
		  "\xdd\xb3\xba\x69\x5a\x2e\x6f\x58",
	.ptext	=  "\x00\x01\x02\x03\x04\x05\x06\x07"
		  "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
		  "\x10\x11\x12\x13\x14\x15\x16\x17"
		  "\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f",
	.ctext	= "\xd2\x96\xcd\x94\xc2\xcc\xcf\x8a"
		  "\x3a\x86\x30\x28\xb5\xe1\xdc\x0a"
		  "\x75\x86\x60\x2d\x25\x3c\xff\xf9"
		  "\x1b\x82\x66\xbe\xa6\xd6\x1a\xb1",
	.len	= 32,
};

int aes_ctx_init(struct cryptodev_ctx* ctx, int cfd, const char *key, unsigned int key_size,enum cryptodev_crypto_op_t alg)
{
#ifdef CIOCGSESSINFO
	struct session_info_op siop;
#endif

	memset(ctx, 0, sizeof(*ctx));
	ctx->cfd = cfd;

	ctx->sess.cipher = alg;
	ctx->sess.keylen = key_size;
	ctx->sess.key = (void*)key;
	if (ioctl(ctx->cfd, CIOCGSESSION, &ctx->sess)) {
		perror("ioctl(CIOCGSESSION)");
		return -1;
	}

#ifdef CIOCGSESSINFO
	memset(&siop, 0, sizeof(siop));

	siop.ses = ctx->sess.ses;
	if (ioctl(ctx->cfd, CIOCGSESSINFO, &siop)) {
		perror("ioctl(CIOCGSESSINFO)");
		return -1;
	}
	printf("Got %s with driver %s\n",
			siop.cipher_info.cra_name, siop.cipher_info.cra_driver_name);
	/*printf("Alignmask is %x\n", (unsigned int)siop.alignmask); */
	ctx->alignmask = siop.alignmask;
#endif
	return 0;
}

void aes_ctx_deinit(struct cryptodev_ctx* ctx)
{
	if (ioctl(ctx->cfd, CIOCFSESSION, &ctx->sess.ses)) {
		perror("ioctl(CIOCFSESSION)");
	}
}

int
aes_encrypt(struct cryptodev_ctx* ctx, const void* iv, const void* plaintext, void* ciphertext, size_t size)
{
	struct crypt_op cryp;
	void* p;

	/* check plaintext and ciphertext alignment */
	if (ctx->alignmask) {
		p = (void*)(((unsigned long)plaintext + ctx->alignmask) & ~ctx->alignmask);
		if (plaintext != p) {
			fprintf(stderr, "plaintext is not aligned\n");
			return -1;
		}

		p = (void*)(((unsigned long)ciphertext + ctx->alignmask) & ~ctx->alignmask);
		if (ciphertext != p) {
			fprintf(stderr, "ciphertext is not aligned\n");
			return -1;
		}
	}

	memset(&cryp, 0, sizeof(cryp));

	/* Encrypt data.in to data.encrypted */
	cryp.ses = ctx->sess.ses;
	cryp.len = size;
	cryp.src = (void*)plaintext;
	cryp.dst = ciphertext;
	cryp.iv = (void*)iv;
	cryp.op = COP_ENCRYPT;
	if (ioctl(ctx->cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		return -1;
	}

	return 0;
}

int
aes_decrypt(struct cryptodev_ctx* ctx, const void* iv, const void* ciphertext, void* plaintext, size_t size)
{
	struct crypt_op cryp;
	void* p;

	/* check plaintext and ciphertext alignment */
	if (ctx->alignmask) {
		p = (void*)(((unsigned long)plaintext + ctx->alignmask) & ~ctx->alignmask);
		if (plaintext != p) {
			fprintf(stderr, "plaintext is not aligned\n");
			return -1;
		}

		p = (void*)(((unsigned long)ciphertext + ctx->alignmask) & ~ctx->alignmask);
		if (ciphertext != p) {
			fprintf(stderr, "ciphertext is not aligned\n");
			return -1;
		}
	}

	memset(&cryp, 0, sizeof(cryp));

	/* Encrypt data.in to data.encrypted */
	cryp.ses = ctx->sess.ses;
	cryp.len = size;
	cryp.src = (void*)ciphertext;
	cryp.dst = plaintext;
	cryp.iv = (void*)iv;
	cryp.op = COP_DECRYPT;
	if (ioctl(ctx->cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		return -1;
	}

	return 0;
}
char plaintext_raw[1024] __attribute__((aligned(16)));
char ciphertext_raw[1024] __attribute__((aligned(16)));

void swap_word(const char *src, int len, char * dst)
{
	unsigned int i, j;
	if (len & 0x3)
	{
		printf("length is not aligned 4");
		return;
	}
	len >>= 2;

	for(j = 0; j < len; j++) {
		for (i = 0; i < 4; i++) {
			dst[j * 4 + i] = src[j * 4 + (3 - i)];
		}
	}
}
static int test_aes_efuse_key(int cfd)
{
	struct cryptodev_ctx ctx;
	struct cipher_test_info *cipher_test;
	const struct cipher_testvec *testvec;
	char *plaintext, *ciphertext;
	unsigned int cipher_len, block_size;
	char test_key[256], kkey[32];

	printf("++++++AES EFUSE KEY Test++++++\n");

	memset(test_key, 0, sizeof(test_key));
	memcpy(test_key, KEY_IN_EFUSE, KEY_IN_EFUSE_LEN);
	test_key[KEY_IN_EFUSE_LEN] = 0x20;

	aes_ctx_init(&ctx, cfd, test_key, cipher_key.klen, CRYPTO_AES_CBC);

	if (ctx.alignmask) {
		plaintext = (char *)(((unsigned long)plaintext_raw + ctx.alignmask) & ~ctx.alignmask);
		ciphertext = (char *)(((unsigned long)ciphertext_raw + ctx.alignmask) & ~ctx.alignmask);
	} else {
		plaintext = plaintext_raw;
		ciphertext = ciphertext_raw;
	}
	memset(plaintext, 0 , cipher_key.len);
	memset(ciphertext, 0 , cipher_key.len);

	swap_word(cipher_key.key, cipher_key.klen, kkey);

	memcpy(ciphertext, ckey, cipher_key.klen);
	block_size = ctx.alignmask + 1;
	cipher_len = (cipher_key.len + ctx.alignmask) / block_size * block_size ;

	aes_decrypt(&ctx, cipher_key.iv, ciphertext, plaintext, cipher_key.klen);
	{
		int k;
		for (k = 0; k < cipher_key.klen; k++) {
			printf("%02x ", kkey[k]);
		}
		printf("decode plaintext:");
		for (k = 0; k < cipher_key.klen; k++) {
			printf("%02x ", plaintext[k]);
		}
		printf("\n");
	}

	printf("AES EFUSE KEY cipher_key done\n");
	memcpy(test_key, KEY_IN_EFUSE, KEY_IN_EFUSE_LEN);
	test_key[KEY_IN_EFUSE_LEN] = 0x20;
	memcpy(test_key + KEY_IN_EFUSE_LEN + 4, CIPHER_KEY, CIPHER_KEY_LEN);

	aes_ctx_init(&ctx, cfd, test_key, cipher_key.klen, CRYPTO_AES_CBC);

	memcpy(ciphertext, ckey, cipher_key.klen);
		/* Encrypt data.in to data.encrypted */
	aes_decrypt(&ctx, cipher_key.iv, ciphertext, plaintext, cipher_key.klen);

	printf("AES EFUSE KEY encode/decode use cipher key\n");
	memset(test_key, 0, sizeof(test_key));
	memcpy(test_key, USE_CIPHER_KEY, USE_CIPHER_KEY_LEN);

	aes_ctx_init(&ctx, cfd, test_key, cipher_key.klen, CRYPTO_AES_CBC);

	memset(plaintext, 0 , cipher_key.len);
	memset(ciphertext, 0 , cipher_key.len);
	memcpy(plaintext, cipher_key.ptext, cipher_key.len);
	memcpy(ciphertext, cipher_key.ctext, cipher_key.len);

	block_size = ctx.alignmask + 1;
	cipher_len = (cipher_key.len + ctx.alignmask) / block_size * block_size ;

		/* Encrypt data.in to data.encrypted */
	aes_decrypt(&ctx, cipher_key.iv, ciphertext, plaintext, cipher_len);
			/* Verify the result */
	if (memcmp(plaintext, cipher_key.ptext, cipher_key.len) != 0) {
		int k;
		fprintf(stderr,
			"FAIL: Decrypted data are different from the expect plain data.\n");
		printf("expect plaintext:");
		for (k = 0; k < cipher_key.len; k++) {
			printf("%02x ", cipher_key.ptext[k]);
		}
		printf("decode plaintext:");
		for (k = 0; k < cipher_key.len; k++) {
			printf("%02x ", plaintext[k]);
		}
		printf("\n");
		return 1;
	}

	aes_ctx_deinit(&ctx);

	printf("AES EFUSE KEY Test passed\n");

	return 0;
}

static int aes_test_key(int cfd)
{
	struct cryptodev_ctx ctx;
	struct cipher_test_info *cipher_test;
	const struct cipher_testvec *testvec;
	char *plaintext, *ciphertext;
	unsigned int cipher_len, block_size;
	char test_key[256], kkey[32];

	printf("++++++AES KEY Test++++++\n");

	memcpy(test_key, CIPHER_KEY, CIPHER_KEY_LEN);
	memcpy(test_key + CIPHER_KEY_LEN, cipher_key.key, cipher_key.klen);
	aes_ctx_init(&ctx, cfd, test_key, CIPHER_KEY_LEN + cipher_key.klen, CRYPTO_AES_CBC);

	if (ctx.alignmask) {
		plaintext = (char *)(((unsigned long)plaintext_raw + ctx.alignmask) & ~ctx.alignmask);
		ciphertext = (char *)(((unsigned long)ciphertext_raw + ctx.alignmask) & ~ctx.alignmask);
	} else {
		plaintext = plaintext_raw;
		ciphertext = ciphertext_raw;
	}

	memset(plaintext, 0 , cipher_key.len);
	memset(ciphertext, 0 , cipher_key.len);

	block_size = ctx.alignmask + 1;
	cipher_len = (cipher_key.len + ctx.alignmask) / block_size * block_size ;

	memcpy(ciphertext, ckey, cipher_key.klen);
		/* Encrypt data.in to data.encrypted */
	aes_decrypt(&ctx, cipher_key.iv, ciphertext, plaintext, cipher_key.klen);

	printf("encode/decode use cipher key\n");
	memset(test_key, 0, sizeof(test_key));
	memcpy(test_key, USE_CIPHER_KEY, USE_CIPHER_KEY_LEN);

	aes_ctx_init(&ctx, cfd, test_key, cipher_key.klen, CRYPTO_AES_CBC);

	memset(plaintext, 0 , cipher_key.len);
	memset(ciphertext, 0 , cipher_key.len);
	memcpy(plaintext, cipher_key.ptext, cipher_key.len);
	memcpy(ciphertext, cipher_key.ctext, cipher_key.len);

		/* Encrypt data.in to data.encrypted */
	aes_decrypt(&ctx, cipher_key.iv, ciphertext, plaintext, cipher_len);
			/* Verify the result */
	if (memcmp(plaintext, cipher_key.ptext, cipher_key.len) != 0) {
		int k;
		fprintf(stderr,
			"FAIL: Decrypted data are different from the expect plain data.\n");
		printf("expect plaintext:");
		for (k = 0; k < cipher_key.len; k++) {
			printf("%02x ", cipher_key.ptext[k]);
		}
		printf("decode plaintext:");
		for (k = 0; k < cipher_key.len; k++) {
			printf("%02x ", plaintext[k]);
		}
		printf("\n");
		return 1;
	}

	aes_ctx_deinit(&ctx);

	printf("AES KEY Test passed\n");

	return 0;
}

static int test_aes(int cfd)
{

	struct cryptodev_ctx ctx;
	struct cipher_test_info *cipher_test;
	const struct cipher_testvec *testvec;
	char *plaintext, *ciphertext;
	unsigned int cipher_len, block_size;

	unsigned int i , j;

	printf("++++++AES Test++++++\n");

	for(i = 0; i < ARRAY_SIZE(cipher_test_list); i++) {

		cipher_test = &cipher_test_list[i];
		testvec = cipher_test->vecs;

		for(j = 0; j < cipher_test->count; j++) {

			aes_ctx_init(&ctx, cfd, testvec[j].key, testvec[j].klen, cipher_test->crypt);

			if (ctx.alignmask) {
				plaintext = (char *)(((unsigned long)plaintext_raw + ctx.alignmask) & ~ctx.alignmask);
				ciphertext = (char *)(((unsigned long)ciphertext_raw + ctx.alignmask) & ~ctx.alignmask);
			} else {
				plaintext = plaintext_raw;
				ciphertext = ciphertext_raw;
			}
			memset(plaintext, 0 , testvec[j].len);
			memset(ciphertext, 0 , testvec[j].len);
			memcpy(plaintext, testvec[j].ptext, testvec[j].len);

			block_size = ctx.alignmask + 1;
			cipher_len = (testvec[j].len + ctx.alignmask) / block_size * block_size ;
			/* Encrypt data.in to data.encrypted */
			aes_encrypt(&ctx, testvec[j].iv, plaintext, ciphertext, cipher_len);

			/* Verify the result */
			if (memcmp(ciphertext, testvec[j].ctext, testvec[j].len) != 0) {
				int k;
				fprintf(stderr,
					"FAIL: Encrypted data are different from the expect cipher data.\n");
				printf("expect ciphertext:");
				for (k = 0; k < testvec[j].len; k++) {
					printf("%02x ", testvec[j].ctext[k]);
				}
				printf("encode ciphertext:");
				for (k = 0; k < testvec[j].len; k++) {
					printf("%02x ", ciphertext[k]);
				}
				printf("\n");
				return 1;

			}

			memset(plaintext, 0 , testvec[j].len);
			memset(ciphertext, 0 , testvec[j].len);
			memcpy(ciphertext, testvec[j].ctext, testvec[j].len);

			/* Encrypt data.in to data.encrypted */
			aes_decrypt(&ctx, testvec[j].iv, ciphertext, plaintext, cipher_len);

			/* Verify the result */
			if (memcmp(plaintext, testvec[j].ptext, testvec[j].len) != 0) {
				int k;
				fprintf(stderr,
					"FAIL: Decrypted data are different from the expect plain data.\n");
				printf("expect plaintext:");
				for (k = 0; k < testvec[j].len; k++) {
					printf("%02x ", testvec[j].ptext[k]);
				}
				printf("decode plaintext:");
				for (k = 0; k < testvec[j].len; k++) {
					printf("%02x ", plaintext[k]);
				}
				printf("\n");
				return 1;

			}

			aes_ctx_deinit(&ctx);
		}
	}
	printf("AES Test passed\n");

	return 0;
}

int
main()
{
	int cfd = -1;

	/* Open the crypto device */
	cfd = open("/dev/crypto", O_RDWR, 0);
	if (cfd < 0) {
		perror("open(/dev/crypto)");
		return 1;
	}

	/* Set close-on-exec (not really neede here) */
	if (fcntl(cfd, F_SETFD, 1) == -1) {
		perror("fcntl(F_SETFD)");
		return 1;
	}
	/* Run the test itself */
	if (test_aes(cfd)) {
		printf("test aes failed");
		goto err;
	}

	if (aes_test_key(cfd)) {
		printf("test aes key failed");
		goto err;
	}

	if (test_aes_efuse_key(cfd)) {
		printf("test aes efuse key failed");
		goto err;
	}

	/* Close the original descriptor */
err:
	if (close(cfd)) {
		perror("close(cfd)");
		return 1;
	}

	return 0;
}


