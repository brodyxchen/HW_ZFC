#pragma once
#include<iostream>
#include<vector>
#include<list>
#include<queue>
#include<string>
#include<set>
#include<sstream>
#include<ctime>
#include<cstdlib>

#include "sendCmd.h"

using namespace std;


/**********************************************************************************
* 游戏类  
**********************************************************************************/
class Game
{
private:
	//============================================================================
	// 角色客户端类型
	//============================================================================
	enum Roles
	{
		PoliceClient, ThiefClient
	};

	//============================================================================
	// 地形类型
	//============================================================================
	enum MapType
	{
		//未探测  平地   障碍物 
		Unknown, Empty, Block
	};

	//============================================================================
	// 移动类型
	//============================================================================
	enum Move
	{
		East, South, West, North, Keep
	};

	//============================================================================
	// 位置结点类型
	//============================================================================
	struct Node
	{
		int x;		//当前位置
		int y;
		Move move;	//移动方向
		bool visible;

		//警察搜索参数
		int scanToPreX;		//扫描时大概的方位（可能位于障碍物上）
		int scanToPreY;
		int scanToX;		//扫描时准确的方位(大概方位周围的空白区域)
		int scanToY;
		bool directHoriz;	//当前是否在扫描水平线

		bool isHorizScan;	//总体上，是在做横向还是纵向扫描
		int sober;

	};

	//============================================================================
	// 警察与方向的夹角结点
	//============================================================================
	struct AngleNode
	{
		Move direct;
		int index;
		double value;
		AngleNode(int _index, Move _direct, double _value):index(_index),direct(_direct),value(_value){}
	};

	//============================================================================
	// 警察与方向的夹角结点的比较函数
	//============================================================================
	struct AngleNodeCmp
	{
		bool operator()(const AngleNode& a, const AngleNode& b)
		{
			return a.value < b.value;
		}
	};

private:
	//============================================================================
	// 变量
	//============================================================================
	Roles role;						//角色类型
	vector<vector<MapType> > map;	//地图（二维数组）
	int step;						//当前是第几轮

	int mapWidth;					//地图宽
	int mapHeight;					//地图高

	int policeRange;				//警察视野大小
	int thiefRange;					//小偷视野大小

	bool hasEnemy;					//是否有敌人

	vector<Node> polices;			//警察 k号位置存储id=k的警察数据（位置XY，下一步应该如何移动等）
	int lockedPolice;				//追击的警察Id，其他围捕

	int soloPolice;					//某个特殊警察Id (一个离中心最近的警察负责从中心开始的螺旋形扫描，其他负责刷子扫描)
	Move soloDirect;				//螺旋方向
	vector<int> helixDists;			//记录螺旋的距离 （1,1,2,2,3,3,4,4,5,5...）
	int helixDistIndex;				//螺旋距离数组的索引

	vector<Node> thiefs;			//小偷
	int lockedThief;				//被追击的小偷Id


	vector<vector<int> > scanHorLine;		//水平扫描线
	vector<vector<int> > scanVerLine;		//垂直扫描线
	vector<int> scanHorIndex;				//水平扫描索引
	vector<int> scanVerIndex;				//垂直扫描索引


public:
	void setRole(string str);		//设置客户端的角色类型
	void receiveINI(char* str);		//接收INI指令，并进行处理
	void receiveINF(char* str);		//接收INF指令，并进行处理
	void sendMOV();					//发送MOV指令

private:
	//============================================================================
	// 解析字符串
	//============================================================================
	void splitIdXY(string str, vector<Node> &outList);				//处理 id1,x1,y1;id2,x2,y2;id3,x3,y3;...类型字符串
	void splitXY(string str, int &outx, int &outy);					//处理 x,y... 字符串
	void splitXYs(string str, vector<vector<MapType> > &outTable);	//处理 x1,y1;x2,y2;x3,y3;... 字符串

	//============================================================================
	// 解析命令
	//============================================================================
	void analyzeINI(string s);		//分析INI命令
	void analyzeINF(string s);		//分析INF命令

	//============================================================================
	// 构建命令
	//============================================================================
	string buildMOV();				//根据role类型，读取polices/thiefs里数据构建 MOV指令

	//============================================================================
	// 计算移动方向
	//============================================================================
	void computeMove();				//计算下一步移动
	void policeMove();				//计算警察下一步
	void thiefMove();				//计算小偷下一步

	//============================================================================
	// 警察的移动
	//============================================================================
	void policeScan();				//警察扫描（刷子法）
	void helixScan();				//螺旋扫描（螺旋法），只有一个警察使用
	void policeScan2();
	void policePursue();			//警察追捕

	bool isValidXY(int x, int y);
	bool isValidMove(int x, int y, Move move);
	//============================================================================
	// 小偷的移动
	//============================================================================
	void thiefScan();				//小偷扫描
	void thiefEscape();				//小偷逃脱

	//============================================================================
	// 计算移动的辅助函数
	//============================================================================
	void findValidPositionAroundXY(int x, int y, int& outx, int& outy, bool isThief=false);						//搜索(x,y)及其周围最近的有效位置，赋值给tox,toy
	bool findValidNeighborByDirect(int x, int y, Move direct, int& outx, int& outy, bool isRecursion=false);	//搜索(x,y)在指定方向的一步可达位置，若不存在，则顺时针换方向寻找			
	double calcRadian(int x, int y, int centerx, int centery, Move move);										//计算向量夹角：向量(center->xy) 与 向量(move)的夹角
	void calcScanLine();																						//计算扫描线和螺旋距离与负责螺旋扫描的警察等



//////////////////////////////////////////////////////////////////////////////////////////////////
// A*算法
//////////////////////////////////////////////////////////////////////////////////////////////////
private:
	//============================================================================
	// Astar结点类型
	//============================================================================
	class ANode
	{
	public:
		int x;			//坐标
		int y;
		int step;		//已走步数
		int remain;		//剩余代价
		int total;		//总代价=step+remain
		ANode* parent;	

		ANode(int _x, int _y, int _step, int _remain, ANode* _parent=NULL):x(_x),y(_y),step(_step),remain(_remain),total(step+remain),parent(_parent){}
	};

	//============================================================================
	// Astar算法结点的比较类
	//============================================================================
	struct ANodeLess
	{
		bool operator() (const ANode* a, const ANode* b) const
		{
			return a->total < b->total;
		}
	};

	//============================================================================
	// Astar算法中用到的容器变量（减少重复构建的开销）
	//============================================================================
	multiset<ANode*, ANodeLess> openList;
	multiset<ANode*, ANodeLess> closeList;
	vector<vector<int> > marks;								//0=未访问 1=openlist 2=closelist
	vector<vector<multiset<ANode*>::const_iterator > > stores;	//存储每个位置对应的ANode迭代器

	//============================================================================
	// 寻路算法A*
	//============================================================================
	void astar(int x1, int y1, int x2, int y2, Move& outMove, bool isThief=false);		//Astar算法

};