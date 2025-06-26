#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string> 
#include <vector> 
#include <windows.h> // Windows API types and functions



// FunctionBlockData structure to hold extracted data

struct FunctionBlockData {
    std::string blockType;      // Type of the function block
    std::string plcBrand;       // Brand of the PLC
    std::string nameOfFBType;   // Name of the function block type
    std::string version;        // Version of the function block
    std::string Projectname;    // Name of the project

    // Constructor initializes the structure with given values
    FunctionBlockData(const std::string& blockType_, const std::string& plcBrand_, const std::string& nameOfFBType_, const std::string& version_, const std::string& Projectname_)
        : blockType(blockType_), plcBrand(plcBrand_), nameOfFBType(nameOfFBType_), version(version_), Projectname(Projectname_) {}
};


// Message box with a given message and title since "cout" does not work
bool showMessage(const std::string& message, const std::string& title);
std::wstring ChooseFolder(HWND hwnd);
std::string chooseFile();
std::string chooseSaveFile(); // Add chooseSaveFile function declaration
std::string extractFileName(const std::string& filePath);

// Compare XML files located at filePath1 and filePath2, results saved to outputFilePath
void compareXMLFiles(const std::string& filePath1, const std::string& filePath2, const std::string& outputFilePath, std::vector<FunctionBlockData>& extractedData);

// Extract function blocks from a file located at filePath and save results to outputFilePath
void extractFunctionBlocks(const std::string& filePath, const std::string& outputFilePath, std::vector<FunctionBlockData>& extractedData);

// Extract XML data from a folder selected by the user
void extractXMLfromfolder();

#endif // FUNCTIONS_H
