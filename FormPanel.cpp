//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include <FMX.Platform.Win.hpp>

#include <algorithm>
#include <memory>
#include <filesystem>
#include <cassert>
#include <iterator>

#include "FormPanel.h"
#include "DataModStyleRes.h"

using Fmx::Platform::Win::WindowHandleToPlatform;

using std::make_unique;
using std::swap;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;
using std::filesystem::is_directory;
using std::transform;
using std::begin;
using std::end;
using std::towlower;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "FMXFormPanelBase"
#pragma resource "*.fmx"
//TfrmPanel *frmPanel;
//---------------------------------------------------------------------------

__fastcall TfrmPanel::TfrmPanel( TComponent* Owner,
                                 int MechSoundVolume,
                                 bool FanNoise,
                                 int NoiseSoundVol,
                                 String PicturesPath,
                                 bool RecursivePicturesSearch,
                                 FMXWinDisplayDev const * Display,
                                 StoreOpts StoreOptions,
                                 Anafestica::TConfigNode* const RootNode )
    : TfrmPanelBase( Owner, Display, StoreOptions, RootNode )
    , mechSoundVolume_{ MechSoundVolume }
    , fanNoise_{ FanNoise }
    , noiseSoundVolume_{ NoiseSoundVol }
    , player_ { new WavePlayer{ WindowHandleToPlatform( Handle )->Wnd } }
    , playerNoise_ { new WavePlayer{ WindowHandleToPlatform( Handle )->Wnd } }
    , picturesPath_{ PicturesPath }
    , recursivePicturesSearch_{ RecursivePicturesSearch }
{
    mechSoundVolume_ = MechSoundVolume;
    noiseSoundVolume_ = NoiseSoundVolume;
    RestoreProperties();
    LoadMechanicalSound();
    LoadPictures();
}
//---------------------------------------------------------------------------

__fastcall TfrmPanel::~TfrmPanel()
{
    try {
        SaveProperties();
    }
    catch ( ... ) {
    }
}
//---------------------------------------------------------------------------

void TfrmPanel::RestoreProperties()
{
    // Put code here to restore attribute(s)
}
//---------------------------------------------------------------------------

void TfrmPanel::SaveProperties() const
{
    // Put code here to save attribute(s)
}
//---------------------------------------------------------------------------

void __fastcall TfrmPanel::FloatAnimation2Finish(TObject *Sender)
{
    FloatAnimation2->Inverse = !FloatAnimation2->Inverse;
    ++phase_;
    if ( ++FloatAnimation2->Tag & 1 ) {
        if ( backward_ ) {
            idx_ = ( idx_ ? idx_ : entries_.size() ) - 1;
        }
        else {
            idx_ = ( idx_ + 1 ) % entries_.size();
        }
        LoadImage( idx_ );
        FloatAnimation2->Start();
        FloatAnimation1->Start();
    }
}
//---------------------------------------------------------------------------

void TfrmPanel::LoadPictures()
{
	//std::wstring Path = LR"=(C:\Users\Giuliano\Desktop\SlideShow)=";
	//std::wstring Path = LR"=(C:\Users\Giuliano\Desktop\Lesso\SLIDE)=";
    std::wstring Path = picturesPath_.c_str();
	if ( is_directory( Path ) ) {
        auto Inserter = [this]( auto const & Entry )
        {
            auto Path = Entry.path();
            String Ext = String{ Path.extension().c_str() };
			if ( SameText( Ext, _D( ".jpg" ) ) || SameText( Ext, _D( ".png" ) ) ) {
				entries_.emplace_back( Path.c_str() );
			}
		};
        if ( recursivePicturesSearch_ ) {
            for ( decltype( auto ) Entry : recursive_directory_iterator( Path ) ) {
                Inserter( Entry );
            }
        }
        else {
    		for ( decltype( auto ) Entry : directory_iterator( Path ) ) {
                Inserter( Entry );
            }
        }
	}
	if ( !entries_.empty() ) {
		LoadImage( idx_ );
	}
}
//---------------------------------------------------------------------------

void TfrmPanel::SetNoiseSoundVolume( int Val )
{
    if ( noiseSoundVolume_ != Val ) {
        auto PlayerNoise = make_unique<WavePlayer>( WindowHandleToPlatform( Handle )->Wnd );
        noiseSoundVolume_ = Val;
        LoadNoiseSound( *PlayerNoise );
        if ( fanNoise_ ) {
            PlayerNoise->Play( true );
        }
        else {
            PlayerNoise->Stop();
        }
        playerNoise_ = std::move( PlayerNoise );
    }
}
//---------------------------------------------------------------------------

void TfrmPanel::LoadImage( size_t Index )
{
    auto FileName = entries_[Index];
    auto Bmp = make_unique<TBitmap>( FileName );
    ImageViewer2->Bitmap->Clear( {} );
    auto WFactor = static_cast<float>( ImageViewer2->Width ) / Bmp->Width;
    auto HFactor = static_cast<float>( ImageViewer2->Height ) / Bmp->Height;
    if ( HFactor <= WFactor ) {
        // H
        ImageViewer2->BitmapScale = HFactor;
    }
    else {
        // W
        ImageViewer2->BitmapScale = WFactor;
    }
    ImageViewer2->Bitmap->Assign( Bmp.get() );
}
//---------------------------------------------------------------------------

bool TfrmPanel::GetVignetting() const
{
    return NormalBlendEffect1->Enabled;
}
//---------------------------------------------------------------------------

void TfrmPanel::SetVignetting( bool Val )
{
    NormalBlendEffect1->Enabled = Val;
}
//---------------------------------------------------------------------------

TfrmPanel::ImageFileNameCont& TfrmPanel::GetImages()
{
    return entries_;
}
//---------------------------------------------------------------------------

bool TfrmPanel::IsIdle() const
{
    return ( phase_ % 3 ) == 0;
}
//---------------------------------------------------------------------------

void TfrmPanel::LoadMechanicalSound()
{
    if ( player_ ) {
        player_->LoadWaveFromResource(
            HInstance, _D( "sound" ),
            static_cast<float>( mechSoundVolume_ ) / 100.0F
        );
    }
}
//---------------------------------------------------------------------------

void TfrmPanel::LoadNoiseSound( WavePlayer& Player )
{
    Player.LoadWaveFromResource(
        HInstance, _D( "noise" ),
        static_cast<float>( noiseSoundVolume_ ) / 100.0F
    );
}
//---------------------------------------------------------------------------

void TfrmPanel::SetMechSoundVolume( int Val )
{
    if ( mechSoundVolume_ != Val ) {
        mechSoundVolume_ = Val;
        LoadMechanicalSound();
    }
}
//---------------------------------------------------------------------------

void TfrmPanel::PlayMechanicalSound()
{
    if ( player_ ) {
        player_->Play( false );
    }
}
//---------------------------------------------------------------------------

void TfrmPanel::PlayNoiseSound()
{
    if ( playerNoise_ ) {
        if ( fanNoise_ ) {
            playerNoise_->Play( true );
        }
        else {
            playerNoise_->Stop();
        }
    }
}
//---------------------------------------------------------------------------

void TfrmPanel::Next()
{
    PlayMechanicalSound();
    ++phase_;
    backward_ = false;
    FloatAnimation2->StopValue = ImageViewer2->Width * 2;
    FloatAnimation2->Start();
    FloatAnimation1->Start();
}
//---------------------------------------------------------------------------

void TfrmPanel::Prior()
{
    PlayMechanicalSound();
    ++phase_;
    backward_ = true;
    FloatAnimation2->StopValue = ImageViewer2->Width * 2;
    FloatAnimation2->Start();
    FloatAnimation1->Start();
}
//---------------------------------------------------------------------------

void __fastcall TfrmPanel::FormShow(TObject *Sender)
{
    LoadNoiseSound( *playerNoise_ );
    PlayNoiseSound();
}
//---------------------------------------------------------------------------

bool TfrmPanel::GetFanNoise() const
{
    return fanNoise_;
}
//---------------------------------------------------------------------------

void TfrmPanel::SetFanNoise( bool Val )
{
    if ( fanNoise_ != Val ) {
        fanNoise_ = Val;
        PlayNoiseSound();
    }
}
//---------------------------------------------------------------------------

