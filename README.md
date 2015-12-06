war2edit
========

A modest clone of Warcraft II World Map Editor.


Build
=====

- Create build directory: `mkdir -b build && cd build`.
- Run cmake: `cmake ..`.
- Compile: `make`.
- Install: `sudo make install`.
- Launch: `war2edit`.


TODO - Showstoppers
===================

- [ ] Redo completly the graphical part. Elm_Bitmap is waayyy to heavy and inefficient.
      Seems the best option is Evas_GL. Evas_Image + proxys are so easy to use,
      and so efficient, but the smart_move kills it...
- [ ] Minimap window is annoying and suboptimal (let's Evas do the scaling)
- [ ] Place textures
- [ ] Different brush types
- [ ] Different brush sizes
- [X] Load a map
- [ ] Auto-run war2 to test the map
- [ ] Implement properties editors
- [ ] Help/About panel
- [ ] Toggle race in menu
- [ ] Toggle extension mode
- [X] Selection mode to select units
- [X] Delete a unit
- [ ] Undo/Redo


License
=======

All resources (sprites, tiles, .PUD, .war) are property of Blizzard Entertainment.
The following license applies only to all source files.


The MIT License (MIT)

Copyright (c) 2014 - 2015 Jean Guyomarc'h

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

