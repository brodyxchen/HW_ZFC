#include<iostream>
#include<vector>
#include<queue>
#include<list>
#include<string>
#include<sstream>
#include<ctime>
#include<cstdlib>

#include "sendCmd.h"

using namespace std;



/********
astar结点类型
**********/
class ANode
{
public:
	int x;
	int y;
	int step;
	int remain;
	int total;
	struct ANode* parent;

	ANode(int _x, int _y, int _step, int _remain, ANode* _parent=NULL)
	{
		x = _x;
		y = _y;
		step = _step;
		remain = _remain;
		total = step + remain;
		parent = _parent;
	}

	bool operator<(const ANode &a) const{
		return total > a.total;
	}
};





/*******************************
游戏类
*******************************/
class Game
{
private:
	/**********
	地形类型
	**********/
	enum MapType
	{
		//未探测  平地    障碍物  警察    小偷
		Unknown, Empty, Block, Police, Thief
	};
	/**********
	移动类型
	**********/
	enum Move
	{
		East, South, West, North, Keep
	};
	/**********
	结点类型
	**********/
	struct Node
	{
		int x;
		int y;
		Move move;
	};


	/**********
	客户端角色类型
	**********/
	enum Roles
	{
		PoliceClient, ThiefClient
	};

private:
	Roles role;						//角色
	vector<vector<int> > map;		//地图（二维数组）
	int step;						//当前是第几轮

	int mapWidth;					//地图宽
	int mapHeight;					//地图高

	int policeRange;				//警察视野大小
	int thiefRange;					//小偷视野大小

	int policeNum;					//警察数量
	int thiefNum;					//小偷数量

	bool hasEnemy;

	vector<Node> polices;			//警察 k号位置存储id=k的警察数据（位置XY，下一步应该如何移动等）
	vector<Node> thiefs;			//小偷 k号位置存储id=k的小偷数据（位置XY，下一步应该如何移动等）

private:
	void splitIdXY(string str, vector<Node> &list);		//处理 id1,x1,y1;id2,x2,y2;id3,x3,y3;...类型字符串
	void splitXY(string str, int &x, int &y);			//处理 x,y... 字符串
	void splitXYs(string str, vector<vector<int> > &table);	//处理 x1,y1;x2,y2;x3,y3;... 字符串

	void analyzeINI(string s);		//分析INI命令
	void analyzeINF(string s);		//分析INF命令
	string buildMOV();				//根据role类型，读取polices/thiefs里数据构建 MOV指令
	void computeMove();				//计算下一步移动
	void policeMove();				//计算警察下一步
	void thiefMove();				//计算小偷下一步

	//Astar算法和相关辅助方法
	int astar(int x1, int y1, int x2, int y2); //Astar算法 	return int:0=east, 1=south, 2=west, 3=north, 4=keep
	void astar_InsertList(list<ANode*> &list, ANode* node);
	ANode* astar_RemoveList(list<ANode*> &list, int x, int y, int step, ANode* parent);
	void astar_UpdateList(list<ANode*> &list, int x, int y, int step, ANode* parent);


public:
	void setRole(string str);		//设置客户端的角色类型
	void receiveINI(char* str);		//接收INI指令，并进行处理
	void receiveINF(char* str);		//接收INF指令，并进行处理
	void sendMOV();					//发送MOV指令

};