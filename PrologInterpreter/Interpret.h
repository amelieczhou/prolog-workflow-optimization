#include<vector>
#include<list>

#include "term.h"


//unify function
bool Unify(Term* x, Term* y);
//bool Unify(Constant* x, Constant* y);
//bool Unify(Constant* x, Variable* y);
//bool Unify(Variable* x, Variable* y);
void findall(Term* x, Term* y, std::vector<Term*> Bag);
void setof(std::pair<Term*, Term*> x, Term* y, std::vector<std::pair<Term*,Term*> > z);
void PrologEval(std::vector<RuleList> rs, RuleList q);
