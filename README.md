# Snake
---

This game is a multiplayer snake game. The idea is basically to have a worse slither.io.

I wanted to implement it pretty much from scratch so right now there is a client that works, but a lot of the internals still need to be written.

# Stack
---

For the client desktop application I am using C++ for all of the client side logic and the netoworking. I am using OpenGL for the graphics (it is super simple,
just some 2D graphics)

For the server, I am implementing a custom UDP server in Rust. The game is going to have a reletively simple server, it just needs to manage the players and 
kick people off if they are inactive.
