#pragma once
#include <deque>
#include "Engine\Core\Events\Event.hpp"

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
    unsigned int GetQueueLength();
    inline void SetLoopMode(LoopMode mode) { m_loopMode = mode; };
    void OnSongPlaybackFinished();
    void OnSongBeginPlay();
    void StopSong();
    void SetRPM(float rpm, bool changeInstantly = false);

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static SongManager* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::deque<Song*> m_songQueue;
    Event<> m_eventSongFinished;
    Event<> m_eventSongBeginPlay;
    Song* m_activeSong = nullptr;
    LoopMode m_loopMode = NO_LOOP;
};