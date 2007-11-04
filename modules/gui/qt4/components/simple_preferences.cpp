/*****************************************************************************
 * simple_preferences.cpp : "Simple preferences"
 ****************************************************************************
 * Copyright (C) 2006-2007 the VideoLAN team
 * $Id: preferences.cpp 16348 2006-08-25 21:10:10Z zorglub $
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Antoine Cellerier <dionoea@videolan.org>
 *          Jean-Baptiste Kempf <jb@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "components/simple_preferences.hpp"
#include "components/preferences_widgets.hpp"

#include "ui/sprefs_input.h"
#include "ui/sprefs_audio.h"
#include "ui/sprefs_video.h"
#include "ui/sprefs_subtitles.h"
#include "ui/sprefs_hotkeys.h"
#include "ui/sprefs_interface.h"

#include <vlc_config_cat.h>
#include <vlc_configuration.h>

#include <QString>
#include <QFont>
#include <QToolButton>
#include <QButtonGroup>
#include <QUrl>
#include <QVBoxLayout>

#define ICON_HEIGHT 64
#define BUTTON_HEIGHT 74

/*********************************************************************
 * The List of categories
 *********************************************************************/
SPrefsCatList::SPrefsCatList( intf_thread_t *_p_intf, QWidget *_parent ) :
                                  QWidget( _parent ), p_intf( _p_intf )
{
    QVBoxLayout *layout = new QVBoxLayout();

    QButtonGroup *buttonGroup = new QButtonGroup( this );
    buttonGroup->setExclusive ( true );
    CONNECT( buttonGroup, buttonClicked ( int ),
            this, switchPanel( int ) );

#define ADD_CATEGORY( button, label, icon, numb )                           \
    QToolButton * button = new QToolButton( this );                         \
    button->setIcon( QIcon( ":/pixmaps/" #icon ) );                         \
    button->setIconSize( QSize( ICON_HEIGHT , ICON_HEIGHT ) );              \
    button->setText( label );                                               \
    button->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );              \
    button->resize( BUTTON_HEIGHT , BUTTON_HEIGHT);                         \
    button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding) ;  \
    button->setAutoRaise( true );                                           \
    button->setCheckable( true );                                           \
    buttonGroup->addButton( button, numb );                                 \
    layout->addWidget( button );

    ADD_CATEGORY( SPrefsInterface, qtr("Interface"),
                  spref_cone_Interface_64.png, 0 );
    ADD_CATEGORY( SPrefsAudio, qtr("Audio"), spref_cone_Audio_64.png, 1 );
    ADD_CATEGORY( SPrefsVideo, qtr("Video"), spref_cone_Video_64.png, 2 );
    ADD_CATEGORY( SPrefsSubtitles, qtr("Subtitles"),
                  spref_cone_Subtitles_64.png, 3 );
    ADD_CATEGORY( SPrefsInputAndCodecs, qtr("Input and Codecs"),
                  spref_cone_Input_64.png, 4 );
    ADD_CATEGORY( SPrefsHotkeys, qtr("Hotkeys"), spref_cone_Hotkeys_64.png, 5 );

    SPrefsInterface->setChecked( true );
    layout->setMargin( 0 );
    layout->setSpacing( 1 );

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setLayout( layout );

}

void SPrefsCatList::switchPanel( int i )
{
    emit currentItemChanged( i );
}

/*********************************************************************
 * The Panels
 *********************************************************************/
SPrefsPanel::SPrefsPanel( intf_thread_t *_p_intf, QWidget *_parent,
                          int _number ) : QWidget( _parent ), p_intf( _p_intf )
{
    module_config_t *p_config;
    ConfigControl *control;
    number = _number;

#define CONFIG_GENERIC( option, type, label, qcontrol )                   \
            p_config =  config_FindConfig( VLC_OBJECT(p_intf), option );  \
            if( p_config )                                                \
            {                                                             \
                control =  new type ## ConfigControl( VLC_OBJECT(p_intf), \
                           p_config, label, ui.qcontrol, false );         \
                controls.append( control );                               \
            }

#define CONFIG_GENERIC_NO_BOOL( option, type, label, qcontrol )           \
            p_config =  config_FindConfig( VLC_OBJECT(p_intf), option );  \
            if( p_config )                                                \
            {                                                             \
                control =  new type ## ConfigControl( VLC_OBJECT(p_intf), \
                           p_config, label, ui.qcontrol );                \
                controls.append( control );                               \
            }

#define CONFIG_GENERIC_FILE( option, type, label, qcontrol, qbutton )         \
                p_config =  config_FindConfig( VLC_OBJECT(p_intf), option );  \
                if( p_config )                                                \
                {                                                             \
                    control =  new type ## ConfigControl( VLC_OBJECT(p_intf), \
                               p_config, label, ui.qcontrol, ui.qbutton,      \
                            false );                                          \
                    controls.append( control );                               \
                }

#define START_SPREFS_CAT( name , label )    \
        case SPrefs ## name:                \
        {                                   \
            Ui::SPrefs ## name ui;          \
            ui.setupUi( panel );            \
            panel_label->setText( label );

#define END_SPREFS_CAT      \
            break;          \
        }

    QVBoxLayout *panel_layout = new QVBoxLayout();
    QWidget *panel = new QWidget();
    panel_layout->setMargin( 3 );

    // Title Label
    QLabel *panel_label = new QLabel;
    QFont labelFont = QApplication::font( static_cast<QWidget*>(0) );
    labelFont.setPointSize( labelFont.pointSize() + 6 );
    labelFont.setFamily( "Verdana" );
    panel_label->setFont( labelFont );

    // Title <hr>
    QFrame *title_line = new QFrame;
    title_line->setFrameShape(QFrame::HLine);
    title_line->setFrameShadow(QFrame::Sunken);

    QFont italicFont = QApplication::font( static_cast<QWidget*>(0) );
    italicFont.setItalic( true );

    switch( number )
    {
        /******************************
         * VIDEO Panel Implementation *
         ******************************/
        START_SPREFS_CAT( Video , qtr("General video settings") );
            CONFIG_GENERIC( "video", Bool, NULL, enableVideo );

            CONFIG_GENERIC( "fullscreen", Bool, NULL, fullscreen );
            CONFIG_GENERIC( "overlay", Bool, NULL, overlay );
            CONFIG_GENERIC( "video-on-top", Bool, NULL, alwaysOnTop );
            CONFIG_GENERIC( "video-deco", Bool, NULL, windowDecorations );
            CONFIG_GENERIC( "skip-frames" , Bool, NULL, skipFrames );
            CONFIG_GENERIC( "overlay", Bool, NULL, overlay );
            CONFIG_GENERIC( "vout", Module, NULL, outputModule );

#ifdef WIN32
            CONFIG_GENERIC( "directx-wallpaper" , Bool , NULL, wallpaperMode );
            CONFIG_GENERIC( "directx-device", StringList, NULL,
                    dXdisplayDevice );
#else
            ui.directXBox->setVisible( false );
#endif

            CONFIG_GENERIC_FILE( "snapshot-path", Directory, NULL,
                    snapshotsDirectory, snapshotsDirectoryBrowse );
            CONFIG_GENERIC( "snapshot-prefix", String, NULL, snapshotsPrefix );
            CONFIG_GENERIC( "snapshot-sequential", Bool, NULL,
                            snapshotsSequentialNumbering );
            CONFIG_GENERIC( "snapshot-format", StringList, NULL,
                            snapshotsFormat );
         END_SPREFS_CAT;

        /******************************
         * AUDIO Panel Implementation *
         ******************************/
        START_SPREFS_CAT( Audio, qtr("General audio settings") );

            CONFIG_GENERIC( "audio", Bool, NULL, enableAudio );

            /* General Audio Options */
            CONFIG_GENERIC_NO_BOOL( "volume" , IntegerRangeSlider, NULL,
                                     defaultVolume );
            CONFIG_GENERIC( "audio-language" , String , NULL,
                            preferredAudioLanguage );

            CONFIG_GENERIC( "spdif", Bool, NULL, spdifBox );
            CONFIG_GENERIC( "force-dolby-surround" , IntegerList , NULL,
                            detectionDolby );

            CONFIG_GENERIC( "headphone-dolby" , Bool , NULL, headphoneEffect );

            CONFIG_GENERIC_NO_BOOL( "norm-max-level" , Float , NULL,
                                    volNormSpin );
            CONFIG_GENERIC( "audio-visual" , Module , NULL, visualisation);

            /* Audio Output Specifics */
            CONFIG_GENERIC( "aout", Module, NULL, outputModule );

            CONNECT( ui.outputModule, currentIndexChanged( int ), this,
                             updateAudioOptions( int ) );
            audioOutput = ui.outputModule;

        //TODO: use modules_Exists
#ifndef WIN32
            CONFIG_GENERIC( "alsadev" , StringList , ui.alsaLabel, alsaDevice );
            CONFIG_GENERIC_FILE( "dspdev" , File , ui.OSSLabel, OSSDevice,
                                 OSSBrowse );
#else
            CONFIG_GENERIC( "directx-audio-device", IntegerList,
                    ui.DirectXLabel, DirectXDevice );
#endif
        // File exists everywhere
            CONFIG_GENERIC_FILE( "audiofile-file" , File , ui.fileLabel,
                                 fileName, fileBrowseButton );
            alsa_options = ui.alsaControl;
            oss_options = ui.OSSControl;
            directx_options = ui.DirectXControl;
            file_options = ui.fileControl;

        /* and hide if necessary */
#ifdef WIN32
            oss_options->hide();
            alsa_options->hide();
#else
            directx_options->hide();
#endif

            updateAudioOptions( audioOutput->currentIndex() );

            /* LastFM */
            CONFIG_GENERIC( "lastfm-username", String, ui.lastfm_user_label,
                         lastfm_user_edit );
            CONFIG_GENERIC( "lastfm-password", String, ui.lastfm_pass_label,
                         lastfm_pass_edit );
            ui.lastfm_user_edit->hide();
            ui.lastfm_user_label->hide();
            ui.lastfm_pass_edit->hide();
            ui.lastfm_pass_label->hide();

            if( config_ExistIntf( VLC_OBJECT( p_intf ), "audioscrobbler" ) )
                ui.lastfm->setCheckState( Qt::Checked );
            else
                ui.lastfm->setCheckState( Qt::Unchecked );
            CONNECT( ui.lastfm, stateChanged( int ), this ,
                    lastfm_Changed( int ) );

            /* Normalizer */
            normalizerBox = ui.volNormBox;
            CONNECT( ui.volNormBox, toggled( bool ), ui.volNormSpin, setEnabled( bool ) );
            qs_filter = qfu( config_GetPsz( p_intf, "audio-filter" ) );
            bool b_normalizer = ( qs_filter.contains( "volnorm" ) );
            {
                ui.volNormBox->setChecked( b_normalizer );
                ui.volNormSpin->setEnabled( b_normalizer );
            }

        END_SPREFS_CAT;

        /* Input and Codecs Panel Implementation */
        START_SPREFS_CAT( InputAndCodecs, qtr("Input & Codecs settings") );
            inputDevice = ui.DVDDevice;
          /* Disk Devices */
            {
                ui.DVDDevice->setToolTip(
                    qtr( "If this propriety is blank, then you have\n"
                         "values for DVD, VCD, and CDDA.\n"
                         "You can define a unique one or set that in"
                         "the advanced preferences" ) );
                char *psz_dvddiscpath = config_GetPsz( p_intf, "dvd" );
                char *psz_vcddiscpath = config_GetPsz( p_intf, "vcd" );
                char *psz_cddadiscpath = config_GetPsz( p_intf, "cd-audio" );
                if( ( *psz_cddadiscpath == *psz_dvddiscpath )
                   && ( *psz_dvddiscpath == *psz_vcddiscpath ) )
                {
                    ui.DVDDevice->setText( qfu( psz_dvddiscpath ) );
                }
                delete psz_cddadiscpath; delete psz_dvddiscpath;
                delete psz_vcddiscpath;
            }

            CONFIG_GENERIC_NO_BOOL( "server-port", Integer, NULL, UDPPort );
            CONFIG_GENERIC( "http-proxy", String , NULL, proxy );
        
          /* Caching */
/*          CONFIG_GENERIC( );*/ //FIXME

            CONFIG_GENERIC_NO_BOOL( "ffmpeg-pp-q", Integer, NULL, PostProcLevel );
            CONFIG_GENERIC( "avi-index", IntegerList, NULL, AviRepair );
            CONFIG_GENERIC( "rtsp-tcp", Bool, NULL, RTSP_TCPBox );
#ifdef WIN32
            CONFIG_GENERIC( "prefer-system-codecs", Bool, NULL, systemCodecBox );
#else
            ui.systemCodecBox->hide();
#endif  
            /* Access Filters */
            qs_filter = qfu( config_GetPsz( p_intf, "access-filter" ) );
            ui.timeshiftBox->setChecked( qs_filter.contains( "timeshift" ) );
            ui.dumpBox->setChecked( qs_filter.contains( "dump" ) );
            ui.recordBox->setChecked( qs_filter.contains( "record" ) );
            ui.bandwidthBox->setChecked( qs_filter.contains( "bandwidth" ) );
            timeshiftBox = ui.timeshiftBox;
            recordBox = ui.recordBox;
            dumpBox = ui.dumpBox;
            bandwidthBox = ui.bandwidthBox;
        END_SPREFS_CAT;

        /* Interface Panel */
        START_SPREFS_CAT( Interface, qtr("Interface settings") );
            ui.defaultLabel->setFont( italicFont );
            ui.skinsLabel->setFont( italicFont );

#if defined( WIN32 ) || defined (__APPLE__)
            CONFIG_GENERIC( "language", StringList, NULL, language );
#else
            ui.language->hide();
            ui.languageLabel->hide();
#endif

           /* interface */
            char *psz_intf = config_GetPsz( p_intf, "intf" );
            if( psz_intf )
            {
                msg_Dbg( p_intf, "Interface in config file: %s", psz_intf );
                if( strstr( psz_intf, "skin" ) )
                    ui.skins->setChecked( true );
                else if( strstr( psz_intf, "qt" ) )
                    ui.qt4->setChecked( true );
            }
            skinInterfaceButton = ui.skins;
            qtInterfaceButton = ui.qt4;
            delete psz_intf;

            CONFIG_GENERIC( "qt-always-video", Bool, NULL, qtAlwaysVideo );
            CONFIG_GENERIC_FILE( "skins2-last", File, NULL, fileSkin,
                    skinBrowse );
#if defined( WIN32 ) || defined(HAVE_DBUS_3)
            CONFIG_GENERIC( "one-instance", Bool, NULL, OneInterfaceMode );
            CONFIG_GENERIC( "playlist-enqueue", Bool, NULL,
                    EnqueueOneInterfaceMode );
#else
            ui.OneInterfaceBox->hide();
#endif
        END_SPREFS_CAT;

        START_SPREFS_CAT( Subtitles, qtr("Subtitles & OSD settings") );
            CONFIG_GENERIC( "osd", Bool, NULL, OSDBox);

            CONFIG_GENERIC( "subsdec-encoding", StringList, NULL, encoding );
            CONFIG_GENERIC( "sub-language", String, NULL, preferredLanguage );
            CONFIG_GENERIC_FILE( "freetype-font", File, NULL, font,
                            fontBrowse );
            CONFIG_GENERIC( "freetype-color", IntegerList, NULL, fontColor );
            CONFIG_GENERIC( "freetype-rel-fontsize", IntegerList, NULL,
                            fontSize );
            CONFIG_GENERIC( "freetype-effect", IntegerList, NULL, effect );

        END_SPREFS_CAT;

        START_SPREFS_CAT( Hotkeys, "Configure Hotkeys" );
        //FIMXE
        END_SPREFS_CAT;
        }

    panel_layout->addWidget(panel_label);
    panel_layout->addWidget(title_line);
    panel_layout->addWidget( panel );
    panel_layout->addStretch( 2 );

    this->setLayout(panel_layout);
}

void SPrefsPanel::updateAudioOptions( int number)
{
    QString value = audioOutput->itemData( number ).toString();

#ifndef WIN32
    oss_options->hide();
    alsa_options->hide();
#else
    directx_options->hide();
#endif
    file_options->hide();

   if( value == "aout_file" )
         file_options->show();
#ifndef WIN32
    else if( value == "alsa" )
        alsa_options->show();
    else if( value == "oss" )
        oss_options->show();
#else
    else if( value == "directx" )
        directx_options->show();
#endif
}

void SPrefsPanel::apply()
{
    msg_Dbg( p_intf, "Trying to save the %i simple panel", number );

    /* Generic save for ever panel */
    QList<ConfigControl *>::Iterator i;
    for( i = controls.begin() ; i != controls.end() ; i++ )
    {
        ConfigControl *c = qobject_cast<ConfigControl *>(*i);
        c->doApply( p_intf );
    }

    /* Devices */
    if( number == SPrefsInputAndCodecs )
    {
        char *psz_devicepath = qtu( inputDevice->text() );
        if( !EMPTY_STR( psz_devicepath ) )
        {
            config_PutPsz( p_intf, "dvd", psz_devicepath );
            config_PutPsz( p_intf, "vcd", psz_devicepath );
            config_PutPsz( p_intf, "cd-audio", psz_devicepath );
        }

        bool b_first = true;
#define saveBox( name, box ) {\
        if( box->isChecked() ) { \
            if( b_first ) { \
                qs_filter.append( name ); \
                b_first = false; \
            } \
            else qs_filter.append( ":" ).append( name ); \
        } } 
    
        saveBox( "record", recordBox );
        saveBox( "dump", dumpBox );
        saveBox( "timeshift", timeshiftBox );
        saveBox( "bandwidth", bandwidthBox );
        config_PutPsz( p_intf, "access-filter", qtu( qs_filter ) );
    }

    /* Interfaces */
    if( number == SPrefsInterface )
    {
        if( skinInterfaceButton->isChecked() )
            config_PutPsz( p_intf, "intf", "skins2" );
        if( qtInterfaceButton->isChecked() )
            config_PutPsz( p_intf, "intf", "qt4" );
    }

    if( number == SPrefsAudio )
    {
        bool b_normChecked = normalizerBox->isChecked();
        if( qs_filter.isEmpty() )
        {
            /* the psz_filter is already empty, so we just append it needed */
            if( b_normChecked ) qs_filter = "volnorm";
        }
        else /* Not Empty */
        {
            if( qs_filter.contains( "volnorm" ) )
            {
                /* The qs_filter not empty and contains "volnorm" 
                   that we have to remove */
                if( !b_normChecked )
                {
                    /* Ugly :D */
                    qs_filter.remove( "volnorm:" );
                    qs_filter.remove( ":volnorm" );
                    qs_filter.remove( "volnorm" );
                }
            }
            else /* qs_filter not empty, but doesn't have volnorm inside */
                if( b_normChecked ) qs_filter.append( ":volnorm" );
        }
        config_PutPsz( p_intf, "audio-filter", qtu( qs_filter ) );
    }
}

void SPrefsPanel::clean()
{}

void SPrefsPanel::lastfm_Changed( int i_state )
{
    if( i_state == Qt::Checked )
        config_AddIntf( VLC_OBJECT( p_intf ), "audioscrobbler" );
    else if( i_state == Qt::Unchecked )
        config_RemoveIntf( VLC_OBJECT( p_intf ), "audioscrobbler" );
}
