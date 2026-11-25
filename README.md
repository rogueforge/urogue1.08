```
      _   _ ____
     | | | |  _ \ ___   __ _ _   _  ___
     | | | | |_) / _ \ / _` | | | |/ _ \
     | |_| |  _ < (_) | (_| | |_| |  __/
      \___/|_| \_\___/ \__, |\__,_|\___|
                       |___/
   o()xxxx[{:::::::::::::::::::::::::::::>

   ------------------        -------------
   |.......=........|        |...........|
   |..?.......@.....+########+.....%.....|
   |.........B..*...|        |......L....|
   ------------------        -------------

               UltraRogue 1.08
         by Herb Chong, August 2006

     based on Advanced Rogue (1984-1985)
      by Michael Morgan and Ken Dalka,
   Super-Rogue (1984) by Robert D. Kindelberger,
      and Rogue (1980-1981) by Michael Toy,
        Ken Arnold and Glenn Wichman
```

UltraRogue is a dungeon game in the line of Rogue, written by Herb
Chong. You choose one of ten character classes, from fighter to ninja,
and go down through fifty levels of rooms, mazes and throne rooms, past
trading posts and monsters that cast spells and summon others, to bring
back the Amulet of Yendor and the other artifacts.

This is UltraRogue 1.08, which was never released: 1.0.7 as it stood
on August 31, 2006.

What is different from 1.0.7:

- Keys are read through readchar() again, and ^C asks whether to quit.
- The source compiles without warnings: values that may not be set are
  given one, conversions that can lose data are written out, unused
  arguments are marked, and every function is declared before it is
  used.
