

**Note:** Shortcuts can be customized using [keymap presets](Keymaps.md).

# Global keyboard shortcuts #

| cursor keys | move around/change values |
|:------------|:--------------------------|
| TAB | change focused element (where applicable) |
| F2 | pattern editor |
| Shift+F2 | wavetable editor |
| F3 | instrument editor |
| Shift+F3 | effect editor |
| F4 | sequence editor |
| Shift+F4 | classic editor |
| F5 | play |
| F6 | play from cursor |
| Shift+F6 | loop patterns at cursor |
| F8 | stop |
| F9/F10 | octave down/up |
| Ctrl+0..9 | Set edit jump (number of rows pattern editor advances when data is entered) |
| CTRL+n | new song |
| CTRL+s/o | save/open song |
| CTRL+c/v / CTRL+ins/SHIFT+ins | copy/paste |
| CTRL+j | join paste |
| CTRL+a | select all |
| CTRL+d | select nothing |
| CTRL+b | begin selection at cursor |
| CTRL+e | end selection at cursor |
| SHIFT+Up/Down| select block |
| CTRL+m | Unmute all channels |
| keypad plus/minus | select instrument |
| CTRL+keypad plus/minus | change song speed  |
| ALT+keypad plus/minus | change second song speed the two speeds mean the first speed is used for even steps and the other for odd steps. e.g. 8+4 will have a distinct shuffle in the tempo. |
| CTRL+F9/F10 | change song player rate (i.e. 50 Hz means every second 50 frames are processed, which would translate to 50/6 steps per second for the default speeds) |
| SHIFT+CTRL+F9/F10 | change time signature (only has a visual effect, the spacing of the different colored pattern steps changes) |
| CTRL+R | Change screen scaling |
| CTRL+Z | Undo |
| CTRL+Y | Redo |

# Pattern editor #

| Return | jump to sequence editor |
|:-------|:------------------------|
| ALT+insert/delete | change pattern length |
| Cursor keys/pg up/pg dn | move cursor |
| Ctrl+left/right || Single pattern edit: change current|
pattern<br>Multiple pattern edit: Jump to neighboring channel <br>
<table><thead><th> Ctrl+Shift+left/right </th><th> Jump to the same parameter column in neighboring channel </th></thead><tbody>
<tr><td> Ctrl+up/down </td><td> Move sequence editor position </td></tr>
<tr><td> Ctrl+I </td><td> Interpolate selection </td></tr>
<tr><td> Ctrl+K </td><td> Clone pattern </td></tr>
<tr><td> Ctrl+U </td><td> Jump to an unused pattern </td></tr>
<tr><td> 1 </td><td> Note colum: Note-off </td></tr>
<tr><td> 0/1 </td><td> Set legato etc. bits </td></tr>
<tr><td> Insert/delete/backspace </td><td> do what you would expect </td></tr>
<tr><td> Shift + Delete </td><td> Empty the whole pattern row (In Protracker mode) </td></tr>
<tr><td> . </td><td> no note/no instrument number etc. </td></tr>
<tr><td> most other keys </td><td> enter note </td></tr></tbody></table>

<table><thead><th> Ctrl+Shift+E </th><th> Expand pattern x2 </th></thead><tbody></tbody></table>

<h1>Sequence editor</h1>

<table><thead><th> 0-9, a-z, Shift+a-z </th><th> enter pattern </th></thead><tbody>
<tr><td> Return </td><td> edit currently selected pattern in the pattern editor (actually edits all patterns on the current sequence position) </td></tr>
<tr><td> Period </td><td> Set current pattern to none </td></tr>
<tr><td> ALT+up/down </td><td> adjust transpose for current pattern </td></tr>
<tr><td> CTRL+U </td><td> Add unused pattern at cursor </td></tr>
<tr><td> CTRL+K </td><td> Clone pattern at cursor </td></tr>
<tr><td> CTRL+down/up </td><td> edit song length </td></tr>
<tr><td> CTRL+left/right </td><td> edit sequence editor pattern step </td></tr>
<tr><td> insert/delete/backspace </td><td> do what you would expect </td></tr></tbody></table>

<h1>Instrument editor</h1>

<table><thead><th> Up/down </th><th> select parameter </th></thead><tbody>
<tr><td> Left/right/return </td><td> Modify parameter </td></tr>
<tr><td> Tab </td><td> switch between the program and the parameters </td></tr></tbody></table>

<h2>Program editor</h2>

<table><thead><th> Return </th><th> Edit program command </th></thead><tbody>
<tr><td> Space </td><td> Combine/detach currently selected command to the command below </td></tr>