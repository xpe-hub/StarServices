/*
 * ╔══════════════════════════════════════════════════════════════════════════════╗
 * ║                                                                              ║
 * ║                          STAR SERVICES                                       ║
 * ║                                                                              ║
 * ║                         Star Cup System                                      ║
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
    "Program Compatibility Assistant",
    "Plug and Play",
    "Diagnostic Policy Service",
    "Telemetry and Experiences",
    "Superfetch",
    "Windows Event Log",
    "System Monitor"
};

static const char* service_desc[TOTAL_SERVICES] = {
    "Servicio de compatibilidad de programas",
    "Administrador de dispositivos",
    "Servicio de politicas de diagnostico",
    "Experiencias y telemetria",
    "Optimizacion de memoria y rendimiento",
    "Registro de eventos de Windows",
    "Monitor del sistema"
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

void PrintLine(void) {
    SetColor(COLOR_WHITE);
    printf("\n");
    printf("                                                                              ");
    printf("\n");
    SetColor(COLOR_DEFAULT);
}

void PrintSeparator(void) {
    SetColor(COLOR_CYAN);
    printf("\n");
    printf("                                                                              ");
    printf("\n");
    SetColor(COLOR_DEFAULT);
}

void PrintSpace(void) {
    printf("\n");
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

BOOL InstallSysmon(void) {
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hService = NULL;
    BOOL result = FALSE;
    char sysmonPath[MAX_PATH];
    char command[MAX_PATH * 2];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DWORD exitCode;
    
    GetSystemDirectoryA(sysmonPath, MAX_PATH);
    strcat(sysmonPath, "\\sysmon.exe");
    
    /* Check if Sysmon is already installed */
    hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCManager != NULL) {
        hService = OpenServiceA(hSCManager, "Sysmon", SERVICE_QUERY_STATUS);
        if (hService != NULL) {
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCManager);
            return TRUE; /* Already installed */
        }
        CloseServiceHandle(hSCManager);
    }
    
    /* Download Sysmon */
    SetColor(COLOR_CYAN);
    printf("\n");
    printf("  DESCARGANDO SYSMON...\n");
    printf("  Por favor espera un momento...\n");
    SetColor(COLOR_DEFAULT);
    
    char tempPath[MAX_PATH];
    char sysmonZip[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    sprintf(sysmonZip, "%ssysmon.zip", tempPath);
    
    if (!DownloadFile("https://download.sysinternals.com/files/Sysmon.zip", sysmonZip)) {
        SetColor(COLOR_RED);
        printf("  [ERROR] No se pudo descargar Sysmon\n");
        SetColor(COLOR_DEFAULT);
        return FALSE;
    }
    
    /* Extract Sysmon */
    SetColor(COLOR_CYAN);
    printf("  EXTRACTENDO ARCHIVOS...\n");
    SetColor(COLOR_DEFAULT);
    
    char extractCmd[MAX_PATH * 3];
    sprintf(extractCmd, "powershell -Command \"Expand-Archive -Path '%s' -DestinationPath '%ssysmon_temp -Force\"", sysmonZip, tempPath);
    
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    
    if (CreateProcessA(NULL, extractCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    /* Copy Sysmon to System32 */
    char sysmonTemp[MAX_PATH];
    sprintf(sysmonTemp, "%ssysmon_temp\\Sysmon.exe", tempPath);
    
    if (CopyFileA(sysmonTemp, sysmonPath, FALSE)) {
        /* Install Sysmon with accept eula */
        sprintf(command, "\"%s\" -accepteula -i", sysmonPath);
        
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
        si.cb = sizeof(si);
        
        SetColor(COLOR_CYAN);
        printf("  INSTALANDO SYSMON...\n");
        SetColor(COLOR_DEFAULT);
        
        if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 30000);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            
            if (exitCode == 0) {
                result = TRUE;
                sysmon_installed = 1;
                
                /* Cleanup */
                char cleanupCmd[MAX_PATH * 2];
                sprintf(cleanupCmd, "rmdir /s /q \"%ssysmon_temp\" & del \"%s\"", tempPath, sysmonZip);
                system(cleanupCmd);
            }
        }
    }
    
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

BOOL GetServiceStatusExEx(SC_HANDLE hService, SERVICE_STATUS* pStatus) {
    DWORD bytesNeeded;
    return QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)pStatus, sizeof(SERVICE_STATUS), &bytesNeeded);
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

const char* GetServiceStatusString(DWORD state) {
    switch (state) {
        case SERVICE_STOPPED:
            return "DETENIDO";
        case SERVICE_START_PENDING:
            return "INICIANDO...";
        case SERVICE_STOP_PENDING:
            return "DETENIENDO...";
        case SERVICE_RUNNING:
            return "EJECUTANDOSE";
        case SERVICE_CONTINUE_PENDING:
            return "REANUDANDO...";
        case SERVICE_PAUSE_PENDING:
            return "PAUSANDO...";
        case SERVICE_PAUSED:
            return "PAUSADO";
        default:
            return "DESCONOCIDO";
    }
}

/* ==================== PRINT FUNCTIONS ==================== */
void PrintHeader(void) {
    ClearScreen();
    
    SetColor(COLOR_CYAN);
    printf("\n");
    printf("                                                                              ");
    printf("\n");
    printf("                                                                              ");
    printf("\n");
    
    SetColor(COLOR_WHITE);
    printf("                              STAR SERVICES                                    ");
    printf("\n");
    
    SetColor(COLOR_CYAN);
    printf("                              STAR CUP SYSTEM                                  ");
    printf("\n");
    
    printf("                                                                              ");
    printf("\n");
    printf("                                                                              ");
    printf("\n");
    SetColor(COLOR_DEFAULT);
}

void PrintAdminWarning(void) {
    SetColor(COLOR_RED);
    printf("\n");
    printf("                      [X] ERROR: SE REQUIEREN PRIVILEGIOS DE ADMINISTRADOR\n");
    printf("\n");
    SetColor(COLOR_YELLOW);
    printf("                      Para ejecutar correctamente:\n");
    printf("                      1. Cierra este programa\n");
    printf("                      2. Clic derecho en el archivo .exe\n");
    printf("                      3. Selecciona 'Ejecutar como administrador'\n");
    printf("\n");
    SetColor(COLOR_GREEN);
    printf("                      Presiona cualquier tecla para salir...\n");
    SetColor(COLOR_DEFAULT);
    getchar();
}

void PrintServiceStatus(int index, const char* name, const char* displayName,
                        const char* desc, BOOL success, BOOL exists) {
    
    if (!exists) {
        SetColor(COLOR_YELLOW);
        printf("  [?] %-12s NO ENCONTRADO", name);
        SetColor(COLOR_DEFAULT);
        printf("  %s\n", desc);
    } else if (success) {
        SetColor(COLOR_GREEN);
        printf("  [OK] %-12s ACTIVADO", name);
        SetColor(COLOR_DEFAULT);
        printf("  %s\n", displayName);
        services_started++;
    } else {
        SetColor(COLOR_RED);
        printf("  [!] %-12s ERROR", name);
        SetColor(COLOR_DEFAULT);
        printf("  %s\n", displayName);
        services_failed++;
    }
}

void PrintSummary(void) {
    PrintSeparator();
    PrintSpace();
    
    SetColor(COLOR_WHITE);
    printf("                           RESUMEN DE OPERACION                               ");
    PrintSpace();
    PrintSpace();
    
    SetColor(COLOR_GREEN);
    printf("                           [+] Servicios iniciados: %d\n", services_started);
    if (services_already_running > 0) {
        SetColor(COLOR_BLUE);
        printf("                           [i] Ya estaban activos: %d\n", services_already_running);
    }
    SetColor(COLOR_RED);
    printf("                           [-] Servicios con error: %d\n", services_failed);
    SetColor(COLOR_YELLOW);
    printf("                           [?] No encontrados: %d\n", services_not_found);
    PrintSpace();
    
    if (services_failed == 0 && services_not_found == 0) {
        SetColor(COLOR_GREEN);
        printf("                    [SUCCESS] Todos los servicios fueron activados!         ");
        PrintSpace();
    }
    
    PrintSeparator();
    PrintSpace();
    
    SetColor(COLOR_CYAN);
    printf("                              STAR CUP SYSTEM                                 ");
    PrintSpace();
    SetColor(COLOR_GRAY);
    printf("                              Copyright 2025-2030                             ");
    PrintSpace();
    SetColor(COLOR_DEFAULT);
}

/* ==================== MAIN PROCESSING FUNCTIONS ==================== */
void ProcessServices(void) {
    SC_HANDLE hSCManager = NULL;
    BOOL configResult;
    
    PrintHeader();
    
    /* Install Sysmon first */
    SetColor(COLOR_CYAN);
    printf("                      VERIFICANDO E INSTALANDO COMPONENTES...                ");
    PrintSpace();
    SetColor(COLOR_DEFAULT);
    
    InstallSysmon();
    
    PrintSpace();
    
    /* Open Service Control Manager */
    hSCManager = OpenSCManagerWrapper();

    if (hSCManager == NULL) {
        SetColor(COLOR_RED);
        printf("  [ERROR] No se pudo abrir el Administrador de Servicios.\n");
        SetColor(COLOR_DEFAULT);
        return;
    }
    
    PrintSpace();
    SetColor(COLOR_CYAN);
    printf("                      CONFIGURANDO SERVICIOS PARA INICIO AUTOMATICO...       ");
    PrintSpace();
    PrintSpace();
    SetColor(COLOR_DEFAULT);

    for (int i = 0; i < TOTAL_SERVICES; i++) {
        configResult = SetServiceAutoStart(hSCManager, services[i]);
        if (configResult) {
            SetColor(COLOR_GREEN);
            printf("  [OK] %-12s CONFIGURADO\n", services[i]);
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
                SetColor(COLOR_YELLOW);
                printf("  [?] %-12s NO ENCONTRADO\n", services[i]);
            } else {
                SetColor(COLOR_GRAY);
                printf("  [i] %-12s %lu\n", services[i], error);
            }
        }
        SetColor(COLOR_DEFAULT);
        SafeSleep(30);
    }

    PrintSpace();
    PrintSeparator();
    PrintSpace();

    SetColor(COLOR_CYAN);
    printf("                              INICIANDO SERVICIOS...                          ");
    PrintSpace();
    PrintSpace();
    SetColor(COLOR_DEFAULT);

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

        PrintServiceStatus(i + 1, services[i], service_display[i],
                          service_desc[i], started, exists);

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
    SetConsoleTitleW(L"StarServices - Star Cup System");

    if (!IsAdmin()) {
        PrintHeader();
        PrintAdminWarning();
        return 1;
    }

    ProcessServices();

    PrintSpace();
    SetColor(COLOR_GRAY);
    printf("                      Presiona cualquier tecla para salir...                 ");
    SetColor(COLOR_DEFAULT);

    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    getchar();

    return 0;
}
