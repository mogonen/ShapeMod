#include "Curve2Mesh.h"

void K2M::buildRings(){
	//updateCurves();
	_rings.clear();
	//cout<<"build Rings!!!!"<<endl;
	for (list<ArrCurve*>::iterator it = _curves.begin(); it != _curves.end(); it++){
		ArrCurve* ac = (*it);
		RingPtr root = 0;
		RingPtr pre = 0;
		for(int i=0; i<ac->size(); i++){
			Vec3 T(ac->getT(i));
			RingPtr r = new Ring(ac->getP(i), T, rad);
			if (pre)
				r->setPrevNext(pre, 0);
			else 
				root = r;
			pre = r;
		}
		if (ac->isClosed())
			root->close();
		_rings.push_back(root);
	}
	//cout<<"end!"<<endl;
}

int K2M::getUpDown(int i){
	if (updown){
		int k = i;
		while(updown[k]==0 && k<size-1) 
			k++;

		if (updown[k]==0)
			while(updown[k]==0 && k>0) 
				k--;

		if (updown[k]==0)

			return 1;

		return updown[k];
	}
}

void K2M::solveKnots(double r){

	updateCurves();
	updown = 0;
	size = 0;
	rad = r;
	skip = 4;
	double eps = 0.0001;
	double diam = rad*2;
	_curvenum = _curves.size();
	_inds = new int[_curvenum];
	int i =0;

	for (list<ArrCurve*>::iterator it = _curves.begin(); it != _curves.end(); it++){
		ArrCurve* aci = (*it);
		aci->resample( diam/skip  + eps);
		_inds[i++] = size; 
		size+=aci->size();
	}

	orgp = new Vec3[size];
	int ind = 0;

	for (list<ArrCurve*>::iterator it = _curves.begin(); it != _curves.end(); it++){
		ArrCurve* ac = (*it);
		for(int i=0; i<ac->size(); i++)	
			orgp[ind++] = Vec3(ac->getP(i));
	}
	solveKnots(0);
}

void K2M::goForIt(){

	Vec3* newp = solveKnots(0);
	int ind = 0;

	for (list<ArrCurve*>::iterator it = _curves.begin(); it != _curves.end(); it++){
		ArrCurve* ac = (*it);
		ac->set(&newp[ind], ac->size());
		ind+=ac->size();
	}
	
	for (list<ArrCurve*>::iterator it = _curves.begin(); it != _curves.end(); it++){
		ArrCurve* aci = (*it);
		aci->movingAverage(4);
	}
}

Vec3 * K2M::solveKnots(int iter){

	Vec3 up(0.0, 0.0, 1.0);
	int istop = 1;

	bool writeupdown = false; 
	if (!updown){
		writeupdown = true;
		updown = new int[size];
	}else{
		cout<<"regular pass!"<<endl;
		istop = getUpDown(0);	
	}

	Vec3 * newp = new Vec3[size];
	for(int i=0; i < size; i++){
		newp[i] = orgp[i];
		if (writeupdown)
			updown[i] = 0;
	}

	double eps = 0.0001;
	double diam = rad*2;
	Vec3 ax(0,0,1);

	int occ  = 0;
	int preocc = 0;
	int coll = 0;

	bool crossed = false;
	for(int i=0; i<size; i++){

		Vec3 dir = orgp[i] - orgp[i-1];
		double len  = dir.norm();
		Vec3 ncross = dir%ax;
		dir = dir.normalize();
		double tmax = -DBL_MAX;
		Vec3 move(0,0,0);

		int indi = 0;
		for(int ii = 0; ii<_curvenum; ii++)
			if (i>=_inds[ii])
				indi = _inds[ii];

		coll = 0; occ = 0;
		
		for(int j = skip; j < size-skip; j++){
						
			if (abs(i-j) <= skip || abs(indi-j)<=skip)
				continue;

			int indj = 0;
			for(int ii = 0; ii<_curvenum; ii++)
				if (j>=_inds[ii])
					indj = _inds[ii];

			if (writeupdown && !iter &&  abs(indi-i)>skip && abs(indj-j)>skip ){
	
				Vec3 dir2 = (orgp[j] - orgp[j-1]);
				double len2 = dir2.norm();
				dir2 = dir2.normalize();
				double d = getIntersectionDist(Vec2(orgp[i-1]), Vec2(dir),  Vec2(orgp[j-1]), Vec2(dir2));
				double d2 = getIntersectionDist(Vec2(orgp[j-1]), Vec2(dir2),  Vec2(orgp[i-1]), Vec2(dir));
				if ( (d>=0 && d<=len) && (d2>=0 && d2<=len2) ){
					crossed = true;
					Vec3 loc = d*dir + orgp[i-1];				
					if (i<=j)
						CrossPoint * cp = new CrossPoint(&updown[i], &updown[j], loc);
					cout<<"new cross: "<<i<<" x "<<j<<":"<<istop<<endl;
					updown[i] = istop;
					updown[j] = istop*(-1);
				}
			}
			
			Vec3 v =  (newp[j] - newp[i]);
			double d = v.norm();

			Vec3 vcol = up*istop;
			double h = vcol*v;
			double b = sqrt(d*d - h*h);
			double t = sqrt(diam*diam - b*b);

			if (b < rad)
				occ = j;
			if (h+eps > -t && (b+eps <diam) ){ // any collision on 2D projection
				double mul = (i > j || iter)? 1 : 0;
				double tt = t*mul + h;
				if (tmax < tt ){
					tmax = tt;
					move  = vcol*tt;
					coll = j;
				}
				occ = j;
			}
			skiploopj:;
		}

		if (move.norm())
			newp[i].set(newp[i] + move);

		if ( (preocc && !occ) && !iter){
			if (!writeupdown){
				istop = getUpDown(i);
				cout<<i<<" - istop"<<istop<<endl;
			} else if (crossed)
				istop = istop*-1;
			crossed = false;
		}
		preocc = occ;
	}

	if (writeupdown)
		for(int i=0; i<size; i++)
			cout<<" "<<updown[i];

	cout<<endl;
	return newp;
}

/*
*********************************DEPRECATED STUFF


Vec3 * K2M::solveKnots(Vec3 * orgp, int size, double rad, int iter){

	Vec3 * newp = new Vec3[size];
	int * colls = new int[size];
	for(int i=0; i < size; i++){
		colls[i] = 0;
		newp[i] = orgp[i]; 
	}

	Vec3 up(0.0, 0.0, 1.0);
	int istop = 1;
	int precoll = 0;
	int skip = 4;
	double eps = 0.0001;
	double diam = rad*2;
	Vec3 ax(0,0,1);
	cout<<"solving Knots!"<<_curves.size()<<endl;


	for (list<ArrCurve*>::iterator it = _curves.begin(); it != _curves.end(); it++){
		ArrCurve* aci = (*it);
		int sizei = aci->size();
		bool crossed = false;
		for(int i = skip; i < sizei; i++){
			int acii = aci0 + i;

			Vec3 dir = (aci->getP(i) - aci->getP(i-2));
			double len  = dir.norm();
			Vec3 ncross = dir%ax;
			dir = dir.normalize();
			double tmax = 0;
			Vec3 move(0,0,0);

			int acj0 = 0;
			for (list<ArrCurve*>::iterator itj = _curves.begin(); itj != _curves.end(); itj++){

				ArrCurve* acj = (*itj);
				int sizej = acj->size();
	
				bool isself = (aci == acj)?true:false;
				int fix = 1;
				for(int j = skip; j < sizej; j++){

					if (isself && abs(i-j) <= skip)
						continue;

					int acjj = acj0+j;

					if (!iter && !crossed){
						Vec3 dir2 = (acj->getP(j) - acj->getP(j-1));
						double len2 = dir2.norm();
						dir2 = dir2.normalize();
						double d = getIntersectionDist(Vec2(aci->getP(i-1)), Vec2(dir),  Vec2(acj->getP(j-1)), Vec2(dir2));
						double d2 = getIntersectionDist(Vec2(acj->getP(j-1)), Vec2(dir2),  Vec2(aci->getP(i-1)), Vec2(dir));

						if ( (d>=0 && d<=len) && (d2>=0 && d2<=len2) ){
							crossed = true;
						}
					}

					Vec3 pj = newp[acjj];
					Vec3 vij =  (newp[acii] - pj);
					double dij = vij.norm();
					if (dij + eps < diam ){ // is there any collision?
						
						double tt = dij/diam;
						//tt = 1 0/ (1+ exp(-tt));
						tt = tt * tt;
						tt = 0;
						Vec3 dir = (up*istop*(1-tt) + vij.normalize()*tt).normalize();
						//Vec3 dir = vij.normalize();
						double h = dir*vij;
						double b = sqrt(dij*dij - h*h);

						double mul = ( (acii > acjj || iter)? 1 : 0.5);
						double t = (sqrt(diam*diam - b*b) - abs(h))*mul;
						if (t>tmax){
							colls[acii] = acjj;
							colls[acjj] = acii;
							move = dir*t;							
							tmax = t;
						}
					}
				}
				acj0+=sizej;
			}
			if (move.norm())
				newp[acii].set(newp[acii] + move);
			//cout<<"-------tmax:"<<tmax<<" i:"<< acii<<endl;

			//bool collision_end = precoll && (abs(colls[acii] - precoll) > skip);
			bool collision_end = precoll && !colls[acii];

			if (collision_end && !iter){
				cout<<"cross:"<<crossed<<endl;
				if (crossed)
					istop = istop*-1;
				crossed = false;
			}

			precoll = colls[acii];
		}

		//aci->resample( (diam / skip) +eps);
		aci0+=sizei;
		cout<<endl<<endl;
		//break;
	}

	for(int i=0; i<size; i++)
		cout<<colls[i]<<" ";
	cout<<endl<<endl;

	delete [] colls;
	return newp;
}

*/