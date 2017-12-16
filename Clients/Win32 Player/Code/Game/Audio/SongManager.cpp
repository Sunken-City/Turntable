#include "Game/Audio/SongManager.hpp"
#include "Game/Audio/Song.hpp"
#include "Game/TheGame.hpp"
#include "Game/Renderables/VinylRecord.hpp"
#include "Game/UserData/AchievementManager.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Audio/Audio.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/Widgets/LabelWidget.hpp"
#include "Engine/Core/Events/EventSystem.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "../UserData/UserProfile.hpp"

SongManager* SongManager::instance = nullptr;

//-----------------------------------------------------------------------------------
SongManager::SongManager()
    : m_needleDropSound(AudioSystem::instance->CreateOrGetSound("Data/SFX/needleDrop.ogg"))
    , m_recordCracklesSound(AudioSystem::instance->CreateOrGetSound("Data/SFX/crackles.ogg"))
{
    m_eventSongFinished.RegisterMethod(this, &SongManager::OnSongPlaybackFinished);
    m_eventSongBeginPlay.RegisterMethod(this, &SongManager::OnSongBeginPlay);
    EventSystem::RegisterEventCallback("TogglePlayPause", &OnTogglePlayPause);
    EventSystem::RegisterEventCallback("SkipBack", &OnSkipBack);
    EventSystem::RegisterEventCallback("SkipNext", &OnSkipNext);
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
    UNUSED(deltaSeconds);
    if (m_activeSong)
    {
        if (m_recordCracklesHandle)
        {
            if (AudioSystem::instance->IsPlaying(m_recordCracklesHandle))
            {
                AudioSystem::instance->SetLooping(m_recordCracklesHandle, false);
                AudioSystem::instance->SetVolume(m_recordCracklesHandle, Clamp01(AudioSystem::instance->GetVolume(m_recordCracklesHandle) - deltaSeconds));
            }
            else
            {
                //This SHOULD be okay, since FMOD is supposed to manage the lifecycle of these channels. http://www.fmod.org/questions/question/forum-39357
                //The song object itself stays around in memory (which is what requires release() to be called)
                AudioSystem::instance->StopChannel(m_recordCracklesHandle);
                m_recordCracklesHandle = nullptr;
            }
        }
        
        if (m_wiggleRPM)
        {
            float wiggleAmount = MathUtils::GetRandomFloat(-m_wiggleDelta, m_wiggleDelta);
            SetRPM(m_currentRPM + wiggleAmount);
        }

        if (m_activeSong->m_ignoresFrequency)
        {
            static float currentFrequencyMultiplier = 1.0f;
            float targetFrequencyMultiplier = m_currentRPM / TheGame::instance->m_currentRecord->m_baseRPM;
            currentFrequencyMultiplier = Lerp(0.1f, currentFrequencyMultiplier, targetFrequencyMultiplier);
            AudioSystem::instance->SetMIDISpeed(m_activeSong->m_audioChannelHandle, currentFrequencyMultiplier);
        }
        else
        {
            m_currentFrequency = Lerp(0.1f, m_currentFrequency, m_targetFrequency);
            AudioSystem::instance->SetFrequency(m_activeSong->m_audioChannelHandle, m_currentFrequency);
        }

        CheckForHotkeys(); //This could technically end the song we're playing, so we have to keep validating we have an active song.
        UpdateUIWidgetText();

        if (m_currentRPM != 0.0f) //We set this value to 0.0f when we pause, no need to worry about roundoff <3
        {
            AchievementManager::instance->IncrementLifetimeSeconds(deltaSeconds);
        }

        if (m_activeSong && !AudioSystem::instance->IsPlaying(m_activeSong->m_audioChannelHandle))
        {
            AwardExpForSongProgress(true); //Since the playback position is no longer at the end of the song, forcibly give experience here.
            m_eventSongFinished.Trigger();
        }
    }
    else if (m_songQueue.size() > 0)
    {
        Song* nextSongInQueue = m_songQueue[0];
        Song::State initialState = nextSongInQueue->m_state;
        nextSongInQueue->RequestSongHandle();

        if (nextSongInQueue->m_state == Song::READY_TO_PLAY)
        {
            m_songQueue.pop_front();
            if (m_songCache.IsValid(nextSongInQueue->m_songID))
            {
                Play(nextSongInQueue);
            }
            else
            {
                //Something happened while loading this song. Log it and continue.
                Console::instance->PrintLine(Stringf("Error while loading song %s. Skipping to next song in queue. Reason:", nextSongInQueue->m_fileName.c_str()), RGBA::RED);
                m_songCache.PrintErrorInConsole(nextSongInQueue->m_songID);
                delete nextSongInQueue;
            }
        }
        else if (initialState == Song::State::NOT_LOADED && !m_recordCracklesHandle)
        {
            StartLoadingSound();
        }
        
    }
}

//-----------------------------------------------------------------------------------
void SongManager::StartLoadingSound()
{
    AudioSystem::instance->PlaySound(m_needleDropSound);
    AudioSystem::instance->PlayLoopingSound(m_recordCracklesSound);
    m_recordCracklesHandle = AudioSystem::instance->GetChannel(m_recordCracklesSound);
}

//-----------------------------------------------------------------------------------
void SongManager::Play(Song* songToPlay)
{
    if (m_loopMode == NO_LOOP && IsPlaying())
    {
        StopSong();
    }
    m_activeSong = songToPlay;
    m_baseFrequency = (float)m_activeSong->m_samplerate;

    m_activeSong->m_audioChannelHandle = AudioSystem::instance->PlayRawSong(m_activeSong->m_songHandle, m_songVolume);
    AudioSystem::instance->SetLooping(m_activeSong->m_audioChannelHandle, false);

    if (SongManager::instance->m_targetFrequency < 0)
    {
        unsigned int endOfSongTimestampMS = AudioSystem::instance->GetSoundLengthMS(m_activeSong->m_songHandle) - 200;
        AudioSystem::instance->SetPlaybackPositionMS(m_activeSong->m_audioChannelHandle, endOfSongTimestampMS);
    }
    
    m_eventSongBeginPlay.Trigger();
    SetNowPlayingTextFromMetadata(m_activeSong);

    //Load album art if we haven't already
    if (!m_activeSong->m_albumArt)
    {
        m_activeSong->m_albumArt = GetImageFromFileMetadata(songToPlay->m_filePath);
    }
    if (m_activeSong->m_albumArt)
    {
        TheGame::instance->m_currentRecord->m_innerMaterial->SetDiffuseTexture(m_activeSong->m_albumArt);
        TheGame::instance->m_fboMaterial->SetNormalTexture(m_activeSong->m_albumArt);
    }
}

//-----------------------------------------------------------------------------------
void SongManager::StopAll()
{
    Console::instance->PrintLine("Stopping the music. Party's over, people. :c", RGBA::GBLIGHTGREEN);
    if (IsPlaying())
    {
        AudioSystem::instance->StopChannel(m_activeSong->m_songHandle);
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
    AudioChannelHandle channel = m_activeSong->m_audioChannelHandle;
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
    if (!m_activeSong)
    {
        return;
    }

    AwardExpForSongProgress();

    if (m_loopMode == SONG_LOOP)
    {
        Play(m_activeSong);
    }
    else
    {
        StopSong();
        if (m_songQueue.size() == 0)
        {
            StopAll();
        }
    }
}

//-----------------------------------------------------------------------------------
void SongManager::AwardExpForSongProgress(bool isSongFinished)
{
    unsigned int currentSongLengthSeconds = AudioSystem::instance->GetSoundLengthMS(m_activeSong->m_songHandle) / 1000;
    unsigned int currentPlaybackPositionSeconds = AudioSystem::instance->GetPlaybackPositionMS(m_activeSong->m_audioChannelHandle) / 1000;
    unsigned int level = AchievementManager::instance->m_currentProfile->m_level;
    float songLengthMultiplier = (float)m_activeSong->m_lengthInSeconds / 60.0f;
    float levelMultiplier = (float)level / 100.0f;
    float songProgressMultiplier = isSongFinished ? 1.0f : static_cast<float>(currentPlaybackPositionSeconds) / static_cast<float>(currentSongLengthSeconds);
    float totalMultiplier = (songLengthMultiplier + levelMultiplier) * songProgressMultiplier;

    if (totalMultiplier > 0.0f) //Don't give experience if we have none to give.
    {
        AchievementManager::instance->AddExperience(ExperienceValues::EXP_FOR_PLAY, totalMultiplier);
    }
}

//-----------------------------------------------------------------------------------
void SongManager::OnSongBeginPlay()
{
    float songLengthMultiplier = (float)m_activeSong->m_lengthInSeconds / 60.0f;
    unsigned int level = AchievementManager::instance->m_currentProfile->m_level;
    //float levelMultiplier = (float)level / 100.0f;

    if (m_activeSong->m_playcount == 0)
    {
        AchievementManager::instance->AddExperience(ExperienceValues::EXP_FOR_NEW_SONG);
    }

    IncrementPlaycount(m_activeSong->m_filePath);
    ++m_activeSong->m_playcount;
    AchievementManager::instance->IncrementLifetimePlaycount();
}

//-----------------------------------------------------------------------------------
void SongManager::StopSong()
{
    AudioSystem::instance->StopChannel(m_activeSong->m_audioChannelHandle);
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
        OnTogglePlayPause();
    }

    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::LEFT))
    {
        OnSkipBack();
    }
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::RIGHT))
    {
        OnSkipNext();
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

    ASSERT_RECOVERABLE(songNameWidget, "Couldn't find the SongName label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_RECOVERABLE(artistNameWidget, "Couldn't find the ArtistName label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_RECOVERABLE(albumNameWidget, "Couldn't find the AlbumName label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_RECOVERABLE(yearWidget, "Couldn't find the Year label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_RECOVERABLE(genreWidget, "Couldn't find the Genre label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_RECOVERABLE(playcountWidget, "Couldn't find the Playcounts label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_RECOVERABLE(playingTimeWidget, "Couldn't find the PlayingTime label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    if (!(songNameWidget && artistNameWidget && albumNameWidget && yearWidget && genreWidget && playcountWidget && playingTimeWidget))
    {
        UISystem::instance->ReloadUI("Data/UI/BackupLayout.xml");
        SetNowPlayingTextFromMetadata(currentSong); //this will cause an infinite loop if you break BackupLayout.xml
        return;
    }

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
    if (!m_activeSong)
    {
        return;
    }

    LabelWidget* rpmWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("RPM"));
    LabelWidget* playingTimeWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("PlayingTime"));

    ASSERT_RECOVERABLE(rpmWidget, "Couldn't find the RPM label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_RECOVERABLE(playingTimeWidget, "Couldn't find the PlayingTime label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    if (!rpmWidget || !playingTimeWidget)
    {
        UISystem::instance->ReloadUI("Data/UI/BackupLayout.xml");
        UpdateUIWidgetText(); //this will cause an infinite loop if you break BackupLayout.xml
        return;
    }

    unsigned int currentSongLengthSeconds = AudioSystem::instance->GetSoundLengthMS(m_activeSong->m_songHandle) / 1000;
    unsigned int currentPlaybackPositionSeconds = AudioSystem::instance->GetPlaybackPositionMS(m_activeSong->m_audioChannelHandle) / 1000;
    std::string playingTime = Stringf("%02i:%02i / %02i:%02i", currentPlaybackPositionSeconds / 60, currentPlaybackPositionSeconds % 60, currentSongLengthSeconds / 60, currentSongLengthSeconds % 60);
    std::string rpm = Stringf("RPM: %i", (int)m_currentRPM);

    rpmWidget->m_propertiesForAllStates.Set("Text", rpm, false);
    playingTimeWidget->m_propertiesForAllStates.Set("Text", playingTime, false);
}

//-----------------------------------------------------------------------------------
void SongManager::SavePlaylist(const std::string& name)
{
    //Saves the current song queue to an XML formatted playlist in the user's AppData directory
    std::string appdata = GetAppDataDirectory();
    std::string savePath = Stringf("%s\\Turntable\\Playlists\\%s.xml", appdata.c_str(), name.c_str());

    if (m_songQueue.size() != 0)
    {
        XMLNode playlist = OpenPlaylist(name);
        for (unsigned int i = 0; i < m_songQueue.size(); ++i)
        {
            AddToPlaylist(playlist, m_songQueue.at(i));
        }

        playlist.writeToFile(savePath.c_str());
    }
    else if (m_activeSong)
    {
        XMLNode playlist = OpenPlaylist(name);
        AddToPlaylist(playlist, m_activeSong);

        playlist.writeToFile(savePath.c_str());
    }
    else
    {
        Console::instance->PrintLine("Couldn't save the playlist; there are no songs in the queue.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
bool SongManager::CheckForPlaylistOnDisk(const std::string& name) const
{
    std::string appdata = GetAppDataDirectory();
    EnsureDirectoryExists(appdata + "\\Turntable");
    EnsureDirectoryExists(appdata + "\\Turntable\\Playlists");
    std::string fullPath = appdata + "\\Turntable\\Playlists\\" + name + ".xml";

    return FileExists(fullPath);
}

//-----------------------------------------------------------------------------------
//This updates the filepath if you find it in the cwd path instead of the normal path.
bool SongManager::CheckForSongOnDisk(std::wstring& filepath) const
{
    bool foundFile = FileExists(filepath);
    if (!foundFile)
    {
        //Check with the cwd path as well.
        std::wstring cwd = Console::instance->GetCurrentWorkingDirectory();
        std::wstring cwdFilepath = cwd + L"\\" + filepath;
        if (FileExists(cwdFilepath))
        {
            filepath = cwdFilepath;
            foundFile = true;
        }
    }
    return foundFile;
}

//-----------------------------------------------------------------------------------
XMLNode SongManager::OpenPlaylist(const std::string& name)
{
    //Opens a playlist for writing
    XMLNode playlist;

    CheckForPlaylistOnDisk(name); //TODO: Use this to decide if we should create a new playlist or load an existing one 
    playlist = XMLNode::createXMLTopNode("Playlist");

    return playlist;
}

//-----------------------------------------------------------------------------------
void SongManager::LoadPlaylist(const XMLNode& playlist)
{
    //Loads a playlist into the queue
    std::vector<XMLNode> songs = XMLUtils::GetChildren(playlist);
    for (unsigned int i = 0; i < songs.size(); ++i)
    {
        XMLNode& currentSongNode = songs[i];
        if (currentSongNode.isEmpty() || currentSongNode.IsContentEmpty())
        {
            continue;
        }
        std::string path = currentSongNode.getAttribute("FilePath");
        std::wstring widePath = std::wstring(path.begin(), path.end());
        SongID id = m_songCache.RequestSongLoad(widePath);
        Song* nextSong = new Song(widePath, id);
        m_songQueue.push_back(nextSong);
    }
}

//-----------------------------------------------------------------------------------
void SongManager::AddToPlaylist(XMLNode& playlist, Song* currentSong)
{
    //Create entries for a song in the playlist
    XMLNode songNode = playlist.addChild("Song");
    std::string strFilePath = std::string(currentSong->m_filePath.begin(), currentSong->m_filePath.end());
    songNode.addAttribute("FilePath", strFilePath.c_str());
}

//UI EVENT FUNCTIONS/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------
void OnSkipNext(NamedProperties& params)
{
    UNUSED(params);
    if (SongManager::instance->m_loopMode == SongManager::LoopMode::SONG_LOOP)
    {
        SongManager::instance->SetLoopMode(SongManager::LoopMode::NO_LOOP);
        SongManager::instance->m_eventSongFinished.Trigger();
        SongManager::instance->SetLoopMode(SongManager::LoopMode::SONG_LOOP);
    }
    else
    {
        SongManager::instance->m_eventSongFinished.Trigger();
    }
}

//-----------------------------------------------------------------------------------
void OnSkipBack(NamedProperties& params)
{
    UNUSED(params);
    if (!SongManager::instance->m_activeSong)
    {
        return;
    }
    SongManager::instance->AwardExpForSongProgress();
    AudioSystem::instance->SetPlaybackPositionMS(SongManager::instance->m_activeSong->m_audioChannelHandle, 0);
}

//-----------------------------------------------------------------------------------
void OnTogglePlayPause(NamedProperties& params)
{
    UNUSED(params);
    WidgetBase* playPauseButton = UISystem::instance->FindWidgetByName("Play/Pause Button");

    if (SongManager::instance->m_currentRPM != 0)
    {
        SongManager::instance->m_lastRPM = SongManager::instance->m_currentRPM;
        Console::instance->RunCommand("pause");
        if (playPauseButton)
        {
            playPauseButton->SetProperty<std::string>("Texture", "Data/Images/UI/play.png");
            playPauseButton->m_texture = Texture::CreateOrGetTexture("Data/Images/UI/play.png");
        }
    }
    else
    {
        Console::instance->RunCommand("setrpm " + std::to_string(SongManager::instance->m_lastRPM));
        if (playPauseButton)
        {
            playPauseButton->SetProperty<std::string>("Texture", "Data/Images/UI/pause.png");
            playPauseButton->m_texture = Texture::CreateOrGetTexture("Data/Images/UI/pause.png");
        }
    }
}

//-----------------------------------------------------------------------------------
void SongManager::QueueRandomSong(bool playWholeAlbum /*= false*/)
{
    std::string& musicRoot = AchievementManager::instance->m_currentProfile->m_musicRootPath;
    std::wstring currentMusicRoot = std::wstring(musicRoot.begin(), musicRoot.end());
    if (currentMusicRoot.empty())
    {
        Console::instance->PrintLine("Please set a root music directory with setmusicroot first.", RGBA::RED);
        return;
    }

    bool foundAlbum = false;
    do 
    {
        std::vector<std::wstring> folders = EnumerateWideDirectories(currentMusicRoot);
        foundAlbum = folders.size() == 0;

        if (!foundAlbum)
        {
            unsigned int randDirectory = MathUtils::GetRandomInt(0, folders.size() - 1);
            currentMusicRoot = WStringf(L"%s\\%s", currentMusicRoot.c_str(), folders[randDirectory].c_str());
        }
    } while (!foundAlbum);

    std::vector<std::wstring> mp3s = EnumerateWideFiles(currentMusicRoot, L"*.mp3");
    std::vector<std::wstring> flacs = EnumerateWideFiles(currentMusicRoot, L"*.flac");
    std::vector<std::wstring> oggs = EnumerateWideFiles(currentMusicRoot, L"*.ogg");
    std::vector<std::wstring> wavs = EnumerateWideFiles(currentMusicRoot, L"*.wav");
                     
    std::vector<std::wstring> songs;
    songs.reserve(mp3s.size() + flacs.size() + oggs.size() + wavs.size());
    songs.insert(songs.end(), mp3s.begin(), mp3s.end());
    songs.insert(songs.end(), flacs.begin(), flacs.end());
    songs.insert(songs.end(), oggs.begin(), oggs.end());
    songs.insert(songs.end(), wavs.begin(), wavs.end());

    if (songs.size() == 0)
    {
        Console::instance->PrintLine("Tried to load directory %s but it had no songs.", RGBA::RED);
        return;
    }

    if (playWholeAlbum)
    {
        for (std::wstring& song : songs)
        {
            std::wstring songPath = WStringf(L"%s\\%s", currentMusicRoot.c_str(), song.c_str());
            Console::instance->RunCommand(WStringf(L"addtoqueue \"%s\"", songPath.c_str()), true);
        }
    }
    else
    {
        unsigned int randSong = MathUtils::GetRandomInt(0, songs.size() - 1);
        std::wstring songPath = WStringf(L"%s\\%s", currentMusicRoot.c_str(), songs[randSong].c_str());
        Console::instance->RunCommand(WStringf(L"play \"%s\"", songPath.c_str()), true);
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

    std::wstring filepath = args.GetWStringArgument(0);

    if (!SongManager::instance->CheckForSongOnDisk(filepath))
    {
        Console::instance->PrintLine("Could not find file.", RGBA::RED);
        return;
    }
    SongID songID = SongManager::instance->m_songCache.RequestSongLoad(filepath);

    SongManager::instance->StartLoadingSound();
    Song* newSong = new Song(filepath, songID);
    SongManager::instance->FlushSongQueue();
    SongManager::instance->AddToQueue(newSong);
    SongManager::instance->m_baseFrequency = (float)newSong->m_samplerate;

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
    std::wstring filepath = args.GetWStringArgument(0);
    if (!SongManager::instance->CheckForSongOnDisk(filepath))
    {
        Console::instance->PrintLine("Could not find file.", RGBA::RED);
        return;
    }
    SongID songID = SongManager::instance->m_songCache.RequestSongLoad(filepath);

    Song* newSong = new Song(filepath, songID);
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
    std::wstring filepath = args.GetWStringArgument(0);
    if (!SongManager::instance->CheckForSongOnDisk(filepath))
    {
        Console::instance->PrintLine("Could not find file.", RGBA::RED);
        return;
    }
    SongID songID = SongManager::instance->m_songCache.RequestSongLoad(filepath);

    Song* newSong = new Song(filepath, songID);
    SongManager::instance->PlayNext(newSong);
    Console::instance->PrintLine(Stringf("Added %s to the top of the queue.", newSong->m_title.c_str()));
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(printqueue)
{
    UNUSED(args)
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
CONSOLE_COMMAND(loadplaylist)
{
    if (!(args.HasArgs(1)))
    {
        Console::instance->PrintLine("loadplaylist <name>", RGBA::RED);
        return;
    }

    if (!SongManager::instance->CheckForPlaylistOnDisk(args.GetStringArgument(0).c_str()))
    {
        Console::instance->PrintLine("Could not load playlist from disk.", RGBA::RED);
        return;
    }
    std::string appdata = GetAppDataDirectory();
    std::string filePath = Stringf("%s\\Turntable\\Playlists\\%s.xml", appdata.c_str(), args.GetStringArgument(0).c_str());
    XMLNode root = XMLUtils::OpenXMLDocument(filePath);
    XMLNode playlist = XMLUtils::GetChildNodeAtPosition(root, "Playlist");
    SongManager::instance->LoadPlaylist(playlist);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(saveplaylist)
{
    if (!(args.HasArgs(1)))
    {
        Console::instance->PrintLine("saveplaylist <name>", RGBA::RED);
        return;
    }

    SongManager::instance->SavePlaylist(args.GetStringArgument(0));
    Console::instance->PrintLine("Saved playlist.", RGBA::GBLIGHTGREEN);
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
    if (SongManager::instance->m_wiggleRPM)
    {
        Console::instance->RunCommand("wigglerpm");
    }
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
    Console::instance->PrintLine(Stringf("Volume level set to %i%%", sanitizedVolume), RGBA::GOLD);

    if (SongManager::instance->m_activeSong && SongManager::instance->m_activeSong->m_audioChannelHandle)
    {
    AudioSystem::instance->SetVolume(SongManager::instance->m_activeSong->m_audioChannelHandle, SongManager::instance->m_songVolume);
    }
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

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(setmusicroot)
{
    if (!(args.HasArgs(1)))
    {
        Console::instance->PrintLine("setmusicroot <Full Directory Path>", RGBA::RED);
        return;
    }
    std::wstring filepath = args.GetWStringArgument(0);
    if (!DirectoryExists(filepath))
    {
        Console::instance->PrintLine("Could not find directory.", RGBA::RED);
        return;
    }

    AchievementManager::instance->m_currentProfile->m_musicRootPath = std::string(filepath.begin(), filepath.end());
    Console::instance->PrintLine(Stringf("Music folder root is now at %s.", AchievementManager::instance->m_currentProfile->m_musicRootPath.c_str()));
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(getmusicroot)
{
    UNUSED(args)
    Console::instance->PrintLine(Stringf("Music folder root is %s.", AchievementManager::instance->m_currentProfile->m_musicRootPath.c_str()));
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(playme)
{
    if (!(args.HasArgs(1)))
    {
        Console::instance->PrintLine("playme <'song' | 'album'>", RGBA::RED);
        return;
    }
    std::string command = args.GetStringArgument(0);
    ToLower(command);

    if (command == "song" | command == "asong")
    {
        SongManager::instance->QueueRandomSong(false);
    }
    else if (command == "album" | command == "analbum")
    {
        SongManager::instance->QueueRandomSong(true);
    }
    else
    {
        Console::instance->PrintLine("playme <'song' | 'album'>", RGBA::RED);
        Console::instance->PrintLine("Please type in song or album to find a random song or album to play.", RGBA::RED);
        return;
    }
}