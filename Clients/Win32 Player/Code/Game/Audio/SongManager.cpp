#include "Game/Audio/SongManager.hpp"
#include "Game/Audio/Song.hpp"
#include "Game/TheGame.hpp"
#include "Game/Renderables/VinylRecord.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "Engine/Renderer/Material.hpp"

SongManager* SongManager::instance = nullptr;

//-----------------------------------------------------------------------------------
SongManager::SongManager()
{

}

//-----------------------------------------------------------------------------------
SongManager::~SongManager()
{

}

//-----------------------------------------------------------------------------------
void SongManager::FlushSongQueue()
{
    Console::instance->RunCommand("stop");
    for (Song* song : m_songQueue)
    {
        delete song;
    }
    m_songQueue.clear();
}

//-----------------------------------------------------------------------------------
void SongManager::Update(float deltaSeconds)
{
    if (m_activeSong)
    {
        m_activeSong->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void SongManager::Play(Song* songToPlay)
{
    if (m_activeSong)
    {
        AudioChannelHandle channel = AudioSystem::instance->GetChannel(m_activeSong->m_fmodID);
        if (AudioSystem::instance->IsPlaying(channel))
        {
            AudioSystem::instance->StopChannel(channel);
        }
        FlushSongQueue();
    }
    m_activeSong = songToPlay;

    AudioSystem::instance->PlayLoopingSound(songToPlay->m_fmodID);
    IncrementPlaycount(songToPlay->m_filePath);

    //Load album art
    Texture* albumArtTexture = GetImageFromFileMetadata(songToPlay->m_filePath);
    if (albumArtTexture)
    {
        TheGame::instance->m_currentRecord->m_innerMaterial->SetDiffuseTexture(albumArtTexture);
    }
}

//-----------------------------------------------------------------------------------
void SongManager::Stop()
{
    if (!IsPlaying())
    {
        Console::instance->PrintLine("No song is currently playing. Play a song using playsong first.", RGBA::RED);
        return;
    }
    else
    {
        Console::instance->PrintLine("Stopping the music. Party's over, people. :c", RGBA::GBLIGHTGREEN);
        AudioSystem::instance->StopChannel(AudioSystem::instance->GetChannel(m_activeSong->m_fmodID));
        TheGame::instance->m_currentRecord->m_currentRotationRate = 0;
        FlushSongQueue();
    }
}

//-----------------------------------------------------------------------------------
bool SongManager::IsPlaying()
{
    if (!m_activeSong)
    {
        return false;
    }
    AudioChannelHandle channel = AudioSystem::instance->GetChannel(m_activeSong->m_fmodID);
    return (channel && AudioSystem::instance->IsPlaying(channel));
}

//CONSOLE COMMANDS/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(play)
{
    if (!(args.HasArgs(1) || args.HasArgs(2)))
    {
        Console::instance->PrintLine("play <filename> (rpm)", RGBA::RED);
        return;
    }
    std::string filepath = args.GetStringArgument(0);
    SoundID song = AudioSystem::instance->CreateOrGetSound(filepath);
    if (song == MISSING_SOUND_ID)
    {
        //Try again with the current working directory added to the path
        std::wstring cwd = Console::instance->GetCurrentWorkingDirectory();
        filepath = std::string(cwd.begin(), cwd.end()) + "\\" + filepath;
        song = AudioSystem::instance->CreateOrGetSound(filepath);

        if (song == MISSING_SOUND_ID)
        {
            Console::instance->PrintLine("Could not find file.", RGBA::RED);
            return;
        }
    }

    Song* newSong = new Song(filepath);

    float frequencyMultiplier = 1.0f;
    if (args.HasArgs(2))
    {
        float rpm = args.GetFloatArgument(1);
        frequencyMultiplier = rpm / TheGame::instance->m_currentRecord->m_baseRPM;
        TheGame::instance->m_currentRecord->m_currentRotationRate = TheGame::instance->CalculateRotationRateFromRPM(rpm);
    }
    else
    {
        TheGame::instance->m_currentRecord->m_currentRotationRate = TheGame::RPS_45;
    }

    SongManager::instance->Play(newSong);

    newSong->m_baseFrequency = AudioSystem::instance->GetFrequency(song);
    newSong->m_targetFrequency = newSong->m_baseFrequency * frequencyMultiplier;
    newSong->m_currentFrequency = newSong->m_targetFrequency;
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(stop)
{
    UNUSED(args)
    SongManager::instance->Stop();
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(pause)
{
    Console::instance->RunCommand("setrpm 0");
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(setrpm)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("setrpm <rpm>", RGBA::RED);
        return;
    }

    if (!SongManager::instance->IsPlaying())
    {
        Console::instance->PrintLine("No song is currently playing. Play a song using playsong first.", RGBA::RED);
        return;
    }

    float rpm = args.GetFloatArgument(0);
    float frequencyMultiplier = rpm / TheGame::instance->m_currentRecord->m_baseRPM;
    TheGame::instance->m_currentRecord->m_currentRotationRate = TheGame::instance->CalculateRotationRateFromRPM(rpm);
    SongManager::instance->m_activeSong->m_targetFrequency = SongManager::instance->m_activeSong->m_baseFrequency * frequencyMultiplier;
}