#include "functions.h"
#include <windows.h>
#include <vector>
#include <commdlg.h> // Common Dialog Box Library


// isMouseOverButton globally and initialize it to false
bool isMouseOverButton = false;

// Unique IDs for buttons
#define ID_BUTTON_COMPARISON 1001 //compare button
#define ID_BUTTON_EXTRACT 1002 // extract button
#define ID_BUTTON_EXTRACT_FOLDER 1003  //extract from folder

 
// Global vector to hold the extracted data
std::vector<FunctionBlockData> extractedData; 
// Global variables for button handles
 HWND hWndButtonComparison; // compare two files
 HWND hWndButtonExtract;    //extract from file button
 HWND hWndButtonExtractFolder; //extract from folder button
 HWND hWndText; //text box

// Function to update the text of the text element
void UpdateWindowText(const std::wstring& newText) {
    SetWindowText(hWndText, newText.c_str());
}




// Window procedure to handle messages sent to the window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
     


            // Create comparison mode button
            hWndButtonComparison = CreateWindow(
                L"BUTTON",
                L"Comparison",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 50, 90, 30,
                hwnd,
                (HMENU)ID_BUTTON_COMPARISON, // Assign a unique ID to this button
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );

    
            // Create extract mode button
            hWndButtonExtract = CreateWindow(
                L"BUTTON",
                L"Extract",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                150, 50, 80, 30,
                hwnd,
                (HMENU)ID_BUTTON_EXTRACT, // Assign a unique ID to this button
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );

            // Create extract from folder button
            hWndButtonExtractFolder = CreateWindow(
                L"BUTTON",
                L"Extract from Folder", // Text for the button
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
                250, 50, 140, 30, // Position and size
                hwnd,
                (HMENU)ID_BUTTON_EXTRACT_FOLDER, // Assign a unique ID to this button
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );


            // Create text element
            hWndText = CreateWindow(
                L"STATIC", // Static control
                L"Initial Text", // Default text
                WS_VISIBLE | WS_CHILD | SS_LEFT, // Styles
                50, 90, 400, 150, // Position and size
                hwnd,
                NULL,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );

        }
        break;


        case WM_COMMAND:
        {
            if (LOWORD(wParam) == ID_BUTTON_COMPARISON) {
                // Update the text of the static control
                SetWindowText(hWndText, L"Comparision of functionblocks between two files of the same PLC provider \n\nSchneider = .xef, .xbd\nSiemens = .xml");

               if(showMessage("Choose the first file for comparison", "Comparison of function blocks")) {
               
                   // Prompt the user to choose the first XML file
                   std::string filePath1 = chooseFile();
                   if (filePath1.empty()) {
                       showMessage("No first file selected.", "Error");
                       return 0;
                   }

                   // Comparison button clicked second file
                   showMessage("Choose the Second file for comparison", "Comparison of function blocks");

                   // Prompt the user to choose the second XML file
                   std::string filePath2 = chooseFile();
                   if (filePath2.empty()) {
                       showMessage("No second file selected.", "Error");
                       return 0;
                   }


                   // Prompt the user to choose the output file location and name
                   showMessage("Choose location and name for Outputfile", "Comparison of function blocks");

                   std::string outputFilePath = chooseSaveFile();
                   if (outputFilePath.empty()) {
                       showMessage("No output file selected.", "Error");
                       return 0;
                   }
                   
                   
                   compareXMLFiles(filePath1, filePath2, outputFilePath, extractedData); 
               }

            }
            else if (LOWORD(wParam) == ID_BUTTON_EXTRACT) {
                // Update the text of the static control
                SetWindowText(hWndText, L"Extraction of functionblocks from a PLC programs .xml format.\n\nSchneider = .xef, .xbd\nSiemens = .xml");

                // Show message and proceed only if user clicks OK
                if (showMessage("Choose the XML file format of your PLC program to extract the function blocks.\n\nSchneider = .xef, .xbd\nSiemens = .xml", "Extraction of Functionblock data")) {
                    // Call extractFunctionBlocks function to extract function blocks from the selected file
                    std::string filePath = chooseFile();
                    if (!filePath.empty()) {
                        // Perform actions with the selected file
                        showMessage("Selected file: " + filePath, "Extraction of Functionblock data");
                        showMessage("Choose location and name for Outputfile", "Extraction of Functionblock data");

                        // Prompt the user to choose the output file location and name
                        std::string outputFilePath = chooseSaveFile();
                        if (outputFilePath.empty()) {
                            showMessage("No output file selected.", "Error");
                            return 0;
                        }

                      

                        // Call the extractFunctionBlocks function with the selected file path and desired output file path
                        extractFunctionBlocks(filePath, outputFilePath, extractedData);
                    }
                    else {
                        showMessage("No file selected.", "Extraction of Functionblock data");
                    }
                }
            }
            else if (LOWORD(wParam) == ID_BUTTON_EXTRACT_FOLDER) {
                // Update the text of the static control
                SetWindowText(hWndText, L"Extraction of function blocks from a PLC program where the blocks are saved in separate .xml instead of in a single file \n\nSiemens API stores the function blocks separatly when extracted");

                if (showMessage("Choose the folder for XML extracion", "Extraction from folder")) {
                    extractXMLfromfolder();
                }
            }
        }
        break;

        // drawing the window correctly when resizing
        case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Get the client area dimensions
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);

            // Paint the background
            FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));

            EndPaint(hwnd, &ps);
        }
        break;



            case WM_DESTROY:
            PostQuitMessage(0);
            break;


        default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}







// Function to create and display the main window
void CreateMainWindow(HINSTANCE hInstance) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MainWindowClass";

    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        0,
        L"MainWindowClass",
        L"Function block extraction from PLC code",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 350,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);
}

// entry point of the program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    
  

    // Create the main window
    CreateMainWindow(hInstance);

    // Message loop
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    


    return 0;
}


