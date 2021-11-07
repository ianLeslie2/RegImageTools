#include "CmdArgManager.h"
#include <tuple>
#include <regex>
#include "UtilF.h"

using namespace std;

CmdArgManager::CmdArgManager(){
	_argMap = map<string,tuple<int,void*>>();
	_argList = list<tuple<string,tuple<int,void*>>>();
	_flagSet = list<tuple<string,void*>>();
}

void CmdArgManager::defIntArg(char const* name, int* varRef) {
	defIntArg(name, varRef, false);
}
	
void CmdArgManager::defIntArg(char const* name, int* varRef, bool positional){
	auto entry = make_tuple(0,(void*)varRef);
	if(positional){
		_argList.push_back(make_tuple(string(name),entry));
	}
	else{
		_argMap[string(name)] = entry;
	}
}

void CmdArgManager::defFloatArg(char const* name, float* varRef) {
	defFloatArg(name, varRef, false);
}

void CmdArgManager::defFloatArg(char const* name, float* varRef, bool positional){
	auto entry = make_tuple(1,(void*)varRef);
	if(positional){
		_argList.push_back(make_tuple(string(name),entry));
	}
	else{
		_argMap[string(name)] = entry;
	}
}

void CmdArgManager::defStringArg(char const* name, char** varRef) {
	defStringArg(name, varRef, false);
}

void CmdArgManager::defStringArg(char const* name, char** varRef, bool positional){
	auto entry = make_tuple(2,(void*)varRef);
	if(positional){
		_argList.push_back(make_tuple(string(name),entry));
	}
	else{
		_argMap[string(name)] = entry;
	}
}

void CmdArgManager::defBoolArg(char const* name, bool* varRef){
	*varRef = false; //ensure flag var is false by default
	_flagSet.push_back(make_tuple(string(name),(void*)varRef));
}

tuple<int,void*> CmdArgManager::_getArgTuple(char* name){
	string nameStr = string(name);
	
	if(_argMap.find(nameStr) == _argMap.end()){
		printf("Invalid argument name: %s\n", name);
		throw runtime_error("Invalid argument name");
	}
	
	return _argMap[nameStr];
}

void CmdArgManager::_processArg(tuple<int,void*> entry , char* argS){
	// Determine category of argument (int,float,string)
	int cat = get<0>(entry);
	
	// Pointer to storage variable
	void* varRef = get<1>(entry);
	
	// Convert and store value
	switch(cat){
		case 0:
			if(!regex_match(argS,regex("[0-9]+"))){
				printf("Invalid int value: %s\n", argS);
				throw runtime_error("Invalid int value");
			}
			*((int*)varRef) = atoi(argS);
			break;
		case 1:
			if(!regex_match(argS,regex("[0-9]+([.][0-9]+)?"))){
				printf("Invalid float value: %s\n", argS);
				throw runtime_error("Invalid float value");
			}
			*((float*)varRef) = (float)atof(argS);
			break;
		default:
			*((char**)varRef) = argS;
			break;
	}
}

inline const char* _catStr(int cat) {
	switch (cat) {
		case 0:
			return "int";
		case 1:
			return "flt";
		default:
			return "str";
	}
}

void CmdArgManager::showHelp(char** argv) {
	char* exeName = UtilF::getFName(argv[0],false);
	printf("Usage: %s", exeName);

	string argName;
	int cat;
	// List positional arguments
	for (auto it = _argList.begin(); it != _argList.end(); it++) {
		argName = get<0>(*it);
		cat = get<0>(get<1>(*it));
		printf(" %s:%s", argName.c_str(), _catStr(cat));
	}

	// List flag options
	if (!_flagSet.empty()) {
		for (auto it = _flagSet.begin(); it != _flagSet.end(); it++) {
			argName = get<0>(*it);
			printf(" [-%s]", argName.c_str());
		}
	}

	if (!_argMap.empty()) {
		// List optional arguments
		bool first = true;
		printf(" [");
		for (auto it = _argMap.begin(); it != _argMap.end(); it++) {
			argName = get<0>(*it);
			cat = get<0>(get<1>(*it));
			if (!first) {
				printf(" ");
			}
			else {
				first = false;
			}
			printf("%s:%s", argName.c_str(), _catStr(cat));
		}
		printf("]");
	}

	free(exeName);
}

bool CmdArgManager::processArgs(int argc, char** argv) {
	try {
		_processArgs(argc, argv);
	}
	catch (runtime_error err) {
		printf("Couldn't process arguments: %s\n", err.what());
		return false;
	}
	return true;
}

void CmdArgManager::_processArgs(int argc, char **argv){
	if(argc-1 < (int)_argList.size()){
		throw runtime_error("Not enough positional arguments.");
	}
	
	int i = 0;
	for(auto it=_argList.begin(); it!=_argList.end(); it++){
		_processArg(get<1>(*it),argv[i+1]);
		i++;
	}
	
	char argName[200];
	char c;
	bool gotArg;
	string tmpStr;
	bool* boolRef;
	for(i=(int)_argList.size()+1; i<argc; i++){
		if (strlen(argv[i]) >= 2 && argv[i][0] == '-') {
			// Flag option
			tmpStr = string(argv[i] + 1);
			for (auto it = _flagSet.begin(); it != _flagSet.end(); it++) {
				if (get<0>(*it) == tmpStr) {
					boolRef = (bool*)get<1>(*it);
					*boolRef = true;
					break;
				}
			}
			continue;
		}

		gotArg = false;
		for(int j=0; !gotArg && argv[i][j] != '\0'; j++){
			c = argv[i][j];
			if(c == '='){
				argName[j] = '\0';
				_processArg(_getArgTuple(argName),&(argv[i][j+1]));
				gotArg=true;
			}
			else{
				argName[j] = c;
			}
		}
		if(!gotArg){
			printf("\n%s\n",argv[i]);
			throw runtime_error("Invalid argument format (format is key=value)");
		}
	}
}
