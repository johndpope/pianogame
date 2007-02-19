Piano Hero teaches you to play piano using any MIDI file.
Visit http://pianohero.sourceforge.net/ for more information.

--------------------------------------------------------------------------------

Piano Hero is written by Nicholas Piegdon.

The following people have also contributed to the project:
- Luis Anton with an early version of the MIDI input code
- Troy Ramos with a suggestion to use Klavarskribo staff notation

Piano Hero uses the TinyXml XML parser.
See http://tinyxml.sourceforge.net/ for more information.

--------------------------------------------------------------------------------

Piano Hero's video game music samples are provided by Game Music Themes.
Visit http://www.gamemusicthemes.com/ for high quality piano MIDI and sheets.

TODO: Song titles and composer/arranger here!

--------------------------------------------------------------------------------

Changes

Release 0.5.1
-------------
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

