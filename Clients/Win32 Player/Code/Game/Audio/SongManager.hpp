#pragma once
#include <list>
#include "Engine\Core\Events\Event.hpp"
#include "Engine\Core\Events\NamedProperties.hpp"
#include "SongCache.hpp"

class Song;
struct XMLNode;
//typedef std::_List_iterator<std::_List_val<std::_List_simple_types<Song*>>> SongIterator;
typedef std::list<Song*>::iterator SongIterator;

class SongManager
{
public:
    //ENUMS/////////////////////////////////////////////////////////////////////
    enum LoopMode
    {
        NO_LOOP,
        QUEUE_LOOP,
        SONG_LOOP,
        NUM_LOOP_MODES
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    SongManager();
    ~SongManager();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void FlushSongQueue();
    void Update(float deltaSeconds);
    void Play(Song* songToPlay);
    void StopAll();
    bool IsPlaying();
    void AddToQueue(Song* newSong);
    void PlayNext(Song* newSong);
    unsigned int GetQueueLength();
    inline void SetLoopMode(LoopMode mode) { m_loopMode = mode; };
    void StopSong();
    void SetRPM(float rpm, bool changeInstantly = false);
    void CheckForHotkeys(); 
    void SetNowPlayingTextFromMetadata(Song* currentSong);
    void UpdateUIWidgetText();
    void SavePlaylist(const std::string& name);
    void AddToPlaylist(XMLNode& playlist, Song* currentSong);
    XMLNode OpenPlaylist(const std::string& name);
    bool CheckForPlaylistOnDisk(const std::string& name) const;
    bool CheckForSongOnDisk(std::wstring& filepath) const;
    void LoadPlaylist(const XMLNode& playlist);
    void StartLoadingSound();
    void QueueRandomSong(bool playWholeAlbum = false);
    void LoadAlbumArt(Song* songToPlay);

    //EVENTS/////////////////////////////////////////////////////////////////////
    void OnSongPlaybackFinished();

    void AwardExpForSongProgress(bool isSongFinished = false);

    void OnSongBeginPlay();

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static SongManager* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::list<Song*> m_songQueue;
    SongIterator m_songPositionInQueue;
    //std::deque<Song*> m_songQueue;
    SongCache m_songCache;
    Event<> m_eventSongFinished;
    Event<> m_eventSongBeginPlay;
    Song* m_activeSong = nullptr;
    SoundID m_needleDropSound;
    SoundID m_recordCracklesSound;
    AudioChannelHandle m_recordCracklesHandle = nullptr;
	DSPHandle* m_dsp = nullptr;
    DSPConnection* m_dspConnection = nullptr;
    float m_currentRPM = 0.0f;
    float m_lastRPM = 0.0f; //Cached value of previous rpm before pausing
    float m_wiggleDelta = 1.0f;
    float m_baseFrequency = 0.0f; //Frequency of what the song is supposed to be played at, based on what kind of vinyl we're playing
    float m_targetFrequency = 0.0f; //The target frequency of the track. m_currentFrequency lerps towards this value.
    float m_currentFrequency = 0.0f; //Actual frequency of the track that was last passed to FMOD
    float m_songVolume = 0.8f;
    unsigned int m_lastPlaybackPositionMS = 0;
    bool m_wiggleRPM = false;
    bool m_loadAlbumArt = true;
    LoopMode m_loopMode = NO_LOOP;
};

//UI EVENTS/////////////////////////////////////////////////////////////////////
void OnSkipNext(NamedProperties& params = NamedProperties::NONE);
void OnSkipBack(NamedProperties& params = NamedProperties::NONE);
void OnTogglePlayPause(NamedProperties& params = NamedProperties::NONE);