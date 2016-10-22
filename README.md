war2edit
========

<a href="https://scan.coverity.com/projects/jeanguyomarch-war2edit">
    <img alt="Coverity Scan Build Status"
         src="https://scan.coverity.com/projects/6936/badge.svg"/>
</a>

A modest clone of Warcraft II World Map Editor.


Build
=====

- `EFL` must be installed (https://www.enlightenment.org/docs);
- `cairo` must be installed (https://www.cairographics.org/);
- `war2tools` must be installed (https://github.com/war2/war2tools)

- Create build directory: `mkdir -p build && cd build`.
- Run cmake: `cmake ..`.
- Compile: `cmake --build .`.
- Install: `sudo make install`.
- Launch: `war2edit`.


License
=======

All resources (sprites, tiles, .PUD, .war) are property of Blizzard Entertainment.
Code is under the MIT License. For more details, plus refer to the `COPYING` file.
