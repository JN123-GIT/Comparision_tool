#include "functions.h"
#include "tinyxml2.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <tchar.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <limits> // Include directive for the <limits> header
#include <commdlg.h> // For file dialog
#include <vector>
#include <filesystem>
#include <Shlobj.h> // Include the Shlobj.h header file for SHBrowseForFolder function
#include <utility> // Include the <utility> header for std::pair
#include <algorithm> // For std::sort

using namespace tinyxml2;
namespace fs = std::filesystem;

// Function to display a message in a Windows popup and return true if user clicked "OK"
bool showMessage(const std::string& message, const std::string& title) {
    // Convert narrow-character strings to wide-character strings
    std::wstring wMessage(message.begin(), message.end());
    std::wstring wTitle(title.begin(), title.end());

    // Display message box with OK and Cancel buttons
    int result = MessageBoxW(NULL, wMessage.c_str(), wTitle.c_str(), MB_OKCANCEL | MB_SYSTEMMODAL);

    // Return true if user clicked "OK", false otherwise
    return (result == IDOK);
}


std::wstring ChooseFolder(HWND hwnd) {
    std::wstring folderPath;

    // Initialize COM for this thread
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        // Create the File Open dialog object
        IFileDialog* pFolderDialog;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFolderDialog));
        if (SUCCEEDED(hr)) {
            // Set options for the dialog
            DWORD dwOptions;
            hr = pFolderDialog->GetOptions(&dwOptions);
            if (SUCCEEDED(hr)) {
                // Update options to show folders only and hide file system items
                hr = pFolderDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
                if (SUCCEEDED(hr)) {
                    // Show the dialog
                    hr = pFolderDialog->Show(hwnd);
                    if (SUCCEEDED(hr)) {
                        // Get the result (selected folder)
                        IShellItem* pItem;
                        hr = pFolderDialog->GetResult(&pItem);
                        if (SUCCEEDED(hr)) {
                            // Get the path of the selected folder
                            PWSTR pszFolderPath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
                            if (SUCCEEDED(hr)) {
                                // Convert PWSTR to wstring
                                folderPath = pszFolderPath;
                                CoTaskMemFree(pszFolderPath);
                            }
                            pItem->Release();
                        }
                    }
                }
            }
            pFolderDialog->Release();
        }
        // Uninitialize COM for this thread
        CoUninitialize();
    }

    return folderPath;
}

// Function to prompt the user to choose a file using the file dialog for extraction of blocks
std::string chooseFile() {
    // File dialog structure
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH] = { 0 }; // Array to hold file path
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = _T("Open the file for extraction"); // Instruction within the file dialog
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    // Prompt user to choose a file
    if (GetOpenFileName(&ofn) == TRUE) {
#ifdef UNICODE
        // Convert TCHAR to std::wstring and then to std::string
        std::wstring wstr(szFile);
        return std::string(wstr.begin(), wstr.end());
#else
        // Direct conversion from TCHAR to std::string
        return std::string(szFile);
#endif
    }
    else {
        return ""; // Return an empty string if no file selected or dialog canceled
    }
}

// Function to show a file save dialog and return the selected file path
std::string chooseSaveFile() {
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"txt"; // Default extension
    ofn.lpstrTitle = L"Save File As";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileName(&ofn) == TRUE) {
        // Check if the user provided an extension
        std::wstring filePath = ofn.lpstrFile;
        if (filePath.find_last_of(L'.') == std::wstring::npos) {
            // If no extension provided, append ".txt"
            filePath += L".txt";
        }
        // Convert the TCHAR string to std::string
        std::string utf8FilePath(filePath.begin(), filePath.end());
        return utf8FilePath;
    }
    else {
        return ""; // Return empty string if user cancels the dialog
    }
}

// Function to choose the location and name of the outputfile
std::wstring ChooseOutputFile(HWND hwnd) {
    wchar_t szFilePath[MAX_PATH];
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFilePath;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFilePath);
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = L"Please choose the output filename and location"; // Instruction within the file dialog
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE) {
        std::wstring filePath = ofn.lpstrFile;
        // Ensure the file has the ".txt" extension
        if (filePath.find(L".txt") == std::wstring::npos) {
            filePath += L".txt";
        }
        return filePath;
    }
    else {
        return L"";
    }
}

// Function to get the filename in string
std::string extractFileName(const std::string& filePath) {
    // Find the position of the last directory separator
    size_t lastSlash = filePath.find_last_of("\\/");

    // Extract the substring after the last directory separator
    std::string fileName = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;

    // Find the position of the last dot
    size_t lastDot = fileName.find_last_of(".");

    // Extract the substring before the last dot (if found), which is the filename without extension
    return (lastDot != std::string::npos) ? fileName.substr(0, lastDot) : fileName;
}


std::string getFileExtension(const std::string& filePath) {
    size_t dotIndex = filePath.find_last_of('.');
    if (dotIndex != std::string::npos && dotIndex != filePath.size() - 1) {
        return filePath.substr(dotIndex);
    }
    else {
        return ""; // No extension found
    }
}
// Function to compare the function blocks between two files


// Helper function to load an XML file and return its root element
tinyxml2::XMLElement* loadXMLRoot(const std::string& filePath) {
    static tinyxml2::XMLDocument doc; // Static to avoid re-declaration issues
    if (doc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
        showMessage("Failed to load XML file: " + filePath, "Error");
        return nullptr;
    }
    return doc.RootElement();
}

void compareXMLFiles(const std::string& filePath1, const std::string& filePath2, const std::string& outputFilePath, std::vector<FunctionBlockData>& extractedData) {


    // Load the first XML file
    tinyxml2::XMLDocument xmlDoc1;
    if (xmlDoc1.LoadFile(filePath1.c_str()) != tinyxml2::XML_SUCCESS) {
        showMessage("Failed to load XML file: " + filePath1, "Error");
        return;
    }

    // Load the second XML file
    tinyxml2::XMLDocument xmlDoc2;
    if (xmlDoc2.LoadFile(filePath2.c_str()) != tinyxml2::XML_SUCCESS) {
        showMessage("Failed to load XML file: " + filePath2, "Error");
        return;
    }

    // Extract PLC brand and content header name from the first XML file
    std::string plcBrand1;
    std::string Projectname1;
    tinyxml2::XMLElement* root1 = xmlDoc1.RootElement();
    if (root1) {
        tinyxml2::XMLElement* fileHeader1 = root1->FirstChildElement("fileHeader");
        if (fileHeader1) {
            const char* company = fileHeader1->Attribute("company");
            if (company) {
                plcBrand1 = company;
            }
        }
        tinyxml2::XMLElement* contentHeader1 = root1->FirstChildElement("contentHeader");
        if (contentHeader1) {
            const char* name = contentHeader1->Attribute("name");
            if (name) {
                Projectname1 = name;
            }
        }
    }

    // Extract PLC brand and content header name from the second XML file
    std::string plcBrand2;
    std::string Projectname2;
    tinyxml2::XMLElement* root2 = xmlDoc2.RootElement();
    if (root2) {
        tinyxml2::XMLElement* fileHeader2 = root2->FirstChildElement("fileHeader");
        if (fileHeader2) {
            const char* company = fileHeader2->Attribute("company");
            if (company) {
                plcBrand2 = company;
            }
        }
        tinyxml2::XMLElement* contentHeader2 = root2->FirstChildElement("contentHeader");
        if (contentHeader2) {
            const char* name = contentHeader2->Attribute("name");
            if (name) {
                Projectname2 = name;
            }
        }
    }

    // Create maps to store function block names along with their versions from each file
    std::unordered_map<std::string, std::string> fbVersions1;
    std::unordered_map<std::string, std::string> fbVersions2;

    // Extract function block names and versions from the first XML file
    if (root1) {
        for (tinyxml2::XMLElement* fbSource = root1->FirstChildElement("FBSource"); fbSource; fbSource = fbSource->NextSiblingElement("FBSource")) {
            const char* nameOfFBType = fbSource->Attribute("nameOfFBType");
            const char* version = fbSource->Attribute("version");
            if (nameOfFBType && version) {
                fbVersions1[std::string(nameOfFBType)] = std::string(version);
            }
        }
    }

    // Extract function block names and versions from the second XML file
    if (root2) {
        for (tinyxml2::XMLElement* fbSource = root2->FirstChildElement("FBSource"); fbSource; fbSource = fbSource->NextSiblingElement("FBSource")) {
            const char* nameOfFBType = fbSource->Attribute("nameOfFBType");
            const char* version = fbSource->Attribute("version");
            if (nameOfFBType && version) {
                fbVersions2[std::string(nameOfFBType)] = std::string(version);
            }
        }
    }

    // Open a text file for writing
    std::ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        showMessage("Failed to open output file: " + outputFilePath, "Error");
        return;
    }

    // Write PLC brand and content header name for both files
    outputFile << "PLC Brand and Filename:\n";
    outputFile << "File 1: " << extractFileName(filePath1) << " - PLC Brand: " << plcBrand1 << ", PLCprojectname: " << Projectname1 << "\n";
    outputFile << "File 2: " << extractFileName(filePath2) << " - PLC Brand: " << plcBrand2 << ", PLCprojectname: " << Projectname2 << "\n\n";

    // Compare function block versions between the two files
    outputFile << "Function Blocks with Different Versions:\n";
    for (const auto& entry : fbVersions2) {
        const std::string& fbName = entry.first;
        const std::string& version2 = entry.second;
        if (fbVersions1.find(fbName) != fbVersions1.end()) {
            const std::string& version1 = fbVersions1[fbName];
            if (version1 != version2) {
                outputFile << "FB: " << fbName << ", Version:" << version1 << "  -  " << extractFileName(filePath1) << "  ----->  " << extractFileName(filePath2) << "  -  " << "Version: " << version2 << std::endl;
            }
        }
        else {
            // The function block is present in the second file but not in the first file
            outputFile << "FB: " << fbName << " - Added in " << extractFileName(filePath2) << " - Version: " << version2 << std::endl;
        }
    }

    // Check for function blocks present in the first file but not in the second file
    outputFile << "\nFunction Blocks Present Only in " << extractFileName(filePath1) << ":\n";
    for (const auto& entry : fbVersions1) {
        const std::string& fbName = entry.first;
        if (fbVersions2.find(fbName) == fbVersions2.end()) {
            // The function block is present in the first file but not in the second file
            const std::string& version1 = entry.second;
            outputFile << "FB: " << fbName << " - Deleted from " << extractFileName(filePath2) << " - Version: " << version1 << std::endl;
        }
    }

    // Close the output file
    outputFile.close();

    showMessage("Comparison completed. Results written to: " + outputFilePath, "Information");
}

bool processXMLFile(const std::filesystem::path& filePath, std::ofstream& outputFile) {
    tinyxml2::XMLDocument xmlDoc;
    if (xmlDoc.LoadFile(filePath.string().c_str()) != tinyxml2::XML_SUCCESS) {
        showMessage("Failed to load XML file: " + filePath.string(), "Error");
        return false;
    }

    tinyxml2::XMLElement* root = xmlDoc.RootElement();
    if (!root) {
        showMessage("Failed to parse XML file: " + filePath.string(), "Error");
        return false;
    }

    for (tinyxml2::XMLElement* fbElement = root->FirstChildElement(); fbElement != nullptr; fbElement = fbElement->NextSiblingElement()) {
        std::string elementName = fbElement->Name();
        if (!(elementName == "SW.Blocks.FB" || elementName == "SW.Blocks.FC" || elementName == "SW.Blocks.GlobalDB" || elementName == "SW.Blocks.OB")) continue;

        tinyxml2::XMLElement* attributeListElement = fbElement->FirstChildElement("AttributeList");
        if (!attributeListElement) continue;

        tinyxml2::XMLElement* headerVersionElement = attributeListElement->FirstChildElement("HeaderVersion");
        if (!headerVersionElement) continue;

        const char* headerVersion = headerVersionElement->GetText();
        if (!headerVersion) continue;

        outputFile << elementName << "," << "Siemens" << "," << filePath.filename().replace_extension("").string() << "," << headerVersion << "," << "cleaningstation" << std::endl;
        return true;
    }
    return false;
}

void extractXMLfromfolder(const std::wstring& folderPath, std::ofstream& outputFile) {
    try {
        outputFile << "Block Type, PLC Brand, Function Block, Version, FileName" << std::endl;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) {
            if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ".xml") {
                processXMLFile(entry.path(), outputFile);
            }
        }
    }
    catch (const std::exception& e) {
        showMessage("Exception occurred: " + std::string(e.what()), "Error");
    }
}

void extractXMLfromfolder() {
    std::wstring folderPath = ChooseFolder(NULL);
    if (folderPath.empty()) {
        showMessage("No folder selected.", "Info");
        return;
    }

    std::wstring outputFilePath = ChooseOutputFile(NULL);
    if (outputFilePath.empty()) {
        showMessage("No output file selected.", "Info");
        return;
    }

    std::ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        showMessage("Failed to open output file.", "Error");
        return;
    }

    extractXMLfromfolder(folderPath, outputFile);
    outputFile.close();
    showMessage("Extraction and writing completed.", "Information");
}



// Function to extract function block data from an XML file and write to a CSV file
void extractFunctionBlocks(const std::string& filePath, const std::string& outputFilePath, std::vector<FunctionBlockData>& extractedData) {

    // Load the XML file
    tinyxml2::XMLDocument xmlDoc;
    if (xmlDoc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
        showMessage("Failed to load XML file: " + filePath, "Error");
        return; // Exit if the file cannot be loaded
    }

    // Derive the output directory from the provided output file path
    std::string outputDirectory = outputFilePath.substr(0, outputFilePath.find_last_of("\\/") + 1);

    // Construct the CSV file name using the output directory and replacing the extension
    std::string csvOutputFileName = outputDirectory + extractFileName(outputFilePath) + ".csv";

    // Attempt to access the root element of the XML document
    tinyxml2::XMLElement* root = xmlDoc.RootElement();
    if (!root) return; 
    // Early exit if no root element

    // Extract PLC brand and project name from the XML
    std::string plcBrand, projectName;
    if (tinyxml2::XMLElement* fileHeader = root->FirstChildElement("fileHeader")) {
        plcBrand = fileHeader->Attribute("company") ? fileHeader->Attribute("company") : "";
    }
    if (tinyxml2::XMLElement* contentHeader = root->FirstChildElement("contentHeader")) {
        projectName = contentHeader->Attribute("name") ? contentHeader->Attribute("name") : "";
    }

    // Iterate through all function blocks, extract their data, and store in the vector
    for (tinyxml2::XMLElement* fbSource = root->FirstChildElement("FBSource"); fbSource; fbSource = fbSource->NextSiblingElement("FBSource")) {
        const char* nameOfFBType = fbSource->Attribute("nameOfFBType");
        const char* version = fbSource->Attribute("version");
        if (nameOfFBType && version) {
            // Add the extracted data to the vector
            extractedData.emplace_back("function block", plcBrand, nameOfFBType, version, projectName);
        }
    }

    // Sort the function block data alphabetically by the name of the function block
    std::sort(extractedData.begin(), extractedData.end(), [](const FunctionBlockData& a, const FunctionBlockData& b) {
        return a.nameOfFBType < b.nameOfFBType;
        });

    // Open the CSV output file
    std::ofstream csvOutputFile(csvOutputFileName);
    if (!csvOutputFile.is_open()) {
        showMessage("Failed to open output file: " + csvOutputFileName, "Error");
        return; // Exit if the file cannot be opened for writing
    }

    // Write the header row to the CSV file
    csvOutputFile << "Block Type, PLC Brand, Function Block, Block Version, Filename" << std::endl;

    // Write each function block's data to the CSV file
    for (const auto& fbData : extractedData) {
        csvOutputFile << fbData.blockType << "," << fbData.plcBrand << "," << fbData.nameOfFBType << "," << fbData.version << "," << fbData.Projectname << std::endl;
    }

    // Clean up by closing the file
    csvOutputFile.close();

    // Notify the user that the extraction and writing process is complete
    showMessage("Extraction completed. CSV file written to: " + csvOutputFileName, "Information");
}