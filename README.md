# Snake
---

This game is a multiplayer snake game. The idea is basically to have a worse slither.io.

I wanted to implement it pretty much from scratch so right now there is a client that works, but a lot of the internals still need to be written.

# Stack
---

For the client desktop application I am using C++ for all of the client side logic and the networking. I am using OpenGL for the graphics (it is super simple,
just some 2D graphics)

For the server, I am implementing a custom UDP server in Rust. The game is going to have a relatively simple server, it just needs to manage the players and 
kick people off if they are inactive.

# Installation
---

Eventually I will have zip a binary and put it on github but for now you have to build what there is of this project from source.

Disclaimer: I only test on Ubuntu, I can not guarentee that this will work on any other platform.

# Dependencies
---

Building this project requires the following dependencies:
1. GLFW
2. GLAD
3. CJSON

CJSON is a custom JSON serialization and deserialization library written in C. Follow these instructions add it as a dependency.
1. Clone [the cjson repo](https://github.com/jsypal04/cjson)
2. Run `mkdir build`
3. Run `./build.sh -clean`
4. The files you will need are `bin/libcjson.a` and `src/cjson.h`

# Building from Source
---

Follow these steps to build snake from source.

1. Clone this repo.
2. Copy `libcjson.a` to the `lib` directory
3. Copy `cjson.h` to `.`
4. Run ./start.sh (this should start the client)

To run the server navigate to the server dir and run `cargo run`.

# Roadmap
---

- Change apple location to be managed by the server and not the client
- Test multiple clients connecting to the server
    - Specifically make sure clients are disconnected properly
- Implement the client's listener thread
- Iron out issues with the broadcast functionality of the server
- Add different colors for different players
- Implement some level of security so that only real clients can hit the server
- Limit the number of players per game
- Support multiple games
    - I suspect that I will need to make the server multithreaded
