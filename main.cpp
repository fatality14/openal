#include <iostream>

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "sndfile.h"

#include "al.h"
#include "alext.h"

/*struct RIFF_Header {
    char chunkID[4];
    long chunkSize;
    char format[4];
};

struct WAVE_Format {
    char subChunkID[4];
    long subChunkSize;
    short audioFormat;
    short numChannels;
    long sampleRate;
    long byteRate;
    short blockAlign;
    short bitsPerSample;
};

struct WAVE_Data {
    char subChunkID[4];
    long subChunk2Size;
};

bool loadWavFile(const char* filename, WAVE_Format& wave_format,
                 RIFF_Header& riff_header, WAVE_Data& wave_data,
                 unsigned char*& data) {
  FILE* soundFile = NULL;

  try {
    soundFile = fopen(filename, "rb");
    if (!soundFile)
      throw (filename);

    fread(&riff_header, sizeof(RIFF_Header), 1, soundFile);

    if ((riff_header.chunkID[0] != 'R' ||
         riff_header.chunkID[1] != 'I' ||
         riff_header.chunkID[2] != 'F' ||
         riff_header.chunkID[3] != 'F') &&
        (riff_header.format[0] != 'W' ||
         riff_header.format[1] != 'A' ||
         riff_header.format[2] != 'V' ||
         riff_header.format[3] != 'E'))
             throw ("Invalid RIFF or WAVE Header");

    fread(&wave_format, sizeof(WAVE_Format), 1, soundFile);

    if (wave_format.subChunkID[0] != 'f' ||
        wave_format.subChunkID[1] != 'm' ||
        wave_format.subChunkID[2] != 't' ||
        wave_format.subChunkID[3] != ' ')
             throw ("Invalid Wave Format");

    if (wave_format.subChunkSize > 16)
        fseek(soundFile, sizeof(short), SEEK_CUR);

    fread(&wave_data, sizeof(WAVE_Data), 1, soundFile);

    if (wave_data.subChunkID[0] != 'd' ||
        wave_data.subChunkID[1] != 'a' ||
        wave_data.subChunkID[2] != 't' ||
        wave_data.subChunkID[3] != 'a')
             throw ("Invalid data header");

    //wave_data.subChunk2Size = 32;
    data = new unsigned char[wave_data.subChunk2Size];

    if (!fread(data, sizeof (unsigned char), wave_data.subChunk2Size, soundFile))
        throw ("error loading WAVE data into struct!");

    fclose(soundFile);
    return true;
  } catch(char* error) {
    if (soundFile != NULL)
        fclose(soundFile);
    return false;
  }
}

void reverseChunks(unsigned char*& data, size_t arrSize, size_t chunkSize){
    //arrSize /= 2;
    size_t chunkAmount = arrSize/chunkSize;//3
    size_t lastChunkSize = arrSize - chunkAmount*chunkSize;//6

    const int shift = 0;
    unsigned char* newdata = new unsigned char[arrSize-shift];
    for(size_t i = shift; i < arrSize; ++i){
        newdata[i-shift] = data[i];
    }
    delete[] data;
    data = newdata;

    unsigned char c;
    for(size_t i = 0; i < chunkAmount; ++i){
        for(size_t j = 0; j < chunkSize/2; ++j){
            c = data[chunkSize - j + i * chunkSize - 1];
            data[chunkSize - j + i * chunkSize - 1] = data[j + i * chunkSize];
            data[j + i * chunkSize] = c;
            //std::cout << chunkSize - j + i * chunkSize - 1 << std::endl;
            //std::cout << j + i * chunkSize << std::endl;
        }
    }
    for(size_t i = 0; i < lastChunkSize/2; ++i){
        c = data[arrSize - i - 1];
        data[arrSize - i - 1] = data[i + arrSize - lastChunkSize];
        data[i + arrSize - lastChunkSize] = c;
        //std::cout << arrSize - i - 1 << std::endl;
        //std::cout << i + arrSize - lastChunkSize << std::endl;
    }
}

bool createALBuffer(WAVE_Format& wave_format, RIFF_Header& riff_header,
                    WAVE_Data& wave_data, unsigned char* data,
                    ALuint* buffer, ALsizei* size,
                    ALsizei* frequency, ALenum* format) {
  try {
    *size = wave_data.subChunk2Size;
    *frequency = wave_format.sampleRate;

    if (wave_format.numChannels == 1) {
        if (wave_format.bitsPerSample == 8 )
            *format = AL_FORMAT_MONO8;
        else if (wave_format.bitsPerSample == 16)
            *format = AL_FORMAT_MONO16;
    } else if (wave_format.numChannels == 2) {
        if (wave_format.bitsPerSample == 8 )
            *format = AL_FORMAT_STEREO8;
        else if (wave_format.bitsPerSample == 16)
            *format = AL_FORMAT_STEREO16;
    }

    alGenBuffers(1, buffer);
    alBufferData(*buffer, *format, (void*)data,
                 *size, *frequency);
    return true;
  } catch(char* error) {
    return false;
  }
}

int main(){

    //Sound play data
    ALint state;                            // The state of the sound source
    ALuint bufferID;                        // The OpenAL sound buffer ID
    ALuint sourceID;                        // The OpenAL sound source
    ALenum format;                          // The sound data format
    ALsizei freq;                           // The frequency of the sound data
    ALsizei size;                           // Data size

    ALCdevice* device = alcOpenDevice(NULL);
    ALCcontext* context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    // Create sound buffer and source
    alGenBuffers(1, &bufferID);
    alGenSources(1, &sourceID);

    // Set the source and listener to the same location
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);

    WAVE_Format wave_format;
    RIFF_Header riff_header;
    WAVE_Data wave_data;
    unsigned char* data;
    loadWavFile("C:\\Users\\1234\\Documents\\oal\\1234.wav", wave_format, riff_header, wave_data, data);
    reverseChunks(data, wave_data.subChunk2Size, 2);
    createALBuffer(wave_format, riff_header, wave_data, data, &bufferID, &size, &freq, &format);

    alSourcei(sourceID, AL_BUFFER, bufferID);

    alSourcePlay(sourceID);

    do{
        alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
    } while (state != AL_STOPPED);


    alDeleteBuffers(1, &bufferID);
    alDeleteSources(1, &sourceID);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}*/


struct RIFF_Header {
    char chunkID[4];
    long chunkSize;
    char format[4];
};

struct WAVE_Format {
    char subChunkID[4];
    long subChunkSize;
    short audioFormat;
    short numChannels;
    long sampleRate;
    long byteRate;
    short blockAlign;
    short bitsPerSample;
};

struct WAVE_Data {
    char subChunkID[4];
    long subChunk2Size;
};

bool loadWavFile(const char* filename, WAVE_Format& wave_format,
                 RIFF_Header& riff_header, WAVE_Data& wave_data,
                 unsigned char*& data) {
  FILE* soundFile = NULL;

  try {
    soundFile = fopen(filename, "rb");
    if (!soundFile)
      throw (filename);

    fread(&riff_header, sizeof(RIFF_Header), 1, soundFile);

    if ((riff_header.chunkID[0] != 'R' ||
         riff_header.chunkID[1] != 'I' ||
         riff_header.chunkID[2] != 'F' ||
         riff_header.chunkID[3] != 'F') &&
        (riff_header.format[0] != 'W' ||
         riff_header.format[1] != 'A' ||
         riff_header.format[2] != 'V' ||
         riff_header.format[3] != 'E'))
             throw ("Invalid RIFF or WAVE Header");

    fread(&wave_format, sizeof(WAVE_Format), 1, soundFile);

    if (wave_format.subChunkID[0] != 'f' ||
        wave_format.subChunkID[1] != 'm' ||
        wave_format.subChunkID[2] != 't' ||
        wave_format.subChunkID[3] != ' ')
             throw ("Invalid Wave Format");

    if (wave_format.subChunkSize > 16)
        fseek(soundFile, sizeof(short), SEEK_CUR);

    fread(&wave_data, sizeof(WAVE_Data), 1, soundFile);

    if (wave_data.subChunkID[0] != 'd' ||
        wave_data.subChunkID[1] != 'a' ||
        wave_data.subChunkID[2] != 't' ||
        wave_data.subChunkID[3] != 'a')
             throw ("Invalid data header");

    //wave_data.subChunk2Size = 32;
    data = new unsigned char[wave_data.subChunk2Size];

    if (!fread(data, sizeof (unsigned char), wave_data.subChunk2Size, soundFile))
        throw ("error loading WAVE data into struct!");

    fclose(soundFile);
    return true;
  } catch(char* error) {
    if (soundFile != NULL)
        fclose(soundFile);
    return false;
  }
}

void reverseChunks(unsigned short* data, size_t arrSize, size_t chunkSize){
    arrSize /= 2;
    size_t chunkAmount = arrSize/chunkSize;//3
    size_t lastChunkSize = arrSize - chunkAmount*chunkSize;//6

    size_t currShift = 0;
    unsigned short c;
    for(size_t i = 0; i < chunkAmount; ++i){
        for(size_t j = 0; j < chunkSize/2; ++j){
            c = data[chunkSize - j + i * chunkSize - 1];
            data[chunkSize - j + i * chunkSize - 1] = data[j + i * chunkSize];
            data[j + i * chunkSize] = c;
            //std::cout << chunkSize - j + i * chunkSize - 1 << std::endl;
            //std::cout << j + i * chunkSize << std::endl;
        }
        currShift += chunkSize;
    }
    for(size_t i = 0; i < lastChunkSize/2; ++i){
        c = data[arrSize - i - 1];
        data[arrSize - i - 1] = data[i + arrSize - lastChunkSize];
        data[i + arrSize - lastChunkSize] = c;
        //std::cout << arrSize - i - 1 << std::endl;
        //std::cout << i + arrSize - lastChunkSize << std::endl;
    }
}

bool createALBuffer(WAVE_Format& wave_format, RIFF_Header& riff_header,
                    WAVE_Data& wave_data, unsigned char* data,
                    ALuint* buffer, ALsizei* size,
                    ALsizei* frequency, ALenum* format) {
  try {
    *size = wave_data.subChunk2Size;
    *frequency = wave_format.sampleRate;

    if (wave_format.numChannels == 1) {
        if (wave_format.bitsPerSample == 8 )
            *format = AL_FORMAT_MONO8;
        else if (wave_format.bitsPerSample == 16)
            *format = AL_FORMAT_MONO16;
    } else if (wave_format.numChannels == 2) {
        if (wave_format.bitsPerSample == 8 )
            *format = AL_FORMAT_STEREO8;
        else if (wave_format.bitsPerSample == 16)
            *format = AL_FORMAT_STEREO16;
    }

    alGenBuffers(1, buffer);
    alBufferData(*buffer, *format, (void*)data,
                 *size, *frequency);
    return true;
  } catch(char* error) {
    return false;
  }
}

int main(){

    //Sound play data
    ALint state;                            // The state of the sound source
    ALuint bufferID;                        // The OpenAL sound buffer ID
    ALuint sourceID;                        // The OpenAL sound source
    ALenum format;                          // The sound data format
    ALsizei freq;                           // The frequency of the sound data
    ALsizei size;                           // Data size

    ALCdevice* device = alcOpenDevice(NULL);
    ALCcontext* context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    // Create sound buffer and source
    alGenBuffers(1, &bufferID);
    alGenSources(1, &sourceID);

    // Set the source and listener to the same location
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);

    WAVE_Format wave_format;
    RIFF_Header riff_header;
    WAVE_Data wave_data;
    unsigned char* data;
    loadWavFile("C:\\Users\\1234\\Documents\\oal\\1234.wav", wave_format, riff_header, wave_data, data);
    reverseChunks((unsigned short*)data, wave_data.subChunk2Size, 2);
    createALBuffer(wave_format, riff_header, wave_data, data, &bufferID, &size, &freq, &format);

    alSourcei(sourceID, AL_BUFFER, bufferID);

    alSourcePlay(sourceID);

    do{
        alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
    } while (state != AL_STOPPED);


    alDeleteBuffers(1, &bufferID);
    alDeleteSources(1, &sourceID);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}
