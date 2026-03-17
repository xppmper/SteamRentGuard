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

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Psapi.lib")

// --- НАСТРОЙКИ TELEGRAM ---
std::wstring botToken = L"YOUR_BOT_TOKEN_HERE";
std::wstring chatId = L"Your id in tg";

// Сайты для блокировки в HOSTS
std::vector<std::string> blockedSites = {
    "xone.fun", "midnight.im", "interium.ooo", "fatality.win", "neverlose.cc",
    "www.xone.fun", "www.midnight.im", "www.interium.ooo", "www.fatality.win", "www.neverlose.cc"
};

// СПИСОК ЦЕЛЕЙ (ПРОЦЕССЫ И ОКНА)
std::vector<std::wstring> cheatTargets = {
    L"exloader", L"com.swiftsoft", L"xone", L"interium", L"skinchanger", L"extrimhack",
    L"nix", L"memesense", L"mvploader", L"sharkhack", L"exhack", L"neverkernel",
    L"vredux", L"mason", L"predator", L"aquila", L"luno", L"fecurity", L"cartel",
    L"aimstar", L"tkazer", L"naim", L"pellix", L"pussycat", L"axios", L"onemacro",
    L"softhub", L"proext", L"sapphire", L"interwebz", L"plague", L"vapehook",
    L"smurfwrecker", L"iniuria", L"yeahnot", L"legendware", L"hauntedproject",
    L"phoenixhack", L"onebyteradar", L"reborn", L"onebyte", L"osiris", L"ev0lve",
    L"ghostware", L"dexterion", L"basicmultihack", L"pudra", L"iCheat", L"sneakys",
    L"krazyhack", L"muhprime", L"drcheats", L"rootcheat", L"aeonix", L"zedt.pw",
    L"devcore", L"legifard", L"katebot", L"imxnoobx", L"w1nner", L"ekknod",
    L"neoxahack", L"warware", L"weave", L"aimmy", L"paradise", L"xenon", L"easysp",
    L"en1gma", L"injector", L".ahk", L"macros", L"bhop", L"bunnyhop", L"espd2x",
    L"avira", L"pphud", L"primordial", L"nonagon", L"legit", L"hvh", L"aimbot",
    L"s1mple", L"semirage", L"cheat", L"cs2.glow", L"invision", L"undetek",
    L"spurdo", L"webradar", L"valthrun", L"midnight", L"interium.ooo",
    L"interium.ooo/buy", L"interium.ooo/faq", L"interium.ooo/support", L"interium.ooo/account" // Прямые ссылки
};

std::wstring GetPCName() {
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameW(buffer, &size)) return std::wstring(buffer);
    return L"Unknown_PC";
}

void Log(std::string text) {
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    printf("[%02d:%02d:%02d] %s\n", ltm.tm_hour, ltm.tm_min, ltm.tm_sec, text.c_str());
}

// ФУНКЦИЯ БЛОКИРОВКИ + ОЧИСТКА КЭША
void BlockCheatSites() {
    std::ofstream hostsFile;
    hostsFile.open("C:\\Windows\\System32\\drivers\\etc\\hosts", std::ios::app);
    if (hostsFile.is_open()) {
        hostsFile << "\n# SteamRentGuard Block\n";
        for (const auto& site : blockedSites) {
            hostsFile << "127.0.0.1 " << site << "\n";
        }
        hostsFile.close();
        system("ipconfig /flushdns >nul 2>&1"); // Очищаем кэш DNS
        Log("Сетевой фильтр: Сайты читов заблокированы и кэш DNS очищен.");
    }
    else {
        Log("!!! ОШИБКА: Запустите от имени АДМИНИСТРАТОРА для блокировки сайтов !!!");
    }
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

void AutoStatusThread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(4300));
        SendToTelegram(L"🟢 СТАТУС: Защита активна");
    }
}

void PrintLogo() {
    system("cls");
    system("color 0B");
    std::wcout << L"############################################################" << std::endl;
    std::wcout << L"#                                                          #" << std::endl;
    std::wcout << L"#                SteamRentGuard SECURITY 1.0               #" << std::endl;
    std::wcout << L"#          СТАТУС: АКТИВНЫЙ МОНИТОРИНГ [ПОЛНЫЙ СПИСОК]     #" << std::endl;
    std::wcout << L"############################################################" << std::endl;
    std::wcout << L"" << std::endl;
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

void SecurityAction(std::wstring name, std::wstring path, DWORD pid = 0) {
    Log("!!! ВНИМАНИЕ: ОБНАРУЖЕНА УГРОЗА !!!");
    if (pid != 0) {
        HANDLE hKill = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hKill) { TerminateProcess(hKill, 9); CloseHandle(hKill); }
    }
    system("taskkill /F /IM cs2.exe /T >nul 2>&1");
    system("taskkill /F /IM steam.exe /T >nul 2>&1");
    system("taskkill /F /IM steamwebhelper.exe /T >nul 2>&1");

    SendToTelegram(L"🚨 ОБНАРУЖЕНА УГРОЗА!\n🔹 Объект: " + name + L"\n📍 Путь/Окно: " + path + L"\n❌ СТАТУС: Steam закрыт.");
    MessageBoxW(NULL, L"Угроза зафиксирована! Steam закрыт в целях безопасности.", L"SteamRentGuard 0.53 BETA", MB_ICONERROR | MB_OK);
    exit(0);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    wchar_t title[512]; // Увеличил буфер для длинных заголовков браузера
    if (GetWindowTextW(hwnd, title, 512)) {
        std::wstring wsTitle(title);
        std::transform(wsTitle.begin(), wsTitle.end(), wsTitle.begin(), ::tolower);
        for (const auto& target : cheatTargets) {
            if (wsTitle.find(target) != std::wstring::npos) {
                SecurityAction(L"Запрещенный ресурс", wsTitle);
            }
        }
    }
    return TRUE;
}

int main() {
    system("chcp 1251 > nul");
    setlocale(LC_ALL, "Russian");

    PrintLogo();
    Log("Запуск системы...");

    BlockCheatSites();

    Log("Инициализация базы данных сигнатур...");
    Log("Загружено " + std::to_string(cheatTargets.size()) + " паттернов поиска.");

    Log("Скрытие консоли из панели задач...");
    HWND hwnd = GetConsoleWindow();
    long style = GetWindowLong(hwnd, GWL_EXSTYLE);
    style |= WS_EX_TOOLWINDOW; style &= ~WS_EX_APPWINDOW;
    SetWindowLong(hwnd, GWL_EXSTYLE, style);

    Log("Запуск фонового потока статуса...");
    std::thread statusThread(AutoStatusThread);
    statusThread.detach();

    Log("Подключение к Telegram API...");
    SendToTelegram(L"✅ ЗАЩИТА ВКЛЮЧЕНА [0.53 BETA]\n📍 Мониторинг активен.");

    Log("------------------------------------------------------------");
    Log("ЯДРО ЗАЩИТЫ: АКТИВНО");
    Log("Мониторинг запущен.");
    Log("------------------------------------------------------------");

    while (true) {
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

        // Эта функция теперь проверяет и заголовки браузеров на наличие /buy, /faq и т.д.
        EnumWindows(EnumWindowsProc, NULL);

        Sleep(1500);
    }
    return 0;
}
