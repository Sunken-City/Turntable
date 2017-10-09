#pragma once
#include <deque>
#include "Engine\Core\Events\Event.hpp"
#include "Engine\Core\Events\NamedProperties.hpp"

class Song;

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

    //EVENTS/////////////////////////////////////////////////////////////////////
    void OnSongPlaybackFinished();

    void AwardExpForSongProgress(bool isSongFinished = false);

    void OnSongBeginPlay();

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static SongManager* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::deque<Song*> m_songQueue;
    Event<> m_eventSongFinished;
    Event<> m_eventSongBeginPlay;
    Song* m_activeSong = nullptr;
    float m_currentRPM = 0.0f;
    float m_lastRPM = 0.0f; //Cached value of previous rpm before pausing
    LoopMode m_loopMode = NO_LOOP;
    bool m_wiggleRPM = false;
    float m_wiggleDelta = 1.0f;
    unsigned int m_lastPlaybackPositionMS = 0;
    float m_baseFrequency = 0.0f; //Frequency of what the song is supposed to be played at, based on what kind of vinyl we're playing
    float m_targetFrequency = 0.0f; //The target frequency of the track. m_currentFrequency lerps towards this value.
    float m_currentFrequency = 0.0f; //Actual frequency of the track that was last passed to FMOD
    float m_songVolume = 0.8f;
};

//UI EVENTS/////////////////////////////////////////////////////////////////////
void OnSkipNext(NamedProperties& params = NamedProperties::NONE);
void OnSkipBack(NamedProperties& params = NamedProperties::NONE);
void OnTogglePlayPause(NamedProperties& params = NamedProperties::NONE);