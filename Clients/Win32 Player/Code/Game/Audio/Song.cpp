#include "Game/Audio/Song.hpp"
#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "ThirdParty/taglib/include/taglib/mpegfile.h"
#include "ThirdParty/taglib/include/taglib/id3v2tag.h"
#include "ThirdParty/taglib/include/taglib/attachedpictureframe.h"
#include "ThirdParty/taglib/include/taglib/flacfile.h"
#include "ThirdParty/taglib/include/taglib/unknownframe.h"
#include "ThirdParty/taglib/include/taglib/tfile.h"
#include "ThirdParty/taglib/include/taglib/tpropertymap.h"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include "ThirdParty/taglib/include/taglib/wavfile.h"
#include "ThirdParty/taglib/include/taglib/rifffile.h"
#include "ThirdParty/taglib/include/taglib/oggflacfile.h"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "SongManager.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/Widgets/LabelWidget.hpp"

//-----------------------------------------------------------------------------------
Song::Song(const std::string& fullPathToFile)
{
    m_filePath = fullPathToFile;
    m_fileExtension = GetFileExtension(fullPathToFile);
    m_fileName = GetFileName(fullPathToFile);
    SetMetadataFromFile(fullPathToFile);

    m_fmodID = AudioSystem::instance->CreateOrGetSound(fullPathToFile);
}

//-----------------------------------------------------------------------------------
Song::~Song()
{

}

//-----------------------------------------------------------------------------------
void Song::SetMetadataFromFile(const std::string& fileName)
{
    //Taglib doesn't implement bits per sample for every file type (mp3). It's assumed to be 16 right now
    //which could be wrong in some edge cases
    m_bitdepth = 16;

    if (m_fileExtension == "flac")
    {
        TagLib::FLAC::File flacFile(fileName.c_str());
        TagLib::PropertyMap map = flacFile.properties();

        auto playcountPropertyIter = map.find("PCNT");
        if (playcountPropertyIter != map.end())
        {
            bool wasInt = false;
            m_playcount = playcountPropertyIter->second.toString().toInt(&wasInt);
            ASSERT_OR_DIE(&wasInt, "Tried to grab the playcount, but found a non-integer value in the PCNT field.");
        }
        else
        {
            m_playcount = 0;
        }

        m_bitdepth = flacFile.audioProperties()->bitsPerSample();
    }
    else if (m_fileExtension == "mp3")
    {
        TagLib::MPEG::File mp3File(fileName.c_str());
        TagLib::PropertyMap map = mp3File.properties();

        auto playcountPropertyIter = map.find("PCNT");
        if (playcountPropertyIter != map.end())
        {
            bool wasInt = false;
            m_playcount = playcountPropertyIter->second.toString().toInt(&wasInt);
            ASSERT_OR_DIE(&wasInt, "Tried to grab the playcount, but found a non-integer value in the PCNT field.");
        }
        else
        {
            m_playcount = 0;
        }
    }
    else if (m_fileExtension == "wav")
    {
        TagLib::RIFF::WAV::File wavFile(fileName.c_str());
        TagLib::PropertyMap map = wavFile.properties();

        auto playcountPropertyIter = map.find("PCNT");
        if (playcountPropertyIter != map.end())
        {
            bool wasInt = false;
            m_playcount = playcountPropertyIter->second.toString().toInt(&wasInt);
            ASSERT_OR_DIE(&wasInt, "Tried to grab the playcount, but found a non-integer value in the PCNT field.");
        }
        else
        {
            m_playcount = 0;
        }

        m_bitdepth = wavFile.audioProperties()->bitsPerSample();
    }

    TagLib::FileRef file(fileName.c_str());

    m_artist = file.tag()->artist().toCString();
    m_album = file.tag()->album().toCString();
    m_genre = file.tag()->genre().toCString();
    m_title = file.tag()->title().toCString();
    m_year = file.tag()->year();
    m_trackNum = file.tag()->track();

    m_lengthInSeconds = file.audioProperties()->lengthInSeconds();
    m_samplerate = file.audioProperties()->sampleRate();
    m_bitrate = file.audioProperties()->bitrate();
    m_numChannels = file.audioProperties()->channels();
}

//-----------------------------------------------------------------------------------
void Song::SetNowPlayingTextFromMetadata()
{
    LabelWidget* songNameWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("SongName"));
    LabelWidget* artistNameWidget = dynamic_cast<LabelWidget*>(UISystem::instance->FindWidgetByName("ArtistName"));

    ASSERT_OR_DIE(songNameWidget, "Couldn't find the SongName label widget. Have you customized Data/UI/PlayerLayout.xml recently?");
    ASSERT_OR_DIE(artistNameWidget, "Couldn't find the ArtistName label widget. Have you customized Data/UI/PlayerLayout.xml recently?");

    songNameWidget->m_propertiesForAllStates.Set("Text", Stringf("Title: %s", m_title.c_str()), false);
    artistNameWidget->m_propertiesForAllStates.Set("Text", Stringf("Artist: %s", m_artist.c_str()), false);
}

//-----------------------------------------------------------------------------------
void Song::Update(float deltaSeconds)
{
    UNUSED(deltaSeconds);
}
