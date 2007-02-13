Piano Hero teaches you to play piano using any MIDI file.
Visit http://pianohero.sourceforge.net/ for more information.

--------------------------------------------------------------------------------

Piano Hero was originally written by Nicholas Piegdon.

The following people have also contributed to the project:
- Luis Anton with an early version of the MIDI input code
- Troy Ramos with a suggestion to use Klavarskribo staff notation

Piano Hero uses the TinyXml XML parser.
See http://tinyxml.sourceforge.net/ for more information.

Piano Hero's sample video game music tracks are provided by Game Music Themes.
See http://www.gamemusicthemes.com/ for more high quality piano MIDI.

--------------------------------------------------------------------------------

Changes

Releaes 0.5.1
- NEW: "Stray Notes" metric on stats screen
- NEW: "Average Speed" metric on stats screen
- NEW: Box around MIDI input test on title screen
- BUG: Disabled MIDI input while game is paused
- BUG: Made MidiCommOut::Reset() actually reset everything by fully closing and
       reopening the device.  midiOutReset() apparently only turns off the keys
       and pedals on each track.  It *doesn't* reset patches or volume.
- CHG: ReasonableSynthVolume object replaces MidiCommOut::SetReasonableSynthVol
       and now works for all mixers in the system instead of only the first.


Release 0.5.0
- NEW: Klavarskribo staff notation (thanks to Troy Ramos for the suggestion!)
- NEW: MIDI input instrument selection (early version by Luis Anton)
- NEW: MIDI input with note matching and scoring (early version by Luis Anton)
- NEW: Note hit/miss effects
- NEW: If muted, temporarily unmute the selected MIDI device during gameplay
- NEW: Level progress bar along top of game play screen
- NEW: In-game new file dialog
- NEW: Song finish screen with score, rating, and statistics


Release 0.4.0
- NEW: Initial Release

