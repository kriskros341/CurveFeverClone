# CurveFeverAlike

This is our final project for second semester. Documentation is in "".

## Details

In this project we focused on mimicking the mechanics used in original web game "CurveFever" in order to create clone of it. It uses SFML and ImGui libraries.
* ImGui is used to create simple menu in which you can switch between different game modes 
* SFML is used to provide graphics to represent mechanics and game itself 
Game is really simple to handle, you only need few keyboard keys.
Original game [CurveFeverPro](https://curvefever.pro/)

## How it works?
<sub>This is simplified description, for more look into documentation</sub>

* Players spawning point: Each player has its own random angle (float type) drawn by lot. It is passed to the constructor of *PositionManager* class and then used in matrix equation to optain random spawn point on the circle perimeter (which radius can be changed inside the code).
* Placing lines: Each player has its own *VertexArray* of pointers. In this place function adds every following point to it, then using funtion *draw*, draws lines using *TriangleStrip* for better performance.
* Players movement: When line navigation by keyboard keys occurs, there is an angle in which the line curves. The line itself is a vector and we change its sense. Value of the angle is incremented every moment we press key or hold it.
* Collision between player and edge: Functions *checkForCllision* and *updateCollisionQueue* iterate while the game occurs, when the distance is too short to the edges of the window first of them return information which is then captured by other functions in order to restart the game. They count the distance using pointers.
* Collision between players: There is a vector array of pointers with atributes from *Player* class and for loop with another for loop inside it, checking through whole gameplay if any collision occured between *p* and *q* points in above-mentioned array.
