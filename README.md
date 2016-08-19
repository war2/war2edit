war2edit
========

A modest clone of Warcraft II World Map Editor.


Build
=====

- `EFL` must be installed (https://www.enlightenment.org/docs);
- `cairo` must be installed (https://www.cairographics.org/);
- `war2tools` must be installed (https://github.com/jeanguyomarch/war2tools)

- Create build directory: `mkdir -b build && cd build`.
- Run cmake: `cmake ..`.
- Compile: `make`.
- Install: `sudo make install`.
- Launch: `war2edit`.


TODO & BUGS
===========

- [ ] Tiles algorithm is broken with dark tiles
- [ ] Overhaul minimap
- [ ] Scaling of map
- [ ] Different brush types
- [ ] Implement properties editors
- [ ] Help/About panel
- [ ] Toggle extension mode
- [ ] Undo/Redo
- [ ] Oil Patch can be placed too close to land
- [ ] Oil Patch and Gold Mines must prevent collectors to be placed too close
- [ ] Walls must be implemented (currently broken)
- [ ] DOSBox/Boxer fancy selection panel
- [ ] Gold Mine and Oil Patches values must be editable



Contributors
============

- Lucie Guyomarc'h:
   - Fix mispelled names

- Jean-Luc Guyomarc'h
   - Toolbar icons
   - Game icon

License
=======

All resources (sprites, tiles, .PUD, .war) are property of Blizzard Entertainment.
The following license applies only to source files.


The MIT License (MIT)

Copyright (c) 2014 - 2016 Jean Guyomarc'h

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
