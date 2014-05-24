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
		
		table[y][x] = MapType::Block;
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
	if(role == Roles::PoliceClient)
	{
		splitIdXY(s.substr(start,end-start), polices);
		for(int i = 0; i < thiefNum; i++)
		{
			thiefs[i].x = -1;
			thiefs[i].y = -1;
		}
	}
	else 
	{
		splitIdXY(s.substr(start,end-start), thiefs);
		for(int i = 0; i < policeNum; i++)
		{
			polices[i].x = -1;
			polices[i].y = -1;
		}
	}

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

	if(role == Roles::ThiefClient && end-start > 1)
	{
		hasEnemy = true;
	}

	start = s.find('<', end+1)+1;
	end = s.find('>', start);
	splitIdXY(s.substr(start,end-start), thiefs);

	if(role == Roles::ThiefClient && end-start > 1)
	{
		hasEnemy = true;
	}

	//视距范围地图信息
	start = s.find('(', end+1)+1;
	end = s.find(')', start);
	string subStr = s.substr(start, end-start);
	splitXYs(subStr, map);

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
			case Move::Keep:
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
			case Move::Keep:
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
		policeMove();
	}else
	{
		thiefMove();
	}
}


void Game::policeMove()				//计算警察下一步
{
	bool hasThief = hasEnemy;
	int endx;
	int endy;
	for(int i = 0; i < thiefNum; i++)
	{
		if(thiefs[i].x != -1 && thiefs[i].y != -1)
		{
			hasThief = true;
			endx = thiefs[i].x; endy = thiefs[i].y;
			break;
		}
	}
	if(hasThief)
	{
		for(int i = 0; i < policeNum; i++)
		{
			int move = astar(polices[i].x, polices[i].y, endx, endy);
			if(move < 0) 
				polices[i].move = Move::Keep;
			else
				polices[i].move = (Move)move;
		}
		int f = 0;
		int kk = f*2;
	}else
	{
		srand(time(0));
		for(int i = 0; i < polices.size(); i++)
		{
			polices[i].move = (Move)(int)(rand() % 5);
		}
	}

}
void Game::thiefMove()				//计算小偷下一步
{
	srand(time(0));
	for(int i = 0; i < thiefs.size(); i++)
	{
		thiefs[i].move = (Move)(int)(rand() % 5);
	}
}














/***************************
Astar算法的辅助方法
******************************/
void Game::astar_InsertList(list<ANode*> &alist, ANode* node)
{
	list<ANode*>::iterator iter = alist.begin();
	while(iter != alist.end())
	{
		if((*iter)->total >= node->total) break;
		iter++;
	}
	alist.insert(iter, node);

}
ANode* Game::astar_RemoveList(list<ANode*> &alist, int x, int y, int step, ANode* parent)
{
	list<ANode*>::iterator iter = alist.begin();
	ANode* aNode = NULL;
	for(; iter != alist.end(); iter++)
	{
		if((*iter)->x == x && (*iter)->y == y)
		{
			if((*iter)->step > step)	//需要更新
			{
				aNode = (*iter);
				aNode->step = step;
				aNode->total = aNode->step + aNode->remain;
				aNode->parent = parent;

				alist.remove(*iter);
				return aNode;

			}
			return NULL;
		}
	}

	return NULL;

}
void Game::astar_UpdateList(list<ANode*> &alist, int x, int y, int step, ANode* parent)
{
	list<ANode*>::iterator iter = alist.begin();
	ANode* aNode = NULL;
	for(; iter != alist.end(); iter++)
	{
		if((*iter)->x == x && (*iter)->y == y)
		{
			if((*iter)->step > step)	//需要更新
			{
				aNode = (*iter);
				aNode->step = step;
				aNode->total = aNode->step + aNode->remain;
				aNode->parent = parent;

				alist.remove(*iter);
				astar_InsertList(alist, aNode);
				return;
			}
			return;
		}
	}
	return;

}

/***************************
Astar算法 	return int:0=east, 1=south, 2=west, 3=north, 4=keep
******************************/
int Game::astar(int x1, int y1, int x2, int y2)
{
	int movex[] = {1,0,-1,0};
	int movey[] = {0,1,0,-1};

	int startx = x1;
	int starty = y1;
	int endx = x2;
	int endy = y2;

	ANode startNode(x1, y1, 0, abs(x1-x2)+abs(y1-y2));

	vector<vector<int> > marks(mapHeight, vector<int>(mapWidth, 0));	//0=未访问 1=openlist 2=closelist

	list<ANode*> openList;
	list<ANode*> closeList;
	ANode* findNode = NULL;

	openList.push_back(&startNode);
	marks[startNode.y][startNode.x] = 1;
	while(!openList.empty())
	{
		ANode* node = openList.front();
		openList.pop_front();
		closeList.push_back(node);
		marks[node->y][node->x] = 2;

		if(node->x == endx && node->y == endy)
		{
			findNode = node;
			break;
		}

		for(int i = 0; i < 4; i++)
		{
			int newx = node->x + movex[i];
			int newy = node->y + movey[i];
			if(newx >= 0 && newx < mapWidth && newy >= 0 && newy < mapHeight && map[newy][newx] != MapType::Block)
			{
				if(marks[newy][newx] == 0)	//未访问
				{
					ANode* newNode = new ANode(newx, newy, node->step+1, abs(newx-endx)+abs(newy-endy), node);
					astar_InsertList(openList, newNode);
					marks[newy][newx] = 1;
				}else if(marks[newy][newx] == 1)	//openlist
				{
					int newStep = node->step + 1;
					astar_UpdateList(openList, newx, newy, newStep, node);
				}else if(marks[newy][newx] == 2)	//closelist
				{
					int newStep = node->step + 1;
					ANode* newNode = astar_RemoveList(closeList, newx, newy, newStep, node);
					if(newNode != NULL)
					{
						astar_InsertList(openList, newNode);
						marks[newy][newx] = 1;
					}
				}
			}
		}

	}

	if(findNode == NULL)
	{//没找到
		return -1;
	}

	if(findNode->parent != NULL)
		while(findNode->parent->parent != NULL)
			findNode = findNode->parent;
	else
		return 4;	//原地重合

	//0=east, 1=south, 2=west, 3=north, 4=keep
	ANode* parentNode = findNode->parent;
	if(parentNode->x < findNode->x)
		return 0;						
	else if(parentNode->x > findNode->x)
		return 2;
	if(parentNode->y < findNode->y)
		return 1;
	else if(parentNode->y > findNode->y)
		return 3;
	return -1;

}