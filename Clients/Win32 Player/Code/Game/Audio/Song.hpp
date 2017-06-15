#pragma once

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
    TagLib::String m_artist;
    TagLib::String m_album;
    TagLib::String m_genre;
    TagLib::String m_title;
    int m_trackNum;
    int m_year;
    int m_playcount;
    int m_lengthInSeconds;
    int m_bitdepth;
    int m_samplerate;
    int m_bitrate;
    int m_numChannels;

    //Variables
    float m_baseFrequency;
    float m_targetFrequency;
    float m_currentFrequency;
    AudioChannelHandle m_fmodChannel = nullptr;
    SoundID m_fmodID = MISSING_SOUND_ID;
};
