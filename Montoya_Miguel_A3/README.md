# Assignment 3

Miguel Montoya (montoya5@purdue.edu)

## Libraries used

- [Freeglut](http://freeglut.sourceforge.net/)

## References

- [Vector and sphere intersection](https://math.stackexchange.com/questions/1939423/calculate-if-vector-intersects-sphere)
- [Vector and plane intersection](https://math.stackexchange.com/questions/100439/determine-where-a-vector-will-intersect-a-plane)
- [Materials reference](http://www.it.hiof.no/~borres/j3d/explain/light/p-materials.html)

## What's implemented

Ray tracing with interface for world customization. World contains a sphere resting in a chess board. The sphere has a jade-like material.
GUI includes methods for:

- Changing resolution
- Activate shadows
- Sphere:
  - Modify radius
  - Modify distance from view plane
- Camera:
  - Modify distance from plane (Equivalent to changing fov)
- Lights:
  - Modify diffuse color
  - Modify ambient color
  - Modify specular color
  - Change light type from directional to point
  - Modify direction (Or position if light is a point)