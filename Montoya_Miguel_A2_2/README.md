# Assignment 2

Miguel Montoya (montoya5@purdue.edu)

## Libraries and headers used

- [Freeglut](http://freeglut.sourceforge.net/)

## What's implemented (Part 2)

- Menu as stated in HW description.
- Clear button that clear everything in the screen (And resets all variables to their initial value).
- Refresh button that redraws the graphics.
- Display axis button that draws the x, y, and z axis in the window.
- Rotate world button. It lets the user rotate the teapot according to a point. Is implemented as follows:
    1. The user clicks a point.
    2. The axis of rotation is calculated in the XY plane. The vector is perpendicular to the line from the point to the origin. The axis is located at the world origin.
    3. The world is rotated proportionally to the perpendicular distance of the mouse to the vector of rotation.
    4. The rotation is stored as a quaternion after the user releases the mouse.
    5. From this point forward, the figure will conserve the rotation, and future rotations will be multiplied with the current quaternion for storage.
- Rotate the teapot button. It lets the user rotate the teapot according to a point. Is implemented as follows:
    1. The user clicks a point.
    2. The axis of rotation is calculated in the XY plane. The vector is perpendicular to the line from the point to the origin. The axis is located at the teapot origin.
    3. The teapot is rotated proportionally to the perpendicular distance of the mouse to the vector of rotation.
    4. The rotation is stored as a quaternion after the user releases the mouse.
    5. From this point forward, the figure will conserve the rotation, and future rotations will be multiplied with the current quaternion for storage.
- World shifting at the XY plane using shift key
- World shifting at the YZ plane using alt key
- FOV change using ctrl key
- AUTO ROTATE WASN'T IMPLEMENTED