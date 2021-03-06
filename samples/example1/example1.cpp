#include "muParserTest.h"

/** \brief This macro will enable mathematical constants like M_PI. */
#define _USE_MATH_DEFINES		

#include <cmath>
#include <string>
#include <iostream>
#include <locale>
#include <limits>
#include <ios> 
#include <iomanip>
#include <numeric>

#include "muParser.h"

using namespace std;
using namespace mp;
using namespace mp::details;


//---------------------------------------------------------------------------
void Ping(double* arg, int)
{
	cout << "ping\n";
	arg[0] = 0;
}

//---------------------------------------------------------------------------
void Debug(double* arg, int /*argc*/)
{
	double v1 = arg[0];
	double v2 = arg[1];
	ParserBase<double, string>::EnableDebugDump(v1 != 0, v2 != 0);
	cout << "Bytecode dumping " << ((v1 != 0) ? "active" : "inactive") << "\n";
	arg[0] = 1;
}

//---------------------------------------------------------------------------
// Factory function for creating new parser variables
// This could as well be a function performing database queries.
double* AddVariable(const char* a_szName, void* a_pUserData)
{
	// I don't want dynamic allocation here, so i used this static buffer
	// If you want dynamic allocation you must allocate all variables dynamically
	// in order to delete them later on. Or you find other ways to keep track of 
	// variables that have been created implicitely.
	static double afValBuf[100];
	static int iVal = 0;

	std::cout << "Generating new variable \""
			  << a_szName << "\" (slots left: "
		      << 99 - iVal << ")"
		      << " User data pointer is:"
		      << hex << a_pUserData << endl;

	afValBuf[iVal] = 0;
	if (iVal >= 99)
		throw ParserError<string>("Variable buffer overflow.");

	return &afValBuf[iVal++];
}

//---------------------------------------------------------------------------
int IsHexValue(const char* a_szExpr, int* a_iPos, double* a_fVal)
{
	if (a_szExpr[1] == 0 || (a_szExpr[0] != '0' || a_szExpr[1] != 'x'))
		return 0;

	unsigned iVal(0);

	// New code based on streams for UNICODE compliance:
	stringstream::pos_type nPos(0);
	stringstream ss(a_szExpr + 2);
	ss >> hex >> iVal;
	nPos = ss.tellg();

	if (nPos == (stringstream::pos_type)0)
		return 1;

	*a_iPos += (int)(2 + nPos);
	*a_fVal = (double)iVal;
	return 1;
}

//---------------------------------------------------------------------------
void Splash()
{
	std::cout << "                 __________                                       \n";
	std::cout << "    _____   __ __\\______   \\_____  _______  ______  ____ _______\n";
	std::cout << "   /     \\ |  |  \\|     ___/\\__  \\ \\_  __ \\/  ___/_/ __ \\\\_  __ \\ \n";
	std::cout << "  |  Y Y  \\|  |  /|    |     / __ \\_|  | \\/\\___ \\ \\  ___/ |  | \\/ \n";
	std::cout << "  |__|_|  /|____/ |____|    (____  /|__|  /____  > \\___  >|__|    \n";
	std::cout << "        \\/                       \\/            \\/      \\/         \n";
	std::cout << "  Version " << Parser<double>().GetVersion(pviFULL) << "\n";
	std::cout << "  (C) 2020 Ingo Berg\n";
}

//---------------------------------------------------------------------------
void SelfTest(double* /*arg*/, int /*argc*/)
{
	cout << "-----------------------------------------------------------\n";
	Test::ParserTester<double, wstring> pt;
	pt.Run();
}

//---------------------------------------------------------------------------
void Help(double*, int /*argc*/)
{
	cout << "-----------------------------------------------------------\n";
	cout << "Commands:\n\n";
	cout << "  list var     - list parser variables\n";
	cout << "  list exprvar - list expression variables\n";
	cout << "  list const   - list all numeric parser constants\n";
	cout << "  quit         - exits the parser\n";
	cout << "\nConstants:\n\n";
	cout << "  \"_e\"   2.718281828459045235360287\n";
	cout << "  \"_pi\"  3.141592653589793238462643\n";
	cout << "-----------------------------------------------------------\n";
}

//---------------------------------------------------------------------------
void ListVar(const ParserBase<double, string>& parser)
{
	// Query the used variables (must be done after calc)
	map<string, double*> variables = parser.GetVar();
	if (!variables.size())
		return;

	std::cout << "\nParser variables:\n";
	std::cout << "-----------------\n";
	std::cout << "Number: " << (int)variables.size() << "\n";
	auto item = variables.begin();
	for (; item != variables.end(); ++item)
		std::cout << "Name: " << item->first << "   Address: [0x" << item->second << "]\n";
}

//---------------------------------------------------------------------------
void ListConst(const ParserBase<double, string>& parser)
{
	std::cout << "\nParser constants:\n";
	std::cout << "-----------------\n";

	map<string, double> cmap = parser.GetConst();
	if (!cmap.size())
	{
		std::cout << "Expression does not contain constants\n";
	}
	else
	{
		auto item = cmap.begin();
		for (; item != cmap.end(); ++item)
			cout << "  " << item->first << " =  " << item->second << "\n";
	}
}

//---------------------------------------------------------------------------
void ListExprVar(const ParserBase<double, string>& parser)
{
	string sExpr = parser.GetExpr();
	if (sExpr.length() == 0)
	{
		std::cout << "Expression string is empty\n";
		return;
	}

	// Query the used variables (must be done after calc)
	std::cout << "\nExpression variables:\n";
	std::cout << "---------------------\n";
	std::cout << "Expression: " << parser.GetExpr() << "\n";

	map<string, double*> variables = parser.GetUsedVar();
	if (!variables.size())
	{
		std::cout << "Expression does not contain variables\n";
	}
	else
	{
		std::cout << "Number: " << (int)variables.size() << "\n";
		auto item = variables.begin();
		for (; item != variables.end(); ++item)
			std::cout << "Name: " << item->first << "   Address: [0x" << item->second << "]\n";
	}
}

//---------------------------------------------------------------------------
int CheckKeywords(const char* a_szLine, Parser<double>& a_Parser)
{
	string sLine(a_szLine);

	if (sLine == "quit")
	{
		return -1;
	}
	else if (sLine == "list var")
	{
		ListVar(a_Parser);
		return 1;
	}
	else if (sLine == "list const")
	{
		ListConst(a_Parser);
		return 1;
	}
	else if (sLine == "list exprvar")
	{
		ListExprVar(a_Parser);
		return 1;
	}
	else if (sLine == "list const")
	{
		ListConst(a_Parser);
		return 1;
	}

	return 0;
}

//---------------------------------------------------------------------------
void CalcInt()
{
	Parser<int>  parser;

	// Add some variables
	int  vVarVal[] = { 111, 222 };            // Values of the parser variables
	parser.DefineVar("a", &vVarVal[0]);  // Assign Variable names and bind them to the C++ variables
	parser.DefineVar("b", &vVarVal[1]);

#ifdef _DEBUG
	parser.EnableDebugDump(1, 0);
#endif

	for (;;)
	{
		try
		{
			string sLine;
			getline(cin, sLine);
			/*
				  switch (CheckKeywords(sLine.c_str(), parser))
				  {
				  case  0: break;
				  case  1: continue;
				  case -1: return;
				  }
			*/

			if (!sLine.length())
				continue;

			parser.SetExpr(sLine);
			std::cout << setprecision(12);

			// There are multiple ways to retrieve the result...
			// 1.) If you know there is only a single return value or in case you only need the last 
			//     result of an expression consisting of comma separated subexpressions you can 
			//     simply use: 
			std::cout << "ans=" << parser.Eval() << "\n";
		}
		catch (ParserError<string>& e)
		{
			std::cout << "\nError:\n";
			std::cout << "------\n";
			std::cout << "Message:     " << e.GetMsg() << "\n";
			std::cout << "Expression:  \"" << e.GetExpr() << "\"\n";
			std::cout << "Token:       \"" << e.GetToken() << "\"\n";
			std::cout << "Position:    " << (int)e.GetPos() << "\n";
			std::cout << "Errc:        " << dec << e.GetCode() << "\n";
		}
	} // while running
}

//---------------------------------------------------------------------------
void Calc()
{
	Parser<double>  parser;

	// Add some variables
	double  vVarVal[] = { 1, 2 };            // Values of the parser variables
	parser.DefineVar("a", &vVarVal[0]);  // Assign Variable names and bind them to the C++ variables
	parser.DefineVar("b", &vVarVal[1]);
	parser.AddValIdent(IsHexValue);

	// Add user defined unary operators
	parser.DefineFun("ping", Ping, 0);
	parser.DefinePostfixOprt("{m}", Ping);

	// These are service and debug functions
	parser.DefineFun("debug", Debug, 2);
	parser.DefineFun("selftest", SelfTest, 0);
	parser.DefineFun("help", Help, 0);

#ifdef _DEBUG
	parser.EnableDebugDump(1, 0);
#endif

	// Define the variable factory
	parser.SetVarFactory(AddVariable, &parser);

	for (;;)
	{
		try
		{
			string sLine;
			getline(cin, sLine);

			switch (CheckKeywords(sLine.c_str(), parser))
			{
			case  0: break;
			case  1: continue;
			case -1: return;
			}

			if (!sLine.length())
				continue;

			//sLine = "1 - ((a * b) + (a / b)) - 3";
			parser.SetExpr(sLine);
			cout << setprecision(12);

			// There are multiple ways to retrieve the result...
			// 1.) If you know there is only a single return value or in case you only need the last 
			//     result of an expression consisting of comma separated subexpressions you can 
			//     simply use: 
			cout << "ans=" << parser.Eval() << "\n";
		}
		catch (ParserError<string>& e)
		{
			cout << "\nError:\n";
			cout << "------\n";
			cout << "Message:     " << e.GetMsg() << "\n";
			cout << "Expression:  \"" << e.GetExpr() << "\"\n";
			cout << "Token:       \"" << e.GetToken() << "\"\n";
			cout << "Position:    " << (int)e.GetPos() << "\n";
			cout << "Errc:        " << dec << e.GetCode() << "\n";
		}
	} // while running
}

//---------------------------------------------------------------------------
int main(int, char**)
{
	Splash();

	SelfTest(nullptr, 0);
	Help(nullptr, 0);

	cout << "Enter an expression or a command:\n";

	try
	{
		//    CalcInt();
		Calc();
	}
	catch (ParserError<string>& e)
	{
		// Only erros raised during the initialization will end up here
		// formula related errors are treated in Calc()
		cout << "Initialization error:  " << e.GetMsg() << endl;
		cout << "aborting..." << endl;
		string sBuf;
		cin >> sBuf;
	}
	catch (exception& /*exc*/)
	{
		// there is no unicode compliant way to query exc.what()
		// so i'll leave it for this example.
		cout << "aborting...\n";
	}

	return 0;
}
