/*
 * ╔══════════════════════════════════════════════════════════════════════════════╗
 * ║                                                                              ║
 * ║                          STAR SERVICES                                       ║
 * ║                         ORG STAR CUP                                         ║
 * ║                                                                              ║
 * ║    Author: Star                                                             ║
 * ║    Copyright (c) 2025-2030 Star Cup                                         ║
 * ║                                                                              ║
 * ╚══════════════════════════════════════════════════════════════════════════════╝
 *
 * Compile: gcc -o StarServices.exe StarServices.c -ladvapi32 -lkernel32 -static
 *
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <urlmon.h>

#pragma comment(lib, "urlmon.lib")

/* ==================== COLOR DEFINITIONS ==================== */
#define COLOR_DEFAULT         7
#define COLOR_BLUE            9
#define COLOR_GREEN          10
#define COLOR_CYAN           11
#define COLOR_RED            12
#define COLOR_MAGENTA        13
#define COLOR_YELLOW         14
#define COLOR_WHITE          15
#define COLOR_GRAY            8

/* ==================== SERVICE CONFIGURATION ==================== */
#define TOTAL_SERVICES 7

static const char* services[TOTAL_SERVICES] = {
    "PcaSvc",
    "PlugPlay", 
    "DPS",
    "DiagTrack",
    "SysMain",
    "EventLog",
    "Sysmon"
};

static const char* service_display[TOTAL_SERVICES] = {
    "PROGRAM COMPATIBILITY ASSISTANT",
    "PLUG AND PLAY",
    "DIAGNOSTIC POLICY SERVICE",
    "TELEMETRY AND EXPERIENCES",
    "SUPERFETCH",
    "WINDOWS EVENT LOG",
    "SYSTEM MONITOR"
};

/* ==================== GLOBAL VARIABLES ==================== */
HANDLE hConsole = NULL;
int services_started = 0;
int services_failed = 0;
int services_not_found = 0;
int services_already_running = 0;
int sysmon_installed = 0;

/* ==================== UTILITY FUNCTIONS ==================== */
void SetColor(int color) {
    if (hConsole != NULL) {
        SetConsoleTextAttribute(hConsole, color);
    }
}

void ClearScreen(void) {
    system("cls");
}

void SafeSleep(DWORD milliseconds) {
    Sleep(milliseconds);
}

BOOL IsAdmin(void) {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin;
}

/* ==================== SYSMON INSTALLATION ==================== */
BOOL DownloadFile(const char* url, const char* outputPath) {
    HRESULT hr = URLDownloadToFileA(NULL, url, outputPath, 0, NULL);
    return SUCCEEDED(hr);
}

BOOL ExtractZip(const char* zipPath, const char* destPath) {
    char command[1024];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    sprintf(command, "powershell -Command \"Expand-Archive -Path '%s' -DestinationPath '%s' -Force\"", zipPath, destPath);
    
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    
    if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 60000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return TRUE;
    }
    return FALSE;
}

BOOL InstallSysmon(void) {
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hService = NULL;
    BOOL result = FALSE;
    char sysmonPath[MAX_PATH];
    char command[MAX_PATH * 2];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DWORD exitCode;
    
    /* Check if Sysmon is already installed */
    hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCManager != NULL) {
        hService = OpenServiceA(hSCManager, "Sysmon", SERVICE_QUERY_STATUS);
        if (hService != NULL) {
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCManager);
            sysmon_installed = 1;
            return TRUE;
        }
        CloseServiceHandle(hSCManager);
    }
    
    GetSystemDirectoryA(sysmonPath, MAX_PATH);
    
    /* Download Sysmon */
    SetColor(COLOR_GREEN);
    printf("\n");
    printf("  [INSTALANDO] SYSMON - POR FAVOR ESPERA...\n");
    SetColor(COLOR_DEFAULT);
    
    char tempPath[MAX_PATH];
    char sysmonZip[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    sprintf(sysmonZip, "%ssysmon.zip", tempPath);
    
    if (!DownloadFile("https://download.sysinternals.com/files/Sysmon.zip", sysmonZip)) {
        SetColor(COLOR_RED);
        printf("  [ERRO] FALHA AO BAIXAR SYSMON\n");
        SetColor(COLOR_DEFAULT);
        return FALSE;
    }
    
    /* Extract and install Sysmon */
    char extractPath[MAX_PATH];
    sprintf(extractPath, "%ssysmon_temp", tempPath);
    CreateDirectoryA(extractPath, NULL);
    
    ExtractZip(sysmonZip, extractPath);
    
    /* Find Sysmon.exe in extracted files */
    char sysmonExe[MAX_PATH];
    char searchCmd[MAX_PATH * 2];
    sprintf(searchCmd, "dir /b \"%s\\Sysmon*.exe\" 2>nul", extractPath);
    
    FILE* pipe = _popen(searchCmd, "r");
    if (pipe != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            /* Remove newline */
            buffer[strcspn(buffer, "\r\n")] = 0;
            sprintf(sysmonExe, "%s\\%s", extractPath, buffer);
            
            /* Copy to System32 */
            char destPath[MAX_PATH];
            sprintf(destPath, "%s\\Sysmon.exe", sysmonPath);
            CopyFileA(sysmonExe, destPath, FALSE);
            
            /* Install with accept eula */
            sprintf(command, "\"%s\" -accepteula -i", destPath);
            
            memset(&si, 0, sizeof(si));
            memset(&pi, 0, sizeof(pi));
            si.cb = sizeof(si);
            
            if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                WaitForSingleObject(pi.hProcess, 30000);
                GetExitCodeProcess(pi.hProcess, &exitCode);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                
                if (exitCode == 0) {
                    result = TRUE;
                    sysmon_installed = 1;
                }
            }
        }
        _pclose(pipe);
    }
    
    /* Cleanup */
    char cleanupCmd[MAX_PATH * 3];
    sprintf(cleanupCmd, "rmdir /s /q \"%s\" & del \"%s\"", extractPath, sysmonZip);
    system(cleanupCmd);
    
    return result;
}

/* ==================== SERVICE MANAGEMENT FUNCTIONS ==================== */
SC_HANDLE OpenSCManagerWrapper(void) {
    return OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
}

SC_HANDLE OpenServiceHandle(SC_HANDLE hSCManager, const char* serviceName, DWORD access) {
    return OpenServiceA(hSCManager, serviceName, access);
}

void CloseServiceHandleSafe(SC_HANDLE hService) {
    if (hService != NULL) {
        CloseServiceHandle(hService);
    }
}

BOOL GetServiceStatusEx(SC_HANDLE hService, SERVICE_STATUS* pStatus) {
    return QueryServiceStatus(hService, pStatus);
}

BOOL StartServiceByName(SC_HANDLE hSCManager, const char* serviceName) {
    SC_HANDLE hService = NULL;
    BOOL result = FALSE;
    SERVICE_STATUS serviceStatus;
    DWORD lastError;

    hService = OpenServiceHandle(hSCManager, serviceName,
                                  SERVICE_START | SERVICE_QUERY_STATUS);

    if (hService != NULL) {
        if (StartServiceA(hService, 0, NULL)) {
            while (GetServiceStatusEx(hService, &serviceStatus)) {
                if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
                    result = TRUE;
                    break;
                }
                if (serviceStatus.dwCurrentState == SERVICE_START_PENDING) {
                    SafeSleep(100);
                } else {
                    break;
                }
            }
        } else {
            lastError = GetLastError();
            if (lastError == ERROR_SERVICE_ALREADY_RUNNING) {
                result = TRUE;
                services_already_running++;
            } else if (lastError == ERROR_SERVICE_REQUEST_TIMEOUT) {
                if (GetServiceStatusEx(hService, &serviceStatus)) {
                    if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
                        result = TRUE;
                    }
                }
            }
        }

        CloseServiceHandleSafe(hService);
    } else {
        services_not_found++;
    }

    return result;
}

BOOL SetServiceAutoStart(SC_HANDLE hSCManager, const char* serviceName) {
    SC_HANDLE hService = NULL;
    BOOL result = FALSE;

    hService = OpenServiceHandle(hSCManager, serviceName,
                                  SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG);

    if (hService != NULL) {
        result = ChangeServiceConfigA(hService,
                                       SERVICE_NO_CHANGE,
                                       SERVICE_AUTO_START,
                                       SERVICE_NO_CHANGE,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL);

        CloseServiceHandleSafe(hService);
    }

    return result;
}

BOOL ServiceExists(SC_HANDLE hSCManager, const char* serviceName) {
    SC_HANDLE hService = NULL;
    BOOL exists = FALSE;

    hService = OpenServiceHandle(hSCManager, serviceName, SERVICE_QUERY_CONFIG);

    if (hService != NULL) {
        exists = TRUE;
        CloseServiceHandleSafe(hService);
    }

    return exists;
}

/* ==================== PRINT FUNCTIONS ==================== */
void PrintHeader(void) {
    ClearScreen();
    
    SetColor(COLOR_WHITE);
    printf("\n");
    printf("  ===============================================================================\n");
    printf("                                                                              \n");
    printf("                         STAR SERVICES - ORG STAR CUP                        \n");
    printf("                                                                              \n");
    printf("  ===============================================================================\n");
    SetColor(COLOR_DEFAULT);
}

void PrintAdminWarning(void) {
    SetColor(COLOR_RED);
    printf("\n");
    printf("  [X] ERRO: PRIVILEGIOS DE ADMINISTRADOR NECESARIOS!\n");
    printf("\n");
    SetColor(COLOR_YELLOW);
    printf("  PARA EJECUTAR CORRECTAMENTE:\n");
    printf("  1. CIERRA ESTE PROGRAMA\n");
    printf("  2. CLIC DERECHO EN EL ARCHIVO .EXE\n");
    printf("  3. SELECCIONA 'EJECUTAR COMO ADMINISTRADOR'\n");
    printf("\n");
    SetColor(COLOR_GREEN);
    printf("  PRESIONA CUALQUIER TECLA PARA SALIR...\n");
    SetColor(COLOR_DEFAULT);
    getchar();
}

void PrintSectionTitle(const char* title) {
    printf("\n");
    SetColor(COLOR_WHITE);
    printf("  %s\n", title);
    SetColor(COLOR_DEFAULT);
}

void PrintSeparator(void) {
    SetColor(COLOR_WHITE);
    printf("  ");
    for (int i = 0; i < 76; i++) {
        printf("=");
    }
    printf("\n");
    SetColor(COLOR_DEFAULT);
}

void PrintServiceStatus(const char* name, const char* status, const char* message) {
    SetColor(COLOR_GREEN);
    printf("  [%-10s] %s %s\n", status, name, message);
    SetColor(COLOR_DEFAULT);
}

void PrintServiceError(const char* name, const char* message) {
    SetColor(COLOR_RED);
    printf("  [%-10s] %s %s\n", "ERRO", name, message);
    SetColor(COLOR_DEFAULT);
}

void PrintSummary(void) {
    printf("\n");
    PrintSeparator();
    
    SetColor(COLOR_WHITE);
    printf("  RESUMO DA ATIVACAO\n");
    SetColor(COLOR_DEFAULT);
    
    PrintSeparator();
    
    SetColor(COLOR_GREEN);
    printf("  SERVICOS ATIVADOS: %d/%d\n", services_started, TOTAL_SERVICES);
    SetColor(COLOR_DEFAULT);
    
    PrintSeparator();
    
    SetColor(COLOR_GRAY);
    printf("  PRESIONA CUALQUIER TECLA PARA SALIR...\n");
    SetColor(COLOR_DEFAULT);
}

/* ==================== MAIN PROCESSING FUNCTIONS ==================== */
void ProcessServices(void) {
    SC_HANDLE hSCManager = NULL;
    BOOL configResult;
    
    PrintHeader();
    
    /* Install Sysmon first */
    PrintSectionTitle("INSTALANDO COMPONENTES...");
    InstallSysmon();
    
    /* Open Service Control Manager */
    hSCManager = OpenSCManagerWrapper();

    if (hSCManager == NULL) {
        SetColor(COLOR_RED);
        printf("  [ERRO] NO SE PUDO ABRIR EL ADMINISTRADOR DE SERVICIOS\n");
        SetColor(COLOR_DEFAULT);
        return;
    }
    
    PrintSectionTitle("CONFIGURANDO SERVICIOS PARA INICIO AUTOMATICO...");
    
    for (int i = 0; i < TOTAL_SERVICES; i++) {
        configResult = SetServiceAutoStart(hSCManager, services[i]);
        if (configResult) {
            SetColor(COLOR_GREEN);
            printf("  [OK] %s CONFIGURADO\n", services[i]);
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
                SetColor(COLOR_YELLOW);
                printf("  [?] %s NAO ENCONTRADO\n", services[i]);
            } else {
                SetColor(COLOR_GRAY);
                printf("  [i] %s %lu\n", services[i], error);
            }
        }
        SetColor(COLOR_DEFAULT);
        SafeSleep(30);
    }

    PrintSectionTitle("INICIANDO SERVICIOS...");

    for (int i = 0; i < TOTAL_SERVICES; i++) {
        BOOL exists = ServiceExists(hSCManager, services[i]);
        BOOL started = FALSE;

        if (exists) {
            started = StartServiceByName(hSCManager, services[i]);

            if (!started) {
                DWORD error = GetLastError();
                if (error == ERROR_SERVICE_ALREADY_RUNNING) {
                    started = TRUE;
                }
            }
        }

        if (!exists) {
            SetColor(COLOR_YELLOW);
            printf("  [%-10s] %s NAO ENCONTRADO\n", "INFO", services[i]);
            SetColor(COLOR_DEFAULT);
            services_not_found++;
        } else if (started) {
            PrintServiceStatus(services[i], "ATIVADO", "ESTA ATIVADO!");
            services_started++;
        } else {
            PrintServiceError(services[i], "NAO PUDO SER INICIADO");
            services_failed++;
        }

        SafeSleep(50);
    }

    if (hSCManager != NULL) {
        CloseServiceHandle(hSCManager);
    }

    PrintSummary();
}

/* ==================== ENTRY POINT ==================== */
int main(int argc, char* argv[]) {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTitleW(L"StarServices - ORG STAR CUP");

    if (!IsAdmin()) {
        PrintHeader();
        PrintAdminWarning();
        return 1;
    }

    ProcessServices();

    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    getchar();

    return 0;
}
