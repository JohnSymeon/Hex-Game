/*
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

~Ioannis Symeonidis 29/03/2023
*/
#include <iostream>
#include <vector>
#include <iterator>
#include <ctime>
using namespace std;

const int MONTE_CARLO_REPEATS = 1000; //total monte carlo repetitions
class hex_board
{
	public:
		enum State{N=0,R=1,B=2};// the colours of the tiles
	private:
		State **colour_matrix;//contains the colours of every tile on the board
		int size; //the size of the board
		bool **edge_matrix; //2D matrix containg all edges between every node
	
	friend istream& operator>> ( istream& inp,State& state)//overload input operator to take colour Enum
	{
		int i;
		inp>>i;
		state = static_cast<State>(i);
		return inp;
	}
	
	friend ostream& operator<< ( ostream& out,State& state)//overload output operator for the colours
	{
		if(state==R)
			out<<'X';
		else if(state == B)
			out<<'O';
		else
			out<<' ';
		return out;
	}
	
	public:hex_board(int size=4)://Class constructor takes the board size
		size(size){init();};
	
	public:
		bool Player_Turn(State player); //used for the player's turn, returns true if the player wins
		bool AI_Turn(State computer,State player); //same as Player_turn() but for the computer
		void display_board(); //used to conviniently display the board to the screen
	private:
		void init(); //initialises the board, called by the constructor
		bool check_for_victory(State** cm,State& colour); //returns true if the current player wins
		void AI_Plays(State colour,State player); //called by AI_Turn to search and decide for a best play
		bool place_tile(const int &row, const int &col, const State &colour); //returns true if successfuly placed a tile
};

void hex_board::init()
{
	//init matrices dynamically
	int **ID_matrix = new int*[size];//helps creating the edge matrix
	for(int i=0;i<size;i++)
	{
		ID_matrix[i] = new int[size];
		for(int j=0;j<size;j++)
			ID_matrix[i][j] = 0;				
	}
	
	edge_matrix= new bool*[size*size];
	for(int i=0;i<size*size;i++)
	{	
		edge_matrix[i] = new bool[size*size];
		for(int j=0;j<size*size;j++)
			edge_matrix[i][j] = false;				
	}
	
	colour_matrix = new State*[size];
	for(int i=0;i<size;i++)
	{
		colour_matrix[i] = new State[size];
		for(int j=0;j<size;j++)
			colour_matrix[i][j] = N;
	}
	
	//add values of every node according to the algorithm
	//ex 3,5 is n38 , n21 maps to 21/11=1 21%11=10 ->(1,10)
	for(int i=0;i<size;i++)
		for(int j=0;j<size;j++)
			ID_matrix[i][j] = i*size+j;
		
		
	//init edge_matrix between nodes
	for(int i=0;i<size;i++)
		for(int j=0;j<size;j++)
		{
			if(i==0&&j==0)//upper left corner
			{
				edge_matrix[ID_matrix[i][j]][i*size+j+1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j]=true;
			}
			else if(i==size-1 && j==size-1)//lower right corner
			{
				edge_matrix[ID_matrix[i][j]][i*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j]=true;
			}
			else if(i==0 && j == size-1)//upper right corner
			{
				edge_matrix[ID_matrix[i][j]][(i)*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j]=true;
			
			}
			else if(i==size-1 && j ==0)//lower left corner
			{
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j]=true;
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j+1]=true;
				edge_matrix[ID_matrix[i][j]][(i)*size+j+1]=true;
			}
			else if(j==0)//left column
			{	
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j+1]=true;
				edge_matrix[ID_matrix[i][j]][i*size+j+1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j]=true;
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j]=true;
			}
			else if(j==size-1)//right column
			{
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j]=true;
				edge_matrix[ID_matrix[i][j]][(i)*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j]=true;
				
			}
			else if(i==0)//upper row
			{
				edge_matrix[ID_matrix[i][j]][(i)*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i)*size+j+1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j]=true;
			}
			else if(i == size-1)//lower row
			{
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j]=true;
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j+1]=true;
				edge_matrix[ID_matrix[i][j]][(i)*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i)*size+j+1]=true;
			}
			else if(i>0 && j>0 && (i<size-1)&& ( j<size-1))//the rest nodes
			{
				edge_matrix[ID_matrix[i][j]][i*size+j+1]=true;
				edge_matrix[ID_matrix[i][j]][i*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j]=true;
				edge_matrix[ID_matrix[i][j]][(i+1)*size+j-1]=true;
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j]=true;
				edge_matrix[ID_matrix[i][j]][(i-1)*size+j+1]=true;
			}
			
		}			
}

void hex_board::display_board()
{
	
	for(int i=0;i<size;i++)
		cout<<"  "<<i<<" ";
	cout<<endl;
	for(int i=0;i<size;i++)//row
	{
		cout<< i <<" ";
		for(int j=0;j<size;j++)//column
		{
			cout<<colour_matrix[i][j];
			if(j!=size-1)
				cout<<" - ";
		}
		cout<<endl;
		if(i<size-1)
		{
			for(int k=0;k<i+1;k++)
				cout<<"  ";
			cout<<" \\";
			for(int k=0;k<size-1;k++)
				cout<<" / \\";
			cout<<endl;
			for(int k=0;k<i+1;k++)
				cout<<"  ";
		}
	}
}

bool hex_board::place_tile(const int &row, const int &col, const State &colour)
{	
	if(colour_matrix[row][col]!=N)
		return false;//return false if the placed tile is invalid
	else
		colour_matrix[row][col] = colour;
	return true;	
}

bool hex_board::check_for_victory(State** cm,State& colour)
{
	
	vector<bool> Reached(size*size,false);//init reached nodes status as unreached
	for(int j=0;j<size;j++)
	{
		//for all elements of first row if a red exists
		//or first column if blue start checking for reached nodes
		//from the starting position
		if((cm[0][j]==R && colour==R) || 
		(cm[j][0]==B && colour==B) )
		{
			bool found_Reached=true;
			while(found_Reached) //if a new node is found, restart the search
			{
				found_Reached = false;
				//init that as starting node
				//depending on colour
				if(colour==R)
					Reached.at(j) = true;
				else
					Reached.at(j*size) = true;
				
				for(int k=0;k<size*size;k++)
				{ 
					for(int w=0;w<size*size;w++)
					{
						//if a path exists between a reached and an 
						//unreached node and both of them are of
						//same colour, mark the unreached as reached
						if(cm[k/size][k%size]==colour && 
						edge_matrix[k][w] && 
						cm[w/size][w%size]==colour && 
						Reached.at(w) && !Reached.at(k))
						{
							Reached.at(k) = true;
							found_Reached = true;
						}		
					}
				}
			}
		}
	}
	
	if(colour==R)
	{
		for(int k=size*size-size;k<size*size;k++)//check at the last row if a reached Red node exists
		{
			if(Reached.at(k))
			{
				return true;
			}
		}
	}
	else if(colour==B)
	{
		for(int k = size-1;k<size*size;k=k+size)//check last column if a reached Blue node exists
		{
			if(Reached.at(k))
			{
				return true;
			}
		}
	}
	return false;//return false if no victory condition is met
}

void hex_board::AI_Plays(State colour,State player)
{	
	//init played tiles
	vector<bool> Tiles_Played(size*size,false);
	for(int i=0;i<size;i++)
	{
		for(int j=0;j<size;j++)
		{
			if(colour_matrix[i][j]==colour)
			{
				Tiles_Played[i*size+j] = true;
			}
		}
	}
	vector<bool> Tiles_Played_By_Player(size*size,false);
	for(int i=0;i<size;i++)
	{
		for(int j=0;j<size;j++)
		{
			if(colour_matrix[i][j]==player)
			{
				Tiles_Played_By_Player[i*size+j] = true;
			}
		}
	}
	
	//init arrays for Monte Carlo evaluation
	int Victories[size][size];
	for(int i=0;i<size;i++)
		for(int j =0;j<size;j++)
			Victories[i][j]=0;
	
	int Losses[size][size];
	for(int i=0;i<size;i++)
		for(int j=0;j<size;j++)
			Losses[i][j]=0;

	for(int count =0; count<MONTE_CARLO_REPEATS;count++)
	{
		//test board
		State** cm_cpy;
		cm_cpy = new State*[size];
		for(int i=0;i<size;i++)
		{
			cm_cpy[i] = new State[size];
			for(int j=0;j<size;j++)
				cm_cpy[i][j] = N;
		}
		
		//place tiles randomly on the board
		for(int i=0;i<size;i++)
		{
			for(int j=0;j<size;j++)
			{
				if(colour_matrix[i][j]!=N)
					cm_cpy[i][j] = colour_matrix[i][j];
				else
					cm_cpy[i][j]= static_cast <State> (1+ rand()/(RAND_MAX/2));	
			}
		}
		//count the times the computer wins with the random board
		if( check_for_victory(cm_cpy,colour) == true )
		{
			for(int i=0;i<size;i++)
				for(int j =0;j<size;j++)
					if(cm_cpy[i][j]==colour && Tiles_Played[i*size+j]==false)
						Victories[i][j] = Victories[i][j]+1;
		}
		//count the times the player wins with the random board
		if( check_for_victory(cm_cpy,player) == true )
		{
			for(int i=0;i<size;i++)
				for(int j =0;j<size;j++)
					if(cm_cpy[i][j]==player && Tiles_Played_By_Player[i*size+j]==false)
						Losses[i][j] = Losses[i][j]+1;
		}
	
		for(int i = 0; i< size; ++i)//delete the board
		{
			delete[] cm_cpy[i];  
	    }
  		delete[] cm_cpy;
	}
	
	//Implement the evaluation as the play with the maximum Wins to Losses ratio
	//as it is better than Wins to total tests ratio
	int max = 0;
	int ID_temp = 0;
	for(int i=0;i<size;i++)
	{
		for(int j=0;j<size;j++)
		{
			if( (Losses[i][j]>0) && ( Victories[i][j] / Losses[i][j] ) > max )
			{	
				max = Victories[i][j]/Losses[i][j];
				ID_temp = i*size + j;
			}
			else if(Victories[i][j] > max)//case the tile has 0 Losses to avoid dividing by 0
			{
				max = Victories[i][j];
				ID_temp = i*size + j;
			}
		}
	}
	
	cout<<endl<<"Computer plays: Row = "<<ID_temp/size<<" Col = "<<ID_temp%size<<endl<<endl;
	place_tile(ID_temp/size,ID_temp%size,colour);

}

bool hex_board::Player_Turn(State player)
{
	int row;
	int col;
	display_board();
	while(1) //for loop to check the validity of player's inputs
	{
		while(1)
		{
			cout<<endl<<"Insert the row you want to play:"<<endl;
			cin>>row;
			if(row>size-1 || row<0)
				cout<<"Value out of bounds!"<<endl;
			else
				break;
		}
		while(1)
		{
			cout<<"Insert the coloumn you want to play:"<<endl;
			cin>>col;
			if(col>size-1 || col<0)
				cout<<"Value out of bounds!"<<endl;
			else
				break;
		}
		if(place_tile(row,col,player)==false)
			cout<<endl<<"Tile is already occupied!"<<endl;
		else
			break;
	}
	if(check_for_victory(colour_matrix , player))
	{
		cout<<endl<<"PLAYER WON"<<endl<<endl;
		display_board();
		return true;
	}
	return false;//return flase if the player does not win
}

bool hex_board::AI_Turn(State computer,State player)
{
	AI_Plays(computer,player);
	if(check_for_victory(colour_matrix,computer))
	{
		cout<<endl<<"COMPUTER WON"<<endl<<endl;
		display_board();
		return true;
	}
	return false;//return false if the computer does not win
}


inline void PlayHex()
{
	hex_board::State player;
	hex_board::State computer;
	int row;
	int col;
	int input;
	const int size = 11;//the size of the hex board
	cout<<"Choose a colour Red or Blue <1,2>:";
	cin>>player;
	cout<<endl;
	if(player==1)
		computer = static_cast<hex_board::State>(2);
	else
		computer = static_cast<hex_board::State>(1);
	hex_board g1(size);//create a game
	
	if(player == 1)//if player goes first
	{
		while(1)//repeat until somebody wins
		{
			//Player's turn
			if(g1.Player_Turn(player))//user plays and check for player victory
				break;
			cout<<endl<<"Computer is deciding..."<<endl;
			//Computer's turn
			if(g1.AI_Turn(computer, player))//computer plays and check for computer victory
				break;
		}
	}
	else
	{
		while(1)
		{
			cout<<endl<<"Computer is deciding..."<<endl;
			//Computer's turn
			if(g1.AI_Turn(computer, player))
				break;
			//Player's turn
			if(g1.Player_Turn(player))
				break;
		}
	}
	
}

int main(void)
{
	srand(time(0));
	cout<<endl<<"Welcome to HEX! by Symeonidis"<<endl<<endl;
	cout<<"Red wins by connecting North and South\nand Blue wins by connecting East and West."<<endl;
	cout<<"Red always goes first."<<endl<<endl;
	cout<<"Tip:Put me on Full Screen"<<endl<<endl;
	cout<<"Tip 2: If it takes more than a couple of seconds for computer to play,"<<endl;
	cout<<"(it should not in most systems) and you would prefer faster times"<<endl;
	cout<<"just lower the repetitions global variable from the code"<<endl<<endl;
	while(1)
	{
		PlayHex();
		cout<<endl<<endl<< "Game Over, play again? <y,n>";
		char choice;
		cin>> choice;
		cout<<endl;
		if(choice !='y')
			break;
	}
	
}
