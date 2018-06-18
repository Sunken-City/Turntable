#pragma once

class SongState
{
public:
    enum State
    {
        NOT_LOADED,
        LOADING,
        LOADED,
        CANT_LOAD,
        PLAYING,
        UNLOADED,
        INVALID_STATE,
        NUM_STATES
    };
};