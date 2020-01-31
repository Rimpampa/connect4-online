# Connect 4
Connect Four is a two-player connection game in which the players first choose a color and then take turns dropping one colored disc from the top into a seven-column, six-row vertically suspended grid. The pieces fall straight down, occupying the lowest available space within the column. The objective of the game is to be the first to form a horizontal, vertical, or diagonal line of four of one's own discs.

Source: [Wikipedia](https://en.wikipedia.org/wiki/Connect_Four)

# How it works
## The client:
When the program starts the user will be asked to insert the IP address of the server to which he wants to connect.
After a successful connection the program will wait until another player connects to the same server.

Once the other player connects, the game starts and on each turn the user will type in the number of the column on which he wants to put his disc.

When the game ends, either because someone wins, the board gets filled (which is a draw) or the other player quits the game, the program will close automatically.

## The server:
All you have to do is start the program and the users will be able to play on it.

As this project is intended for casual use, there is no need to host it on a static IP address.
So if you want to play with others outside your network you must allow port mapping on your home router (the service port is 4730)
