!include "MUI.nsh"

!define VERSION 0.5.1

Name "Piano Hero ${VERSION}"
OutFile "PianoHero-${VERSION}-installer.exe"
InstallDir "$PROGRAMFILES\Piano Hero"
BrandingText " "

!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_SMALLDESC 

!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
  
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\PianoHero "Install_Dir"

ComponentText "This will install Piano Hero ${VERSION} to your computer."
DirText "Choose a directory to install in to:"



Section "!Piano Hero" PianoHero
SectionIn RO
  SetOutPath $INSTDIR

  File "Release\Piano Hero.exe"
  File "readme.txt"
  File "license.txt"

  CreateDirectory "$DOCUMENTS\Piano Hero Music"
  WriteRegStr HKCU "SOFTWARE\Piano Hero" "Default Music Directory" "$DOCUMENTS\Piano Hero Music"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\PianoHero "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PianoHero" "DisplayName" "Piano Hero (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PianoHero" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
SectionEnd



Section "Sample Music" MusicSamples
  SetOutPath "$DOCUMENTS\Piano Hero Music"
  File "music\Bubble Bobble - Main Theme.mid"
  File "music\Dragon Warrior - Town Theme.mid"
  File "music\Sonic the Hedgehog - Green Hill Zone.mid"
  File "music\Super Mario Bros - Overworld.mid"
  File "music\Super Mario Bros - Underwater.mid"
  File "music\Tetris - Theme A.mid"
  File "music\The Sims - Buying Theme 1.mid"
  File "music\Zelda A Link to the Past - Overworld Theme.mid"
  File "music\Zelda Ocarina of Time - Lost Woods.mid"
  File "music\Zelda Ocarina of Time - Zelda's Lullaby.mid"
SectionEnd



Section "Start Menu Shortcuts" ShortcutMenu
  CreateDirectory "$SMPROGRAMS\Piano Hero"
  CreateShortCut "$SMPROGRAMS\Piano Hero\Play Piano Hero.lnk" "$INSTDIR\Piano Hero.exe" "" "$INSTDIR\Piano Hero.exe" 0
  CreateShortCut "$SMPROGRAMS\Piano Hero\View Readme.lnk" "$INSTDIR\readme.txt"
  CreateShortCut "$SMPROGRAMS\Piano Hero\View License.lnk" "$INSTDIR\license.txt"
  CreateShortCut "$SMPROGRAMS\Piano Hero\Visit the Piano Hero Website.lnk" "http://www.halitestudios.com/pianohero.aspx"
  CreateShortCut "$SMPROGRAMS\Piano Hero\Uninstall Piano Hero.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd



Section /o "Desktop Icon" DesktopIcon
  CreateShortCut "$DESKTOP\Play Piano Hero.lnk" "$INSTDIR\Piano Hero.exe" "" "$INSTDIR\Piano Hero.exe" 0
SectionEnd



UninstallText "This will uninstall Piano Hero ${VERSION}. Click next to continue."
Section "Uninstall"

  ; remove registry keys
  ;
  ; NOTE: this intentionally leaves the "Install_Dir"
  ; entry in HKLM\SOFTWARE\[program-name] in the event of
  ; subsequent reinstalls/upgrades
  ;
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PianoHero"
  DeleteRegKey HKCU "SOFTWARE\Piano Hero"

  ; delete program files
  Delete $INSTDIR\readme.txt
  Delete $INSTDIR\license.txt
  Delete $INSTDIR\uninstall.exe
  Delete "$INSTDIR\Piano Hero.exe"
  RMDir /r "$INSTDIR"

  ; delete included music
  Delete "$DOCUMENTS\Piano Hero Music\Bubble Bobble - Main Theme.mid"
  Delete "$DOCUMENTS\Piano Hero Music\Dragon Warrior - Town Theme.mid"
  Delete "$DOCUMENTS\Piano Hero Music\Sonic the Hedgehog - Green Hill Zone.mid"
  Delete "$DOCUMENTS\Piano Hero Music\Super Mario Bros - Overworld.mid"
  Delete "$DOCUMENTS\Piano Hero Music\Super Mario Bros - Underwater.mid"
  Delete "$DOCUMENTS\Piano Hero Music\Tetris - Theme A.mid"
  Delete "$DOCUMENTS\Piano Hero Music\The Sims - Buying Theme 1.mid"
  Delete "$DOCUMENTS\Piano Hero Music\Zelda A Link to the Past - Overworld Theme.mid"
  Delete "$DOCUMENTS\Piano Hero Music\Zelda Ocarina of Time - Lost Woods.mid"
  Delete "$DOCUMENTS\Piano Hero Music\Zelda Ocarina of Time - Zelda's Lullaby.mid"
  RMDir  "$DOCUMENTS\Piano Hero Music" ; this won't delete the directory if the user has added anything

  ; remove Start Menu shortcuts
  Delete "$SMPROGRAMS\Piano Hero\*.*"
  RMDir "$SMPROGRAMS\Piano Hero"

  ; remove Desktop shortcut
  Delete "$DESKTOP\Play Piano Hero.lnk"
SectionEnd




LangString DESC_PianoHero ${LANG_ENGLISH} "Install the Piano Hero application files (required)."
LangString DESC_MusicSamples ${LANG_ENGLISH} "Install 10 sample video game MIDI songs from Game Music Themes."
LangString DESC_ShortcutMenu ${LANG_ENGLISH} "Create a Piano Hero Start Menu group on the 'All Programs' section of your Start Menu."
LangString DESC_DesktopIcon ${LANG_ENGLISH} "Create a Piano Hero icon on your Windows Desktop."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${PianoHero} $(DESC_PianoHero)
  !insertmacro MUI_DESCRIPTION_TEXT ${MusicSamples} $(DESC_MusicSamples)
  !insertmacro MUI_DESCRIPTION_TEXT ${ShortcutMenu} $(DESC_ShortcutMenu)
  !insertmacro MUI_DESCRIPTION_TEXT ${DesktopIcon} $(DESC_DesktopIcon)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
