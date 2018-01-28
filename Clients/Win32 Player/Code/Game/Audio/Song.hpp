#pragma once

#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include <string>
#include "Engine/Audio/Audio.hpp"
#include "Game/Audio/SongCache.hpp"

class Song
{
public:
    enum State
    {
        NOT_LOADED,
        LOADING,
        READY_TO_PLAY,
        NUM_STATES
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Song(const std::wstring& fullPathToFile, SongID songID);
    ~Song();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetMetadataFromFile(const std::wstring& fileName);
    void Update(float deltaSeconds);
    void RequestSongHandle();
    void GenerateProceduralAlbumArt();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    //Metadata
    Texture* m_albumArt = nullptr;
    std::wstring m_filePath;
    std::string m_fileName;
    std::string m_fileExtension;
    std::string m_artist;
    std::string m_album;
    std::string m_genre;
    std::string m_title;
    int m_trackNum;
    int m_year;
    int m_playcount;
    int m_rating;
    int m_lengthInSeconds;
    int m_bitdepth;
    int m_samplerate;
    int m_bitrate;
    int m_numChannels;
    bool m_ignoresFrequency = false; //MIDI's can't have frequency set on them without failing to play.

    //Variables
    AudioChannelHandle m_audioChannelHandle = nullptr;
    RawSoundHandle m_songHandle = nullptr;
    SongID m_songID = 0;
    State m_state = NOT_LOADED;
};
