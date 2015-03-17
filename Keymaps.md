Keymaps can be used to adjust the keyboard commands to match e.g. Protracker key commands.

Keymap presets are in key subdirectory in the program directory. The directory has a file for each preset, similarly to the [theme](Themes.md)/skin system.

# Format #

The file has one shortcut mapping on each line:

```
<FROM> = <TO>
```

`FROM` and `TO` are combinations of modifiers (M_prefix) and keypresses (K_ prefix). Generally, Alt, Ctrl and Shift are to be used as modifiers when used in combination with other keys. If e.g. pressing just the right Ctrl does something, it has to be used as a key (K\_RCTRL).

Hash (#) can be used for comments.

Valid key definitions can be found in [keytab.c](http://code.google.com/p/klystrack/source/browse/trunk/src/keytab.c). E.g. `KEYDEF(KP_DIVIDE)` (keypad slash/divide) becomes K\_KP\_DIVIDE. `MODDEF(LCTRL)` (left Ctrl) becomes M\_LCTRL.

The file is divided in sections so that shortcuts apply globally or to specific editors. The sections are marked as follows:

```
[global]
M_LCTRL K_Q = K_ESCAPE

[pattern]
M_LCTRL K_Q = K_SPACE
```

The above would map `LeftCtrl-Q` the same as pressing `escape` except in the pattern editor where it would be the same as pressing `space bar`. Valid sections are global, pattern, instrument and sequence.