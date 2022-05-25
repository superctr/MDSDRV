const char* note_name[17] =
{
	"cn","cs","df","dn","ds","ef","en","fn","fs","gf","gn","gs","af","an","as","bf","bn",
};
const int note_value[17] =
{
	0,1,1,2,3,3,4,5,6,6,7,8,8,9,10,10,11
};

enum cmd_flag
{
	FLG_FINISH = 1,
	FLG_LP = 2,
	FLG_LPF = 4,
	FLG_LPB = 8,
	FLG_PAT_ID = 16,
	FLG_INS_ID = 32,
	FLG_PCM_ID = 64,
	FLG_PEG_ID = 128,
	FLG_FLG_ID = 256,
	FLG_MTAB_ID = 512,
};

struct cmd_def
{
	const char* name;
	int length; // length of arguments, -1 indicates 8-byte offset, -2 indicates 16-byte offset
	int flag;
};

#define rest_cmd 0x80
#define tie_cmd 0x81
#define note_start 0x82
#define note_end 0xdf
#define chn_cmd_base(cmd) (0xe0 + cmd)
#define chn_cmd_cnt 19

const struct cmd_def chn_cmd[chn_cmd_cnt] =
{
	{"slr", 0, 0},					// 0 slur
	{"ins",	1, FLG_INS_ID},			// 1 set instrument
	{"vol",	1, 0},					// 2 set volume
	{"volm", 1, 0},					// 3 change volume
	{"trs",	1, 0},					// 4 set transpose
	{"trsm", 1, 0},					// 5 change transpose
	{"dtn",	1, 0},					// 6 set detune
	{"pta",	1, 0},					// 7 set portamento
	{"peg",	1, FLG_PEG_ID},			// 8 set pitch envelope
	{"pan",	1, 0},					// 9 set panning
	{"lfo",	1, 0},					//10 set lfo
	{"mtab", 1, FLG_MTAB_ID},		//11 set macro table
	{"flg",	1, FLG_FLG_ID},			//12 set flag
	{"fmcreg", 2, 0},				//13 fm channel write
	{"fmtl", 2, 0},					//14 fm tl write
	{"fmtlm", 2, 0},				//15 fm tl change
	{"pcm", 1, FLG_PCM_ID},			//16 pcm instrument
	{"pcmrate", 1, 0},				//17 pcm mix rate
	{"pcmmode", 1, 0}				//18 pcm mix mode
};

#define sys_cmd_base(cmd) (0x100-sys_cmd_cnt + cmd)
#define sys_cmd_cnt 11
const struct cmd_def sys_cmd[sys_cmd_cnt] =
{
	{"jump", -2, FLG_FINISH},		//-11 jump
	{"fmreg", 2, 0},				//-10 fm register write
	{"dmfinish", 1, FLG_FINISH},	// -9 drum mode finish
	{"comm", 1, 0},					// -8 communication byte
	{"tempo", 1, 0},				// -7 set tempo
	{"lp", 0, FLG_LP},				// -6 loop start
	{"lpf", 1, FLG_LPF},			// -5 loop finish
	{"lpb", -1, FLG_LPB},			// -4 loop break
	{"lpbl", -2, FLG_LPB},			// -3 loop break long
	{"pat", 1, FLG_PAT_ID},			// -2 subroutine(pattern)
	{"finish", 0, FLG_FINISH}		// -1 finish
};
