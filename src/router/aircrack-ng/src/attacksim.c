#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "aircrack-ptw-lib.h"


#define STARTSESS (20000)
#define INCSESS (2500)
#define ENDSESS (300000)
#define KEYBYTES (16)
#define HSBYTES (13)
#define IVBYTES (3)
#define KEYLIMIT (1000000)
#define n 256



typedef struct {
	uint8_t i;
	uint8_t j;
	uint8_t s[n];
} rc4state;


const uint8_t rc4initial[] =
{0,1,2,3,4,5,6,7,8,9,10,
11,12,13,14,15,16,17,18,19,20,
21,22,23,24,25,26,27,28,29,30,
31,32,33,34,35,36,37,38,39,40,
41,42,43,44,45,46,47,48,49,50,
51,52,53,54,55,56,57,58,59,60,
61,62,63,64,65,66,67,68,69,70,
71,72,73,74,75,76,77,78,79,80,
81,82,83,84,85,86,87,88,89,90,
91,92,93,94,95,96,97,98,99,100,
101,102,103,104,105,106,107,108,109,110,
111,112,113,114,115,116,117,118,119,120,
121,122,123,124,125,126,127,128,129,130,
131,132,133,134,135,136,137,138,139,140,
141,142,143,144,145,146,147,148,149,150,
151,152,153,154,155,156,157,158,159,160,
161,162,163,164,165,166,167,168,169,170,
171,172,173,174,175,176,177,178,179,180,
181,182,183,184,185,186,187,188,189,190,
191,192,193,194,195,196,197,198,199,200,
201,202,203,204,205,206,207,208,209,210,
211,212,213,214,215,216,217,218,219,220,
221,222,223,224,225,226,227,228,229,230,
231,232,233,234,235,236,237,238,239,240,
241,242,243,244,245,246,247,248,249,250,
251,252,253,254,255};


void printKey(uint8_t * key, int keylen) {
        int i;
	printf("key: ");
        for (i = 0; i < keylen; i++) {
                printf("%02X ", key[i]);
        }
        printf("\n");
}


uint8_t rc4update(rc4state * state) {
	uint8_t tmp;
	uint8_t k;
	state->i++;
	state->j += state->s[state->i];
	tmp = state->s[state->i];
	state->s[state->i] = state->s[state->j];
	state->s[state->j] = tmp;
	k = state->s[state->i] + state->s[state->j];

	return state->s[k];
}

void rc4init ( uint8_t * key, int keylen, rc4state * state) {
	int i;
	int j;
	uint8_t tmp;
	memcpy(state->s, &rc4initial, n);
	j = 0;
	for (i = 0; i < n; i++) {
		j = (j + state->s[i] + key[i % keylen]) % n;
		tmp = state->s[i];
		state->s[i] = state->s[j];
		state->s[j] = tmp;
	}
	state->i = 0;
	state->j = 0;
}


void addRound(uint8_t * key, PTW_attackstate * state) {
	int j;
	uint8_t iv[3];
	uint8_t ks[KEYBYTES];
	rc4state rc4s;
	for (j = 0; j < IVBYTES; j++) {
		key[j] = rand()%n;
		iv[j] = key[j];
	}
	rc4init(key, KEYBYTES, &rc4s);
	for (j = 0; j < KEYBYTES; j++) {
		ks[j] = rc4update(&rc4s);
	}
	PTW_addsession(state, iv, ks);
	return;
}


void printStats(PTW_attackstate * state, int numsess, uint8_t * key) {
	uint8_t buf[13];
	if (PTW_computeKey(state, buf, 13, KEYLIMIT) == 1) {
		printf("ok at %d\n", numsess);
	} else {
		printf("wrong at %d\n", numsess);
	}
	return;
}


void doSim() {
	uint8_t key[KEYBYTES];
	int i,j;
	PTW_attackstate * state;
	state = PTW_newattackstate();
	printf("starting new sim\n");
	for (i = 0; i < HSBYTES; i++) {
		key[IVBYTES + i] = rand()%n;
	}
	
	for (i = 0; i < STARTSESS; i++) {
		addRound(key, state);
	}
	printStats(state, i, &key[3]);
	while (i < ENDSESS) {
		for (j = 0; j < INCSESS; j++) {
			addRound(key, state);
		}
		i += INCSESS;
		printStats(state, i, &key[3]);

	}
	printKey(&key[3], 13);
	PTW_freeattackstate(state);
}

int main(int argc, char ** argv) {
	srand(time(NULL) * getpid());
	printf("This is attacksim version 1.0.0\nFor more informations see http://www.cdc.informatik.tu-darmstadt.de/aircrack-ptw/\n");
		doSim();
	printf("done with all\n");
	return 0;
}

