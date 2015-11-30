
#include "stdafx.h"
#include "Interpret.h"


/*
	findall : Clocksin, Mellish PROLOG implementation

findall(X,G,_):-
	asserta(found('$mark')), 
	call(G), 
	asserta(found(X)), 
	fail.
findall(_,_,N) :-
	collect_found([],N), 
collect_found(S,L) :-
	getnext(X), 
	!, 
	collect_found([X|S],L).
collect_found(L,L).
getnext(X) :-
	retract(found(X)), 
	!, 
	X \= '$mark'.
*/
//int IntlogExec::findall(Term X, Term G, Term L)
//{
//	Term cG = copy(G),
//		 N = ListNULL, V;
//
//	OffVars oG(cG);
//
//	DbIntlog *dbc = ps->topget()->call->get_clause()->get_db();
//	if (dbc == 0)
//		dbc = db;
//
//	Clause c(cG, 0, dbc);
//	c.set_nvars(oG.nvars());
//
//	// correct immediate assert() behaviour,
//	//	due to test of src in Clause::Clause, cyclic reference error
//	c.set_source(kstring("findall"));
//
//	stkpos	pX = ps->topget()->vspos,
//			tc = ps->curr_dim(),
//			tp = ts->curr_dim();
//
//	// count new variables required
//	unsigned nnv = 0, offv = vs->curr_dim() - pX;
//
//	int qr = query(&c);
//	if (qr == 1)
//	{      
//		int qp = ts->curr_dim();
//		for (Var v = 0; v < oG.nvars(); v++)
//		{
//			Var bv = oG.new2old(Var(v));
//			ts->bind(bv);
//			vs->setshare(bv, ps->get(tc)->vspos + v);
//		}
//
//		V = copy_term(X, pX);
//		unbind(qp);
//
//		OffVars oV(V, offv);
//		nnv += oV.nvars();
//
//		Term S(V, N);
//		N = S;
//
//		while ((qr = query()) == 1)
//		{
//			qp = ts->curr_dim();
//			for (Var v = 0; v < oG.nvars(); v++)
//			{
//				Var bv = oG.new2old(Var(v));
//				ts->bind(bv);
//				vs->setshare(bv, ps->get(tc)->vspos + v);
//			}
//
//			V = copy_term(X, pX);
//			unbind(qp);
//
//			oV.SetTerm(V, offv + nnv);
//			nnv += oV.nvars();
//
//			S.setarg(1, Term(V, Term(ListNULL)));
//			S = S.getarg(1);
//		}
//	}
//
//	if (qr == -1)
//		return 0;
//
//	unbind(tp);
//	ps->pop();	// release dummy top node
//
//	// allocate space to new vars
//	vs->reserve(nnv);
//
//	UnifyStack us(vs, ts);
//	return us.work(L, pX, save(N), pX);
//}