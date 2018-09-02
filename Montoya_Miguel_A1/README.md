# Assignment 1"

Miguel Montoya (montoya5@purdue.edu)

## Libraries and headers used

- [Freeglut](http://freeglut.sourceforge.net/)
- Simple Polynomials in K[x,y] by [Christoph M. Hoffmann](https://www.cs.purdue.edu/homes/cmh/)

## What's implemented

- Menu as stated in the HW description.
  - Has one extra button called "Draw scalar field", which purpose is to draw each point of the scalar field.
- Multiple sample polynomials for the user to work with. By default, when opening the program, the circle polynomial is selected.
- User entry so he can write its own polynomial. The user can do every program's function as if it was a sample polynomial.
- The user can select a different N value. By default, the programs starts with N = 20.
- When user clicks clear, windows it's completely cleared, and the axis are redrawn.
- When user clicks refresh, the points and lines will be recalculated, according to the latest selected polynomial and N value.
- If the user clicks exit, the program finishes.
- If the user resizes the windows, the content will preserve its aspect ratio (1:1).
- If the user clicks evaluate all, the program will evaluate every point of the scalar field and then will draw the corresponding line inside each box. The process is as follows:
  1. Algorithm evaluates each point on the scalar field
  2. Iteration trough boxes, drawing the line inside each box
  3. Evaluations are stored in case of resizing.
- If the user selects find curve, the program will prompt the user for a valid line segment. If the line intersects with the curve, only the boxes that contain the curve will be evaluated and their lines displayed. (NOTE: If selected N-value is bigger than 25, for complex polynomials, process might take a few seconds.). The algorithm works as follows:
  1. Algorithm prompts user to select two points. If those points create a segment that intersect the curve (Have opposite signs in the scalar field), the algorithm proceed. Else, will prompt the user for a new second coordinate.
  2. Using binary search, the algorithm selects midpoints in the segment to find the exact box of intersection.
  3. The algorithm does DFS to find all the boxes of the curve.
  4. The curve is displayed.