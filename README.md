# ShipGame
ShipGame is a game designed for two players, where one player is the captain of the ship (using an LCD screen) and the other is the hunter (using a COM terminal, e.g., Tera Term). The captain’s task is to hide from incoming missiles. At the beginning of the game, the ship’s captain chooses a convenient position by pressing buttons and then passes (presses Select) the turn to the other player. From this point on, each player has a limited number of missiles/position changes to three. The hunter chooses a position to fire at by entering the command shoot x y, where x and y are coordinates. After sinking the ship, the players switch positions and now the hunter is the ship’s captain. The game is won by the player who destroys the ship with the fewest shots.

Tera Term Settings:
- Serial Port Speed: 115200
- Data: 8 bit
- Parity: none
- Stop bits: 1 bit
- Flow control: none 

Used Items:
-	LCD Keypad Shield,
-	NUCLEO-L476RG.

To play the game you might need to change voltage for each button in main.c:
The base used voltage
- double VSelect = 2.6;
- double VLeft = 1.75;
- double VDown = 1.1;
- double VUp = 0.4;
- double VRight = 0;
