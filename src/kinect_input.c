#include "kinect_input.h"
#include "gesture.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <NuiApi.h>
#endif

/* Verbosidad (0 = silencioso) */
#define KI_VERBOSE 0

/* Estado interno */
static INuiSensor* g_pNui = NULL;
static int g_initialized = 0;

static double now_ms(void){
#ifdef _WIN32
    return (double)GetTickCount64();
#else
    return 0.0;
#endif
}

int ki_init(void){
#ifdef _WIN32
    int count = 0;
    HRESULT hr = NuiGetSensorCount(&count);
    if (FAILED(hr) || count<1){
        if (KI_VERBOSE) fprintf(stderr,"[kinect_input] No hay sensores Kinect.\n");
        return -1;
    }
    hr = NuiCreateSensorByIndex(0, &g_pNui);
    if (FAILED(hr) || !g_pNui){
        if (KI_VERBOSE) fprintf(stderr,"[kinect_input] NuiCreateSensorByIndex fallo.\n");
        return -1;
    }

    hr = g_pNui->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
    if (FAILED(hr)){
        if (KI_VERBOSE) fprintf(stderr,"[kinect_input] NuiInitialize fallo 0x%08X\n",(unsigned)hr);
        g_pNui->Release(); g_pNui=NULL; return -1;
    }
    hr = g_pNui->NuiSkeletonTrackingEnable(NULL, 0);
    if (FAILED(hr)){
        if (KI_VERBOSE) fprintf(stderr,"[kinect_input] NuiSkeletonTrackingEnable fallo 0x%08X\n",(unsigned)hr);
        g_pNui->NuiShutdown(); g_pNui->Release(); g_pNui=NULL; return -1;
    }

    g_initialized = 1;
    printf("[kinect_input] Kinect v1 inicializado (SDK 1.8)\n");
    return 0;
#else
    (void)g_initialized;
    return -1;
#endif
}

void ki_shutdown(void){
#ifdef _WIN32
    if (g_pNui){ g_pNui->NuiShutdown(); g_pNui->Release(); g_pNui=NULL; }
    g_initialized=0;
#endif
}

static void fill_hand(Vector4 v, HandSample* h, double tms){
    h->x = v.x; h->y = v.y; h->z = v.z;
    h->t_ms = tms; h->visible = 1;
}

static int choose_primary_user(const NUI_SKELETON_FRAME* f){
    int best = -1; float bestZ=9999.0f;
    for (int i=0;i<NUI_SKELETON_COUNT;i++){
        const NUI_SKELETON_DATA* s = &f->SkeletonData[i];
        if (s->eTrackingState == NUI_SKELETON_TRACKED){
            float z = s->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER].z;
            if (z < bestZ){ bestZ=z; best=i; }
        }
    }
    return best;
}

void ki_read_hands(HandSample* right, HandSample* left){
#ifdef _WIN32
    if(!g_initialized){ memset(right,0,sizeof(*right)); memset(left,0,sizeof(*left)); return; }

    NUI_SKELETON_FRAME sf; ZeroMemory(&sf,sizeof(sf));
    HRESULT hr = g_pNui->NuiSkeletonGetNextFrame(0,&sf);
    double tms = now_ms();

    if (FAILED(hr)){
        /* Silencioso: no spamear el FAILED */
        memset(right,0,sizeof(*right)); memset(left,0,sizeof(*left));
        right->t_ms = left->t_ms = tms; return;
    }

    g_pNui->NuiTransformSmooth(&sf,NULL);

    int idx = choose_primary_user(&sf);
    memset(right,0,sizeof(*right));
    memset(left,0,sizeof(*left));
    right->t_ms=left->t_ms=tms;

    if (idx < 0) return;

    const NUI_SKELETON_DATA* s = &sf.SkeletonData[idx];

    if (s->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] != NUI_SKELETON_POSITION_NOT_TRACKED){
        fill_hand(s->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], right, tms);
    }
    if (s->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] != NUI_SKELETON_POSITION_NOT_TRACKED){
        fill_hand(s->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], left, tms);
    }
#else
    (void)right; (void)left;
#endif
}
