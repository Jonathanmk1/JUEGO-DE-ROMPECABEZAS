#include "kinect_input.h"

/* IMPLEMENTACIÓN PARA KINECT v1 (Xbox 360) CON MICROSOFT KINECT SDK 1.8
   Requiere:
     - Incluir headers del SDK: <Windows.h>, <NuiApi.h>
     - Vincular con Kinect10.lib
     - Compilar con MSVC (Visual Studio). MinGW NO enlaza con Kinect10.lib.
*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <NuiApi.h>
#include <stdio.h>
#include <string.h>

static int g_inited = 0;
static HANDLE g_skelEvent = NULL;

/* Utilidad: timestamp en ms */
static double now_ms(void){
    return (double)GetTickCount64();
}

int ki_init(void){
    if (g_inited) return 0;

    HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
    if (FAILED(hr)) {
        fprintf(stderr, "[kinect_input] NuiInitialize falló (hr=0x%08X)\n", (unsigned)hr);
        return -1;
    }

    /* Crear evento para frames de esqueleto */
    g_skelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!g_skelEvent){
        fprintf(stderr, "[kinect_input] CreateEvent falló\n");
        NuiShutdown();
        return -1;
    }

    /* Habilitar skeleton tracking (con soporte de seated para estabilidad en escritorio) */
    DWORD flags = NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT;
    hr = NuiSkeletonTrackingEnable(g_skelEvent, flags);
    if (FAILED(hr)){
        fprintf(stderr, "[kinect_input] NuiSkeletonTrackingEnable falló (hr=0x%08X)\n", (unsigned)hr);
        CloseHandle(g_skelEvent); g_skelEvent = NULL;
        NuiShutdown();
        return -1;
    }

    g_inited = 1;
    fprintf(stdout, "[kinect_input] Kinect v1 inicializado (SDK 1.8)\n");
    return 0;
}

void ki_shutdown(void){
    if (!g_inited) return;
    NuiShutdown();
    if (g_skelEvent){ CloseHandle(g_skelEvent); g_skelEvent = NULL; }
    g_inited = 0;
}

/* Convierte NUI_SKELETON_DATA a HandSample (mano específica) */
static void hand_from_skel(const NUI_SKELETON_DATA* sd, NUI_SKELETON_POSITION_INDEX hand_idx, HandSample* out){
    if (!sd || !out) return;
    NUI_SKELETON_POSITION_TRACKING_STATE tstate = sd->eSkeletonPositionTrackingState[hand_idx];
    if (sd->eTrackingState == NUI_SKELETON_TRACKED &&
        (tstate == NUI_SKELETON_POSITION_TRACKED || tstate == NUI_SKELETON_POSITION_INFERRED)){
        Vector4 p = sd->SkeletonPositions[hand_idx];
        out->x = p.x;  /* Coordenadas en metros aprox */
        out->y = p.y;
        out->z = p.z;
        out->visible = (tstate == NUI_SKELETON_POSITION_TRACKED) ? 1 : 0;
        out->t_ms = now_ms();
    } else {
        out->x = out->y = out->z = 0.0f;
        out->visible = 0;
        out->t_ms = now_ms();
    }
}

int ki_read_hands(HandSample* right, HandSample* left){
    if (right){ memset(right, 0, sizeof(*right)); right->t_ms = now_ms(); }
    if (left){  memset(left,  0, sizeof(*left));  left->t_ms  = now_ms(); }

    if (!g_inited) return -1;

    static double last_diag = 0.0;
    NUI_SKELETON_FRAME frame;
    HRESULT hr = NuiSkeletonGetNextFrame(0, &frame);  // consulta directa (sin evento)

    if (FAILED(hr)) {
        // Diagnóstico cada ~1s para no inundar
        double t = now_ms();
        if (t - last_diag > 1000.0) {
            fprintf(stderr, "[kinect_input] NuiSkeletonGetNextFrame FAILED hr=0x%08X\n", (unsigned)hr);
            last_diag = t;
        }
        return -1;
    }

    // Suavizado opcional
    NuiTransformSmooth(&frame, NULL);

    // Busca TRACKED, y si no, POSITION_ONLY como fallback
    const NUI_SKELETON_DATA* best_tracked = NULL;
    const NUI_SKELETON_DATA* best_posonly = NULL;
    int tracked_count = 0, posonly_count = 0;

    for (int i=0; i<NUI_SKELETON_COUNT; ++i) {
        const NUI_SKELETON_DATA* sd = &frame.SkeletonData[i];
        if (sd->eTrackingState == NUI_SKELETON_TRACKED) {
            tracked_count++;
            if (!best_tracked) best_tracked = sd;
        } else if (sd->eTrackingState == NUI_SKELETON_POSITION_ONLY) {
            posonly_count++;
            if (!best_posonly) best_posonly = sd;
        }
    }

    // Log de diagnóstico 1 vez/seg
    double t = now_ms();
    if (t - last_diag > 1000.0) {
        fprintf(stdout, "[kinect_input] tracked=%d posOnly=%d\n", tracked_count, posonly_count);
        last_diag = t;
    }

    const NUI_SKELETON_DATA* use = best_tracked ? best_tracked : best_posonly;
    if (!use) {
        // No ve cuerpo: coords quedan en 0 / visible=0
        return 0;
    }

    if (right) hand_from_skel(use, NUI_SKELETON_POSITION_HAND_RIGHT, right);
    if (left)  hand_from_skel(use, NUI_SKELETON_POSITION_HAND_LEFT,  left);

    return 0;
}
