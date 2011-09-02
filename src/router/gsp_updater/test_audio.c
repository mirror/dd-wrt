#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>
#include <math.h>

#define PI  ((float) 3.141592654)
#define MAXFREQ   50000
#define BUFSIZE   (2*MAXFREQ)

short buf[BUFSIZE];

static short linear16FromuLaw(unsigned char uLawByte) {
  static int exp_lut[8] = {0,132,396,924,1980,4092,8316,16764};
	int sign;
  uLawByte = ~uLawByte;

  sign = (uLawByte & 0x80) != 0;
  unsigned char exponent = (uLawByte>>4) & 0x07;
  unsigned char mantissa = uLawByte & 0x0F;

  short result = exp_lut[exponent] + (mantissa << (exponent+3));
  if (sign) result = -result;
  return result;
}
#define         BIAS            (0x84)      /* Bias for linear code. */
#define CLIP 32635
static unsigned char uLawFrom16BitLinear(unsigned short sample) {
  static int exp_lut[256] = {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
           4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
           5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
           5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
           6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
           6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
           6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
           6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
  unsigned char sign = (sample >> 8) & 0x80;
  if (sign != 0) sample = -sample; // get the magnitude
  if (sample > CLIP) sample = CLIP; // clip the magnitude
  sample += BIAS;

  unsigned char exponent = exp_lut[(sample>>7) & 0xFF];
  unsigned char mantissa = (sample >> (exponent+3)) & 0x0F;
  unsigned char result = ~(sign | (exponent << 4) | mantissa);
  if (result == 0 ) result = 0x02;  // CCITT trap

  return result;
}

int mksinebuf(short *bp, int rf, int wf)
{
  float radspersample;
  int samplespercycle, i;
  short val;

  samplespercycle = rf / wf;
  radspersample = (2 * PI) / samplespercycle;

  for (i = 0; (i < samplespercycle); i++) {
    val = (short) (32767 * sin(i * radspersample));
    *bp++ = val;
    *bp++ = val;
  }

  return(samplespercycle);
}


int main(int argc, char **argv)
{
	unsigned char raw[2048];
	int i, fd;
	int size;
	char rec_name[64];
	char play_name[64];
	char file_name[64];

	memset(rec_name, 0, 64);
	memset(play_name, 0, 64);
	memset(file_name, 0, 64);

	int frames;

	snd_pcm_t *handle;

	snd_pcm_t *handle2;

	if (argc < 2) {
		printf("\n");
		printf("\ttest_audio 1 <chan>       \tloopback chan to itself\n");
		printf("\ttest_audio 2 <chan>       \trecord chan to audio<chan>.bin\n");
		printf("\ttest_audio 3 <chan>       \tplay audio<chan>.bin to chan\n");
		printf("\ttest_audio 4 <freq> <chan>\tplay sine wave freq to chan\n");
		printf("\ttest_audio 5 <chan>       \tplay nothing to chan\n");
		printf("\ttest_audio 6 <chan>       \tplay fixed output and record from chan\n");
		printf("\ttest_audio 7 <chan>       \t?\n");
		printf("\ttest_audio 8 <chan>       \tloopback chan to itself at 32K\n");
		printf("\n");
		exit(1);
	}

	if (atol(argv[1]) == 1)
	{
		sprintf(rec_name, "%s%i", "record_channel", atol(argv[2]));
		sprintf(play_name, "%s%i", "play_channel", atol(argv[2]));

    snd_pcm_open(&handle, rec_name, SND_PCM_STREAM_CAPTURE, 0);
    snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 500000);

    snd_pcm_open(&handle2, play_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    snd_pcm_set_params(handle2, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 50000);
		while (1)
		{

		  while ( (frames = snd_pcm_readi(handle, raw, 40)) < 0) {
    		frames = snd_pcm_recover(handle, frames, 1);
  		}

  		while ( (frames = snd_pcm_writei(handle2, raw, 40)) < 0) {
    		snd_pcm_recover(handle2, frames, 1);
  		}
		}
		exit(1);
	}
	else if (atol(argv[1]) == 2)
	{
		sprintf(rec_name, "%s%i", "record_channel", atol(argv[2]));
		sprintf(file_name, "audio%i.bin", atol(argv[2]));

    snd_pcm_open(&handle, rec_name, SND_PCM_STREAM_CAPTURE, 0);
    snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 500000);

		fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT);

		while (1)
		{
		  while ( (frames = snd_pcm_readi(handle, raw, 20)) < 0) // Convert to Frames
		  {
				printf("recover\n");
		    frames = snd_pcm_recover(handle, frames, 1);
		  }
			//printf("write %i\n", frames * 2);
			write(fd, raw, frames * 2);
		}
		close(fd);
		exit(1);
	}
	else if (atol(argv[1]) == 3)
	{
		sprintf(play_name, "%s%i", "play_channel", atol(argv[2]));
		sprintf(file_name, "audio%i.bin", atol(argv[2]));

    snd_pcm_open(&handle2, play_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    snd_pcm_set_params(handle2, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 50000);

		fd = open(file_name, O_RDONLY);

		while (i = read(fd, raw, 80))
		{
		  while ( (frames = snd_pcm_writei(handle2, raw, 40)) < 0)
		  {
		    snd_pcm_recover(handle2, frames, 1);
		  }
		}
		close(fd);
		exit(1);
	}
	else if (atol(argv[1]) == 4)
	{
		sprintf(play_name, "%s%i", "play_channel", atol(argv[3]));

    snd_pcm_open(&handle2, play_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    snd_pcm_set_params(handle2, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 50000);

		size = mksinebuf(buf, 8000, atol(argv[2]));
		for (i = 0; i < size; i++)
		{
			printf("buf[%i] = %i\n", i, buf[i]);
		}
		while (1)
		{
		  while ( (frames = snd_pcm_writei(handle2, buf, size)) < 0)
		  {
				//printf("recover - frames = %i, size = %i, channel = %i\n", frames, size, atol(argv[3]));
		    snd_pcm_recover(handle2, frames, 1);
		  }
		}
		close(fd);
		exit(1);
	}
	else if (atol(argv[1]) == 5)
	{
		sprintf(play_name, "%s%i", "play_channel", atol(argv[2]));

    snd_pcm_open(&handle2, play_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    snd_pcm_set_params(handle2, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 50000);

		memset(buf, 0, BUFSIZE * 2);
		while (1)
		{
		  while ( (frames = snd_pcm_writei(handle2, buf, 40)) < 0)
		  {
		    snd_pcm_recover(handle2, frames, 1);
		  }
		}
		close(fd);
		exit(1);
	}
	else if (atol(argv[1]) == 6)
	{
		sprintf(play_name, "%s%i", "play_channel", atol(argv[2]));
		sprintf(rec_name, "%s%i", "record_channel", atol(argv[2]));

		sprintf(file_name, "audio%i.bin", atol(argv[2]));

    snd_pcm_open(&handle2, play_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    snd_pcm_set_params(handle2, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 50000);

    snd_pcm_open(&handle, rec_name, SND_PCM_STREAM_CAPTURE, 0);
    snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 500000);

		fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT);

		for (i = 0; i < 80; i++)
			buf[i] = i + (atol(argv[2]) * 0x100);
		while (1)
		{
		  while ( (frames = snd_pcm_writei(handle2, buf, 40)) < 0)
		  {
		    snd_pcm_recover(handle2, frames, 1);
		  }
		  while ( (frames = snd_pcm_readi(handle, raw, 40)) < 0) // Convert to Frames
		  {
				printf("recover\n");
		    frames = snd_pcm_recover(handle, frames, 1);
		  }
			//printf("write %i\n", frames * 2);
			write(fd, raw, frames * 2);
		}
		close(fd);
		exit(1);
	}
	else if (atol(argv[1]) == 7)
	{
		sprintf(play_name, "%s%i", "play_channel", atol(argv[3]));

    snd_pcm_open(&handle2, play_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    snd_pcm_set_params(handle2, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 50000);

		buf[0] = 0;
		buf[1] = 0;
		buf[2] = 0xFFFF;
		buf[3] = 0xFFFF;
		while (1)
		{
		  while ( (frames = snd_pcm_writei(handle2, buf, 4)) < 0)
		  {
				//printf("recover - frames = %i, size = %i, channel = %i\n", frames, size, atol(argv[3]));
		    snd_pcm_recover(handle2, frames, 1);
		  }
		}
		close(fd);
		exit(1);
	} else if (atol(argv[1]) == 8) {
		sprintf(rec_name, "%s%i", "record_channel", atol(argv[2]));
		sprintf(play_name, "%s%i", "play_channel", atol(argv[2]));

    snd_pcm_open(&handle, rec_name, SND_PCM_STREAM_CAPTURE, 0);
    snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 32000, 1, 500000);

    snd_pcm_open(&handle2, play_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    snd_pcm_set_params(handle2, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 32000, 1, 50000);
		while (1)
		{

		  while ( (frames = snd_pcm_readi(handle, raw, 40)) < 0) {
    		frames = snd_pcm_recover(handle, frames, 1);
  		}

  		while ( (frames = snd_pcm_writei(handle2, raw, 40)) < 0) {
    		snd_pcm_recover(handle2, frames, 1);
  		}
		}
		exit(1);
	}


}
