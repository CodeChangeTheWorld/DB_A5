
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include <string>
#include <vector>
#include <MyDB_Catalog.h>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
    string type;
	virtual bool validateTree(MyDB_CatalogPtr c)=0;
    virtual string checkType(MyDB_CatalogPtr c)=0;
    virtual bool inGroupBy(MyDB_CatalogPtr c)=0;
	virtual string toString () = 0;
	virtual ~ExprTree () {}
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:

	BoolLiteral (bool fromMe) {
		myVal = fromMe;
        type = "bool";
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}

	bool validateTree(MyDB_CatalogPtr c){
		return true;
	}
    bool inGroupBy(MyDB_CatalogPtr c){
		return true;
	}
    string checkType(MyDB_CatalogPtr c){
        return "bool";
    }
};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
        type = "double";
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}

    bool validateTree(MyDB_CatalogPtr c){
        return true;
    }
    bool inGroupBy(MyDB_CatalogPtr c){
        return true;
    }

    string checkType(MyDB_CatalogPtr c){
        return "double";
    }

	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
        type = "int";
	}


	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

    bool validateTree(MyDB_CatalogPtr c){
        return true;
    }
    bool inGroupBy(MyDB_CatalogPtr c){
        return true;
    }

    string checkType(MyDB_CatalogPtr c){
        return "int";
    }

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
        type = "string";
	}

	string toString () {
		return "string[" + myVal + "]";
	}

    bool validateTree(MyDB_CatalogPtr c){
        return true;
    }
    bool inGroupBy(MyDB_CatalogPtr c){
        return true;
    }

    string checkType(MyDB_CatalogPtr c){
        return "string";
    }

	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
        type = "identifier";
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}

	bool validateTree(MyDB_CatalogPtr c){
		//check if table exist
		if(c->tableIndex(tableName) == -1){
            cout<<"Error: Table '" + tableName + "' doesn't exist" <<endl;
            return false;
        }
        if(c->findAttr(tableName, attName) == false){
            cout<< "Error: Table "<< tableName << " does not have attribute " + attName << "." <<endl;
            return false;
        }
        return true;
	}


    bool inGroupBy(MyDB_CatalogPtr c){
        if(c->inGroupBy(tableName,attName)==-1)
        {
            cout<<"Error: select attribute "<<attName <<" in table "<< tableName <<" are not in group by clause"<<endl;
            return false; // not on the grouping list
        }
        else
            return true;	// on the grouping list
	}

    string checkType(MyDB_CatalogPtr c){
		string type;
        c->getString(c->getFullTableName(tableName)+"."+attName + ".type",type);
        return type;
	}

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
        type = "expression";
	}

	string toString () {
        cout<< "MinusOp is validating..." <<endl;
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~MinusOp () {}

    bool validateTree(MyDB_CatalogPtr c){
       return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("int") == 0 && rtype.compare("int") ==0 ) return "int";
        if((ltype.compare("int")==0 && (rtype.compare("double") == 0 || rtype.compare("int") ==0))
        || (ltype.compare("double")== 0 && (rtype.compare("int")==0 || rtype.compare("double")==0))){
            return "double";
        }
        cout<<"Type Error:"<<ltype << " - " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }

};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~PlusOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("int") == 0 && rtype.compare("int") ==0 ) return "int";
        if(ltype.compare("string")==0 && rtype.compare("string")==0) return "string";
        if((ltype.compare("int")==0 && (rtype.compare("double") == 0 || rtype.compare("int") ==0))
           || (ltype.compare("double")== 0 && (rtype.compare("int")==0 || rtype.compare("double")==0))){
            return "double";
        }

        cout<<"Type Error:"<<ltype << " + " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~TimesOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("int") == 0 && rtype.compare("int") ==0 ) return "int";
        if((ltype.compare("int")==0 && (rtype.compare("double") == 0 || rtype.compare("int") ==0))
           || (ltype.compare("double")== 0 && (rtype.compare("int")==0 || rtype.compare("double")==0))){
            return "double";
        }
        cout<<"Type Error:"<<ltype << " * " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~DivideOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("int") == 0 && rtype.compare("int") ==0 ) return "int";
        if((ltype.compare("int")==0 && (rtype.compare("double") == 0 || rtype.compare("int") ==0))
           || (ltype.compare("double")== 0 && (rtype.compare("int")==0 || rtype.compare("double")==0))){
            return "double";
        }

        cout<<"Type Error:"<<ltype << " / " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
        type = "bool";
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~GtOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("string") == 0 && rtype.compare("string") ==0 ) return "bool";
        if((ltype.compare("int")==0 || ltype.compare("double")== 0)&& (rtype.compare("double") == 0 || rtype.compare("int") ==0)){
            return "bool";
        }


        cout<<"Type Error:"<<ltype << " > " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }

};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~LtOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        cout<< "LtOp is validating..." <<endl;
        return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("string") == 0 && rtype.compare("string") ==0 ) return "bool";
        if((ltype.compare("int")==0 || ltype.compare("double")== 0)&& (rtype.compare("double") == 0 || rtype.compare("int") ==0)){
            return "bool";
        }

        cout<<"Type Error:"<<ltype << " < " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~NeqOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("string") == 0 && rtype.compare("string") ==0 ) return "bool";
        if((ltype.compare("int")==0 || ltype.compare("double")== 0)&& (rtype.compare("double") == 0 || rtype.compare("int") ==0)){
            return "bool";
        }

        cout<<"Type Error:"<<ltype << " != " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~OrOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("bool") == 0 && rtype.compare("bool") ==0 ) return "bool";

        cout<<"Type Error:"<<ltype << " || " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
        type = "bool";
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~EqOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return (lhs->validateTree(c) && rhs->validateTree(c));
    }

    string checkType(MyDB_CatalogPtr c){
        string ltype = lhs->checkType(c), rtype = rhs->checkType(c);
        if(ltype.compare("string") == 0 && rtype.compare("string") ==0 ) return "bool";
        if((ltype.compare("int")==0 || ltype.compare("double")== 0)&& (rtype.compare("double") == 0 || rtype.compare("int") ==0)){
            return "bool";
        }

        cout<<"Type Error:"<<ltype << " == " << rtype << " type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        return lhs->inGroupBy(c) && rhs->inGroupBy(c);
    }
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}	

	~NotOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return child->validateTree(c);
    }

    string checkType(MyDB_CatalogPtr c){
        string ctype = child->checkType(c);
        if(ctype.compare("bool") == 0) return "bool";
        cout<<"Type Error:"<< " !(" << ctype << ") type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        //return child->inGroupBy(c);
        return true;
    }
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}	

	~SumOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return child->validateTree(c);
    }

    string checkType(MyDB_CatalogPtr c){
        string ctype = child->checkType(c);
        if(ctype.compare("int") == 0 || ctype.compare("double") == 0) return "int";
        if(ctype.compare("none")!=0) cout<<"Type Error: Sum() cannot be applied to "<<  ctype << "." <<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        //return child->inGroupBy(c);
        return true;
    }
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}	

	~AvgOp () {}

    bool validateTree(MyDB_CatalogPtr c){
        return child->validateTree(c);
    }

    string checkType(MyDB_CatalogPtr c){
        string ctype = child->checkType(c);
        if(ctype.compare("int") == 0 || ctype.compare("double") == 0) return "double";
        cout<<"Type Error:"<< " avg(" << ctype << ") type does not match."<<endl;
        return "none";
    }

    bool inGroupBy(MyDB_CatalogPtr c){
        //return child->inGroupBy(c);
        return true;
    }
};

#endif
