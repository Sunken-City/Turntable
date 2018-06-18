#include "Game/Audio/Song.hpp"
#include "Game/Audio/SongManager.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
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

//-----------------------------------------------------------------------------------
Song::Song(const std::wstring& fullPathToFile, SongID songID)
    : m_filePath(fullPathToFile)
    , m_songID(songID)
{
    std::string strFilePath = std::string(fullPathToFile.begin(), fullPathToFile.end());
    m_fileExtension = GetFileExtension(strFilePath);
    m_fileName = GetFileName(strFilePath);
    SetMetadataFromFile(m_filePath);
}

//-----------------------------------------------------------------------------------
Song::~Song()
{
    Texture::CleanUpTexture(m_fileName);
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
        m_album = "Unknown";
        m_genre = "MIDI";
        m_title = m_fileName;
        m_year = 0;
        m_trackNum = 0;
        //m_lengthInSeconds = AudioSystem::instance->GetSoundLengthMS(m_fmodID) / 1000;

        m_playcount = 0;
        //m_samplerate = AudioSystem::instance->GetFrequency(m_fmodID); //This should return -1 since it's a MIDI
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

//-----------------------------------------------------------------------------------
void Song::RequestSongHandle()
{
    m_songHandle = SongManager::instance->m_songCache.RequestSoundHandle(m_songID);
    m_state = SongManager::instance->m_songCache.GetState(m_songID);
    //if (m_songHandle)
    //{
    //    m_state = SongState::LOADED;
    //}
    //else
    //{
    //    m_state = SongState::LOADING;
    //}
}

//-----------------------------------------------------------------------------------
void Song::GenerateProceduralAlbumArt()
{
    size_t hashedSongInfo = std::hash<std::string>{}(m_artist + m_title + m_album + m_genre);

    Vector2Int textureSize(256, 256);
    Texture* currentColorTargets[] = { new Texture(textureSize.x, textureSize.y, Texture::TextureFormat::RGBA8) };
    Framebuffer* albumArtBuffer = Framebuffer::FramebufferCreate(1, currentColorTargets, nullptr);
    Renderer::instance->BindFramebuffer(albumArtBuffer);

    AABB2 bounds(Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    Vector2 uvMins(-1.0f);
    Vector2 uvMaxs(1.0f);
    Renderer::instance->SetOrtho(bounds.mins, bounds.maxs);
    MeshBuilder builder;
    Mesh* albumArtMesh = new Mesh();
    builder.Begin();
    {
        builder.SetColor(RGBA::WHITE);
        builder.SetUV(uvMins);
        builder.AddVertex(Vector3(bounds.mins.x, bounds.mins.y, 0.0f));
        builder.SetUV(Vector2(uvMaxs.x, uvMins.y));
        builder.AddVertex(Vector3(bounds.maxs.x, bounds.mins.y, 0.0f));
        builder.SetUV(uvMaxs);
        builder.AddVertex(Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f));
        builder.SetUV(Vector2(uvMins.x, uvMaxs.y));
        builder.AddVertex(Vector3(bounds.mins.x, bounds.maxs.y, 0.0f));
        builder.AddQuadIndices(3, 2, 0, 1);
    }
    builder.End();
    builder.CopyToMesh(albumArtMesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);

    Material* material = TheGame::instance->m_proceduralGenerationMaterials[hashedSongInfo % (TheGame::instance->NUM_PROC_GEN_MATERIALS)];
    material->SetIntUniform(std::hash<std::string>{}("gHashVal"), hashedSongInfo);

    MeshRenderer meshRenderer(albumArtMesh, material);
    meshRenderer.Render();

    Renderer::instance->EndOrtho();
    Renderer::instance->BindFramebuffer(nullptr);
    delete albumArtMesh;
    delete albumArtBuffer;

    //This registers the texture for cleanup later on when we shut down the game.
    std::string strFileName = m_fileName;
    Texture::RegisterTexture(Stringf("AlbumArt:%i", hashedSongInfo), currentColorTargets[0]); 
    m_albumArt = currentColorTargets[0];
}
