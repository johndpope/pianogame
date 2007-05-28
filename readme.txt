Synthesia teaches you to play piano using any MIDI file.
Visit http://www.synthesiagame.com/ for more information.

--------------------------------------------------------------------------------

Synthesia is mostly written by Nicholas Piegdon.

The following people have also contributed:
- Daniel Lawrence for the name Synthesia
- Luis Anton with an early version of the MIDI input code
- Troy Ramos with a suggestion to use Klavarskribo staff notation
- Dave Ray for his STRING/WSTRING macro
- NeHe Productions for their OpenGL base code
- Christian Auby for input octave shifting code

--------------------------------------------------------------------------------

Synthesia's video game music samples are provided by Game Music Themes.
Visit http://www.gamemusicthemes.com/ for high quality piano MIDI and sheets.

Bubble Bobble - Main Theme
  Composed by Zuntata / Arranged by Gori Fater

Dragon Warrior - Town Theme
  Composed by Koichi Sugiyama / Arranged by Russell Copeland

Sonic the Hedgehog - Green Hill Zone
  Composed by Masato Nakamura / Arranged by Gori Fater

Super Mario Bros. - Overworld Theme
  Composed by Koji Kondo / Arranged by BLUESCD

Super Mario Bros. - Underwater Theme
  Composed by Koji Kondo / Arranged by BLUESCD

Tetris - Theme A
  Arranged by Gori Fater

The Sims - Buying Theme 1
  Arranged by Gori Fater

The Legend of Zelda: A Link to the Past - Overworld
  Composed by Koji Kondo / Arranged by w3sp

The Legend of Zelda: Ocarina of Time - Lost Woods
  Composed by Koji Kondo / Arranged by BLUESCD

The Legend of Zelda: Ocarina of Time - Zelda's Lullaby
  Composed by Koji Kondo / Arranged by BLUESCD

--------------------------------------------------------------------------------


Change Log


Release 0.6.1
-------------
NEW: On the Mac, output to external devices is now available.
NEW: On the Mac, you can now drag a MIDI file to the application on the Dock or
     in Finder or right click a MIDI file and choosing "Open With..." to open.
NEW: In Windows, added a low-tech solution to choose between different monitors
     connected to the SAME video card (doesn't work in multi-video-card
     situations.) (Set the string key HKCU\Software\Synthesia\Monitor to "0"
     for primary monitory, "1" for secondary, etc.)
CHG: Mac game window is now much less obtrusive.  Other windows are allowed to
     be on top and the Dock and Application bar can both be made visible by
     hovering the mouse over there positions for a second or so.  The game
     window also no longer must be hidden during file selection or error boxes.
FIX: In Windows, MIDI SysEx messages in input are no longer flagged as errors.
FIX: On the Mac, fixed MIDI that used RPN / NRPN controller events (e.g. to
     set pitch bend) so they're now handled correctly.
FIX: On the Mac, output devices are now reset much harder than before, so
     output device internal state that may be set by one song isn't carried
     over to another.
FIX: On the Mac, there was a potential problem with some input devices that
     might send bursts of events all at once.  Previously, only the first
     event in the incoming list of events was recognized.  Now, any number of
     incoming events will be recognized correctly.
FIX: Picked a font for the Mac version to use that is actually a default
     system font.  (Apparently "Palatino" only ships with iLife now.)
FIX: On the Mac, fixed a problem where the (modal!) file open dialog could
     be hidden underneath the game window, leaving you not choice but to force-
     quit the program.
FIX: Disabled OpenGL depth testing to fix a problem where text would sometimes
     not appear above other controls (TrackBox, SongBox, etc.) in Windows.
FIX: Score state now displays correctly at low resolutions (like 800x600).
FIX: Title bar no longer pops when changing speed at the beginning of gameplay.
FIX: Text centering and placement on the Mac is now properly implemented.
FIX: MIDI files with non-all-lowercase file extensions can now be selected
     in the File Open dialog box.


Release 0.6.0
-------------
NEW: Mac Version!
NEW: All new note and keyboard graphics!
NEW: All new menu graphics.
NEW: Right-click association option in installer.
CHG: Rebranded as Synthesia
CHG: Switched from base 2D OS drawing to 3D accelerated OpenGL drawing.
CHG: Moved status display to the bottom of the screen during play.
CHG: Made default music folder less imposing.  Now the game will only default
     to the directory the very first run.  Afterwards, it will remember the
     last file you attempted to play (like it used to).
CHG: Improved exception handling.
FIX: Don't crash if "Synthesia Music" folder doesn't exist.
FIX: No more crash sometimes when app regains focus on the title screen.


Release 0.5.1
-------------
NEW: Your notes are played (in the correct instrument for the track)
NEW: Tooltips on all menu controls and settings.
NEW: 10 Sample songs from Game Music Themes.
NEW: "Stray Notes" metric on stats screen
NEW: "Average Speed" metric on stats screen
NEW: Box around MIDI input test on title screen
NEW: Combo counter and much improved stats display during play.
CHG: Improved Klavarskribo staff notation.
CHG: Match input to closest unplayed note's opportunity window center instead
     of matching against the first available unplayed note.
CHG: Swapped blue and yellow in the track color order, making blue the new
     "first" color.
CHG: ReasonableSynthVolume object replaces MidiCommOut::SetReasonableSynthVol
     and now works for all mixers in the system instead of only the first.
CHG: Darkened the border for a few different colors to increase the contrast
     between sharp/flat notes in a series.
CHG: Default to new "Piano Hero Music" folder at startup.  (User configurable
     in the future.)
FIX: Disabled MIDI input while game is paused to prevent cheating.
FIX: Made MidiCommOut::Reset() actually reset everything by fully closing and
     reopening the device.  midiOutReset() apparently only turns off the keys
     and pedals on each track.  It *doesn't* reset patches or volume.
FIX: Reworked MIDI engine to correct a sync/lag issue in songs that started
     with a considerable delay before the first note.
FIX: Notes don't draw on top of keys anymore.
FIX: MIDI track default color order is now consistent for all files.
FIX: Two tracks with identical notes would cause the "second" track's note to
     not appear.  This normally would not matter, as they overlap completely,
     but when hiding the first track, the second note would not show.


Release 0.5.0
-------------
NEW: Klavarskribo staff notation (thanks to Troy Ramos for the suggestion!)
NEW: MIDI input instrument selection (early version by Luis Anton)
NEW: MIDI input with note matching and scoring (early version by Luis Anton)
NEW: Note hit/miss effects
NEW: If muted, temporarily unmute the selected MIDI device during gameplay
NEW: Level progress bar along top of game play screen
NEW: In-game open file dialog
NEW: Song finish screen with score, rating, and statistics


Release 0.4.0
-------------
NEW: Initial Release

