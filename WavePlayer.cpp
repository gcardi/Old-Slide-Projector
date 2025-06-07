//---------------------------------------------------------------------------

#pragma hdrstop

#include <memory>
#include <limits>

#include "WavePlayer.h"

#pragma comment( lib, "dsound" )

using std::make_unique;
using std::clamp;
using std::numeric_limits;

//---------------------------------------------------------------------------
#pragma package(smart_init)

WavePlayer::WavePlayer( HWND Handle )
{
    InitializeDirectSound( Handle );
}
//---------------------------------------------------------------------------

void WavePlayer::InitializeDirectSound( HWND Handle )
{
    OleCheck( DirectSoundCreate8( nullptr, &directSound_, nullptr ) );
    OleCheck( directSound_->SetCooperativeLevel( Handle, DSSCL_PRIORITY ) );

    // Create the primary buffer
    DSBUFFERDESC BufferDesc = {};
    BufferDesc.dwSize = sizeof( DSBUFFERDESC );
    BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    BufferDesc.dwBufferBytes = 0; // Ignored for primary buffer
    BufferDesc.lpwfxFormat = nullptr;

    OleCheck( directSound_->CreateSoundBuffer( &BufferDesc, &primaryBuffer_, nullptr ) );
}
//---------------------------------------------------------------------------

static void AdjustVolume( int16_t* Samples, size_t SampleCount, float Volume )
{
    for ( size_t Idx {} ; Idx < SampleCount ; ++Idx ) {
        Samples[Idx] =
            clamp(
                static_cast<int32_t>( Samples[Idx] * Volume ),
                static_cast<int32_t>( numeric_limits<int16_t>::min() ),
                static_cast<int32_t>( numeric_limits<int16_t>::max() )
            );
    }
}
//---------------------------------------------------------------------------

void WavePlayer::LoadWaveFromStream( TStream& Stream, float Volume )
{
    char ChunkId[4];
    DWORD ChunkSize;

    Stream.Read( ChunkId, static_cast<NativeInt>( sizeof ChunkId ) );
    if ( memcmp( ChunkId, "RIFF", 4 ) ) {
        throw Exception ( _D( "Invalid format" ) );
    }

    Stream.Read( &ChunkSize, static_cast<NativeInt>( sizeof ChunkSize ) );
    Stream.Read( ChunkId, static_cast<NativeInt>( sizeof ChunkId ) );

    if ( memcmp( ChunkId, "WAVE", 4 ) ) {
        throw Exception ( _D( "Invalid WAV format" ) );
    }

    // Read chunks
    while ( Stream.Position < Stream.Size ) {
        Stream.Read( ChunkId, static_cast<NativeInt>( 4 ) );
        Stream.Read( &ChunkSize, static_cast<NativeInt>( sizeof ChunkSize ) );

        if ( !memcmp( ChunkId, "fmt ", 4 ) ) {
            Stream.Read( &waveFormat_, static_cast<NativeInt>( ChunkSize ) );
        }
        else if ( !memcmp( ChunkId, "data", 4 ) ) {
            waveData_.resize( ChunkSize );
            Stream.Read( waveData_.data(), static_cast<NativeInt>( ChunkSize ) );
            if ( waveData_.empty() ) {
                throw Exception ( _D( "No wave data for file" ) );
            }
            AdjustVolume(
                reinterpret_cast<int16_t*>( waveData_.data() ),
                waveData_.size() / sizeof( int16_t ),
                Volume
            );
            break;
        }
        else {
            Stream.Seek( static_cast<NativeInt>( ChunkSize ), soFromCurrent );
        }
    }
}
//---------------------------------------------------------------------------

void WavePlayer::LoadWaveFromFile( String FilePath, float Volume )
{
    auto Stream = make_unique<TFileStream>( FilePath, fmOpenRead );
    LoadWaveFromStream( *Stream, Volume );
}
//---------------------------------------------------------------------------

void WavePlayer::LoadWaveFromResource( HINSTANCE HInstance, String Name,
                                       float Volume, String Type )
{
    auto Stream =
        make_unique<TResourceStream>(
            reinterpret_cast<THandle>( HInstance ), Name.c_str(), Type.c_str()
        );

    LoadWaveFromStream( *Stream, Volume );
}
//---------------------------------------------------------------------------

using _di_IDirectSoundBuffer = DelphiInterface<IDirectSoundBuffer>;

void WavePlayer::CreateSoundBuffer()
{
    DSBUFFERDESC BufferDesc = {};
    BufferDesc.dwSize = sizeof( DSBUFFERDESC );
    BufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
    BufferDesc.dwBufferBytes = waveData_.size();
    BufferDesc.lpwfxFormat = &waveFormat_;

    _di_IDirectSoundBuffer tempBuffer;
    OleCheck( directSound_->CreateSoundBuffer( &BufferDesc, &tempBuffer, nullptr ) );

    OleCheck( tempBuffer->QueryInterface( IID_IDirectSoundBuffer8, (void**)&secondaryBuffer_) );

    FillBufferWithWaveData();
}
//---------------------------------------------------------------------------

void WavePlayer::FillBufferWithWaveData()
{
    void* bufferPtr1 = nullptr;
    void* bufferPtr2 = nullptr;
    DWORD bufferSize1 = 0;
    DWORD bufferSize2 = 0;

    OleCheck( secondaryBuffer_->Lock( 0, waveData_.size(), &bufferPtr1, &bufferSize1, &bufferPtr2, &bufferSize2, 0 ) );

    memcpy( bufferPtr1, waveData_.data(), bufferSize1 );

    if ( bufferPtr2 ) {
        memcpy( bufferPtr2, waveData_.data() + bufferSize1, bufferSize2 );
    }

    secondaryBuffer_->Unlock( bufferPtr1, bufferSize1, bufferPtr2, bufferSize2 );
}
//---------------------------------------------------------------------------

void WavePlayer::Play( bool Loop )
{
    CreateSoundBuffer();
    isLooping_ = Loop;
    DWORD playFlags = Loop ? DSBPLAY_LOOPING : 0;
    OleCheck( secondaryBuffer_->Play( 0, 0, playFlags ) );
}
//---------------------------------------------------------------------------

void WavePlayer::Stop()
{
    if ( secondaryBuffer_ ) {
        secondaryBuffer_->Stop();
    }
}
//---------------------------------------------------------------------------


