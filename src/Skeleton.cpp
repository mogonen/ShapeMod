#include "Skeleton.h"

bool DRAW_AUX = false;

CorrPoint::CorrPoint(Vec3 p, Curve* c, double t){
	_p0 = p;
	_c = c;
	_t = t;
	_prev = 0;
	_next = 0;
	_corr = 0;
	_flag  = 0;
	_isshooter = false;
	_spine = 0;
}
void CorrPoint::updateNT(){
	_t0 = ((_next?_next->P():_p0) - (_prev?_prev->P():_p0)).normalize();
	_n0 = (Eye::get()->N%_t0).normalize();
}

void CorrPoint::drawAll(){

	if (!DRAW_AUX)
		return;
	glColor3f(0.0f,1.0f,0.0f);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for(CPPtr cp = this; cp; cp = cp->next()){	
		glVertex3f(cp->P().x, cp->P().y, cp->P().z);
		if (cp->next() == this)
			break;
	}
	glEnd();

	//corr
	glBegin(GL_LINES);
	for(CorrPtr cp = this; cp; cp =(CorrPtr) cp->next()){
		if (cp->_corr){
			
			if (cp->isSplit())
				glColor3f(0.0f,0.0f,1.0f);
			else if (cp->isTip())
				glColor3f(0.0f,0.0f,0.0f);
			else
				glColor3f(0.0f,1.0f,0.0f);

			Vec3 p1 = cp->_corr->_p0;
			glVertex3f(cp->_p0.x, cp->_p0.y, cp->_p0.z);
			glVertex3f(p1.x, p1.y, p1.z);
			/*
			if (DRAW_AUX){
				glColor3f(0.98f,0.98f,0.98f);
				Vec3 pp1 = Eye::get()->P+(cp->_p0 - Eye::get()->P).normalize() * 5.0;
				Vec3 pp2 = Eye::get()->P+(cp->_corr->_p0 - Eye::get()->P).normalize() * 5.0;
				
				glVertex3f(Eye::get()->P.x, Eye::get()->P.y, Eye::get()->P.z);
				glVertex3f(pp1.x, pp1.y, pp1.z);
				
				glVertex3f(Eye::get()->P.x, Eye::get()->P.y, Eye::get()->P.z);
				glVertex3f(pp2.x, pp2.y, pp2.z);
			}*/
		}
		if (cp->next() == this)
			break;

	}
	glEnd();
}

CorrPtr CorrPoint::corr(){return _corr;}

CorrPtr CorrPoint::tipCorr(){
	CorrPtr fp = 0;
	for(CorrPtr cp=this; cp; cp = (CorrPtr)cp->_prev){
		if ( cp->isTip())
			return cp;
		if (cp->_prev == this)
			break;
	}
	return 0;
}

CorrPtr CorrPoint::nextCorr(){
	for(CPPtr cp = _next; cp && cp!=this; cp = cp->next())
		if ( ( (CorrPtr)cp)->_corr)
			return (CorrPtr)cp;
	return 0;
}

CorrPtr CorrPoint::prevCorr(){
	for(CPPtr cp = _prev; cp && cp!=this; cp = cp->prev())
		if (( (CorrPtr)cp)->_corr)
			return (CorrPtr)cp;
	return 0;
}

CorrPtr CorrPoint::corrNextCorr(){
	if (!_corr)
		return 0;
	return _corr->nextCorr();
}

CorrPtr CorrPoint::corrPrevCorr(){
	if (!_corr)
		return 0;
	return _corr->prevCorr();
}

void CorrPoint::setCorr(CPPtr cp){
	if (!cp)
		return;
	if (_corr){
		_corr->_corr = 0;
		_isshooter = false;
	}
	_isshooter = true;
	_corr = (CorrPtr)cp;
	_corr->_corr = this;
}

void CorrPoint::discardCorr(){
	if (_corr)
		_corr->_corr = 0;
	_corr = 0;
}

bool CorrPoint::isIntersecting(CorrPtr cp){

	CorrPtr nc = nextCorr();
	CorrPtr pc = prevCorr();

	Vec3 n0 = cp->_p0 - _p0;
	double len = n0.norm();
	n0 = n0.normalize();

	if (nc){
		Vec3 n1 = (nc->_corr->_p0 - nc->_p0).normalize();
		double t = getIntersectionDist( _p0, n0, nc->_p0, n1);
		if (t>0 && t<len+0.001) 
			return true;
	}

	if (pc){
		Vec3 n1 = (pc->_corr->_p0 - pc->_p0).normalize();
		double t = getIntersectionDist( _p0, n0, pc->_p0, n1);
		if (t>0 && t < len+0.001) 
			return true;
	}

	return false;
}

bool CorrPoint::isSplit(){
	if (!_corr)
		return false;

	CorrPtr nc0 = nextCorr();
	CorrPtr pc0 = prevCorr();

	CorrPtr nc1 = _corr->nextCorr();
	CorrPtr pc1 = _corr->prevCorr();

	if (!(nc0&&pc0&&pc1&&nc1))
		return false;

	if (nc0->_corr!=pc1 || nc1->_corr!=pc0)
		return true;

	return false;
}

bool CorrPoint::isTip(){

	if (!_corr)
		return false;

	CorrPtr c0n = nextCorr();
	CorrPtr c0p = prevCorr();

	CorrPtr c1n = _corr->nextCorr();
	CorrPtr c1p = _corr->prevCorr();

	if ( (!c0n&&!c1p) || (!c0p&&!c1n) )
		return true;

	if ( c0n == _corr || c0p == _corr)
		return true;

	return false;
}

void CorrPoint::setSpine(Curve* sp, CorrPtr end){
	_spine = sp;
	_sptip = end;
	_spdir = 1;
	if (_sptip){
		_sptip->_spine = sp;
		_sptip->_sptip = this;
		_sptip->_spdir = -1;
	}
}

//+forward, -backward, 0 bothways
int CorrPoint::getFreeCount(int dir){
	if (_corr)
		return 0;
	if (dir==0)
		return getFreeCount(-1)+getFreeCount(1)-1;

	CorrPtr it = this;
	int count = 0;
	while( it && !it->_corr){
		it = (CorrPtr) ((dir>0)?it->_next:it->_prev);
		count++;
	}
	return count;
}

CorrPtr CorrPoint::getShoot(Vec3 p, Vec3 n){
		
	CorrPtr to = this;
	int dir = -1;
	if (this->_next){
		double c0 = abs((to->_p0 - p).normalize()*n);
		double c1 = abs((to->next()->P() - p).normalize()*n);
		if (c1<c0)
			dir =1;
	}

	CorrPtr maxcp = 0; 
	double maxcos = 0;
				
	while(to){
		Vec3 nn = (to->_p0 - p).normalize();
		double cos = abs(nn*n);
		if (cos > maxcos && cos>0.707){
			maxcp = to;
			maxcos = cos;
		}
		if (dir>0)
			to = (CorrPtr)to->_next;
		else
			to = (CorrPtr)to->_prev;
	}
	return maxcp;
}

CorrPtr CorrPoint::sampleCurve(Curve* c, double step){

	CorrPtr pre = 0;
	CorrPtr tip = 0;
	int newsize;
	double t = 0;
	Vec3 * p = ArrCurve::resample(c->toArr(), c->size(), step, newsize);
	double tinc = 1.0/(newsize-1);
	for(int i = 0; i<newsize; i++){
		CorrPtr cp = new CorrPoint(p[i], c, t);
		if (pre)
			cp->setPrevNext(pre,0);
		else 
			tip = cp;
		pre = cp;
		t=t+tinc;
	}

	for(CorrPtr cp = tip; cp; cp = (CorrPtr)cp->next())
		cp->updateNT();	

	if (c->isClosed()){
		CorrPtr end = (CorrPtr)tip->last();
		tip->_prev = end;
		end->_next = tip;
	}

	return tip;
}

void C2S::addCurve(Curve* c){
	CorrPtr cp = CorrPoint::sampleCurve(c, STEP);
	cps.push_back(cp);
}

void C2S::draw(){
	for (list<CorrPtr>::iterator it = cps.begin(); it != cps.end(); it++)
		(*it)->drawAll();
}

C2S::C2S(){
}

void C2S::resampleCurves(const list<Curve*> &clist){
	cps.clear();
	for (list<Curve*>::const_iterator it = clist.begin(); it != clist.end(); it++)
			addCurve( (*it));
}

void C2S::buildCorrespondances(){

	for(list<CorrPtr>::iterator it =cps.begin(); it!=cps.end(); it++){
		CorrPtr cp0 = (*it); 
		for(CorrPtr cp = cp0; cp && cp->next()!=cp0; ){
			cp->setCorr(findCorr(cp));
			for(int i=0; cp && i<4 && cp->next()!=cp0; cp = (CorrPtr)cp->next())
				i++;
		}
	}
	//CorrPtr corr = 0;
	//CorrPtr first = cps.front();
		/*if (corr){
			double min = -1;
			corr = findCorr(cp, (CorrPtr)corr->prev(), min);
		}else*/
}


CorrPtr C2S::spot2Shoot(CorrPtr cp){
	//return (CorrPtr)cp->go(4);
	for(int i=0; cp && i<4; cp = (CorrPtr)cp->next())
		i++;
	return cp;
}

CorrPtr C2S::findCorr(CorrPtr cp0){
	double kmin =  99999;
	double min = kmin;
	CorrPtr corr = 0;
	for(list<CorrPtr>::iterator it =cps.begin(); it!=cps.end(); it++){

		CorrPtr last = (CorrPtr)(*it)->last();
		CorrPtr _corr = findCorr(cp0, (CorrPtr)(*it)->last(), min);
		if (min<kmin){
			kmin = min;
			corr = _corr;
		}
	}
	return corr;
}

CorrPtr C2S::findCorr(CorrPtr cp0, CorrPtr cp1, double& kmin){

	bool skip = (kmin<0)?false:true;
	kmin = 99999;
	CorrPtr corr = 0;
	for(CorrPtr cpi=cp1; cpi && cpi!=cp0 ; cpi = (CorrPtr)cpi->prev()){

		if (cp0 == cpi || cp0->corr() || cpi->corr() || cp0->isIntersecting(cpi))
			continue;
	
		/*if (cp0 == cpi || cpi->corr()){
			if (skip)
				continue;
			else 
				break;
		}*/
			
		Vec3 c1mc0 =cpi->P()-cp0->P();
		double cos = cp0->N()*c1mc0.normalize(); 
		if ( cos < 0.707)
				continue;

		double cosi = cpi->N()*(-c1mc0.normalize());
		if ( cosi < 0.707)
				continue;
	
		double r=999999999; 
		/*double dist0 = abs(getIntersectionDist(cp0->P(),cp0->T(), cpi->P(), cpi->T() ));
		double dist1 = abs(getIntersectionDist(cpi->P(),cpi->T(), cp0->P(), cp0->T() ));*/

		Vec3 pAB = getIntersection(cp0->P(),cp0->T(), cpi->P(), cpi->T());
		double dist0 = (cp0->P() - pAB).norm();
		double dist1 = (cp1->P() - pAB).norm();
		double diff = abs(dist0-dist1);

		if (diff <STEP*2)
			r =c1mc0.norm()*0.5 / cos;
		else
			r = (cp0->P()-cpi->P()).norm()/(2.0 * cos);
		
		double k = r; //+ diff;
		if ( kmin > k){
			kmin = k;
			corr = cpi;
		}
	}
	return corr;
}

Vec3 C2S::getOrgbyRad(CorrPtr cp, double rad){
		
	Vec3 n0 = (cp->P() - Eye::get()->P).normalize();
	Vec3 n = ( n0 + (cp->corr()->P() - Eye::get()->P).normalize() ).normalize();		
	double ca = n*n0;
	double z = rad / sqrt(1-ca*ca);
	return z*n + Eye::get()->P;
}

double C2S::getZbyRadius(CorrPtr cp, double rad){
		
	Vec3 n0 = (cp->P() - Eye::get()->P).normalize();
	Vec3 n = ( n0 + (cp->corr()->P() - Eye::get()->P).normalize() ).normalize();		
	double ca = n*n0;
	return rad / sqrt(1-ca*ca);
}

list<Curve*> C2S::buildSpine(){

	list<Curve*> spine;

	double rad[100];
	CorrPtr corrs[100];
	int id =0;
	double radmax =0;

	for(list<CorrPtr>::iterator it = cps.begin(); it!=cps.end(); it++){
		CorrPtr cp = (*it);
		if (!cp->corr())
			cp = cp->nextCorr();
		for( ; cp; cp = cp->nextCorr()){
			if (cp->getFlag(0))
				continue;
			double r = ((cp->P()-cp->corr()->P())%(Eye::get()->P*2.0-cp->P()-cp->corr()->P()).normalize()).norm()/2.0;//nice radious
			rad[id] = r;
			if (r>radmax)
				radmax = r;
			corrs[id] = cp;
			cp->storeD(rad[id]);
			id++;
			//visit
			cp->setFlag(0);
			cp->setFlag(1);
			cp->corr()->setFlag(0);
		}
	}
	//normalize by max radious
	/*
	for(int i=0; i<id; i++){
		double k = rad[i]*0.94+radmax*0.06;
		corrs[i]->storeD(k);
	}*/

	//begin traversal
	map<CorrPtr, JntP> jmap;
	list<CorrPtr> que;
	int i=0;
	for(list<CorrPtr>::iterator it =cps.begin(); it!=cps.end(); it++){
		CorrPtr cp = (*it);
		if (!cp->corr())
			cp = cp->nextCorr();
		if (cp && cp->getFlag(1)){
			que.push_back(cp);
			cout<<"queued:"<<cp<<endl;
		}
	}
	while(!que.empty()){
		CorrPtr cpend = 0;
		CorrPtr cp0 = que.front();
		que.pop_front();
		ArrCurve* ac = getSpine(cp0, cpend);
		cout<<"ac:"<<ac->length()<<" s:"<<ac->size()<<endl;
		//NURBS * nc = NURBS::create(ac, 0.2);

		//spine.push_back(nc);
		//cp0->setSpine(nc, cpend);
		JntP j1 = 0;
		if (cpend && cpend->isSplit()){
			//we just hit a joint
			 j1 = new Joint();
			for(CorrPtr cpi = cpend->nextCorr(); cpi && cpi!=cpend && cpi!=cpend->corr(); cpi = cpi->corr()->nextCorr()){
				que.push_back(cpi);
				jmap[cpi] = j1; 
			}
		}

		JntP j0 = jmap[cp0];
		if (j0)
			j0->addSpine(new Spine(j0, j1));
	}
	return spine;
}

ArrCurve* C2S::getSpine(CorrPtr cp_in, CorrPtr& endcp){
	endcp = 0;
	Vec3 * pts = new Vec3[100];
	double dmax = 0;
	int id=0;
	CorrPtr cp = cp_in;
	for(; cp; cp = cp->nextCorr()){

		Vec3 o = C2S::getOrgbyRad(cp, cp->retrieveD());
		double d = o.norm();
		if (d>dmax)
			dmax = d;
		o.print();cout<<endl;
		pts[id++] = o;
		if (cp != cp_in && (cp->isSplit() || cp->isTip()) )
			break;
	}


	if (!endcp)
		endcp = cp;

	//compress in z
	/*for(int i=0; i<id; i++){
		 Vec3 o = pts[i];
		 double d = o.norm();
		 double a = PZ / (o.normalize()*Eye::get()->N);
		 double t = ((d-a)<0?0:(d-a))*ZCOMPRESS;
		 o = o.normalize()*(a+t);
		 pts[i] = o;
	}*/

	ArrCurve *ac = new ArrCurve(pts, id);
	ac->resample(STEP*3);
	return ac;
}

void C2S::filter(int num){

	map<CorrPtr, CPPtr> new_corrs;		
		for(int i=0; i<num; i++){
			new_corrs.clear();
			for(list<CorrPtr>::iterator it =cps.begin(); it!=cps.end(); it++)
				for(CorrPtr cp = (*it); cp && cp!=(*it)->prev(); cp = (CorrPtr)cp->next()){
					
					if (!cp->isShooter() || cp->isSplit() || cp->isTip() )
						continue;
						
					CorrPtr pre  = cp->prevCorr();
					CorrPtr next = cp->nextCorr();
					
					if (!pre || !next )
						continue;
					
					int dir = pre->corr()->find(next->corr());
					if (dir == 0)
							continue;
					
					CPPtr corr = pre->corr()->go(dir/2);
					if (corr)
						new_corrs.insert( pair<CorrPtr, CPPtr>(cp, corr) );				
				}

			for(map<CorrPtr, CPPtr>::iterator it = new_corrs.begin(); it!=new_corrs.end(); it++)
				(*it).first->setCorr((*it).second);
	}
}

void C2S::bumFilter(){
	for(list<CorrPtr>::iterator it = cps.begin(); it!=cps.end(); it++)
		bumFilter(*it);
}

void C2S::bumFilter(CorrPtr _cp0){
	CorrPtr cp0 = _cp0;
	while(cp0){			
		CorrPtr cp0n = cp0->nextCorr();							
		CorrPtr cp0nn = (!cp0n)?0:cp0n->nextCorr();			
		if (!cp0nn)			break;			
		cp0n->discardCorr();

		Vec3 n  = (cp0->P() - cp0nn->corr()->P()).normalize();
		Vec3 nn = (cp0nn->P() - cp0->corr()->P() ).normalize();

		Vec3 cp0_u  =  (cp0->P() - cp0->corr()->P()).normalize();
		Vec3 cp0nn_u = (cp0nn->P() - cp0nn->corr()->P()).normalize();

		Vec2 pAB  = getIntersection(cp0->P(), cp0_u, cp0nn->P(), cp0nn_u);
		Vec2 pMid = getIntersection(cp0->P(), n, cp0nn->P(), nn);

		Vec3 u = (pAB - pMid).normalize();
			
		CorrPtr newcp0 = cp0->getShoot(pMid, u);
		CorrPtr newcp1 = cp0->corr()->getShoot(pMid, u);
			
		if (newcp0)	
			newcp0->setCorr(newcp1);
		cp0 = newcp0;			
	}
}

void C2S::growCaps(){

	for(list<CorrPtr>::iterator it = cps.begin(); it!=cps.end(); it++){
		CorrPtr cp0 = (*it); 
		for(CorrPtr cp = cp0; cp; cp = (CorrPtr)cp->next() ){
			
			if (cp->corr() && (cp->nextCorr() == cp->corr() || (!cp->nextCorr() && !cp->corr()->prevCorr())) ){
				
				CorrPtr cpc = cp->corr();
				int n = cp->find(cpc);
				while(n>8){
					cp = (CorrPtr)cp->go(4);
					cpc = (CorrPtr)cpc->go(-4);
					cp->setCorr(cpc);
					n-=8;
				}
			}
			if (cp->next() == cp0)
				break;
		}
	}
}

/* ** ** ** ** ** DEPRECATED STUFF
void C2S::buildRings(){

	CorrPtr cp = cps.front();

	if (!cp->corr())
		cp = cp->nextCorr();

	RingPtr rpre = 0;

	for( ; cp; cp = cp->nextCorr()){
		if (cp->getFlag(0))
			continue;
		RingPtr rng = new Ring(cp);
		if(rpre)
			rng->setPrevNext(rpre,0);
		else 
			_rings.push_back(rng);
		rpre = rng;
		//visit
		cp->setFlag(0);
		cp->corr()->setFlag(0);

	}
}

void C2S::rebuildRingsByPerspective(){

	double rads[100];
	int ri = 0;
	double radmax =0;
	for(RingPtr r = _ring; r; r = r->next()){
		double rad = r->getNiceRadius();
		rads[ri++] = rad;
		if (radmax < rad)
			radmax = rad;
	}		
	ri =0;
	for(RingPtr r = _ring; r; r = r->next()){
		double k = rads[ri++] / radmax;
		double newr = sqrt(k) * radmax;
		r->setRadius(newr);//newr);
	}

	//now reorient
	for(RingPtr r = _ring; r; r = r->next()){
		Vec3 n = r->next()?r->next()->P():r->P();
		n = r->prev()?(n-r->prev()->P()):(n-r->P());
		r->reorient(n.normalize());
	}
}
/*
void C2S::buildSpine(){

	double rad[100];
	CorrPtr corrs[100];
	CorrPtr cp = cps.front();
	if (!cp->corr())
		cp = cp->nextCorr();
	int id =0;
	double radmax =0;
	for( ; cp && !cp->isSplit(); cp = cp->nextCorr()){
		if (cp->getFlag(0))
			continue;
		double r = ((cp->P()-cp->corr()->P())%(Eye::get()->P*2.0-cp->P()-cp->corr()->P()).normalize()).norm()/2.0;
		rad[id] = r;
		if (r>radmax)
			radmax = r;
		corrs[id] = cp;
		id++;
		//visit
		cp->setFlag(0);
		cp->corr()->setFlag(0);
	}

	for(int i=0; i<id; i++){
		double k = rad[i] / radmax;
		rad[i] = sqrt(k) * radmax;
	}

	Vec3 * pts = new Vec3[id];
	double dmax = 0;
	for(int i=0; i<id; i++){
		Vec3 o = C2S::getOrgbyRad(corrs[i], rad[i]);
		double d = o.norm();
		if (d>dmax)
			dmax = d;
		pts[i] = o;
	}

	//compress in z
	for(int i=0; i<id; i++){
		 Vec3 o = pts[i];
		 double d = o.norm();
		 double a = PZ / (o.normalize()*Eye::get()->N);
		 double t = ((d-a)<0?0:(d-a))*ZCOMPRESS;
		 o = o.normalize()*a;//(a+t);
		 pts[i] = o;
	}

	ArrCurve *ac = new ArrCurve(pts, id,true);
	ac->drawAux(true);
	_zcurve = ac;

}

void C2S::filterSpine(){
	for (list<Curve*>::iterator it = _spine.begin(); it != _spine.end(); it++)
		filterSpine((*it));
}
void C2S::filterSpine(Curve* sp){
	ArrCurve* ac = ((ArrCurve*)sp);
	ac->movingAverage(5, true);
	ac->resample();
}

list<Curve*> C2S::buildSpine(){

	list<Curve*> spine;

	double rad[100];
	CorrPtr corrs[100];
	int id =0;
	double radmax =0;

	for(list<CorrPtr>::iterator it = cps.begin(); it!=cps.end(); it++){
		CorrPtr cp = (*it);
		if (!cp->corr())
			cp = cp->nextCorr();
		for( ; cp; cp = cp->nextCorr()){
			if (cp->getFlag(0))
				continue;
			double r = ((cp->P()-cp->corr()->P())%(Eye::get()->P*2.0-cp->P()-cp->corr()->P()).normalize()).norm()/2.0;//nice radious
			rad[id] = r;
			if (r>radmax)
				radmax = r;
			corrs[id] = cp;
			cp->storeD(rad[id]);
			id++;
			//visit
			cp->setFlag(0);
			cp->corr()->setFlag(0);
		}
	}
	//normalize by max radious
	for(int i=0; i<id; i++){
		double k = rad[i]*0.94+radmax*0.06;
		corrs[i]->storeD(k);
	}

	//begin traversal
	list<CorrPtr> que;
	CorrPtr cp = cps.front();
	if (!cp->corr())
		cp = cp->nextCorr();
	que.push_back(cp);
	while(!que.empty()){
		CorrPtr cpend = 0;
		CorrPtr cp0 = que.front();
		que.pop_front();
		ArrCurve* ac = getSpine(cp0, cpend);
		NURBS * nc = NURBS::create(ac);

		spine.push_back(nc);
		cp0->setSpine(nc, cpend);

		if (cpend && cpend->isSplit()){
			for(CorrPtr cpi = cpend->nextCorr(); cpi && cpi!=cpend && cpi!=cpend->corr(); cpi = cpi->corr()->nextCorr())
				que.push_back(cpi);
		}
	}
	return spine;
}
*/
