#include "stdio.h"

const char* note_name[17] =
{
    "cn","cs","df","dn","ds","ef","en","fn","fs","gf","gn","gs","af","an","as","bf","bn",
};
const int note_value[17] =
{
	0,1,1,2,3,3,4,5,6,6,7,8,8,9,10,10,11
};

#define rest_cmd 0x80
#define tie_cmd 0x81
#define note_start 0x82
#define note_end 0xdf
#define chn_cmd_base(cmd) (0xe0 + cmd)
#define chn_cmd_cnt 18
const char* chn_cmd[chn_cmd_cnt] =
{
	"slr",		// 0 slur
	"ins",		// 1 set instrument
	"vol",		// 2 set volume
	"volm",		// 3 change volume
	"trs",		// 4 set transpose
	"trsm",		// 5 change transpose
	"dtn",		// 6 set detune
	"pta",		// 7 set portamento
	"peg",		// 8 set pitch envelope
	"lfo",		// 9 set lfo
	"pan",		//10 set panning
	"flg",		//11 set flag
	"pcm",		//12 set pcm instrument
	"lfod",		//13 set lfo delay
	"fmreg",	//14 fm register write
	"fmcreg",	//15 fm channel write
	"fmtl",		//16 fm tl write
	"fmtlm",	//17 fm tl change
};

#define sys_cmd_base(cmd) (0x100-sys_cmd_cnt + cmd)
#define sys_cmd_cnt 8
const char* sys_cmd[sys_cmd_cnt] =
{
	"tempo",	// 0 set tempo
	"lp",		// 1 loop start
	"lpf",		// 2 loop finish
	"lpb",		// 3 loop break
	"lpbl",		// 4 loop break long
	"pat",		// 5 subroutine(pattern)
	"dmode",	// 6 drum mode
	"finish",	// 7 finish
};

int main()
{
	printf("; MDSDRV defines (auto-generated)\n");
	printf("\tlist -\n");
	printf("\tif def(MML)&(~def(MMLDEF))\n");
	printf("MMLDEF equ 1\n");
	int note,oct;
	long i;
	long interval = 96;
	for(i=1;i<64;i<<=1)
	{
		printf("l%d equ $%02X\n",i, interval);
		if(interval < 96 && (~interval & 1))
			printf("l%dd equ $%02X\n",i, interval + (interval>>1));
		interval >>= 1;
	}
	printf("rest equ $%02X\n",rest_cmd);
	printf("tie equ $%02X\n",tie_cmd);
	for(i=0;i<9*17;i++)
	{
		int nn;
		oct = (i/17);
		note = i%17;
		nn = note_start +  (oct*12)+note_value[note];
		if(nn <= note_end)
		printf("%s%d equ $%02X\n",note_name[note],oct+1,nn);
	}
	for(i=0;i<chn_cmd_cnt;i++)
	{
		printf("%s equ $%02X\n",chn_cmd[i],chn_cmd_base(i));
	}
	for(i=0;i<sys_cmd_cnt;i++)
	{
		printf("%s equ $%02X\n",sys_cmd[i],sys_cmd_base(i));
	}
	printf("\telse\n", note_start);
	printf("mds_rest equ $%02X\n", rest_cmd);
	printf("mds_tie equ $%02X\n", tie_cmd);
	printf("mds_note_start equ $%02X\n", note_start);
	printf("mds_chn_cmd_base equ $%02X\n", chn_cmd_base(0));
	printf("mds_sys_cmd_base equ $%02X\n", sys_cmd_base(0));
	printf("\tendif\n", note_start);
	printf("\tlist +\n");
}
