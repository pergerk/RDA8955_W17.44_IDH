/**********************************************************************

 

*******************************************************************//**
brief An effect that generates DTMF tones
*//*******************************************************************/
#include <cs_types.h>
#include <stdio.h>
#include "stdlib.h"
#include "dtmfgen.h"



/*
  --------------------------------------------
              1209 Hz 1336 Hz 1477 Hz 1633 Hz

                          ABC     DEF
   697 Hz          1       2       3       A

                  GHI     JKL     MNO
   770 Hz          4       5       6       B

                  PQRS     TUV     WXYZ
   852 Hz          7       8       9       C

                          oper
   941 Hz          *       0       #       D
  --------------------------------------------
  Essentially we need to generate two sin with
  frequencies according to this table, and sum
  them up.
  sin wave is generated by:
   s(n)=sin(2*pi*n*f/fs)

  We will precalculate:
     A= 2*pi*f1/fs
     B= 2*pi*f2/fs

  And use two switch statements to select the frequency

  Note: added support for letters, like those on the keypad
        This support is only for lowercase letters: uppercase
        are still considered to be the 'military'/carrier extra
        tones.
*/
#define FRAME_LEN 160
#define M_PI 3.1415926
//static const double kFadeInOut = 250.0; // used for fadein/out needed to remove clicking noise
//A = (fs / kFadeInOut);
const  unsigned int dtmfFadInoutTable[32] =
{
	0,  32, 64, 96,  128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480,
	512,544,576,608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992,  
};

 unsigned int g_amplitude = 16384;
 unsigned int f1, f2 = 0;
 unsigned int g_durationnum = 480;
 unsigned int g_silencennum = 160;
 unsigned int g_curSeqPos = 0;

 short sinvalue_1[3] = {0, 0, 0};
 int   sinparam_1[2];
 short sinvalue_2[3] = {0, 0, 0};
 int   sinparam_2[2];



 DTMFGEN_STATE g_DtmfGenState = DTMFGEN_STATE_IDLE;
 DTMFGEN_HANDLER_T g_DtmfgenHandler = NULL;

void StartDtmfTone(unsigned short tone,unsigned int duration, unsigned int silencetime, unsigned int amplitudelevel, unsigned int fs, DTMFGEN_HANDLER_T handle  )
{
 
	g_durationnum = duration*fs/1000;
	g_silencennum = silencetime*fs/1000;
	g_DtmfgenHandler = handle;

     // select low tone: left column
     switch (tone) {
      case '1':   case '2':   case '3':    case 'A':  
         f1=697;
         break;
      case '4':   case '5':   case '6':    case 'B': 
         f1=770;
         break;
      case '7':   case '8':   case '9':    case 'C': 
         f1=852;
         break;
      case '*':   case '0':   case '#':     case 'D': 
         f1=941;
         break;
      default:
         f1=0;
   }

     // select high tone: top row
     switch (tone) {
      case '1':   case '4':   case '7':   case '*':
         f2=1209;
         break;
      case '2':   case '5':   case '8':   case '0':
         f2=1336;
         break;
      case '3':   case '6':   case '9':   case '#':
         f2=1477;
         break;
      case 'A':   case 'B':   case 'C':   case 'D':
         f2=1633;
         break;
      default:
         f2=0;
     }

     if(f1 == 0)
     {
	   f1 = tone;
	   f2 = 0;
      }

 	sinparam_1[0] = 2*cos(2*M_PI*f1/fs)*16384;
	sinparam_1[1] =  -16384;
	sinvalue_1[0] = 0;       
	sinvalue_1[1] = sin(2*M_PI*f1/fs)*16384;
	sinvalue_1[2] = 0;

	sinparam_2[0] = 2*cos(2*M_PI*f2/fs)*16384;
	sinparam_2[1] =  -16384;
	sinvalue_2[0] = 0;       
	sinvalue_2[1] = sin(2*M_PI*f2/fs)*16384;
	sinvalue_2[2] = 0;

      switch(amplitudelevel)
      {
        case 0: //mute
            g_amplitude = 0;
            break;
        case 1: //level1 -42db
            g_amplitude = 130;
            break;
        case 2: //level2 -39db
            g_amplitude = 184;
            break;
        case 3: //level3 -36db
            g_amplitude = 260;
            break;
        case 4: //level4 -33db
            g_amplitude = 367;
            break;
        case 5: //level5 -30db
            g_amplitude = 518;
            break;
        case 6: //level6 -27db
            g_amplitude = 732;
            break;
        case 7: //level7 -24db
            g_amplitude = 1034;
            break;
        case 8: //level8 -21db
            g_amplitude = 1460;
            break;
        case 9: //level9 -18db
            g_amplitude = 2063;
            break;
        case 10: //level10 -15db
            g_amplitude = 2914;
            break;
        case 11: //level11 -12db
            g_amplitude = 4115;
            break;
        case 12: //level12 -9db
            g_amplitude = 5813;
            break;
        case 13: //level13 -6db
            g_amplitude = 8211;
            break;
        case 14: //level14 -3db
            g_amplitude = 11599;
            break;
        case 15: //level15 0db
            g_amplitude = 16384;
            break;
        default:
            g_amplitude = 16384;
            break;
	}
	g_DtmfGenState = DTMFGEN_STATE_TONE;
	g_curSeqPos = 0;
}


void StopDtmfTone()
{
	g_DtmfGenState = DTMFGEN_STATE_IDLE;
	g_amplitude = 16384;
       f1=f2=0;
       g_durationnum = 480;
       g_silencennum = 160;
	g_curSeqPos = 0;
	g_DtmfgenHandler = NULL;
 }

void MakeDtmfTone(short *buffer, unsigned int len, unsigned int fs,  unsigned int last, unsigned int total, unsigned int amplitude)
{
   unsigned int i;
   unsigned int offset;
   //double A,B;

   // precalculations
  // A=B=2*M_PI/fs;
   //A*=f1;
   //B*=f2;

   // now generate the wave: 'last' is used to avoid phase errors
   // when inside the inner for loop of the Process() function.
   for( i = 0; i < len; i++) 
   {
	   //f(n) = 2*cos(W*ts)*f(n-1)-f(n-2) by fixed-point
	  sinvalue_1[0] = (sinparam_1[0]*sinvalue_1[1]+sinparam_1[1]*sinvalue_1[2])>>14;
	  sinvalue_1[2] = sinvalue_1[1];
	  sinvalue_1[1] = sinvalue_1[0];
	  sinvalue_2[0] = (sinparam_2[0]*sinvalue_2[1]+sinparam_2[1]*sinvalue_2[2])>>14;
	  sinvalue_2[2] = sinvalue_2[1];
	  sinvalue_2[1] = sinvalue_2[0];

      buffer[i] = (amplitude*(int)(sinvalue_1[0] +sinvalue_2[0]))>>15;

         /*      
         buffer[i] = amplitude * 0.5 *
         (sin( A * (i + last)) +
          sin( B * (i + last)));
		  */
   }

   // generate a fade-in of duration 1/250th of second
   if (last == 0) 
   {
       for( i = 0; i < 32; i++) 
	   {
		  buffer[i] =  (buffer[i]*dtmfFadInoutTable[i])>>10;
       }
   }

   // generate a fade-out of duration 1/250th of second
   if (last == total - len) {
      // we are at the last buffer of 'len' size, so, offset is to
      // backup 'A' samples, from 'len'
    //  A = (fs / kFadeInOut);
       offset = (len) - (32);
      // protect against negative offset, which can occur if too a
      // small selection is made
      if (offset >= 0) {
         for( i = 0; i < 32; i++) {
            buffer[i + offset] = (buffer[i + offset]*dtmfFadInoutTable[31-i])>>10;
         }
      }
   }
   return ;
}

bool DTMFGen_Process(short* framebuff, unsigned short framelen)
{
	unsigned int processlen = 0;

	if(g_DtmfGenState == DTMFGEN_STATE_TONE )
	{
		if(g_durationnum > g_curSeqPos+framelen ) 
		{
	   		MakeDtmfTone(framebuff, framelen, 8000, g_curSeqPos,  g_durationnum, g_amplitude);
                     g_curSeqPos += framelen;
		}
     	       else
	       {
		       memset(framebuff, 0,framelen*sizeof(short) );
   		       MakeDtmfTone(framebuff, g_durationnum - g_curSeqPos, 8000, g_curSeqPos,  g_durationnum, g_amplitude);
		       g_curSeqPos = framelen + g_curSeqPos - g_durationnum;
			g_DtmfGenState = DTMFGEN_STATE_SILENCE;
		}
       }
	else if(g_DtmfGenState == DTMFGEN_STATE_SILENCE)
 	{ 
		if(g_silencennum > g_curSeqPos+framelen )
		{
	   	      memset(framebuff, 0,framelen*sizeof(short) );
                    g_curSeqPos += framelen;
		}
		else
		{
			memset(framebuff, 0,(g_silencennum - g_curSeqPos)*sizeof(short) );
		
			g_DtmfGenState = DTMFGEN_STATE_IDLE;
			if(g_DtmfgenHandler != NULL)
				g_DtmfgenHandler();
		}
	}
	
	return g_DtmfGenState == DTMFGEN_STATE_IDLE;
}


BOOL isDTMFGEN_Run()
{
    return g_DtmfGenState != DTMFGEN_STATE_IDLE;
}

const  DTMF_Tone_Struct DtmfArray[] =
{
	{'1',90, 50,},
	{'2',90, 50,},
	{'3',90, 50,},
	{'4',90, 50,},
	{'5',90, 50,},
	{'6',90, 50,},
	{'7',90, 50,},
	{'8',90, 50,},
	{'9',90, 50,},
	{'*',90, 50,},
	{'0',90, 50,},
	{'#',90, 50,},
	{'A',90, 50,},
	{'B',90, 50,},
	{'C',90, 50,},
	{'D',90, 50,},
	{1400,90, 50,},
	{2300,90, 50,},
	{1000,90, 50,},
};


 #if 0
short framebuff[160];
 
int main(int argc, char * argv[])
{
	 bool finishflag = false;

	// variable declares
	WavOutFile *outFile = new WavOutFile("dtmf.wav", 8000, 16, 1);

	for(int i=0; i<sizeof(DtmfArray)/sizeof(DTMF_Tone_Struct); i++)
	{
		StartDtmfTone(DtmfArray[i].tone, DtmfArray[i].duration, DtmfArray[i].silencetime, i%16+3,  8000, NULL);


	    while(1)
	    { 
			finishflag = DTMFGen_Process(framebuff,FRAME_LEN);
	 	    outFile->write(framebuff,FRAME_LEN  );
			if(finishflag)	break;
		}


	}

	
    delete outFile;

	return 0;
}
#endif
