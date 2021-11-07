#include <iostream>
#include "ImageSimplifier.h"
#include <math.h>
#include <stdlib.h>
#include <regex>
#include <direct.h>
#include <fstream>
#include "CmdArgManager.h"
#include "Config.h"
#include "global.h"
#include "UtilF.h"
#include "RegDataSmooth.h"
#include "ImageData.h"

void writeArgFile(char* outputFolder, int argc, char** argv){
	char fName[STR_BUFFER_SIZE];
	sprintf(fName,"%s/ArgsUsedToGenerate.txt", outputFolder);
	
	std::ofstream txtFile;
	txtFile.open(fName);
	txtFile << argv[0];
	for(int i=1; i<argc; i++){
		txtFile << " " << argv[i];
	}
	txtFile.close();
}

/*
int doSmooth(char* fPath, int smoothStr){
	const char* outputFolder = "smoothOutput";
	_mkdir(outputFolder);
	
	char fNameBuffer[300];
	RegData* rData = new RegData(fPath);
	sprintf(fNameBuffer,"%s/smooth_s%d_%d.jpg",outputFolder,smoothStr,0);
	rData->snapshot(fNameBuffer);
	
	for(int i=0; i<10; i++){
		printf("Smooth pass %d\n", i+1);
		RegDataSmooth::smoothPass(rData, smoothStr);
		sprintf(fNameBuffer,"%s/smooth_s%d_%d.jpg",outputFolder,smoothStr,i+1);
		rData->snapshot(fNameBuffer);
	}
	printf("Smoothing done.\n");
	delete rData;
	return 0;
}
*/

char* getOutFName(const char* inPath) {
	char* fName = UtilF::getFName(inPath, true);
	// If file-name is of pattern <something>_<num>.regdata
	// return <something>
	if (UtilF::strEndsWith(inPath, ".regdata")) {
		int idx = UtilF::getLastIdx(fName, '_', (int)strlen(fName) - 1);
		if (idx != -1 && isdigit(fName[idx+1])) {
			bool isValid = true;
			for (int i = idx + 2; fName[i] != '\0'; i++) {
				if (!isdigit(fName[i])) {
					isValid = false;
					break;
				}
			}

			if (isValid) {
				fName[idx] = '\0';
				char* tmpS = UtilF::copyStr(fName);
				free(fName);
				return tmpS;
			}
		}
	}

	return fName;
}

int main(int argc, char **argv)
{
	char fNameBuffer[STR_BUFFER_SIZE];

	char* targetFPath;
	char* outFilename;
	if (argc > 1 && UtilF::fileExists(argv[1])) {
		outFilename = getOutFName(argv[1]);
	}
	else {
		// Error will be thrown later but we want that to be handled by the argument-manager
		outFilename = UtilF::copyStr("_undefined_");;
	}
	char* outputFolder = UtilF::copyStr("Output");
	int outputAfter = 1000;
	int stopAt = 1;
	bool regDataOnly = false;
	
	Config config = Config();
	int useContrastMap = 0;
	int useWeightMap = 0;
	
	CmdArgManager argManager = CmdArgManager();
	argManager.defStringArg("targetFile", &targetFPath,true);

	argManager.defStringArg("outFilename",&outFilename);
	argManager.defStringArg("outputFolder",&outputFolder);

	argManager.defIntArg("stopAt", &stopAt);
	argManager.defIntArg("outputAfter", &outputAfter);
	argManager.defBoolArg("regDataOnly", &regDataOnly);
	
	argManager.defIntArg("scale",&config.scaleFactor);
	argManager.defFloatArg("weightMult",&config.weightMult);
	argManager.defFloatArg("contrastMult",&config.contrastMult);
	
	config.contrastMapPath = (char*)malloc(sizeof(char)*STR_BUFFER_SIZE);
	config.weightMapPath = (char*)malloc(sizeof(char)*STR_BUFFER_SIZE);
	config.contrastMapPath[0] = '\0';
	config.weightMapPath[0] = '\0';
	argManager.defStringArg("contrastMap",&config.contrastMapPath);
	argManager.defStringArg("weightMap",&config.weightMapPath);
	
	if (!argManager.processArgs(argc, argv)) {
		argManager.showHelp(argv);
		return 0;
	}
	
	if (!UtilF::fileExists(targetFPath)) {
		printf("Error: Target file \"%s\" does not exist.", targetFPath);
		return 1;
	}

	if (stopAt < 1) {
		printf("Error: stopAt parameter (%d) must be >=1", stopAt);
		return 1;
	}
	
	if(strlen(config.contrastMapPath) == 0){
		delete config.contrastMapPath;
		config.contrastMapPath = NULL;
	}
	if(strlen(config.weightMapPath) == 0){
		delete config.weightMapPath;
		config.weightMapPath = NULL;
	}
	
	if(config.scaleFactor <= 0){
		printf("Error: Scale factor must be positive (%d)",config.scaleFactor);
		return 1;
	}
	
	if(config.contrastMapPath != NULL && !UtilF::fileExists(config.contrastMapPath)){
		printf("Error: Contrast-map file %s doesn't exist.",config.contrastMapPath);
		return 1;
	}
	if(config.weightMapPath != NULL && !UtilF::fileExists(config.weightMapPath)){
		printf("Error: Weight-map file %s doesn't exist.",config.weightMapPath);
		return 1;
	}
	
	int outPathLen = (int)strlen(outputFolder);
	if(outputFolder[outPathLen-1] == '/' || outputFolder[outPathLen-1] == '\\'){
		outputFolder[outPathLen-1] = '\0';
		outPathLen--;
	}
	
	char* fullOutputFolder = (char*)malloc(sizeof(char)*(outPathLen + strlen(outFilename) + 2));
	strncpy(fullOutputFolder,outputFolder,(size_t)outPathLen+1);
	strcat(fullOutputFolder,"/");
	strcat(fullOutputFolder,outFilename);

	UtilF::mkDirs(fullOutputFolder);
	
	// Make regData sub-folder
	sprintf(fNameBuffer,"%s/regData",fullOutputFolder);
	_mkdir(fNameBuffer);
	
	writeArgFile(fullOutputFolder,argc,argv);
	
	printf("Starting...\n");
	ImageData::init();
	
	ImageSimplifier* imageModule = new ImageSimplifier(targetFPath,config);
	
	RegData* rData;
	int maxRegionCount = imageModule->getMaxRegionCount();
	int numSpace = (int)floor(log10(maxRegionCount)) + 1;
	printf("Region Count %.*s",numSpace*2 + 1,"                           ");
	int modulusCheck;
	for(int rCount=imageModule->getRegionCount(); rCount>=stopAt; rCount--){
		if(rCount%1000 == 0 || rCount<1000){
			printf("%.*s%*d/%d",numSpace*2 + 1, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", numSpace,rCount,maxRegionCount);
		}
		
		/*
			Basically, if the region count is in the range 1-10 snapshot every cycle
			if 11-100 snapshot every 10
			if 101-1000 snapshot every 100
			etc.
		*/
		if(rCount > 100 && rCount < 500){
			/* Take more snapshots for this region since I've found it
			    to still be interesting
			   (usually past 1000 the image just looks blurry) */
			modulusCheck = 25;
		}
		else{
			modulusCheck = (int)pow(10,ceil(log10(rCount)-1));
		}
		if(rCount == stopAt || ((modulusCheck <= 1 || rCount%modulusCheck == 0) && rCount <= outputAfter)){
			if (!regDataOnly) {
				// Save snapshot
				sprintf(fNameBuffer, "%s/%s_%d.jpg", fullOutputFolder, outFilename, rCount);
				imageModule->renderCurrentStateTo(fNameBuffer);
			}
			
			sprintf(fNameBuffer, "%s/regData/%s_%d.regdata", fullOutputFolder, outFilename, rCount);
			rData = imageModule->getRegionData();
			rData->saveTo(fNameBuffer);
			delete rData;
		}
		
		if(rCount != stopAt){
			imageModule->peformSimplificationStep();
		}
	}
	printf("\n");
	
	printf("Finished.\n");
	return 0;
}
