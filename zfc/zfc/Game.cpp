#include "stdafx.h"
#include "Game.h"

//===========================================================================
///	设置客户端角色
//===========================================================================
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

//===========================================================================
///	接收INI指令
//===========================================================================
void Game::receiveINI(char* str)
{
	string s(str);
	analyzeINI(s);
}

//===========================================================================
///	接收INF指令
//===========================================================================
void Game::receiveINF(char* str)
{
	string s(str);
	analyzeINF(s);
	computeMove();
}

//===========================================================================
///	发送MOV指令
//===========================================================================
void Game::sendMOV()
{
	string str = buildMOV();
	char* s = const_cast<char*>(str.c_str());
	sendMoveToServer(s);
}


//===========================================================================
///	分割"X,Y"型字符串
//===========================================================================
void Game::splitXY(string str, int &outx, int &outy)
{
	int start = 0;
	int middle = str.find(',');
	int end = str.length();

	outx = stoi(str.substr(start, middle));
	outy = stoi(str.substr(middle+1, end));
}

//===========================================================================
///	分割"X,Y;X,Y;X,Y;..."型字符串
//===========================================================================
void Game::splitXYs(string str, vector<vector<MapType> > &outTable)
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
		
		outTable[y][x] = MapType::Block;
	}
}

//===========================================================================
///	分割"Id,X,Y;Id,X,Y;Id,X,Y;..."型字符串
//===========================================================================
void Game::splitIdXY(string str, vector<Node> &outList)
{
	int start = 0;
	int end = 0;
	for(int i = 0; i < outList.size(); i++)
	{
		outList[i].visible = false;
	}

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

		outList[id].x = x;
		outList[id].y = y;
		outList[id].visible = true;
	}
}

//===========================================================================
///	分析INI指令
//===========================================================================
void Game::analyzeINI(string s)
{
	int start = s.find('[')+1;
	int end = s.find(']', start);
	splitXY(s.substr(start,end-start), mapWidth, mapHeight);

	map.resize(mapHeight, vector<MapType>(mapWidth,MapType::Unknown));

	start = s.find('<', end+1)+1;
	end = s.find('>', start);
	splitXY(s.substr(start,end-start), policeRange, thiefRange);

	start = s.find('(', end+1)+1;
	end = s.find(')', start);
	int policeNum = 0;
	int thiefNum = 0;
	splitXY(s.substr(start,end-start), policeNum, thiefNum);

	polices.resize(policeNum);
	for(int i = 0; i < policeNum; i++)
	{
		polices[i].x = -1;
		polices[i].y = -1;
		polices[i].move = Move::Keep;
		polices[i].visible = false;

		polices[i].scanToPreX = -1;
		polices[i].scanToPreY = -1;
		polices[i].scanToX = -1;
		polices[i].scanToY = -1;
		polices[i].directHoriz = false;
		if(mapHeight >= mapWidth) polices[i].isHorizScan = true;
		else polices[i].isHorizScan = false;
	}
	thiefs.resize(thiefNum);
	for(int i = 0; i < thiefNum; i++)
	{
		thiefs[i].x = -1;
		thiefs[i].y = -1;
		thiefs[i].move = Move::Keep;
		thiefs[i].visible = false;

		thiefs[i].scanToPreX = -1;
		thiefs[i].scanToPreY = -1;
		thiefs[i].scanToX = -1;
		thiefs[i].scanToY = -1;
		thiefs[i].directHoriz = false;
		if(mapHeight >= mapWidth) polices[i].isHorizScan = true;
		else polices[i].isHorizScan = false;
	}

	start = s.find('<', end+1)+1;
	end = s.find('>', start);
	if(role == Roles::PoliceClient)
	{
		splitIdXY(s.substr(start,end-start), polices);
	}
	else 
	{
		splitIdXY(s.substr(start,end-start), thiefs);
	}

	if(role == Roles::PoliceClient)
		calcScanLine();
}

//===========================================================================
///	分析INF指令
//===========================================================================
void Game::analyzeINF(string s)
{
	for(int i = 0; i < polices.size(); i++)
	{
		polices[i].visible = false;
	}
	for(int i = 0; i < thiefs.size(); i++)
	{
		thiefs[i].visible = false;
	}


	int start = s.find('[')+1;
	int end = s.find(']', start);
	step = stoi(s.substr(start,end-start));

	start = s.find('(', end+1)+1;
	end = s.find(')', start);
	splitIdXY(s.substr(start,end-start), polices);

	if(role == Roles::ThiefClient)
	{
		if(end-start > 0)
			hasEnemy = true;
		else
			hasEnemy = false;
	}

	start = s.find('<', end+1)+1;
	end = s.find('>', start);
	splitIdXY(s.substr(start,end-start), thiefs);

	if(role == Roles::PoliceClient)
	{
		if(end-start > 0)
		{
			if(hasEnemy)
			{
				if(lockedThief >= 0 && lockedThief < thiefs.size() && thiefs[lockedThief].visible)
				{

				}else
				{
					lockedThief = -1;
					lockedPolice = -1;
				}
			}
			hasEnemy = true;
		}
		else
		{
			hasEnemy = false;
			lockedThief = -1;
			lockedPolice = -1;
		}
	}

	//视距范围地图信息
	start = s.find('(', end+1)+1;
	end = s.find(')', start);
	string subStr = s.substr(start, end-start);
	splitXYs(subStr, map);

}

//===========================================================================
///	构建MOV指令
//===========================================================================
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

//===========================================================================
///	计算移动
//===========================================================================
void Game::computeMove()
{
	if(role == Roles::PoliceClient)
	{
		policeMove();
	}else
	{
		thiefMove();
	}
}

//===========================================================================
///	计算警察移动
//===========================================================================
void Game::policeMove()		
{
	if(hasEnemy)
	{
		policePursue();
	}else
	{
		policeScan();
	}

}

//===========================================================================
///	计算小偷移动
//===========================================================================
void Game::thiefMove()				
{

	if(hasEnemy)
	{
		thiefEscape();
	}else
	{
		thiefScan();
	}

}

//===========================================================================
///	计算警察扫描线 (横向与纵向)
//===========================================================================
void Game::calcScanLine()
{
	int rangeWidth = (2*policeRange+1);
	int bandWidth = polices.size() * rangeWidth;


	int lineNum = mapHeight / bandWidth * 2;
	if(lineNum == 0) lineNum = 2;

	scanHorLine.resize(polices.size(), vector<int>(lineNum,0));
	scanHorIndex.resize(polices.size(),0);

	for(int i = 0; i < scanHorLine.size(); i++)
	{
		for(int j = 0; j < scanHorLine[0].size() / 2; j++)
		{//正向线
			scanHorLine[i][j] = j * bandWidth + i * rangeWidth + policeRange;
			if(scanHorLine[i][j] >= mapHeight)
			{
				int more = scanHorLine[i][j] - mapHeight;
				scanHorLine[i][j] = mapHeight - 1 - more;
			}
		}
		for(int j = scanHorLine[0].size() / 2; j < scanHorLine[0].size(); j++)
		{//负向线
			scanHorLine[i][j] = (mapHeight-(j-lineNum/2)*bandWidth  -1) - (i * rangeWidth + policeRange);
			if(scanHorLine[i][j] < 0)
			{
				scanHorLine[i][j] = -scanHorLine[i][j];
			}

		}
	}

	lineNum = mapWidth / bandWidth * 2;
	if(lineNum == 0) lineNum = 2;

	scanVerLine.resize(polices.size(), vector<int>(lineNum,0));
	scanVerIndex.resize(polices.size(),0);

	for(int i = 0; i < scanVerLine.size(); i++)
	{
		for(int j = 0; j < scanVerLine[0].size() / 2; j++)
		{//正向线
			scanVerLine[i][j] = j * bandWidth + i * rangeWidth + policeRange;
			if(scanVerLine[i][j] >= mapWidth)
			{
				int more = scanVerLine[i][j] - mapWidth;
				scanVerLine[i][j] = mapWidth - 1 - more;
			}
		}
		for(int j = scanVerLine[0].size() / 2; j < scanVerLine[0].size(); j++)
		{//负向线
			scanVerLine[i][j] = (mapWidth-(j-lineNum/2)*bandWidth  -1) - (i * rangeWidth + policeRange);
			if(scanVerLine[i][j] < 0)
			{
				scanVerLine[i][j] = -scanVerLine[i][j];
			}
		}
	}

}


//===========================================================================
///	警察扫描算法
//===========================================================================
void Game::policeScan()
{
	int rangeWidth = (2*policeRange+1);
	int bandWidth = polices.size() * rangeWidth;
	int maxVerBand = mapWidth / bandWidth;
	int maxHorBand = mapHeight / bandWidth;


	for(int i = 0; i < polices.size(); i++)
	{
		if(polices[i].isHorizScan)
		{
			if(polices[i].scanToPreX == -1 || polices[i].scanToPreY == -1)
			{//初始时
				scanHorIndex[i] = polices[i].y / bandWidth;
				if(polices[i].y >= mapHeight / 2)
				{
					if(scanHorIndex[i] > scanHorLine[0].size()/2)
						scanHorIndex[i] = scanHorLine[0].size()-1 - scanHorIndex[i];
				}

				polices[i].scanToPreX = polices[i].x;
				polices[i].scanToPreY = scanHorLine[i][scanHorIndex[i]];
				findValidPositionAroundXY(polices[i].scanToPreX, polices[i].scanToPreY, polices[i].scanToX, polices[i].scanToY);

				polices[i].directHoriz = false;

				if(polices[i].scanToY == polices[i].y)
				{//若当前就在这个位置，则重新计算位置  水平
					if(polices[i].x < mapWidth-polices[i].x)
					{
						polices[i].scanToPreX = mapWidth-1-policeRange;
					}else
					{
						polices[i].scanToPreX = policeRange;
					}
					findValidPositionAroundXY(polices[i].scanToPreX, polices[i].scanToPreY, polices[i].scanToX, polices[i].scanToY);
					polices[i].directHoriz = true;
				}
			}

			if(map[polices[i].scanToY][polices[i].scanToX] == MapType::Block)
			{//若目标之前为未知区域，现在为不可访问（block），则重新搜索周边空白区域
				findValidPositionAroundXY(polices[i].scanToPreX, polices[i].scanToPreY, polices[i].scanToX, polices[i].scanToY);
			}
			astar(polices[i].x, polices[i].y, polices[i].scanToX, polices[i].scanToY, polices[i].move);	//计算

			if(abs(polices[i].x-polices[i].scanToX) + abs(polices[i].y-polices[i].scanToY) <= 1)
			{//完成
				polices[i].directHoriz = !(polices[i].directHoriz);
				if(polices[i].directHoriz)
				{
					polices[i].scanToPreX = mapWidth-1-polices[i].scanToPreX;
				}else
				{
					scanHorIndex[i]++;
					if(scanHorIndex[i] == scanHorLine[0].size()/2 || scanHorIndex[i] >= scanHorLine[0].size())
					{
						polices[i].isHorizScan = !(polices[i].isHorizScan);
						polices[i].scanToPreX = -1;
						polices[i].scanToPreY = -1;
					}else
					{
						scanHorIndex[i] %= 2*maxHorBand;
						polices[i].scanToPreY = scanHorLine[i][scanHorIndex[i]];
					}


				}
				findValidPositionAroundXY(polices[i].scanToPreX, polices[i].scanToPreY, polices[i].scanToX, polices[i].scanToY);
			}
		}else
		{
			if(polices[i].scanToPreX == -1 || polices[i].scanToPreY == -1)
			{//初始时
				scanVerIndex[i] = polices[i].x / bandWidth;
				if(polices[i].x >= mapWidth / 2)
				{
					if(scanVerIndex[i] > scanVerLine[0].size()/2)
						scanVerIndex[i] = scanVerLine.size() - scanVerIndex[i];
				}
				polices[i].scanToPreX = scanVerLine[i][scanVerIndex[i]];
				polices[i].scanToPreY = polices[i].y;
				findValidPositionAroundXY(polices[i].scanToPreX, polices[i].scanToPreY, polices[i].scanToX, polices[i].scanToY);

				polices[i].directHoriz = true;

				if(polices[i].scanToX == polices[i].x)
				{//若当前就在这个位置，则重新计算位置  垂直
					if(polices[i].y < mapHeight-polices[i].y)
					{
						polices[i].scanToPreY = mapHeight-1-policeRange;
					}else
					{
						polices[i].scanToPreY = policeRange;
					}
					findValidPositionAroundXY(polices[i].scanToPreX, polices[i].scanToPreY, polices[i].scanToX, polices[i].scanToY);
					polices[i].directHoriz = false;
				}
			}

			if(map[polices[i].scanToY][polices[i].scanToX] == MapType::Block)
			{//若目标之前为未知区域，现在为不可访问（block），则重新搜索周边空白区域
				findValidPositionAroundXY(polices[i].scanToPreX, polices[i].scanToPreY, polices[i].scanToX, polices[i].scanToY);
			}
			astar(polices[i].x, polices[i].y, polices[i].scanToX, polices[i].scanToY, polices[i].move);	//计算

			if(abs(polices[i].x-polices[i].scanToX) + abs(polices[i].y-polices[i].scanToY) <= 1)
			{//完成
				polices[i].directHoriz = !(polices[i].directHoriz);
				if(polices[i].directHoriz)
				{
					scanVerIndex[i]++;
					if(scanVerIndex[i] == scanVerLine.size()/2 || scanVerIndex[i] >= scanVerLine.size())
					{
						polices[i].isHorizScan = !(polices[i].isHorizScan);
						polices[i].scanToPreX = -1;
						polices[i].scanToPreY = -1;
					}else
					{
						scanVerIndex[i] %= 2*maxVerBand;
						polices[i].scanToPreX = scanVerLine[i][scanVerIndex[i]];
					}
				}else
				{
					polices[i].scanToPreY = mapHeight-1-polices[i].scanToPreY;

				}
				findValidPositionAroundXY(polices[i].scanToPreX, polices[i].scanToPreY, polices[i].scanToX, polices[i].scanToY);
			}

		}
	}
}

//===========================================================================
///	警察追捕算法
//===========================================================================
void Game::policePursue()
{
	if(lockedThief == -1)
	{
		for(int i = 0; i < thiefs.size(); i++)
		{
			if(thiefs[i].visible)
			{
				lockedThief = i;
				break;
			}
		}
	}
	if(lockedPolice = -1)
	{
		int minDist = INT_MAX;
		lockedPolice = 0;
		for(int i = 0; i < polices.size(); i++)
		{
			int dist = abs(polices[i].x - thiefs[lockedThief].x) +abs(polices[i].y - thiefs[lockedThief].y);
			if(dist < minDist) 
			{
				minDist = dist;
				lockedPolice = i;
			}
		}
	}


	vector<Move> allPolices(polices.size(), Move::Keep);

	vector<bool> policesMark(polices.size(), false);
	vector<bool> directsMark(4, false);

	int lockedMove = 0;
	double minValue = INT_MAX;
	for(int i = 0; i < 4; i++)
	{
		double value = calcRadian(polices[lockedPolice].x, polices[lockedPolice].y, thiefs[lockedThief].x, thiefs[lockedThief].y, (Move)i);
		if(value < minValue)
		{
			lockedMove = i;
			minValue = value;
		}
	}
	allPolices[lockedPolice] = (Move)lockedMove;
	policesMark[lockedPolice] = true;
	directsMark[lockedMove] = true;


	priority_queue<AngleNode, vector<AngleNode>, AngleNodeCmp> priQueue;
	for(int i = 0; i < polices.size(); i++)
	{
		if(lockedPolice == i) continue;
		for(int j = 0; j < 4; j++)
		{
			double value = calcRadian(polices[i].x, polices[i].y, thiefs[lockedThief].x, thiefs[lockedThief].y, (Move)j);
			AngleNode tempNode(i, (Move)j, value);
			priQueue.push(tempNode);
		}
	}

	bool policeBeacon = false;
	bool directBeacon = false;
	int policeEmptyNum = polices.size();
	int directEmptyNum = 4;

	while(!priQueue.empty())
	{
		AngleNode tempNode = priQueue.top();
		priQueue.pop();
		if(policesMark[tempNode.index] == policeBeacon && directsMark[tempNode.direct] == directBeacon)
		{
			allPolices[tempNode.index] = (Move)tempNode.direct;
			policeEmptyNum--;
			directEmptyNum--;
			if(policeEmptyNum == 0)
			{
				policeEmptyNum = polices.size();
				policeBeacon = !policeBeacon;
			}
			if(directEmptyNum == 0)
			{
				directEmptyNum = 4;
				directBeacon = !directBeacon;
			}
		}
	}


	for(int i = 0; i < polices.size(); i++)
	{
		if(i == lockedPolice) 
			astar(polices[i].x, polices[i].y, thiefs[lockedThief].x, thiefs[lockedThief].y, polices[i].move);
		else
		{
			int tox, toy;
			if(findValidNeighborByDirect(thiefs[lockedThief].x, thiefs[lockedThief].y, allPolices[i],tox, toy))
				astar(polices[i].x, polices[i].y, tox,toy, polices[i].move);
			else
				polices[i].move = Move::Keep;
		}
	}

}

//===========================================================================
///	小偷扫描算法
//===========================================================================
void Game::thiefScan()
{
	srand(time(0));
	for(int i = 0; i < thiefs.size(); i++)
	{
		thiefs[i].move = (Move)(int)(rand() % 5);
	}
}

//===========================================================================
///	小偷逃脱算法
//===========================================================================
void Game::thiefEscape()
{
	Node sym;
	sym.x = 0;
	sym.y = 0;
	for(int i = 0; i < thiefs.size(); i++)
	{
		for(int j = 0; j < polices.size(); j++)
		{
			sym.x += polices[i].x;
			sym.y += polices[i].y;
		}
		sym.x /= polices.size();
		sym.y /= polices.size();
		sym.x = 2 * thiefs[i].x - sym.x;
		sym.y = 2 * thiefs[i].y - sym.y;
		findValidPositionAroundXY(sym.x, sym.y, sym.x, sym.y);			//若在不合理位置，则移动到合理位置
		if(sym.x == thiefs[i].x && sym.y == thiefs[i].y)
		{

		}
		astar(thiefs[i].x, thiefs[i].y, sym.x, sym.y, polices[i].move);
	}
}

//===========================================================================
///	计算向量夹角：向量(center->xy) 与 向量(move)的夹角
//===========================================================================
double Game::calcRadian(int x, int y, int centerx, int centery, Move move)
{
	int movex[] = {1,0,-1,0};
	int movey[] = {0,1,0,-1};

	int vx1 = x - centerx;
	int vy1 = y - centery;
	int vx2 = movex[move];
	int vy2 = movey[move];

	double cosRadian = (vx1*vx2+vy1*vy2)/(sqrt(vx1*vx1+vy1*vy1)*sqrt(vx2*vx2+vy2*vy2));
	double radian = acos(cosRadian);  //弧度
	return radian;
}

//===========================================================================
///	搜索(x,y)及其周围最近的有效位置，赋值给tox,toy
//===========================================================================
void Game::findValidPositionAroundXY(int x, int y, int& outx, int& outy, bool isThief)
{
	if(x >= 0 && x < mapWidth && y >= 0 && y < mapHeight && map[y][x] != MapType::Block)
	{
		bool isPoliceLoc = false;
		if(isThief)
		{
			for(int i = 0; i < polices.size(); i++)
			{
				if(x == polices[i].x && y == polices[i].y)
				{
					isPoliceLoc = true;
					break;
				}
			}
		}

		if(!isPoliceLoc)
		{
			outx = x;
			outy = y;
			return;
		}
	}

	int movex[] = {1,0,-1,0};
	int movey[] = {0,1,0,-1};

	if(x < 0) x = 0;
	if(x >= mapWidth) x = mapWidth-1;
	if(y < 0) y = 0;
	if(y >= mapHeight) y = mapWidth-1;

	vector<vector<bool> >  marks(mapHeight, vector<bool>(mapWidth, false));
	queue<pair<int,int> > queue;
	queue.push(make_pair(x,y));
	marks[y][x] = true;
	while(!queue.empty())
	{
		pair<int,int> node = queue.front();
		queue.pop();

		if(map[node.second][node.first] != MapType::Block)
		{
			bool isPoliceLoc = false;
			if(isThief)
			{//若是小偷找路径，则警察所在位置也不可通过
				for(int k = 0; k < polices.size(); k++)
				{
					if(node.first == polices[k].x && node.second == polices[k].y)
					{
						isPoliceLoc = true;
						break;
					}
				}
			}
			if(!isPoliceLoc)
			{
				outx = node.first;
				outy = node.second;
				return;
			}
		}


		for(int i = 0; i < 4; i++)
		{
			int newx = node.first + movex[i];
			int newy = node.second + movey[i];
			if(newx >= 0 && newx < mapWidth && newy >= 0 && newy < mapHeight && marks[newy][newx] == false)
			{
				queue.push(make_pair(newx, newy));
				marks[newy][newx] = true;
			}
		}
	}

}

//===========================================================================
///	搜索(x,y)在指定方向的一步可达位置，若不存在，则顺时针换方向寻找
//===========================================================================
bool Game::findValidNeighborByDirect(int x, int y, Move direct, int& outx, int& outy, bool isRecursion)
{
	if(direct == Move::Keep)
	{
		outx = x;
		outy = y;
		return true;
	}
	int movex[] = {1,0,-1,0};
	int movey[] = {0,1,0,-1};
	int newx = x + movex[direct];
	int newy = y + movey[direct];

	if(newx >= 0 && newx < mapWidth && newy >= 0 && newy < mapHeight && map[newy][newx] != MapType::Block)
	{
		outx = newx;
		outy = newy;
		return true;
	}

	if(newx < 0 || newx >= mapWidth || newy < 0 || newy >= mapHeight)
	{//在地图外
		outx = x;
		outy = y;
		return true;
	}

	if(isRecursion) return false;	//若是递归调用，则不执行后面的语句

	if(map[newy][newx] == MapType::Block)
	{//位置不可用
		for(int i = 1; i <= 3; i++)
		{
			Move newDirect = (Move)( (direct + i) % 4);
			if(findValidNeighborByDirect(x, y, newDirect, outx, outy, true)) return true;
		}
	}
	return false;
}

//===========================================================================
///	A*算法
//===========================================================================
void Game::astar(int x1, int y1, int x2, int y2, Move& outMove, bool isThief)
{
	int movex[] = {1,0,-1,0};
	int movey[] = {0,1,0,-1};

	int startx = x1;
	int starty = y1;
	int endx = x2;
	int endy = y2;

	ANode startNode(x1, y1, 0, (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));

	openList.clear();
	closeList.clear();
	multiset<ANode*>::iterator openEmpty = openList.end();
	marks.assign(mapHeight, vector<int>(mapWidth, 0));

	if(stores.size() != mapHeight || stores[0].size() == mapWidth)
		stores.resize(mapHeight, vector<multiset<ANode*>::iterator >(mapWidth, openEmpty));


	ANode* findNode = NULL;

	multiset<ANode*>::iterator tempIter = openList.insert(&startNode);

	marks[startNode.y][startNode.x] = 1;
	stores[startNode.y][startNode.x] = tempIter;

	while(!openList.empty())
	{
		tempIter = openList.begin();
		ANode* node = *tempIter;
		openList.erase(tempIter);

		tempIter = closeList.insert(node);

		marks[node->y][node->x] = 2;
		stores[node->y][node->x] = tempIter;

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
				bool isPoliceLoc = false;
				if(isThief)	
				{//若是小偷找路径，则警察所在位置也不可通过
					for(int k = 0; k < polices.size(); k++)
					{
						if(newx == polices[i].x && newy == polices[i].y)
						{
							isPoliceLoc = true;
							break;
						}
					}
				}
				if(!isPoliceLoc) 
				{
					if(marks[newy][newx] == 0)	//未访问
					{
						ANode* newNode = new ANode(newx, newy, node->step+1, (newx-endx)*(newx-endx)+(newy-endy)*(newy-endy), node);
						tempIter = openList.insert(newNode);

						marks[newy][newx] = 1;
						stores[newy][newx] = tempIter;
					}else if(marks[newy][newx] == 1)	//openlist
					{
						int newStep = node->step + 1;
						tempIter = stores[newy][newx];
						ANode* newNode = (*tempIter);
						if(newNode->step > newStep)
						{
							openList.erase(tempIter);
							newNode->step = newStep;
							newNode->total = newNode->step + newNode->remain;
							newNode->parent = node;
							tempIter = openList.insert(newNode);
							stores[newy][newx] = tempIter;
						}
					}else if(marks[newy][newx] == 2)	//closelist
					{
						int newStep = node->step + 1;
						tempIter = stores[newy][newx];
						ANode* newNode = (*tempIter);
						if(newNode->step > newStep)
						{
							closeList.erase(tempIter);
							newNode->step = newStep;
							newNode->total = newNode->step + newNode->remain;
							newNode->parent = node;
							tempIter = openList.insert(newNode);
							marks[newy][newx] = 1;
							stores[newy][newx] = tempIter;
						}

					}
				}

			}
		}

	}
	
	if(findNode == NULL)
	{//没找到
		srand(time(0));
		outMove = (Move)(int)(rand() % 5);
		return;
	}

	if(findNode->parent != NULL)
		while(findNode->parent->parent != NULL)
			findNode = findNode->parent;
	else
	{
		outMove = Move::Keep;
		return;
	}

	//0=east, 1=south, 2=west, 3=north, 4=keep
	ANode* parentNode = findNode->parent;
	if(parentNode->x < findNode->x)
	{
		outMove = Move::East;
		return;		
	}
	else if(parentNode->x > findNode->x)
	{
		outMove = Move::West;
		return;
	}
	if(parentNode->y < findNode->y)
	{
		outMove = Move::South;
		return;
	}
	else if(parentNode->y > findNode->y)
	{
		outMove = Move::North;
		return;
	}

	srand(time(0));
	outMove = (Move)(int)(rand() % 5);
	return;

}


