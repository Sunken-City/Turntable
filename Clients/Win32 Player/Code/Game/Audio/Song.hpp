#pragma once

#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include <string>
#include "Engine/Audio/Audio.hpp"

class Song
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Song(const std::string& fullPathToFile);
    ~Song();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetMetadataFromFile(const std::string& fileName);
    void Update(float deltaSeconds);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    //Metadata
    std::string m_filePath;
    std::string m_fileName;
    std::string m_fileExtension;
    std::string m_artist;
    std::string m_album;
    std::string m_genre;
    std::string m_title;
    int m_trackNum;
    int m_year;
    int m_playcount;
    int m_lengthInSeconds;
    int m_bitdepth;
    int m_samplerate;
    int m_bitrate;
    int m_numChannels;

    //Variables
    float m_baseFrequency; //Frequency of what the song is supposed to be played at, based on what kind of vinyl we're playing
    float m_targetFrequency; //The target frequency of the track. m_currentFrequency lerps towards this value.
    float m_currentFrequency; //Actual frequency of the track that was last passed to FMOD
    unsigned int m_lastPlaybackPositionMS = 0;
    AudioChannelHandle m_fmodChannel = nullptr;
    SoundID m_fmodID = MISSING_SOUND_ID;
};
