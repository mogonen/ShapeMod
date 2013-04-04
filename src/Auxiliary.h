#ifndef __AUXILIARY_H__	
#define __AUXILIARY_H__
#include <vector>

class Flag{

	unsigned long _flag;
	unsigned int _counter;
	void* _store;
	void** _storage;

public:

	vector<double>  storeD;
	vector<int>		storeI;

	enum{NEW, OLD, VISITED, SELECTED, DELETED, MARKED, SIDE, BORDER, IN, OUT};
	Flag(){reset();};
	void reset(){_flag = 0; _counter = 0; _store=0; _storage =0;};
	void resetF(){_flag = 0;};
	bool is(unsigned int bit){return _flag & (1 << bit);};
	void set(unsigned int bit){_flag |= (1 << bit);};
	void unset(unsigned int bit){_flag &= ~(1 << bit);};

	int inc(){return ++_counter;};
	int dec(){return --_counter;};
	int getC(){return _counter;};
	void resetC(){_counter = 0;};

	void* store(void* p){void*tmp = _store; _store = p; return tmp;};
	void* retrieve(){return _store;};
	void  initStorage(int n){_storage = new void*[n];};
	void* store(int i, void*p){if (!_storage) return 0; void*tmp = _storage[i]; _storage[i] = p; return tmp;};
	void* retrieve(int i){if (!_storage) return 0; return _storage[i];};
};

static Vec3 quadNormal(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3){
	Vec2 pAB[2];
	pAB[0] = getIntersection(p0, (p1 - p0).normalize(), p3, (p2 - p3).normalize());
	pAB[1] = getIntersection(p1, (p2 - p1).normalize(), p0, (p3 - p0).normalize());
	double k = pAB[0]*pAB[1];
	double x = (k/pAB[0].y - k/pAB[1].y) / ( pAB[0].x / pAB[0].y - pAB[1].x / pAB[1].y);
	double y = (k/pAB[0].x - k/pAB[1].x) / ( pAB[0].y / pAB[0].x - pAB[1].y / pAB[1].x);
	Vec3 n(x,y,1);
	return n.normalize();
}

#endif