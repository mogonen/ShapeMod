#ifndef _GRAPH_H__	
#define __GRAPH_H__

#include <list>
#include "Auxiliary.h"


using namespace std;


class Node{

	void* _data;
	
protected:
	Flag flag;
	list<Node> _to;

public:

	Node(void*d, int type = 0){_data = d;};
	void* get(){return _data;};
	void set(void* data){_data = data;};
};

class Graph{
	
	list<Node> nodes;

};

#endif