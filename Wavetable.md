# What's this? #

Basically, the wavetable makes it possible for an instrument to play an actual sample. It can be combined with the synth. A very simple use would be just to play a sampled instrument instead of chip sounds and a more complex example is to play a sampled "attack" combined with synth "release", MT-32-style.

# How to use waves #

  1. In the wavetable, select the wave you want to use and load a .WAV file from menu (File > Wavetable > Open WAV).
  1. If the wave is tuned, set the base note to match the wave (i.e. if the wave is a sampled A-4 piano sound, set the base note to A-4).
  1. In the instrument editor, enable WAVE in the instrument you want to associate a wave with and set the wave number to the wave you want to use.
  1. Enable OENV if you want the wave to bypass the volume envelope and L if you want the wave to play at the base note set in wavetable (the synth pitch will still play at the played note)
  1. The instrument will now trigger the wave. Multiple instruments can use the same wave.

# Wavetable editor #

Use WAVE to select the wave to be edited. Loading a wave will set the sample rate but it can be fine tuned with RATE. BASE sets the base note (the pitch of the sampled sound). NO INTERPOLATION disables smooth playback (useful for lo-fi samples that get very muddy with interpolation). LOOP enables looping between BEGIN and END, PINGPONG makes the loop run back and forth.

Use the keyboard to preview the currently selected wavetable item.