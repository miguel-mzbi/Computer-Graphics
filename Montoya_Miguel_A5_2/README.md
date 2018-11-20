# Assignment 5

Miguel Montoya (montoya5@purdue.edu)

## Libraries and references

- [Freeglut](http://freeglut.sourceforge.net/)
- [GLUI](https://github.com/libglui/glui)
- [Convex Hull (Jarvis' algorithm)](https://www.geeksforgeeks.org/convex-hull-set-1-jarviss-algorithm-or-wrapping/)
- [Point in segment given two points](https://stackoverflow.com/questions/328107/how-can-you-determine-a-point-is-between-two-other-points-on-a-line-segment)

## What's implemented

### Part 1

- Adding, deleting and moving control points. A max of 32 control points can be created for the curve.
- Show Bezier curve given a number of control points.
- Show convex hull created by the control points.
- deCasteljou algorithm:
  - By default starts in t = 0.5.
  - Spinner provides functionality to modify the t value of the algorithm.
- Change the length of the step when drawing the curve.

### Part 2

- Two different curve can be drawn in the same window
  - Each curve maintains the selected properties between changes. This means a curve can have the convex hull and deCasteljau activated, and the second one may be only showing the control points.
  - The functionality to lower a degree is deactivated if the selected curve to operate is switched.
- The weight windows shows the weights for the selected curve. The weight of the other curve can't be modified thus won't be shown.
  - When hovering over a weight bar, that control point will change of color from yellow to red.
  - The weight for each control point can be changed pressing left click and moving the bar. Max value is 10 and min value is -5.
- The extended curve now has the following range [-5:0] and [0:6].
- deCasteljau algorithm can be modified via the spinner in the GUI or by moving the point in the segment [P0, P1]. Both modes are active at the same one.