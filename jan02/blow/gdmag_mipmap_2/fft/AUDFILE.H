/*===========================================================================

	audfile.h  -  Don Cross, October 1994.

	A class for reading and writing AUD files.
	An AUD file is a floating-point audio format.

===========================================================================*/

#ifndef __ddc_audfile_h
#define __ddc_audfile_h

#pragma pack(1)

#include <stdio.h>
#include <ddc.h>

const char * const AudioFileSignature = "Floating point audio\x1aQwibnar5";
#define __MAX_AUDIO_FILE_CHANNELS 2
const unsigned MaxAudioFileChannels = __MAX_AUDIO_FILE_CHANNELS;


class WaveFile;


struct AudioFileHeaderInfo
{
	char     signature [32];
	UINT16   numChannels;
	UINT32   samplingRate;
	float    minValue;
	float    maxValue;
};


struct AudioFileHeader
{
	AudioFileHeaderInfo   info;
	char                  filler [4096 - sizeof(AudioFileHeaderInfo)];
};


class AudioFile
{
public:
	AudioFile();
	~AudioFile();

	DDCRET openForRead (
		const char *_filename );

	DDCRET read ( float *data, unsigned long numData );
	DDCRET readSample ( double sample [MaxAudioFileChannels] );
	DDCRET seekToSample ( unsigned long sampleIndex );
	DDCRET rewind ()   {return seekToSample(0);}

	// Must call openForRead before the following method
	DDCRET convertToWaveFile ( const char *waveFilename,
							   unsigned bitsPerSample = 16,
							   dBOOLEAN dcBias = FALSE );

	// Can call the following all alone
	DDCRET convertFromWaveFile ( const char *outAudioFilename,
								 WaveFile &inWave );

	DDCRET openForWrite (
		const char    *_filename,
		unsigned       _numChannels,
		unsigned long  _samplingRate );

	DDCRET openForWrite (
		const char    *_filename,
		const AudioFile &other )
	{
		return openForWrite ( 
			_filename, 
			other.queryNumChannels(), 
			other.querySamplingRate() );
	}

	DDCRET write ( const float *data, unsigned long numData );
	DDCRET writeSample ( const double sample [MaxAudioFileChannels] );

	DDCRET close();

	unsigned       queryNumChannels()    const  {return numChannels;}
	unsigned long  querySamplingRate()   const  {return samplingRate;}
	float          queryMinValue()       const  {return float(minValue);}
	float          queryMaxValue()       const  {return float(maxValue);}

	unsigned long queryNumSamples()  const {return numSamples;}

	double queryDuration() const	// return duration in seconds
	{
		return double(numSamples) / double(samplingRate);
	}

	const char *queryFilename() const
	{
		return filename;
	}

	DDCRET openForPatch (
		const char *_filename );

private:
	unsigned       numChannels;
	unsigned long  samplingRate;
	double         minValue;
	double         maxValue;

	unsigned long  numSamples;   // derived from file size on read

	dBOOLEAN       firstWrite;
	dBOOLEAN       flushHeader;

	char   *filename;
	FILE   *fp;
};

#pragma pack()

#endif // __ddc_audfile_h


/*--- end of file audfile.h ---*/
