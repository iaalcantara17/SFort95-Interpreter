/* Implementation of Recursive-Descent Parser
 * for the SFort95 Language
 * parser(SP24).cpp
 * Programming Assignment 2
 * Spring 2024
 * Israel Alcantara
*/

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <queue>
#include <stack>

#include "parserInterp.h"

map<string, bool> defVar;
map<string, Token> SymTable;

map<string, Value> TempsResults;
queue <Value>* ValQue;
string Len = "1";

namespace Parser 
{
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) 
    {
		if (pushed_back) 
        {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem& t) 
    {
		if (pushed_back) 
        {
			abort();
		}
		pushed_back = true;
		pushed_token = t;
	}

}

static int error_count = 0;

int ErrCount()
{
	return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}

bool IdentList(istream& in, int& line);


bool Prog(istream& in, int& line)
{
	bool dl = false, sl = false;
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == PROGRAM) 
    {
		tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == IDENT) 
        {
			dl = Decl(in, line);

			if (!dl)
			{
				ParseError(line, "Incorrect Decl in Prog");
				return false;
			}
			sl = Stmt(in, line);

			if (!sl)
			{
				ParseError(line, "Incorrect Stmt in Prog");
				return false;
			}
			tok = Parser::GetNextToken(in, line);

			if (tok.GetToken() == END) 
            {
				tok = Parser::GetNextToken(in, line);

				if (tok.GetToken() == PROGRAM) 
                {
					tok = Parser::GetNextToken(in, line);

					if (tok.GetToken() == IDENT) 
                    {
						cout << "(DONE)" << endl;
						return true;
					}

					else
					{
						ParseError(line, "Missing PROG");
						return false;
					}
				}

				else
				{
					ParseError(line, "Missing PROG at the End");
					return false;
				}
			}

			else
			{
				ParseError(line, "Missing END of PROG");
				return false;
			}
		}

		else
		{
			ParseError(line, "Missing PROG");
			return false;
		}
	}

	else if (tok.GetToken() == ERR) {
		ParseError(line, "Unrecognized Input");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	else
	{
		ParseError(line, "Missing PROG");
		return false;
	}
}

bool Decl(istream& in, int& line) 
{
	bool status = false;
	LexItem tok;

	LexItem t = Parser::GetNextToken(in, line);

	if (t == INTEGER || t == REAL || t == CHARACTER) 
    {
		tok = t;

		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == DCOLON) 
        {
			status = VarList(in, line, t);

			if (status)
			{
				status = Decl(in, line);
				if (!status)
				{
					ParseError(line, "Declaration Error.");
					return false;
				}
				return status;
			}

			else
			{
				ParseError(line, "Missing VarList.");
				return false;
			}
		}
		else if (t == CHARACTER && tok.GetToken() == LPAREN)
		{
			tok = Parser::GetNextToken(in, line);

			if (tok.GetToken() == LEN)
			{
				tok = Parser::GetNextToken(in, line);

				if (tok.GetToken() == ASSOP)
				{
					tok = Parser::GetNextToken(in, line);

					if (tok.GetToken() == ICONST)
					{
						Len = tok.GetLexeme();

						tok = Parser::GetNextToken(in, line);

						if (tok.GetToken() == RPAREN)
						{
							tok = Parser::GetNextToken(in, line);

							if (tok.GetToken() == DCOLON)
							{
								status = VarList(in, line, t, stoi(Len));

								if (status)
								{
									status = Decl(in, line);

									if (!status)
									{
										ParseError(line, "Declaration Error.");
										return false;
									}
									return status;
								}

								else
								{
									ParseError(line, "Missing VarList.");
									return false;
								}
							}
						}

						else
						{
							ParseError(line, "Missing Right Parenthesis for String Len Def.");
							return false;
						}
					}

					else
					{
						ParseError(line, "Incorrect Initialization of String Len");
						return false;
					}
				}
			}
		}

		else
		{
			ParseError(line, "Missing DCOLON");
			return false;
		}

	}

	Parser::PushBackToken(t);
	return true;
}

bool Stmt(istream& in, int& line) 
{
	bool status;

	LexItem t = Parser::GetNextToken(in, line);

	switch (t.GetToken()) 
    {
	case PRINT:
		status = PrintStmt(in, line);

		if (status)
			status = Stmt(in, line);
		break;

	case IF:
		status = BlockIfStmt(in, line);

		if (status)
			status = Stmt(in, line);
		break;

	case IDENT:
		Parser::PushBackToken(t);
		status = AssignStmt(in, line);

		if (status)
			status = Stmt(in, line);
		break;


	default:
		Parser::PushBackToken(t);
		return true;
	}

	return status;
}

bool SimpleStmt(istream& in, int& line) 
{
	bool status;

	LexItem t = Parser::GetNextToken(in, line);

	switch (t.GetToken()) 
    {
	case PRINT:
		status = PrintStmt(in, line);

		if (!status)
		{
			ParseError(line, "Incorrect PrintStmt");
			return false;
		}
		break;

	case IDENT:
		Parser::PushBackToken(t);
		status = AssignStmt(in, line);

		if (!status)
		{
			ParseError(line, "Incorrect AssignStmt");
			return false;
		}

		break;


	default:
		Parser::PushBackToken(t);
		return true;
	}

	return status;
}

bool VarList(istream& in, int& line, LexItem& idtok, int strlen) 
{
	bool status = false, exprstatus = false;
	string identstr;
	Value retVal;
	strlen = stoi(Len);
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == IDENT) 
    {
		identstr = tok.GetLexeme();

		if (!(defVar.find(identstr)->second)) 
        {
			defVar[identstr] = true;
		}

		else 
        {
			ParseError(line, "Var Redef");
			return false;
		}

		SymTable[tok.GetLexeme()] = idtok.GetToken();

		if (SymTable[tok.GetLexeme()] == CHARACTER) 
        {
			string str(strlen, ' ');
			retVal = Value(str);
			TempsResults[tok.GetLexeme()] = retVal;
		}
	}

	else 
    {
		ParseError(line, "Missing Var Name");
		return false;
	}

	tok = Parser::GetNextToken(in, line);

	if (tok == ASSOP) 
    {
		exprstatus = Expr(in, line, retVal);

		if (!exprstatus) 
        {
			ParseError(line, "Incorrect initialization for var.");
			return false;
		}

		if (idtok == REAL) 
        {
			if (retVal.GetType() == VINT) 
            {
				retVal = double(retVal.GetInt());
			}
		}

		if (idtok == CHARACTER) 
        {
			string str = retVal.GetString();
			int spaces = strlen - str.length();

			if (str.length() > strlen) 
            {
				retVal = Value(str.substr(0, strlen));
			}

			else 
            {
				retVal = Value(str + string(spaces, ' '));
			}
		}

		TempsResults[identstr] = retVal;
		tok = Parser::GetNextToken(in, line);

		if (tok == COMMA) 
        {
			status = VarList(in, line, idtok, strlen);
		}

		else 
        {
			Parser::PushBackToken(tok);
			return true;
		}
	}

	else if (tok == COMMA) 
    {
		status = VarList(in, line, idtok, strlen);
	}

	else if (tok == ERR) 
    {
		ParseError(line, "Unrecognized Input");
		return false;
	}

	else 
    {
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

bool PrintStmt(istream& in, int& line) 
{
	LexItem t;
	ValQue = new queue<Value>;

	t = Parser::GetNextToken(in, line);

	if (t != DEF) 
    {
		ParseError(line, "Print statement error.");
		return false;
	}
	t = Parser::GetNextToken(in, line);

	if (t != COMMA) 
    {
		ParseError(line, "Missing Comma.");
		return false;
	}

	bool ex = ExprList(in, line);

	if (!ex) 
    {
		ParseError(line, "Missing Expr after PrintStmt");
		return false;
	}

	while (!(*ValQue).empty()) 
    {
		Value next = (*ValQue).front();
		cout << next;
		ValQue->pop();
	}
	cout << endl;
	return ex;
}

bool BlockIfStmt(istream& in, int& line) 
{
	bool expr = false, status;
	LexItem t;

	t = Parser::GetNextToken(in, line);

	if (t != LPAREN) 
    {
		ParseError(line, "Missing LPAREN");
		return false;
	}

	Value retVal;
	expr = RelExpr(in, line, retVal);

	if (!expr) 
    {
		ParseError(line, "Missing RelExpr");
		return false;
	}

	if (retVal.GetType() == VERR) 
    {
		ParseError(line, "Illegal operand types for a RelExpr.");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t != RPAREN) 
    {
		ParseError(line, "Missing RPAREN");
		return false;
	}

	if (retVal.GetBool() == 0) 
    {
		while (t != ELSE && t != END) 
        {
			t = Parser::GetNextToken(in, line);
		}

		status = Stmt(in, line);

		if (!status) 
        {
			ParseError(line, "Missing Statement for If-Stmt Else");
			return false;
		}

		else
			t = Parser::GetNextToken(in, line);

		if (t != END) 
        {
			ParseError(line, "Missing END of IF");
			return false;
		}

		t = Parser::GetNextToken(in, line);

		if (t == IF) 
        {
			return true;
		}

		Parser::PushBackToken(t);
		ParseError(line, "Missing IF at End of IF stmt");
		return false;
	}

	t = Parser::GetNextToken(in, line);

	if (t != THEN)
    {
		Parser::PushBackToken(t);
		status = SimpleStmt(in, line);

		if (status) 
        {
			return true;
		}

		else 
        {
			ParseError(line, "If-Stmt Error");
			return false;
		}
	}

	status = Stmt(in, line);

	if (!status) 
    {
		ParseError(line, "Missing Statement for If-Stmt Then");
		return false;
	}

	t = Parser::GetNextToken(in, line);

	if (t == ELSE) 
    {
		status = Stmt(in, line);

		if (!status) 
        {
			ParseError(line, "Missing Statement for If-Stmt Else");
			return false;
		}

		else
			t = Parser::GetNextToken(in, line);
	}

	if (t != END) 
    {
		ParseError(line, "Missing END of IF");
		return false;
	}

	t = Parser::GetNextToken(in, line);

	if (t == IF) 
    {
		return true;
	}

	Parser::PushBackToken(t);
	ParseError(line, "Missing IF at End of IF");
	return false;
}

bool Var(istream& in, int& line, LexItem& idtok) 
{
	string identstr;
	LexItem tok = Parser::GetNextToken(in, line);
	idtok = tok;

	if (tok == IDENT) 
    {
		identstr = tok.GetLexeme();

		if (!(defVar.find(identstr)->second)) 
        {
			ParseError(line, "Undecl Var");
			return false;
		}
		return true;
	}

	else if (tok.GetToken() == ERR) 
    {
		ParseError(line, "Unrecognized Input");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}

bool AssignStmt(istream& in, int& line) 
{
	bool varstatus = false, status = false;
	LexItem t;
	int aline = line;
	varstatus = Var(in, line, t);

	if (varstatus) 
    {
		LexItem tok = Parser::GetNextToken(in, line);

		if (tok == ASSOP) 
        {
			Value retVal;
			status = Expr(in, line, retVal);

			if (!status) 
            {
				ParseError(line, "Missing Expr in AssignStmt");
				return status;
			}

			if (SymTable[t.GetLexeme()] == CHARACTER) 
            {
				if (retVal.GetType() != VSTRING) 
                {
					ParseError(aline, "Illegal mixed-mode assignment op");
					return false;
				}
			}

			if (SymTable[t.GetLexeme()] == CHARACTER) 
            {
				string str = retVal.GetString();
				int spaceNeeded = stoi(Len) - str.length();

				if (str.length() > stoi(Len)) 
                {
					retVal = Value(str.substr(0, stoi(Len)));
				}

				else 
                {
					retVal = Value(str + string(spaceNeeded, ' '));
				}
			}

			TempsResults[t.GetLexeme()] = retVal;

			if (TempsResults[t.GetLexeme()].GetType() == VERR) 
            {
				ParseError(aline, "Illegal operand type for operation.");
				return false;
			}
		}

		else if (tok.GetToken() == ERR) 
        {
			ParseError(line, "Unrecognized Input");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}

		else 
        {
			ParseError(line, "Missing ASSOP");
			return false;
		}
	}

	else 
    {
		ParseError(line, "Missing LH Side Var in AssignStmt");
		return false;
	}
	return status;
}

bool ExprList(istream& in, int& line) 
{
	bool status = false;

	Value retVal;
	status = Expr(in, line, retVal);

	if (!status) 
    {
		ParseError(line, "Missing Expr");
		return false;
	}

	ValQue->push(retVal);
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == COMMA) 
    {
		status = ExprList(in, line);
	}

	else if (tok.GetToken() == ERR) 
    {
		ParseError(line, "Unrecognized Input");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	else 
    {
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

bool RelExpr(istream& in, int& line, Value& retVal) 
{
	bool t1 = Expr(in, line, retVal);
	LexItem tok;

	if (!t1) 
    {
		return false;
	}

	tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == ERR) 
    {
		ParseError(line, "Unrecognized Input");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	if (tok == EQ || tok == LTHAN || tok == GTHAN)
	{
		Value v;
		t1 = Expr(in, line, v);

		if (!t1)
		{
			ParseError(line, "Missing operand after op");
			return false;
		}

		if (tok == EQ) 
        {
			retVal = retVal == v;
		}

		else if (tok == LTHAN) 
        {
			retVal = retVal < v;
		}

		else 
        {
			retVal = retVal > v;
		}

	}
    return true;
}

bool Expr(istream& in, int& line, Value& retVal) 
{
	bool t1 = MultExpr(in, line, retVal);
	LexItem tok;

	if (!t1) 
    {
		return false;
	}

	tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == ERR) 
    {
		ParseError(line, "Unrecognized Input");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	while (tok == PLUS || tok == MINUS || tok == CAT)
	{
		Value v;
		t1 = MultExpr(in, line, v);

		if (!t1)
		{
			ParseError(line, "Missing operand after op");
			return false;
		}

		if (tok == PLUS) 
        {
			retVal = retVal + v;
		}

		else if (tok == MINUS) 
        {
			retVal = retVal - v;
		}

		else 
        {
			retVal = retVal.Catenate(v);
		}

		tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == ERR) 
        {
			ParseError(line, "Unrecognized Input");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
	}
	Parser::PushBackToken(tok);
	return true;
}

bool MultExpr(istream& in, int& line, Value& retVal) 
{
	bool t1 = TermExpr(in, line, retVal);
	LexItem tok;

	if (!t1) 
    {
		return false;
	}

	tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == ERR) 
    {
		ParseError(line, "Unrecognized Input");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	while (tok == MULT || tok == DIV)
	{
		Value v;
		t1 = TermExpr(in, line, v);

		if (!t1) 
        {
			ParseError(line, "Missing operand after op");
			return false;
		}

		if (tok == MULT) 
        {
			retVal = retVal * v;
		}

		else 
        {
			if (v.IsInt()) 
            {

				if (v.GetInt() == 0) 
                {
					ParseError(line - 1, "NO DIVISION BY 0");
					return false;
				}
			}

			if (v.IsReal()) 
            {

				if (v.GetReal() == 0) 
                {
					ParseError(line - 1, "NO DIVISION BY 0");
					return false;
				}
			}
			retVal = retVal / v;
		}

		tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == ERR) 
        {
			ParseError(line, "Unrecognized Input");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
	}
	Parser::PushBackToken(tok);
	return true;
}

bool TermExpr(istream& in, int& line, Value& retVal) 
{
	stack<Value> power;
	Value pow;
	Value base;
	Value v;

	bool t1 = SFactor(in, line, retVal);
	LexItem tok;

	if (!t1) 
    {
		return false;
	}

	tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == ERR) 
    {
		ParseError(line, "Unrecognized Input");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	power.push(retVal);
	while (tok == POW)
	{
		t1 = SFactor(in, line, v);

		if (!t1) 
        {
			ParseError(line, "Missing ** operand");
			return false;
		}

		power.push(v);

		tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == ERR) 
        {
			ParseError(line, "Unrecognized Input ");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
	}

	while (!power.empty()) 
    {
		if (power.size() == 1) 
        {
			break;
		}
		pow = power.top();
		power.pop();
		base = power.top();
		power.pop();
		retVal = base.Power(pow);
		retVal = power.top().Power(retVal);
	}

	Parser::PushBackToken(tok);
	return true;
}

bool SFactor(istream& in, int& line, Value& retVal)
{
	LexItem t = Parser::GetNextToken(in, line);

	bool status;
	int sign = 0;

	if (t == MINUS)
	{
		sign = -1;
	}

	else if (t == PLUS)
	{
		sign = 1;
	}

	else 
    {
		Parser::PushBackToken(t);
	}

	status = Factor(in, line, sign, retVal);
	return status;
}

bool Factor(istream& in, int& line, int sign, Value& retVal) 
{
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == IDENT) 
    {
		string lexeme = tok.GetLexeme();

		if (!(defVar.find(lexeme)->second)) 
        {
			ParseError(line, "Undef Var");
			return false;
		}

		retVal = TempsResults[lexeme];

		if (TempsResults[lexeme].GetType() == VERR) 
        {
			ParseError(line, "Uninitialized Var");
			return false;
		}

		if (sign == -1) 
        {

			if (retVal.IsInt()) 
            {
				retVal = -retVal.GetInt();
			}

			else if (retVal.IsReal()) 
            {
				retVal = -retVal.GetReal();
			}

			if (retVal.IsString()) 
            {
				ParseError(line, "Illegal Type for Sign");
				return false;
			}
		}
		return true;
	}

	else if (tok == ICONST) 
    {
		retVal.SetType(VINT);

		if (sign == -1) 
        {
			retVal.SetInt(-stoi(tok.GetLexeme()));
		}

		else 
        {
			retVal.SetInt(stoi(tok.GetLexeme()));
		}
		return true;
	}

	else if (tok == SCONST) 
    {
		retVal.SetType(VSTRING);
		retVal.SetString(tok.GetLexeme());
		return true;
	}

	else if (tok == RCONST) 
    {
		retVal.SetType(VREAL);

		if (sign == -1) 
        {
			retVal.SetReal(-stod(tok.GetLexeme()));
		}

		else 
        {
			retVal.SetReal(stod(tok.GetLexeme()));
		}
		return true;
	}

	else if (tok == LPAREN) 
    {
		bool ex = Expr(in, line, retVal);

		if (!ex) 
        {
			ParseError(line, "Missing Expr after (");
			return false;
		}

		if (Parser::GetNextToken(in, line) == RPAREN) 
        {
			return ex;
		}

		else 
        {
			Parser::PushBackToken(tok);
			ParseError(line, "Missing ) after Expr");
			return false;
		}
	}

	else if (tok.GetToken() == ERR) 
    {
		ParseError(line, "Unrecognized Input");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}