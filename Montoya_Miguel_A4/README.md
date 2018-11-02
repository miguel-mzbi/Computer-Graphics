# Assignment 4

Miguel Montoya (montoya5@purdue.edu)

## Libraries used

- [Freeglut](http://freeglut.sourceforge.net/)
- [GLUI](https://github.com/libglui/glui)

## What's implemented

Game puzzle with 24 pieces.

- Adjustable timer
  - If timer ends and user didn't found a solution, a solution is displayed.
  - If the user solves the puzzle before, a message displaying the victory of the user appears.
  - Either way, the puzzle will be blocked until the exit or start over button is selected.
  - The timer is blocked until restart.
- Start Over button
  - Resets the puzzle
  - Enables the timer for more adjustments before starting game again.
- EXIT button.