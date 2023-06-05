# Hex-Game
This program implements the hex game with a human player versus 
a computer player. The user chooses the "colour" of their tiles and
then each player plays after the other with the "Red" going first and
the "Blue" second. When the hex board is displayed, X symbolizes the Red
tiles, O the Blue and a blank space the neutrol ones. Red wins by 
connecting North and South rows and Blue wins by connecting East and West
columns. 

The AI player uses a Monte Carlo evaluation to determine the best
available play to win and avoid losing by examining the Victories to Losses
ratio as opposed to simply evaluating the Wins to total Monte Carlo repetitions
for smarter AI play.

The program should be very efficient and the computer player should be 
deciding their move in under two or three seconds at most. In case it runs slow on your machine, 
just lower the global integer of Monte Carlo Repetions.

If your machine can handle it, rather increase the repetitions to get a smarter AI.

~Ioannis Symeonidis 29/03/2023, GitHub: JohnSymeon
