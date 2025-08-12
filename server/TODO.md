- Need to change where the game_state property is stored. It may have to be external to the Game object
  - This is because, in order to run the sender and listener functions in seperate threads, they need to be
    static functions.
  - This means that they cannot access non-static member variables or function.
- Come up with a way to generate ids
