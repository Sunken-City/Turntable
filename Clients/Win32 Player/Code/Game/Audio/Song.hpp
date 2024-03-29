#pragma once

#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include <string>
#include "Engine/Audio/Audio.hpp"
#include "Game/Audio/SongCache.hpp"
#include "Game/Audio/SongState.hpp"

class Song
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Song(const std::wstring& fullPathToFile, SongID songID, SongState::State songState);
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
    int m_lengthInSeconds;
    int m_bitdepth;
    int m_samplerate;
    int m_bitrate;
    int m_numChannels;
    int m_uncompressedFileSizeBytes;
    bool m_ignoresFrequency = false; //MIDI's can't have frequency set on them without failing to play.

    //Variables
    AudioChannelHandle m_audioChannelHandle = nullptr;
    RawSoundHandle m_songHandle = nullptr;
    SongID m_songID = 0;
    SongState::State m_state = SongState::NOT_LOADED;
};
