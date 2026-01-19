/*
 * ╔══════════════════════════════════════════════════════════════════════════╗
 * ║                                                                          ║
 * ║    ███╗   ███╗███████╗██████╗ ██╗   ██╗ █████╗  ██████╗██╗  ██╗          ║
 * ║    ████╗ ████║██╔════╝██╔══██╗╚██╗ ██╔╝██╔══██╗██╔════╝██║  ██║          ║
 * ║    ██╔████╔██║█████╗  ██████╔╝ ╚████╔╝ ███████║██║     ███████║          ║
 * ║    ██║╚██╔╝██║██╔══╝  ██╔══██╗  ╚██╔╝  ██╔══██║██║     ██╔══██║          ║
 * ║    ██║ ╚═╝ ██║███████╗██║  ██║     ██║   ██║  ██║╚██████╗██║  ██║          ║
 * ║    ╚═╝     ╚═╝╚══════╝╚═╝  ╚═╝     ╚═╝   ╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝          ║
 * ║                                                                          ║
 * ║    StarServices - Tournament Anti-Cheat Service Manager                 ║
 * ║    Improved version based on KellerServices reverse engineering          ║
 * ║                                                                          ║
 * ║    Author: xpe.nettt                                                     ║
 * ║    Copyright (c) 2025-2030 xpe.nettt                                    ║
 * ║                                                                          ║
 * ╚══════════════════════════════════════════════════════════════════════════╝
 *
 * Compile: gcc -o StarServices.exe StarServices.c -ladvapi32 -lkernel32 -static
 *
 * Based on reverse engineering analysis of KellerServices.exe
 * Original: https://github.com/kellerzz/KellerServices
 * Original author: KellerSS
 *
 * IMPROVEMENTS OVER ORIGINAL:
 * - Enhanced console UI with better colors and visual effects
 * - More detailed error reporting and status feedback
 * - Improved service verification with state checks
 * - Added EventLog service (found in latest KellerServices.exe)
 * - Better visual feedback and professional ASCII art banner
 * - Optimized code structure and comments
 * - Added comprehensive documentation
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ==================== COLOR DEFINITIONS ==================== */
/* Using SetConsoleTextAttribute API from KERNEL32.dll */
#define COLOR_DEFAULT         7
#define COLOR_BLUE            9
#define COLOR_GREEN          10      /* Verde brillante - Éxito */
#define COLOR_CYAN           11      /* Cyan - Títulos */
#define COLOR_RED            12      /* Rojo brillante - Error */
#define COLOR_MAGENTA        13
#define COLOR_YELLOW         14      /* Amarillo - Advertencias */
#define COLOR_WHITE          15      /* Blanco - Texto normal */
#define COLOR_GRAY            8       /* Gris - Información secundaria */
#define COLOR_BRIGHT_GREEN   10
#define COLOR_BRIGHT_RED     12

/* ==================== SERVICE CONFIGURATION ==================== */
/*
 * Services managed based on KellerServices analysis:
 * Original KellerServices uses: PcaSvc, PlugPlay, DPS, DiagTrack,
 * SysMain, EventLog, Sysmon (7 services total)
 * Found in KellerServices.exe strings:
 * set "services=PcaSvc PlugPlay DPS DiagTrack SysMain EventLog Sysmon"
 */
#define TOTAL_SERVICES 7

/* Service configuration array - based on reverse engineering analysis */
static const char* services[TOTAL_SERVICES] = {
    "PcaSvc",        /* Program Compatibility Assistant */
    "PlugPlay",      /* Plug and Play */
    "DPS",           /* Diagnostic Policy Service */
    "DiagTrack",     /* Connected User Experiences and Telemetry */
    "SysMain",       /* Superfetch - Performance optimization */
    "EventLog",      /* Windows Event Log */
    "Sysmon"         /* System Monitor (modified version by KellerSS) */
};

/* Display names for each service */
static const char* service_display[TOTAL_SERVICES] = {
    "Program Compatibility Assistant",
    "Plug and Play",
    "Diagnostic Policy Service",
    "Telemetry and Experiences",
    "Superfetch",
    "Windows Event Log",
    "System Monitor"
};

/* Descriptions for each service */
static const char* service_desc[TOTAL_SERVICES] = {
    "Servicio de compatibilidad de programas",
    "Administrador de dispositivos Plug and Play",
    "Servicio de políticas de diagnóstico",
    "Experiencias del usuario y telemetría",
    "Optimización de memoria y rendimiento",
    "Registro de eventos de Windows",
    "Monitor del sistema (Sysinternals)"
};

/* ==================== GLOBAL VARIABLES ==================== */
HANDLE hConsole = NULL;
int services_started = 0;
int services_failed = 0;
int services_not_found = 0;
int services_already_running = 0;

/* ==================== UTILITY FUNCTIONS ==================== */

/*
 * Set console text color
 * Uses SetConsoleTextAttribute API from KERNEL32.dll
 * Found in KellerServices.exe imports
 */
void SetColor(int color) {
    if (hConsole != NULL) {
        SetConsoleTextAttribute(hConsole, color);
    }
}

/*
 * Print a separator line with ASCII characters
 */
void PrintSeparator(void) {
    SetColor(COLOR_CYAN);
    printf("  ");
    for (int i = 0; i < 60; i++) {
        printf("%c", 205);
    }
    printf("\n");
    SetColor(COLOR_DEFAULT);
}

/*
 * Print a blank line
 */
void PrintSpace(void) {
    printf("\n");
}

/*
 * Clear screen using system command
 * Based on: system("cls") found in similar tools
 */
void ClearScreen(void) {
    system("cls");
}

/*
 * Sleep for specified milliseconds
 * Uses Sleep() from KERNEL32.dll - found in all Windows executables
 */
void SafeSleep(DWORD milliseconds) {
    Sleep(milliseconds);
}

/*
 * Check if running with administrator privileges
 * Uses CheckTokenMembership from ADVAPI32.dll
 * Critical for service management operations
 */
BOOL IsAdmin(void) {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    /* Allocate and initialize SID for administrators group */
    if (AllocateAndInitializeSid(&ntAuthority, 2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &adminGroup)) {
        /* Check if current token has admin membership */
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin;
}

/* ==================== SERVICE MANAGEMENT FUNCTIONS ==================== */

/*
 * Open Service Control Manager
 * Uses OpenSCManagerA from ADVAPI32.dll
 * Required first step for any service operation
 */
SC_HANDLE OpenSCManagerWrapper(void) {
    /* Open SCManager with all access - based on KellerServices analysis */
    return OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
}

/*
 * Open a service for operations
 * Uses OpenServiceA from ADVAPI32.dll
 */
SC_HANDLE OpenServiceHandle(SC_HANDLE hSCManager, const char* serviceName, DWORD access) {
    return OpenServiceA(hSCManager, serviceName, access);
}

/*
 * Close service handle
 * Uses CloseServiceHandle from ADVAPI32.dll
 */
void CloseServiceHandleSafe(SC_HANDLE hService) {
    if (hService != NULL) {
        CloseServiceHandle(hService);
    }
}

/*
 * Get service status
 * Uses QueryServiceStatus from ADVAPI32.dll
 * Returns TRUE if successful, FALSE otherwise
 */
BOOL GetServiceStatusEx(SC_HANDLE hService, SERVICE_STATUS* pStatus) {
    return QueryServiceStatus(hService, pStatus);
}

/*
 * Get extended service status
 * Uses QueryServiceStatusEx from ADVAPI32.dll
 * Found in KellerServices.exe imports
 */
BOOL GetServiceStatusExEx(SC_HANDLE hService, SERVICE_STATUS* pStatus) {
    DWORD bytesNeeded;
    return QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)pStatus, sizeof(SERVICE_STATUS), &bytesNeeded);
}

/*
 * Start a service
 * Uses StartServiceA from ADVAPI32.dll
 * Main service activation function based on KellerServices logic:
 * "for %%i in (services) do net start %%i"
 */
BOOL StartServiceByName(SC_HANDLE hSCManager, const char* serviceName) {
    SC_HANDLE hService = NULL;
    BOOL result = FALSE;
    SERVICE_STATUS serviceStatus;
    DWORD lastError;

    /* Open service with start and query status access */
    hService = OpenServiceHandle(hSCManager, serviceName,
                                  SERVICE_START | SERVICE_QUERY_STATUS);

    if (hService != NULL) {
        /* Attempt to start the service using StartServiceA */
        if (StartServiceA(hService, 0, NULL)) {
            /* Wait for service to reach RUNNING state */
            while (GetServiceStatusEx(hService, &serviceStatus)) {
                if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
                    result = TRUE;
                    break;
                }
                /* Check for pending states */
                if (serviceStatus.dwCurrentState == SERVICE_START_PENDING) {
                    SafeSleep(100);
                } else {
                    /* Service failed to start or reached other state */
                    break;
                }
            }
        } else {
            /* Get error code using GetLastError from KERNEL32.dll */
            lastError = GetLastError();
            if (lastError == ERROR_SERVICE_ALREADY_RUNNING) {
                result = TRUE; /* Already running is still success */
                services_already_running++;
            } else if (lastError == ERROR_SERVICE_REQUEST_TIMEOUT) {
                /* Timeout - check if service is actually running */
                if (GetServiceStatusEx(hService, &serviceStatus)) {
                    if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
                        result = TRUE;
                    }
                }
            }
        }

        CloseServiceHandleSafe(hService);
    } else {
        /* Service not found - increments counter */
        services_not_found++;
    }

    return result;
}

/*
 * Configure service to start automatically
 * Uses ChangeServiceConfigA from ADVAPI32.dll
 * Found in KellerServices.exe imports
 * Based on logic: "sc config %%i start= auto"
 */
BOOL SetServiceAutoStart(SC_HANDLE hSCManager, const char* serviceName) {
    SC_HANDLE hService = NULL;
    BOOL result = FALSE;

    /* Open service with change config access */
    hService = OpenServiceHandle(hSCManager, serviceName,
                                  SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG);

    if (hService != NULL) {
        /* Change service config to auto-start using ChangeServiceConfigA */
        result = ChangeServiceConfigA(hService,
                                       SERVICE_NO_CHANGE,      /* Service type */
                                       SERVICE_AUTO_START,     /* Start type: automatic */
                                       SERVICE_NO_CHANGE,      /* Error control */
                                       NULL,                   /* Binary path */
                                       NULL,                   /* Load order group */
                                       NULL,                   /* Tag ID */
                                       NULL,                   /* Dependencies */
                                       NULL,                   /* Account name */
                                       NULL,                   /* Password */
                                       NULL);                  /* Display name */

        CloseServiceHandleSafe(hService);
    }

    return result;
}

/*
 * Check if service exists
 * Uses OpenSCManagerA and OpenServiceA from ADVAPI32.dll
 * Returns TRUE if service exists, FALSE otherwise
 */
BOOL ServiceExists(SC_HANDLE hSCManager, const char* serviceName) {
    SC_HANDLE hService = NULL;
    BOOL exists = FALSE;

    /* Try to open service with connect access */
    hService = OpenServiceHandle(hSCManager, serviceName, SERVICE_QUERY_CONFIG);

    if (hService != NULL) {
        exists = TRUE;
        CloseServiceHandleSafe(hService);
    }

    return exists;
}

/*
 * Get detailed service status string for display
 * Returns human-readable status description
 */
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

/*
 * Print main header banner with ASCII art
 * Enhanced visual design based on tournament tool aesthetics
 */
void PrintHeader(void) {
    ClearScreen();

    SetColor(COLOR_CYAN);
    printf("\n");
    printf("  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
           201, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205,
           205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205,
           205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205,
           205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 187);
    printf("  %c                                                              %c\n", 186, 186);
    printf("  %c       StarServices - Tournament Anti-Cheat Manager          %c\n", 186, 186);
    printf("  %c                                                              %c\n", 186, 186);
    printf("  %c       Enhanced version based on KellerServices analysis     %c\n", 186, 186);
    printf("  %c                                                              %c\n", 186, 186);
    printf("  %c       Compatible: Windows 10/11 (x64/x86)                   %c\n", 186, 186);
    printf("  %c       Services: PcaSvc, PlugPlay, DPS, DiagTrack,           %c\n", 186, 186);
    printf("  %c                 SysMain, EventLog, Sysmon                   %c\n", 186, 186);
    printf("  %c                                                              %c\n", 186, 186);
    printf("  %c       Author: xpe.nettt | Copyright 2025-2030               %c\n", 186, 186);
    printf("  %c                                                              %c\n", 186, 186);
    printf("  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
           200, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205,
           205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205,
           205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205,
           205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 188);
    printf("\n");
    SetColor(COLOR_DEFAULT);
}

/*
 * Print administrator privilege warning
 */
void PrintAdminWarning(void) {
    SetColor(COLOR_RED);
    printf("\n");
    printf("  [X] ERROR: Se requieren privilegios de administrador!\n\n");
    SetColor(COLOR_YELLOW);
    printf("  Para ejecutar correctamente, por favor:\n");
    printf("  1. Cierra este programa\n");
    printf("  2. Haz clic derecho en el archivo .exe\n");
    printf("  3. Selecciona 'Ejecutar como administrador'\n\n");
    SetColor(COLOR_GREEN);
    printf("  Presiona cualquier tecla para salir...");
    SetColor(COLOR_DEFAULT);
    getchar();
}

/*
 * Print service status line with colors
 */
void PrintServiceStatus(int index, const char* name, const char* displayName,
                        const char* desc, BOOL success, BOOL exists) {
    printf("  ");

    if (!exists) {
        SetColor(COLOR_YELLOW);
        printf("[?] %-12s NO ENCONTRADO", name);
        SetColor(COLOR_DEFAULT);
        printf("  %s\n", desc);
        printf("    ");
        SetColor(COLOR_GRAY);
        printf("-> Requiere instalacion manual\n");
        services_not_found++;
    } else if (success) {
        SetColor(COLOR_GREEN);
        printf("[OK] %-12s ACTIVADO", name);
        SetColor(COLOR_DEFAULT);
        printf("  %s\n", displayName);
        services_started++;
    } else {
        SetColor(COLOR_RED);
        printf("[!] %-12s ERROR", name);
        SetColor(COLOR_DEFAULT);
        printf("  %s\n", displayName);
        services_failed++;
    }
}

/*
 * Print summary statistics
 */
void PrintSummary(void) {
    PrintSpace();
    PrintSeparator();
    PrintSpace();

    SetColor(COLOR_CYAN);
    printf("  RESUMEN DE OPERACION\n");
    PrintSpace();
    SetColor(COLOR_DEFAULT);

    SetColor(COLOR_GREEN);
    printf("  [+] Servicios iniciados: %d\n", services_started);
    if (services_already_running > 0) {
        SetColor(COLOR_BLUE);
        printf("  [i] Ya estaban activos: %d\n", services_already_running);
    }
    SetColor(COLOR_RED);
    printf("  [-] Servicios con error: %d\n", services_failed);
    SetColor(COLOR_YELLOW);
    printf("  [?] Servicios no encontrados: %d\n", services_not_found);
    PrintSpace();

    int total = services_started + services_already_running + services_failed + services_not_found;

    if (services_failed == 0 && services_not_found == 0) {
        SetColor(COLOR_GREEN);
        printf("  [SUCCESS] Todos los servicios fueron activados correctamente!\n");
        SetColor(COLOR_DEFAULT);
    } else if (services_not_found > 0) {
        SetColor(COLOR_YELLOW);
        printf("  [INFO] Algunos servicios no fueron encontrados.\n");
        SetColor(COLOR_GRAY);
        printf("         Sysmon requiere instalacion manual.\n");
        printf("         Descarga: https://docs.microsoft.com/sysinternals\n");
        SetColor(COLOR_DEFAULT);
    }

    PrintSpace();
    PrintSeparator();
    PrintSpace();

    SetColor(COLOR_CYAN);
    printf("  CREDITOS Y INFORMACION\n");
    PrintSpace();
    SetColor(COLOR_WHITE);
    printf("  StarServices - Enhanced Tournament Anti-Cheat Manager\n");
    printf("  Based on KellerServices reverse engineering analysis\n");
    printf("  Original: github.com/kellerzz/KellerServices | KellerSS\n");
    PrintSpace();
    SetColor(COLOR_GRAY);
    printf("  xpe.nettt - Tournament Anti-Cheat Manager\n");
    printf("  Copyright (c) 2025-2030 xpe.nettt\n");
    SetColor(COLOR_DEFAULT);
}

/* ==================== MAIN PROCESSING FUNCTIONS ==================== */

/*
 * Process all services
 * Main logic based on KellerServices batch script analysis:
 * set "services=PcaSvc PlugPlay DPS DiagTrack SysMain EventLog Sysmon"
 * for %%i in (%services%) do sc config %%i start= auto
 * for %%i in (%services%) do net start %%i
 */
void ProcessServices(void) {
    SC_HANDLE hSCManager = NULL;
    BOOL configResult;

    PrintHeader();
    PrintSpace();

    /* Open Service Control Manager */
    hSCManager = OpenSCManagerWrapper();

    if (hSCManager == NULL) {
        SetColor(COLOR_RED);
        printf("  [ERROR] No se pudo abrir el Administrador de Servicios.\n");
        printf("         Codigo de error: %lu\n", GetLastError());
        SetColor(COLOR_DEFAULT);
        return;
    }

    PrintSpace();
    SetColor(COLOR_CYAN);
    printf("  Procesando servicios requeridos para Anti-Cheat...\n");
    printf("  (Basado en analisis de KellerServices.exe)\n");
    printf("  GitHub: https://github.com/kellerzz/KellerServices\n");
    PrintSpace();
    PrintSeparator();
    PrintSpace();

    /* First, configure all services to auto-start */
    SetColor(COLOR_WHITE);
    printf("  Configurando servicios para inicio automatico...\n\n");
    SetColor(COLOR_DEFAULT);

    for (int i = 0; i < TOTAL_SERVICES; i++) {
        configResult = SetServiceAutoStart(hSCManager, services[i]);
        if (configResult) {
            SetColor(COLOR_GREEN);
            printf("  [OK] %-12s Configurado\n", services[i]);
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
                SetColor(COLOR_YELLOW);
                printf("  [?] %-12s No encontrado\n", services[i]);
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

    /* Then, start all services */
    SetColor(COLOR_WHITE);
    printf("  Iniciando servicios...\n\n");
    SetColor(COLOR_DEFAULT);

    /* Process each service */
    for (int i = 0; i < TOTAL_SERVICES; i++) {
        BOOL exists = ServiceExists(hSCManager, services[i]);
        BOOL started = FALSE;

        if (exists) {
            /* Attempt to start the service using StartServiceByName */
            started = StartServiceByName(hSCManager, services[i]);

            if (!started) {
                DWORD error = GetLastError();
                if (error == ERROR_SERVICE_ALREADY_RUNNING) {
                    started = TRUE; /* Consider already running as success */
                }
            }
        }

        /* Print status for this service */
        PrintServiceStatus(i + 1, services[i], service_display[i],
                          service_desc[i], started, exists);

        /* Small delay for visual effect */
        SafeSleep(50);
    }

    /* Close Service Control Manager handle */
    if (hSCManager != NULL) {
        CloseServiceHandle(hSCManager);
    }

    /* Print summary */
    PrintSummary();
}

/* ==================== ENTRY POINT ==================== */

/*
 * Main function - program entry point
 * Uses GetStdHandle from KERNEL32.dll for console output
 * Uses SetConsoleTitleW from KERNEL32.dll for window title
 * Found in KellerServices.exe: SetConsoleTitleW, GetStdHandle
 */
int main(int argc, char* argv[]) {
    /*
     * Entry point based on reverse engineering of KellerServices.exe
     *
     * APIs used:
     * - GetStdHandle (KERNEL32.dll) - Get console handle
     * - SetConsoleTitleW (KERNEL32.dll) - Set window title
     * - IsAdmin (ADVAPI32.dll) - Check admin privileges
     * - OpenSCManagerA (ADVAPI32.dll) - Open service manager
     * - OpenServiceA (ADVAPI32.dll) - Open service
     * - StartServiceA (ADVAPI32.dll) - Start service
     * - QueryServiceStatus (ADVAPI32.dll) - Query service status
     * - ChangeServiceConfigA (ADVAPI32.dll) - Configure service
     * - SetConsoleTextAttribute (KERNEL32.dll) - Set console colors
     */

    /* Get console handle using GetStdHandle from KERNEL32.dll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    /* Set console window title using SetConsoleTitleW from KERNEL32.dll */
    SetConsoleTitleW(L"StarServices - Tournament Anti-Cheat Manager");

    /* Check administrator privileges - required for service operations */
    if (!IsAdmin()) {
        PrintHeader();
        PrintAdminWarning();
        return 1;
    }

    /* Run main service processing */
    ProcessServices();

    /* Wait for user input before closing */
    PrintSpace();
    SetColor(COLOR_GRAY);
    printf("  Presiona cualquier tecla para salir...");
    SetColor(COLOR_DEFAULT);

    /* Flush input buffer and wait for key press */
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    getchar();

    return 0;
}
