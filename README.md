# drumkv1gen

## What is it?

drumkv1gen is a small helper tool that lets you take a set of .wav
files and create from that a mapping file for drumkv1, Rui Capela's
"old-school drum-kit sampler", see http://drumkv1.sourceforge.io .

## Why do we need this?

drumkv1 is an LV2 plugin with a user interface (UI) that allows you to
assign (interactively) a sample to a MIDI key, one at a time. However, for
large sample sets, this point-and-click adventure can be tedious, so drumkv1gen
was written to make your life easier here.

If you have, say, a 16-sample collection of a Roland TR808 drum machine and
want to use it in a music project via drumkv1, this tools lets you somewhat
easily create the required mapping for it.


## How it works

drumkv1gen has two basic operating modes - an automatic mode and a
manually controlled mode.

In automatic mode, you simply provide a directory containing .wav files (up to
127, as per MIDI standard)i and an output file name, and drumkv1gen will assign
each of these samples (sorted alphabetically) to a MIDI key, starting with MIDI
note 36 (C2), moving up from there by one MIDI key number per sample (C#2, D2
and so on), and writes out the mapping into a .drumkv1 file.
Please note that in this mode you have no control over what sample gets
assigned to what key; this is perhaps acceptable for, say, a collection of
ambient sounds, explosions or other material where no real "standard" exists
which sound to expect on what key.

In manually controlled mode (invoked via the "-m <mapfile>" option), you
write/create an intermediate map file yourself which has a very simple
syntax, and drumkv1gen will create a .drumkv1 file based on your mapping. This
allows you to define what sample goes to what MIDI note.
This is typically needed for MIDI drum kits where you simply expect certain
sounds to be assigned to certain keys (e.g. Kick -> C3, Snare -> D3 etc),
see e.g. https://www.midi.org/specifications/item/gm-level-1-sound-set .


## Mapping file syntax

The manually written map file (I suggest the file name extension .map)
has the following trivial format:

- Comment lines start with a "#" character and are ignored
- White-space lines and blank lines are ignored
- All other lines should follow this syntax:
  ```
  <note number> <path/filename>
  ```
  e.g.
  ```
  36 Drums/kick.wav
  38 Drums/snare.wav
  40 hihat.wav
  ```

This tool comes with a small example of a .map file and samples from
which you can generate a .drumkv1 file yourself.

## Usage

drumkv1gen is invoked like this:

```
$ drumkv1gen
Usage: ./drumkv1gen [options] -d <dir> -n <name>
Options:
 -h, --help       print usage (this text)
 -V, --version    print program version number and exit

 -d <dir>         specifies directories to read sample files from (*.wav, *.WAV)
 -o <name>        specifies name of output file (hint: use'.drumkv1' as extension)
 -f, --force      overwrite existing files
 -m <mapfile>     read mapping from <mapfile>
```

Example invocation in automatic mode:
```
$ drumkv1gen -d ~/media/samples/motorsounds/ -o motors.drumkv1
TODO: Should print out how many samples were mapped, from..to
```

Example invocation in manual (mapping file) mode:
```
$ drumkv1gen -m tr808.map -o tr808.drumkv1
```

## Hints

### Absolute vs. relative paths

The "-d" parameter (directory to read samples from) can be either an
absolute or a relative path. When you use an absolute path, the paths
stored in the generated .drumkv1 file will also be absolute, and might
not work when you shift the project to another machine where the samples
might be stored in a different path.
For portability reasons, I therefore suggest to use relative path names
and store the .drumkv1 file "near" your sample data, like so:

```
.
├── mydrums.drumkv1
└── samples
    ├── closed_hihat.wav
    ├── kickdrum.wav
    ├── open_hihat.wav
    └── snaredrum.wav
```

This way, you can archive the whole folder (samples + .drumkv1 file),
transfer that archive to another computer, unpck it there and continue
working with it immediately.

### Files with white space in their name

In freely available sample libraries you will often encounter sample files
with white space in the name. drumkv1gen should process these correctly by now.

## License

drumkv1gen is license under the GNU Public License (GPL) v2.


## Author

Frank Neumann <beachnase@web.de>

## Thanks

Big thanks go to Rui Capela for writing all of these valuable tools in the
Linux audio world through the last 15 or so years - qtractor, qjackctl,
the "VeeOne" suite - you name them.
Of course, anyone else who has enriched our (Linux Audio) life with free
and open software and the landscape around it deserves a big thanks too!

