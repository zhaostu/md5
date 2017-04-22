/*
 *      md5.c
 *      
 *      2009 Zhao Yanglei <z12y12l12@gmail.com>
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef unsigned int word;
typedef char byte;

word result[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
word temp_result[4];
word table_t[64];
word block[16];
int table_k[4][4][4] =
		{{{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}},
		{{1, 6, 11, 0}, {5, 10, 15, 4}, {9, 14, 3, 8}, {13, 2, 7, 12}},
		{{5, 8, 11, 14}, {1, 4, 7, 10}, {13, 0, 3, 6}, {9, 12, 15, 2}},
		{{0, 7, 14, 5}, {12, 3, 10, 1}, {8, 15, 6, 13}, {4, 11, 2, 9}}};

int table_s[4][4] = {{7, 12, 17, 22}, {5, 9, 14, 20}, {4, 11, 16, 23}, {6, 10, 15, 21}};

int table_i[4][4][4] =
		{{{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}},
		{{16, 17, 18, 19}, {20, 21, 22, 23}, {24, 25, 26, 27}, {28, 29, 30, 31}},
		{{32, 33, 34, 35}, {36, 37, 38, 39}, {40, 41, 42, 43}, {44, 45, 46, 47}},
		{{48, 49, 50, 51}, {52, 53, 54, 55}, {56, 57, 58, 59}, {60, 61, 62, 63}}};

byte PADDING[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned long long flen = 0;
int pad = 0;
FILE *fin;

inline word func_f(word x, word y, word z){
	return (x & y) | ((~x) & z);
}

inline word func_g(word x, word y, word z){
	return (x & z) | (y & (~z));
}

inline word func_h(word x, word y, word z){
	return x ^ y ^ z;
}

inline word func_i(word x, word y, word z){
	return y ^ (x | (~z));
}

word (*funcs[4])(word x, word y, word z) = {func_f, func_g, func_h, func_i};

inline word rol(word a, word x){
	return  a << x | a >> (32 - x);
}

/* Please see the defination in the rfc1321 */
inline word rounds(word a, word b, word c, word d, int k, int s, int i, int rnd){
	return b + rol((a + (funcs[rnd])(b, c, d) + block[k] + table_t[i]), s);
}

void init(){
	/* init t, which could be used */
	int i;
	for(i = 0; i < 64; i++){
		table_t[i] = 4294967296LL * fabs(sin(i + 1));
	}
}

void openfile(char *fname){
	fin = fopen(fname, "rb");
	if(!fin){
		printf("file not found.\n");
		exit(1);
	}
}

int next_16_bytes(){
	int bsize;
	if(pad == 2)
		return 0;
	if(pad == 1){
		memset(block, 0, sizeof(block));
		memcpy(block + 14, &flen, sizeof(flen));
		pad = 2;
	}
	else{
		bsize = fread(block, 1, 64, fin);
		flen += bsize << 3;
		if(bsize < 64){
			memcpy(((byte *)block) + bsize, PADDING, 64 - bsize);
			if(bsize < 56){
				memcpy(block + 14, &flen, sizeof(flen));
				pad = 2;
			}
			else{
				pad = 1;
			}
		}
	}
	return 1;
}

void calc(){

	int i, j;
	while(next_16_bytes()){
		memcpy(temp_result, result, sizeof(result));
		for(i = 0; i < 4; i++){
			for(j = 0; j < 4; j++){
				result[0] = rounds(result[0], result[1], result[2], result[3],
						table_k[i][j][0], table_s[i][0], table_i[i][j][0], i);
				result[3] = rounds(result[3], result[0], result[1], result[2],
						table_k[i][j][1], table_s[i][1], table_i[i][j][1], i);
				result[2] = rounds(result[2], result[3], result[0], result[1],
						table_k[i][j][2], table_s[i][2], table_i[i][j][2], i);
				result[1] = rounds(result[1], result[2], result[3], result[0],
						table_k[i][j][3], table_s[i][3], table_i[i][j][3], i);
			}
		}
		for(i = 0; i < 4; i++){
			result[i] += temp_result[i];
		}
	}
}

void output(){
	int i, j;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if((result[i] & 0xff) < 0x10){
				printf("0%x", result[i] & 0xff);
			}
			else{
				printf("%2x", result[i] & 0xff);
			}
			result[i] >>= 8;
		}
	}
	printf("\n");
}

int main(int argc, char** argv) {
	if(argc != 2){
		printf("Usage: md5 [filename]");
		exit(1);
	}
	init();
	openfile(argv[1]);
	calc();
	output();
	return 0;
}
