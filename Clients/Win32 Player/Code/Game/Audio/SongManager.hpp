#pragma once
#include <deque>

class Song;

class SongManager
{
public:
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
    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static SongManager* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::deque<Song*> m_songQueue;
    Song* m_activeSong = nullptr;
};