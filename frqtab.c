#include "stdio.h"
#include "math.h"

int main()
{
	int octave;
	int note = 0;
	double ym2612_clk = 7670454;
	double ym2612_divider = ym2612_clk / 144./2048./32.;
	double tuning = 440;
	double fnum = 0;
	int offset = 24;

	printf("fmtab:\n");

	for(note = 0; note < 48; note ++)
	{
		fnum = tuning * pow(2,(note-offset+3)/12.) / ym2612_divider;

		if(fnum > 2047)
		{
			printf("; offset %d-%d\n", note - (note % 12), note);
			break;
		}
		else
		{
			if((note % 12) == 0)
				printf("\tdc.w ");
			else
				printf(",");
			printf("%d", (int)(fnum+0.5));
		}
		
		if((note % 12) == 11)
			printf("; offset %d-%d\n", note - (note % 12), note);	
	}
	
}
