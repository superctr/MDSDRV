/*
	'mdsseq.inc' generator.
	Usage: gcc tools/gendef.cpp & a > mdsseq.inc
*/
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
#define chn_cmd_cnt 17
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
	"pan",		// 9 set panning
	"lfo",		//10 set lfo
	"mtab",		//11 set macro table
	"flg",		//12 set flag
	"fmcreg",	//13 fm channel write
	"fmtl",		//14 fm tl write
	"fmtlm",	//15 fm tl change
	"pcm",      //16 pcm instrument
};

#define sys_cmd_base(cmd) (0x100-sys_cmd_cnt + cmd)
#define sys_cmd_cnt 11
const char* sys_cmd[sys_cmd_cnt] =
{
	"jump",		//-11 jump
	"fmreg",	//-10 fm register write
	"dmfinish",	// -9 drum mode finish
	"comm",		// -8 communication byte
	"tempo",	// -7 set tempo
	"lp",		// -6 loop start
	"lpf",		// -5 loop finish
	"lpb",		// -4 loop break
	"lpbl",		// -3 loop break long
	"pat",		// -2 subroutine(pattern)
	"finish",	// -1 finish
};

int main()
{
	printf(";========================================================================\n");
	printf("; MDSDRV sequence format defines\n");
	printf("; This file is autogenerated by 'gendef.c'\n");
	printf(";========================================================================\n");
	printf("\tlist -\n");
	printf("\tif def(MML)&(~def(MMLDEF))\n");
	printf("MMLDEF equ $0002\n");
	printf(";========================================================================\n");
	printf("; note duration parameters (00-7f)\n");
	printf(";========================================================================\n");
	int note,oct;
	long i;
	long interval = 96;
	for(i=1;i<64;i<<=1)
	{
		printf("l%d equ $%02X\n",i, interval-1);
		if(interval < 96 && (~interval & 1))
			printf("l%dd equ $%02X\n",i, (interval + (interval>>1) - 1));
		interval >>= 1;
	}
	printf(";========================================================================\n");
	printf("; note commands (80-df)\n");
	printf(";========================================================================\n");
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
	printf(";========================================================================\n");
	printf("; channel commands (%02x-%02x)\n",chn_cmd_base(0),chn_cmd_base(chn_cmd_cnt-1));
	printf(";========================================================================\n");
	for(i=0;i<chn_cmd_cnt;i++)
	{
		printf("%s equ $%02X\n",chn_cmd[i],chn_cmd_base(i));
	}
	printf(";========================================================================\n");
	printf("; system commands (%02x-%02x)\n",sys_cmd_base(0),sys_cmd_base(sys_cmd_cnt-1));
	printf(";========================================================================\n");
	for(i=0;i<sys_cmd_cnt;i++)
	{
		printf("%s equ $%02X\n",sys_cmd[i],sys_cmd_base(i));
	}
	printf("\telse\n", note_start);
	printf(";========================================================================\n");
	printf("; system defines\n");
	printf(";========================================================================\n");
	printf("mds_rest equ $%02X\n", rest_cmd);
	printf("mds_tie equ $%02X\n", tie_cmd);
	printf("mds_note_start equ $%02X\n", note_start);
	printf("mds_chn_cmd_base equ $%02X\n", chn_cmd_base(0));
	printf("mds_sys_cmd_base equ $%02X\n", sys_cmd_base(0));
	printf("\tendif\n", note_start);
	printf("\tlist +\n");
}
