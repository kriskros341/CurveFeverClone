#CurveFeverAlike

This is our final project for second semester. Documentation is in "".

##Details

In this project we focused on mimicking the mechanics used in original web game "CurveFever" in order to create clone of it. It uses SFML and ImGui libraries.
*ImGui is used to create simple menu in which you can switch between different game modes 
*SFML is used to provide graphics to represent mechanics and game itself 
Game is really simple to handle, you only need few keyboard keys.

##How it works?

*Players spawn:
*Placing lines: Each player has its own VertexArray of pointers. In this place function adds every following point to it, drawing it using TriangleStrip for better performance.
*Players movement:
*Collision between players: 
