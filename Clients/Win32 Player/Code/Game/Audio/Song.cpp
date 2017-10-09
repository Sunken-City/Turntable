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
#include "ThirdParty/taglib/include/taglib/vorbisfile.h"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "SongManager.hpp"

//-----------------------------------------------------------------------------------
Song::Song(const std::string& fullPathToFile)
{
    m_filePath = std::wstring(fullPathToFile.begin(), fullPathToFile.end());
    m_fileExtension = GetFileExtension(fullPathToFile);
    m_fileName = GetFileName(fullPathToFile);
    m_fmodID = AudioSystem::instance->CreateOrGetSound(m_filePath);
    SetMetadataFromFile(m_filePath);
}

//-----------------------------------------------------------------------------------
Song::Song(const std::wstring& fullPathToFile)
{
    m_filePath = fullPathToFile;
    std::string strFilePath = std::string(fullPathToFile.begin(), fullPathToFile.end());
    m_fileExtension = GetFileExtension(strFilePath);
    m_fileName = GetFileName(strFilePath);
    m_fmodID = AudioSystem::instance->CreateOrGetSound(fullPathToFile);
    SetMetadataFromFile(m_filePath);
}

//-----------------------------------------------------------------------------------
Song::~Song()
{

}

//-----------------------------------------------------------------------------------
void Song::SetMetadataFromFile(const std::wstring& fileName)
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
    else if (m_fileExtension == "ogg")
    {
        TagLib::Ogg::Vorbis::File oggFile(fileName.c_str());
        TagLib::PropertyMap map = oggFile.properties();

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

    //Midis have no metadata, so let's fill it out manually.
    if (m_fileExtension == "mid" || m_fileExtension == "midi")
    {
        m_artist = "Unknown";
        m_album = "Unkown";
        m_genre = "MIDI";
        m_title = m_fileName;
        m_year = 0;
        m_trackNum = 0;
        m_lengthInSeconds = AudioSystem::instance->GetSoundLengthMS(m_fmodID) / 1000;

        m_playcount = 0;
        m_samplerate = AudioSystem::instance->GetFrequency(m_fmodID); //This should return -1 since it's a MIDI
        m_ignoresFrequency = true;
    }
    else
    {
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
}

//-----------------------------------------------------------------------------------
void Song::Update(float deltaSeconds)
{
    UNUSED(deltaSeconds);
}
