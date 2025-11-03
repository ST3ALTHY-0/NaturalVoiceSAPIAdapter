#include "Installer.h"
#include <string>
#include <vector>
#include <cwchar>   // for wcscmp
#include <stdexcept>
#include <shellapi.h>

void Register(bool is64Bit);
void Unregister(bool is64Bit);
INT_PTR CALLBACK MainDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// 🔊 You’ll need to implement this or call your app’s internal function
void SpeakText(const std::wstring& text,
               const std::wstring& voice = L"Default",
               float rate = 1.0f,
               int volume = 100,
               const std::wstring& format = L"24kHz16BitStereo");

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Split command line into args
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (!argv || argc <= 1)
    {
        // No CLI args — just open GUI
        DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), nullptr, MainDlg, 0);
        return 0;
    }

    // Handle uninstall (your existing case)
    if (_wcsicmp(lpCmdLine, L"-uninstall") == 0)
    {
        try
        {
            Unregister(false);
            if (Is64BitSystem())
                Unregister(true);

            ReportError(ERROR_SUCCESS);
        }
        catch (const std::system_error& ex)
        {
            DWORD err = ex.code().value();
            ReportError(err);
            return err;
        }
        return 0;
    }

    // 🗣️ CLI Speech mode
    if (_wcsicmp(argv[1], L"-speak") == 0 && argc > 2)
    {
        std::wstring text = argv[2];
        std::wstring voice = L"Default";
        float rate = 1.0f;
        int volume = 100;
        std::wstring format = L"24kHz16BitStereo";

        // Simple flag parsing
        for (int i = 3; i < argc; ++i)
        {
            if (_wcsicmp(argv[i], L"-voice") == 0 && i + 1 < argc)
                voice = argv[++i];
            else if (_wcsicmp(argv[i], L"-rate") == 0 && i + 1 < argc)
                rate = std::stof(argv[++i]);
            else if (_wcsicmp(argv[i], L"-volume") == 0 && i + 1 < argc)
                volume = std::stoi(argv[++i]);
            else if (_wcsicmp(argv[i], L"-format") == 0 && i + 1 < argc)
                format = argv[++i];
        }

        try
        {
            SpeakText(text, voice, rate, volume, format);
        }
        catch (const std::exception& e)
        {
            MessageBoxA(nullptr, e.what(), "TTS Error", MB_ICONERROR);
            return 1;
        }
        return 0;
    }

    // Default to launching the GUI if no recognized CLI flags
    DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), nullptr, MainDlg, 0);
    return 0;
}
