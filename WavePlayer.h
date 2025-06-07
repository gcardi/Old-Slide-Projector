//---------------------------------------------------------------------------

#ifndef WavePlayerH
#define WavePlayerH
//---------------------------------------------------------------------------

#include <windows.h>
#include <dsound.h>
#include <vector>

#include <System.hpp>
#include <System.Win.ComObj.hpp>

class WavePlayer {
public:
    WavePlayer( HWND Handle );
    WavePlayer( WavePlayer const & ) = delete;
    WavePlayer& operator=( WavePlayer const & ) = delete;

    void LoadWaveFromStream( TStream& Stream, float Volume = 1.0F );
    void LoadWaveFromFile( String FilePath, float Volume = 1.0F );
    void LoadWaveFromResource( HINSTANCE HInstance, String Name,
                               float Volume = 1.0F, String Type = _D( "WAVE" ) );
    void Play( bool Loop = false );
    void Stop();

private:
    using _di_IDirectSound8 = DelphiInterface<IDirectSound8>;
    using _di_IDirectSoundBuffer = DelphiInterface<IDirectSoundBuffer>;
    using _di_IDirectSoundBuffer8 = DelphiInterface<IDirectSoundBuffer8>;

    void InitializeDirectSound( HWND Handle );
    void CreateSoundBuffer();
    void FillBufferWithWaveData();

    _di_IDirectSound8 directSound_;
    _di_IDirectSoundBuffer primaryBuffer_;
    _di_IDirectSoundBuffer8 secondaryBuffer_;

    WAVEFORMATEX waveFormat_;
    std::vector<BYTE> waveData_;
    bool isLooping_ = false;
};

#endif
