#include "Game/Audio/SongManager.hpp"
#include "Game/Audio/Song.hpp"
#include "Game/TheGame.hpp"
#include "Game/Renderables/VinylRecord.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Audio/Audio.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/Widgets/LabelWidget.hpp"

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
        m_currentFrequency = Lerp(0.1f, m_currentFrequency, m_targetFrequency);
        AudioSystem::instance->SetFrequency(m_activeSong->m_fmodID, m_currentFrequency);

        CheckForHotkeys();
        UpdateUIWidgetText();

        if (!AudioSystem::instance->IsPlaying(m_activeSong->m_fmodChannel))
        {
            m_eventSongFinished.Trigger();
        }
    }
}

//-----------------------------------------------------------------------------------
void SongManager::Play(Song* songToPlay)
{
    if (m_loopMode == NO_LOOP && IsPlaying())
    {
        StopSong();
    }
    m_activeSong = songToPlay;
    m_baseFrequency = m_activeSong->m_samplerate;

    AudioSystem::instance->PlayLoopingSound(songToPlay->m_fmodID, m_songVolume); //TODO: Find out why PlaySound causes a linker error here
    AudioSystem::instance->SetLooping(songToPlay->m_fmodID, false);
    m_eventSongBeginPlay.Trigger();

    SetNowPlayingTextFromMetadata(m_activeSong);
    m_activeSong->m_fmodChannel = AudioSystem::instance->GetChannel(m_activeSong->m_fmodID);

    //Load album art
    Texture* albumArtTexture = GetImageFromFileMetadata(songToPlay->m_filePath); //TODO: Only load texture once if loop is on
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
        SetNowPlayingTextFromMetadata(nullptr);
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
void SongManager::PlayNext(Song* newSong)
{
    m_songQueue.push_front(newSong);
}

//-----------------------------------------------------------------------------------
unsigned int SongManager::GetQueueLength()
{
    return m_songQueue.size();
}

//-----------------------------------------------------------------------------------
void SongManager::OnSongPlaybackFinished()
{
    if (m_loopMode == SONG_LOOP)
    {
        Play(m_activeSong);
    }
    else
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
}

//-----------------------------------------------------------------------------------
void SongManager::OnSongBeginPlay()
{
    IncrementPlaycount(m_activeSong->m_filePath);
    ++m_activeSong->m_playcount;
}

//-----------------------------------------------------------------------------------
void SongManager::StopSong()
{
    AudioSystem::instance->StopChannel(AudioSystem::instance->GetChannel(m_activeSong->m_fmodID));
    if (m_activeSong)
    {
        delete m_activeSong;
        m_activeSong = nullptr;
        SetNowPlayingTextFromMetadata(nullptr); //Set to default values.
    }
}

//-----------------------------------------------------------------------------------
void SongManager::SetRPM(float rpm, bool changeInstantly /*= false*/)
{
    m_currentRPM = rpm;
    float frequencyMultiplier = rpm / TheGame::instance->m_currentRecord->m_baseRPM;
    TheGame::instance->m_currentRecord->m_currentRotationRate = TheGame::instance->CalculateRotationRateFromRPM(rpm);

    m_targetFrequency = m_baseFrequency * frequencyMultiplier;
    if (changeInstantly)
    {
        m_currentFrequency = m_targetFrequency;
    }
}

//-----------------------------------------------------------------------------------
void SongManager::CheckForHotkeys()
{
    if (Console::instance->IsActive())
    {
        return;
    }

    if (InputSystem::instance->WasKeyJustPressed(' '))
    {
        if (m_currentRPM != 0)
        {
            m_lastRPM = m_currentRPM;
            Console::instance->RunCommand("setrpm 0");
        }
        else
        {
            Console::instance->RunCommand("setrpm " + std::to_string(m_lastRPM));
        }
    }
}

//-----------------------------------------------------------------------------------
void SongManager::SetNowPlayingTextFromMetadata(Song* currentSong)
{
    LabelWidget* songNameWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("SongName"));
    LabelWidget* artistNameWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("ArtistName"));
    LabelWidget* albumNameWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("AlbumName"));
    LabelWidget* yearWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("Year"));
    LabelWidget* genreWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("Genre"));
    LabelWidget* playcountWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("Playcounts"));
    LabelWidget* playingTimeWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("PlayingTime"));

    ASSERT_OR_DIE(songNameWidget, "Couldn't find the SongName label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_OR_DIE(artistNameWidget, "Couldn't find the ArtistName label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_OR_DIE(albumNameWidget, "Couldn't find the AlbumName label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_OR_DIE(yearWidget, "Couldn't find the Year label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_OR_DIE(genreWidget, "Couldn't find the Genre label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_OR_DIE(playcountWidget, "Couldn't find the Playcounts label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_OR_DIE(playingTimeWidget, "Couldn't find the PlayingTime label widget. Have you customized Data/UI/PlayerLayout.xml recently?");

    std::string title = "No Song Playing";
    std::string artist = "No Artist";
    std::string album = "No Album";
    std::string year = "Unknown Year";
    std::string genre = "Unknown Genre";
    std::string playcount = "Playcounts: 0";
    std::string playtime = "00:00 / 00:00";

    if (currentSong)
    {
        title = Stringf("Title: %s", currentSong->m_title.c_str());
        artist = Stringf("Artist: %s", currentSong->m_artist.c_str());
        album = Stringf("Album: %s", currentSong->m_album.c_str());
        year = Stringf("Year: %i", currentSong->m_year);
        genre = Stringf("Genre: %s", currentSong->m_genre.c_str());
        playcount = Stringf("Playcounts: %i", currentSong->m_playcount);
    }

    songNameWidget->m_propertiesForAllStates.Set("Text", title, false);
    artistNameWidget->m_propertiesForAllStates.Set("Text", artist, false);
    albumNameWidget->m_propertiesForAllStates.Set("Text", album, false);
    yearWidget->m_propertiesForAllStates.Set("Text", year, false);
    genreWidget->m_propertiesForAllStates.Set("Text", genre, false);
    playcountWidget->m_propertiesForAllStates.Set("Text", playcount, false);
    playingTimeWidget->m_propertiesForAllStates.Set("Text", playtime, false);
}

//-----------------------------------------------------------------------------------
void SongManager::UpdateUIWidgetText()
{
    LabelWidget* rpmWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("RPM"));
    LabelWidget* playingTimeWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("PlayingTime"));

    ASSERT_OR_DIE(rpmWidget, "Couldn't find the RPM label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_OR_DIE(playingTimeWidget, "Couldn't find the PlayingTime label widget. Have you customized Data/UI/PlayerLayout.xml recently?");

    unsigned int currentSongLengthSeconds = AudioSystem::instance->GetSoundLengthMS(m_activeSong->m_fmodID) / 1000;
    unsigned int currentPlaybackPositionSeconds = AudioSystem::instance->GetPlaybackPositionMS(m_activeSong->m_fmodChannel) / 1000;
    std::string playingTime = Stringf("%02i:%02i / %02i:%02i", currentPlaybackPositionSeconds / 60, currentPlaybackPositionSeconds % 60, currentSongLengthSeconds / 60, currentSongLengthSeconds % 60);
    std::string rpm = Stringf("RPM: %i", (int)m_currentRPM);

    rpmWidget->m_propertiesForAllStates.Set("Text", rpm, false);
    playingTimeWidget->m_propertiesForAllStates.Set("Text", playingTime, false);
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
    if (!(args.HasArgs(1)))
    {
        Console::instance->PrintLine("addtoqueue <filename>", RGBA::RED);
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
CONSOLE_COMMAND(playnext)
{
    if (!(args.HasArgs(1)))
    {
        Console::instance->PrintLine("playnext <filename>", RGBA::RED);
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
    SongManager::instance->PlayNext(newSong);
    Console::instance->PrintLine(Stringf("Added %s to the top of the queue.", newSong->m_title.c_str()));
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
CONSOLE_COMMAND(setvolume)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("setvolume <0-100>", RGBA::RED);
        return;
    }

    int passedVolume = args.GetIntArgument(0);
    int sanitizedVolume = Clamp<int>(passedVolume, 0, 100);
    SongManager::instance->m_songVolume = (float)sanitizedVolume / 100.0f;
    AudioSystem::instance->SetVolume(SongManager::instance->m_activeSong->m_fmodChannel, SongManager::instance->m_songVolume);

    Console::instance->PrintLine(Stringf("Volume level set to %i%%", sanitizedVolume), RGBA::GOLD);
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