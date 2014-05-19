#include "stdafx.h"
#include "Game.h"

void Game::setRole(string str)
{
	if(str == "POL")
	{
		role = Roles::PoliceClient;
	}else
	{
		role = Roles::ThiefClient;
	}
}

void Game::splitXY(string str, int &x, int &y)
{
	int start = 0;
	int middle = str.find(',');
	int end = str.length();

	x = stoi(str.substr(start, middle));
	y = stoi(str.substr(middle+1, end));
}

void Game::splitXYs(string str, vector<vector<int> > &table)
{
	int start = 0;
	int end = 0;
	while(start < str.length())
	{
		end = str.find(',', start);
		int x = stoi(str.substr(start, end-start));
		start = end + 1;

		end = str.find(';', start);
		int y = stoi(str.substr(start, end-start));
		start = end + 1;
		
		table[x][y] = MapType::Block;
	}
}

void Game::splitIdXY(string str, vector<Node> &list)
{
	int start = 0;
	int end = 0;
	while(start < str.length())
	{
		end = str.find(',', start);
		int id = stoi(str.substr(start, end-start));
		start = end + 1;

		end = str.find(',', start);
		int x = stoi(str.substr(start, end-start));
		start = end + 1;

		end = str.find(';', start);
		int y = stoi(str.substr(start, end-start));
		start = end + 1;

		list[id].x = x;
		list[id].y = y;
	}
}

void Game::analyzeINI(string s)
{
	//string s(str);

	int start = s.find('[')+1;
	int end = s.find(']', start);
	splitXY(s.substr(start,end-start), mapWidth, mapHeight);

	map.resize(mapHeight, vector<int>(mapWidth,MapType::Unknown));

	start = s.find('<', end+1)+1;
	end = s.find('>', start);
	splitXY(s.substr(start,end-start), policeRange, thiefRange);

	start = s.find('(', end+1)+1;
	end = s.find(')', start);
	splitXY(s.substr(start,end-start), policeNum, thiefNum);

	polices.resize(policeNum);
	thiefs.resize(thiefNum);

	start = s.find('<', end+1)+1;
	end = s.find('>', start);
	if(role == Roles::PoliceClient) splitIdXY(s.substr(start,end-start), polices);
	else splitIdXY(s.substr(start,end-start), thiefs);

}
void Game::analyzeINF(string s)
{
	//string s(str);

	int start = s.find('[')+1;
	int end = s.find(']', start);
	step = stoi(s.substr(start,end-start));

	start = s.find('(', end+1)+1;
	end = s.find(')', start);
	splitIdXY(s.substr(start,end-start), polices);

	start = s.find('<', end+1)+1;
	end = s.find('>', start);
	splitIdXY(s.substr(start,end-start), thiefs);

	start = s.find('(', end+1)+1;
	end = s.find(')', start);
	string subStr = s.substr(start, end-start);

}
string Game::buildMOV()
{
	stringstream ss;
	ss<<"MOV["<<step<<"](";

	if(role == Roles::PoliceClient)
	{
		for(int i = 0; i < polices.size(); i++)
		{
			ss<<i<<",";
			switch(polices[i].move)
			{
			case Move::East:
				ss<<"D"; break;
			case Move::South:
				ss<<"N"; break;
			case Move::West:
				ss<<"X"; break;
			case Move::North:
				ss<<"B"; break;
			default:
				ss<<"T"; break;
			}
			ss<<";";
		}
	}else
	{
		for(int i = 0; i < thiefs.size(); i++)
		{
			ss<<i<<",";
			switch(thiefs[i].move)
			{
			case Move::East:
				ss<<"D"; break;
			case Move::South:
				ss<<"N"; break;
			case Move::West:
				ss<<"X"; break;
			case Move::North:
				ss<<"B"; break;
			default:
				ss<<"T"; break;
			}
			ss<<";";
		}
	}
	ss<<")";
	string str = ss.str();
	return str;
}



void Game::receiveINI(char* str)
{
	string s(str);
	analyzeINI(s);
}
void Game::receiveINF(char* str)
{
	string s(str);
	analyzeINF(s);
	computeMove();
}
void Game::sendMOV()
{
	string str = buildMOV();
	char* s = const_cast<char*>(str.c_str());
	sendMoveToServer(s);
}


/***********************************
核心的智能算法   PS:目前是随机移动
************************************/
void Game::computeMove()
{
	srand(time(0));
	if(role == Roles::PoliceClient)
	{
		for(int i = 0; i < polices.size(); i++)
		{
			polices[i].move = (Move)(int)(rand() % 5);
		}
	}else
	{
		for(int i = 0; i < thiefs.size(); i++)
		{
			thiefs[i].move = (Move)(int)(rand() % 5);
		}
	}
}