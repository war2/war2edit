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

- [ ] Overhaul minimap
- [ ] Scaling of map
- [ ] Different brush types
- [ ] Implement properties editors
- [ ] Help/About panel
- [ ] Toggle extension mode
- [ ] Undo/Redo
- [ ] Oil Patch can be placed too close to land
- [ ] Oil Patch and Gold Mines must prevent collectors to be placed too close
- [ ] DOSBox/Boxer fancy selection panel



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
Code is under the MIT License. For more details, plus refer to the `COPYING` file.
