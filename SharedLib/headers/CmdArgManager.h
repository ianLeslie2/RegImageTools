#pragma once
#include "global.h"
#include <utility>
#include <string>
#include <map>
#include <list>

using namespace std;

class CmdArgManager{
	private:
		map<string,tuple<int,void*>> _argMap;
		list<tuple<string,tuple<int,void*>>> _argList;
		list<tuple<string,void*>> _flagSet;
		
		tuple<int,void*> _getArgTuple(char* name);
		void _processArg(tuple<int,void*> entry, char* argS);
		void _processArgs(int argc, char** argv);
		
	public:
		
		CmdArgManager();

		void defIntArg(char const* name, int* varRef);
		void defFloatArg(char const* name, float* varRef);
		void defStringArg(char const* name, char** varRef);

		void defIntArg(char const* name, int* varRef, bool positional);
		void defFloatArg(char const* name, float* varRef, bool positional);
		void defStringArg(char const* name, char** varRef, bool positional);
		void defBoolArg(char const* name, bool* varRef);
		
		bool processArgs(int argc, char **argv);
		void showHelp(char** argv);
		
};