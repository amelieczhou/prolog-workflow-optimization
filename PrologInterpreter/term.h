#pragma once
////////////////////////////////////////
// main data structure
//	mimic the recursive nature of TERMs
////////////////////////////////////////

//a term of a rule
class Term;
class RuleList;
class Struct;

typedef char*			Atom;
typedef short			Int;
typedef double			Double;
typedef unsigned		Var;
typedef unsigned long   TermData;

const TermData
#if BUILD_64
    f_VAR		= 0x8000000000000000,
    f_STRUCT	= 0x4000000000000000,
    f_INT		= 0x2000000000000000,
    f_ATOM		= 0x1000000000000000,
    f_DOUBLE	= 0x0800000000000000,
    f_LIST		= 0x0400000000000000,
    f_SYSDATA	= 0x0200000000000000,
    f_NOTERM	= 0x0100000000000000,
    MaskFlags	= 0xFF00000000000000,
#else
	f_VAR		= 0x80000000,
	f_STRUCT	= 0x40000000,
	f_INT		= 0x20000000,
	f_ATOM		= 0x10000000,
	f_DOUBLE	= 0x08000000,
	f_LIST		= 0x04000000,
	f_SYSDATA	= 0x02000000,
	f_NOTERM	= 0x01000000,
	MaskFlags	= 0xFF000000,
#endif
    ListNULL	= f_LIST|~MaskFlags;
///////////////
// term value
//

//////////////////////////////
// structure
//	functor and argument list
//	contiguous memory layout
//
class Term
{

public:

	Term();				// dummy (random init)
	Term(Atom);			// kstring selector
	Term(Int);			// short integer
	Term(Var);			// variable offset
	Term(Double);		// float number

	Term(Atom, int);	// struct (add args later)
	Term(Term, Term);	// list: head and tail


	void NoTerm();		// set to empty or invalid

	void Destroy();		// recursive destructor


	int		is_query() const;		// true if operator '?-'
	int		is_rule() const;		// true if operator ':-'
	int		is_or() const;			// true if operator ';'
	int		is_and() const;			// true if operator ','
	int		is_command() const;		// true if unary operator ':-'

	int		is_cut() const;			// true if atom '!'
	int		LNULL() const;			// true if NULL list (only!)

	Atom	get_funct() const;		// return functor
	int		get_arity() const;		// num of args (0..N)

	Term	LTerm() const;			// left term of expression
	Term	RTerm() const;			// right term of expression

	Term	getarg(int) const;		// return required arg or 0
	void	setarg(int, Term);		// change required argument

	static void FreeMem();			// static memory release

private:

	TermData m_data;

	unsigned ivalue() const;		// return the value part

	static int useOpDecl;			// output flag


};

class RuleList{
public:
	Term head() const; 
	Term tail() const;
private:
	friend class Term;
	TermData h,t;
};

class Struct{
private:
	char* funct;
	int arity;

	// Term args[arity] : a vector of variable dimension

	#define TermArgsVect(sptr)	((Term*)((char*)sptr + sizeof(Struct)))
        friend class Term;
};
//class Term{
//public:
//	int tag;//0:constant;1:variable;
//	char* name;
//	std::vector<char*> cpar;
//	std::vector<float> fpar;
//};
//class Variable: public Term{
//public:
//	Variable(char* n, std::vector<char*> p) {
//		name = n;
//		cpar = p;
//		tag = 1;
//	}
//	Variable(char* n){
//		name = n;
//		tag = 1;
//	}
//};
//class Constant:public Term{
//public:	
//	Constant(char* n, std::vector<float> p) {
//		name = n;
//		fpar = p;
//		tag = 0;
//	}
//};
//rule list
