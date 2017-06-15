#pragma once
#include <deque>

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
    void Stop();
    bool IsPlaying();
    void AddToQueue(Song* newSong);
    unsigned int GetQueueLength();
    inline void SetLoopMode(LoopMode mode) { m_loopMode = mode; };

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static SongManager* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::deque<Song*> m_songQueue;
    Song* m_activeSong = nullptr;
    LoopMode m_loopMode = SONG_LOOP;
};