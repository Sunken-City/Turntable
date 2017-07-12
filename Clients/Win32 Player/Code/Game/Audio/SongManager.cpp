#include "Game/Audio/SongManager.hpp"
#include "Game/Audio/Song.hpp"
#include "Game/TheGame.hpp"
#include "Game/Renderables/VinylRecord.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Audio/Audio.hpp"

SongManager* SongManager::instance = nullptr;

//-----------------------------------------------------------------------------------
SongManager::SongManager()
{
    m_eventSongFinished.RegisterMethod(this, &SongManager::OnSongPlaybackFinished);
    m_eventSongBeginPlay.RegisterMethod(this, &SongManager::OnSongBeginPlay);
}

//-----------------------------------------------------------------------------------
SongManager::~SongManager()
{
    StopAll();
}

//-----------------------------------------------------------------------------------
void SongManager::FlushSongQueue()
{
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
        if (m_wiggleRPM)
        {
            float wiggleAmount = MathUtils::GetRandomFloat(-m_wiggleDelta, m_wiggleDelta);
            SetRPM(m_currentRPM + wiggleAmount);
        }
        m_activeSong->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void SongManager::Play(Song* songToPlay)
{
    if (IsPlaying())
    {
        StopSong();
    }
    m_activeSong = songToPlay;

    AudioSystem::instance->PlayLoopingSound(songToPlay->m_fmodID, 0.8f); //TODO: Find out why PlaySound causes a linker error here
    if (m_loopMode != SONG_LOOP)
    {
        AudioSystem::instance->SetLooping(songToPlay->m_fmodID, false);
    }
    m_activeSong->m_fmodChannel = AudioSystem::instance->GetChannel(m_activeSong->m_fmodID);

    //Load album art
    Texture* albumArtTexture = GetImageFromFileMetadata(songToPlay->m_filePath);
    if (albumArtTexture)
    {
        TheGame::instance->m_currentRecord->m_innerMaterial->SetDiffuseTexture(albumArtTexture);
    }
}

//-----------------------------------------------------------------------------------
void SongManager::StopAll()
{
    Console::instance->PrintLine("Stopping the music. Party's over, people. :c", RGBA::GBLIGHTGREEN);
    if (IsPlaying())
    {
        AudioSystem::instance->StopChannel(AudioSystem::instance->GetChannel(m_activeSong->m_fmodID));
    }
    TheGame::instance->m_currentRecord->m_currentRotationRate = 0;

    FlushSongQueue();
    if (m_activeSong)
    {
        delete m_activeSong;
        m_activeSong = nullptr;
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

//-----------------------------------------------------------------------------------
void SongManager::AddToQueue(Song* newSong)
{
    m_songQueue.push_back(newSong);
}

//-----------------------------------------------------------------------------------
unsigned int SongManager::GetQueueLength()
{
    return m_songQueue.size();
}

//-----------------------------------------------------------------------------------
void SongManager::OnSongPlaybackFinished()
{
    StopSong();
    delete m_activeSong;
    m_activeSong = nullptr;
    if (m_songQueue.size() > 0)
    {
        m_activeSong = m_songQueue.front();
        m_songQueue.pop_front();
        if (m_activeSong)
        {
            Play(m_activeSong);
        }
    }
    else
    {
        StopAll();
    }
}

//-----------------------------------------------------------------------------------
void SongManager::OnSongBeginPlay()
{
    IncrementPlaycount(m_activeSong->m_filePath);
}

//-----------------------------------------------------------------------------------
void SongManager::StopSong()
{
    AudioSystem::instance->StopChannel(AudioSystem::instance->GetChannel(m_activeSong->m_fmodID));
    if (m_activeSong)
    {
        delete m_activeSong;
        m_activeSong = nullptr;
    }
}

//-----------------------------------------------------------------------------------
void SongManager::SetRPM(float rpm, bool changeInstantly /*= false*/)
{
    m_currentRPM = rpm;
    float frequencyMultiplier = rpm / TheGame::instance->m_currentRecord->m_baseRPM;
    TheGame::instance->m_currentRecord->m_currentRotationRate = TheGame::instance->CalculateRotationRateFromRPM(rpm);

    m_activeSong->m_targetFrequency = m_activeSong->m_baseFrequency * frequencyMultiplier;
    if (changeInstantly)
    {
        m_activeSong->m_currentFrequency = m_activeSong->m_targetFrequency;
    }
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
    SongManager::instance->Play(newSong);

    if (args.HasArgs(2))
    {
        float rpm = args.GetFloatArgument(1);
        SongManager::instance->SetRPM(rpm, true);
    }
    else
    {
        SongManager::instance->SetRPM(TheGame::instance->m_currentRecord->m_baseRPM, true);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(addtoqueue)
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
    SongManager::instance->AddToQueue(newSong);
    Console::instance->PrintLine(Stringf("Added %s to the queue at position %i.", newSong->m_title.c_str(), SongManager::instance->GetQueueLength()));
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(printqueue)
{
    Console::instance->PrintLine("----====Current Queue====----", RGBA::ORANGE);

    unsigned int index = 0;
    for (Song* song : SongManager::instance->m_songQueue)
    {
        RGBA lineColor = ++index % 2 == 0 ? RGBA::EARTHBOUND_GREEN : RGBA::EARTHBOUND_BLUE;
        Console::instance->PrintLine(Stringf("[%i] %s", index, song->m_title.c_str()), lineColor);
    }
    if (index == 0)
    {
        Console::instance->PrintLine("<EMPTY>", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(loopoff)
{
    UNUSED(args);
    SongManager::instance->SetLoopMode(SongManager::LoopMode::NO_LOOP);
    Console::instance->PrintLine("Song Looping Disabled", RGBA::MUDKIP_ORANGE);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(loopon)
{
    UNUSED(args);
    SongManager::instance->SetLoopMode(SongManager::LoopMode::SONG_LOOP);
    Console::instance->PrintLine("Song Looping Enabled", RGBA::MUDKIP_BLUE);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(stop)
{
    UNUSED(args);
    SongManager::instance->StopAll();
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(pause)
{
    UNUSED(args);
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
    SongManager::instance->SetRPM(rpm);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(wigglerpm)
{
    if (!(args.HasArgs(1) || args.HasArgs(0)))
    {
        Console::instance->PrintLine("wigglerpm <wiggleRate>", RGBA::RED);
        return;
    }

    if (args.HasArgs(1))
    {
        SongManager::instance->m_wiggleDelta = args.GetFloatArgument(0);
    }
    else
    {
        SongManager::instance->m_wiggleRPM = !SongManager::instance->m_wiggleRPM;
        if (SongManager::instance->m_wiggleRPM)
        {
            Console::instance->PrintLine("RPM wiggling enabled!", RGBA::VAPORWAVE);
        }
        else
        {
            Console::instance->PrintLine("RPM wiggling disabled!", RGBA::MAROON);
        }
    }
}