// src/main_sdl_kinect.cpp
#define SDL_MAIN_HANDLED
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <Windows.h>
#include <Ole2.h>
#include <OleAuto.h>
#include <initguid.h>
#include <NuiApi.h> // Kinect v1.8
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// ===================== CLI =====================
struct Args {
    std::string imagePath;
    int grid = 3;
    int winSize = 800;
};
static void usage(const char* exe){
    std::fprintf(stderr,"Uso: %s --image <ruta> [--grid 2|3|4] [--size 800]\n", exe);
}
static bool parse_args(int argc, char** argv, Args& a){
    if (argc < 3) return false;
    for (int i=1;i<argc;i++){
        std::string s = argv[i];
        if (s=="--image" && i+1<argc) a.imagePath = argv[++i];
        else if (s=="--grid" && i+1<argc){
            a.grid = std::atoi(argv[++i]);
            if(a.grid!=2 && a.grid!=3 && a.grid!=4) a.grid=3;
        }
        else if (s=="--size" && i+1<argc){
            a.winSize = std::max(300, std::atoi(argv[++i]));
        }
    }
    return !a.imagePath.empty();
}

// ===================== Puzzle =====================
struct Puzzle{ int N=3; std::vector<int> t; int empty=0; };
static inline int IX(int r,int c,int N){ return r*N+c; }
static void pu_init(Puzzle& p,int N){ p.N=N; p.t.resize(N*N); for(int i=0;i<N*N;i++) p.t[i]=i; p.empty=N*N-1; }
static bool pu_can(const Puzzle& p,int dr,int dc){
    int r=p.empty/p.N, c=p.empty%p.N; int rr=r-dr, cc=c-dc; return rr>=0&&rr<p.N&&cc>=0&&cc<p.N;
}
static bool pu_move(Puzzle& p,int dr,int dc){
    if(!pu_can(p,dr,dc)) return false;
    int r=p.empty/p.N, c=p.empty%p.N; int rr=r-dr, cc=c-dc; int from=IX(rr,cc,p.N);
    std::swap(p.t[from], p.t[p.empty]); p.empty=from; return true;
}
static bool pu_solved(const Puzzle& p){ for(int i=0;i<p.N*p.N-1;i++) if(p.t[i]!=i) return false; return true; }
static void pu_shuffle(Puzzle& p,unsigned seed,int steps){
    std::srand(seed);
    for(int k=0;k<steps;k++){
        switch(std::rand()%4){ case 0: pu_move(p,0,-1); break; case 1: pu_move(p,0,1); break;
                               case 2: pu_move(p,-1,0); break; case 3: pu_move(p,1,0); break; }
    }
}

// ===================== GFX =====================
struct Tile{ SDL_Rect src{}, dst{}; };
struct Gfx{
    SDL_Window* win=nullptr; SDL_Renderer* ren=nullptr; SDL_Texture* tex=nullptr;
    std::vector<Tile> tiles; int cell=0;
    bool own_sdl=false, own_win=false, own_ren=false;

    // Referencia completa y HUD
    SDL_Rect fullSrc{};   // toda la imagen
    int imgW=0, imgH=0;

    // Cronómetro (con pausa)
    uint32_t startTicks=0;
    bool     isPaused=false;
    uint32_t pauseStart=0;
    uint32_t pauseAccumMs=0;
};
static int clamp_win(int req){
    SDL_Rect u{}; if(SDL_GetDisplayUsableBounds(0,&u)!=0) SDL_GetDisplayBounds(0,&u);
    int m=80; int maxSide = std::min(std::max(300,u.w-m), std::max(300,u.h-m));
    return std::min(req,maxSide);
}
static bool ensure_img(){
    int want=IMG_INIT_PNG|IMG_INIT_JPG; int got=IMG_Init(want);
    if((got&(IMG_INIT_PNG|IMG_INIT_JPG))==0){
        std::fprintf(stderr,"[SDL_image] init falló: %s\n", IMG_GetError()); return false;
    }
    return true;
}

// init: si win/ren existen, reusa; si no, crea todo (modo CLI)
static bool gfx_init(Gfx& g,int& winSize, SDL_Window* existing_win, SDL_Renderer* existing_ren){
    if(!existing_win || !existing_ren){
        if(SDL_Init(SDL_INIT_VIDEO)<0){ std::fprintf(stderr,"[SDL] %s\n",SDL_GetError()); return false; }
        g.own_sdl = true;
        if(!ensure_img()) return false;
        winSize = clamp_win(winSize);
        g.win = SDL_CreateWindow("Rompecabezas Kinect", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winSize, winSize, SDL_WINDOW_ALLOW_HIGHDPI);
        if(!g.win){ std::fprintf(stderr,"[SDL] win: %s\n",SDL_GetError()); return false; }
        g.own_win = true;
        g.ren = SDL_CreateRenderer(g.win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
        if(!g.ren){ std::fprintf(stderr,"[SDL] ren: %s\n",SDL_GetError()); return false; }
        g.own_ren = true;
        return true;
    } else {
        if(!ensure_img()) return false;
        g.win = existing_win; g.ren = existing_ren;
        g.own_sdl = false; g.own_win = false; g.own_ren = false;
        int rw=0, rh=0; SDL_GetRendererOutputSize(g.ren, &rw, &rh);
        int side = std::max(300, std::min(rw, rh));
        winSize = side;
        return true;
    }
}
static void gfx_shutdown(Gfx& g){
    if(g.tex){SDL_DestroyTexture(g.tex); g.tex=nullptr;}
    IMG_Quit();
    if(g.own_ren && g.ren){SDL_DestroyRenderer(g.ren); g.ren=nullptr;}
    if(g.own_win && g.win){SDL_DestroyWindow(g.win); g.win=nullptr;}
    if(g.own_sdl){ SDL_Quit(); }
}

// --------- FONT 3x5 simple (dígitos, ':' y letras para "PAUSA") ----------
static bool font3x5_rows(char ch, uint8_t (&rows)[5]){
    switch(ch){
        // dígitos
        case '0': rows[0]=7; rows[1]=5; rows[2]=5; rows[3]=5; rows[4]=7; return true;
        case '1': rows[0]=2; rows[1]=6; rows[2]=2; rows[3]=2; rows[4]=7; return true;
        case '2': rows[0]=7; rows[1]=1; rows[2]=7; rows[3]=4; rows[4]=7; return true;
        case '3': rows[0]=7; rows[1]=1; rows[2]=7; rows[3]=1; rows[4]=7; return true;
        case '4': rows[0]=5; rows[1]=5; rows[2]=7; rows[3]=1; rows[4]=1; return true;
        case '5': rows[0]=7; rows[1]=4; rows[2]=7; rows[3]=1; rows[4]=7; return true;
        case '6': rows[0]=7; rows[1]=4; rows[2]=7; rows[3]=5; rows[4]=7; return true;
        case '7': rows[0]=7; rows[1]=1; rows[2]=2; rows[3]=4; rows[4]=4; return true;
        case '8': rows[0]=7; rows[1]=5; rows[2]=7; rows[3]=5; rows[4]=7; return true;
        case '9': rows[0]=7; rows[1]=5; rows[2]=7; rows[3]=1; rows[4]=7; return true;
        case ':': rows[0]=0; rows[1]=2; rows[2]=0; rows[3]=2; rows[4]=0; return true;
        // letras (PAUSA)
        case 'P': rows[0]=7; rows[1]=5; rows[2]=7; rows[3]=4; rows[4]=4; return true;
        case 'A': rows[0]=7; rows[1]=5; rows[2]=7; rows[3]=5; rows[4]=5; return true;
        case 'U': rows[0]=5; rows[1]=5; rows[2]=5; rows[3]=5; rows[4]=7; return true;
        case 'S': rows[0]=7; rows[1]=4; rows[2]=7; rows[3]=1; rows[4]=7; return true;
        default: return false;
    }
}
static void draw_bitmap_text(SDL_Renderer* ren, const std::string& s, int x, int y, int scale, SDL_Color col){
    if(scale<1) return;
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    int advance = 3*scale + scale; // 3 cols + 1 gap
    for(char ch : s){
        uint8_t rows[5];
        if(font3x5_rows(ch, rows)){
            for(int r=0;r<5;r++){
                for(int c=0;c<3;c++){
                    if(rows[r] & (1<<(2-c))){
                        SDL_Rect px{ x + c*scale, y + r*scale, scale, scale };
                        SDL_RenderFillRect(ren,&px);
                    }
                }
            }
        }
        x += advance;
    }
}
static int bitmap_text_width(int nchars, int scale){
    if(scale<1||nchars<=0) return 0;
    int advance = 3*scale + scale;
    return nchars*advance - scale;
}

static bool gfx_load(Gfx& g,const std::string& path,int N,int winSize,Puzzle& p){
    SDL_Surface* s=IMG_Load(path.c_str());
    if(!s){ std::fprintf(stderr,"[IMG] %s\n",IMG_GetError()); return false; }
    g.tex=SDL_CreateTextureFromSurface(g.ren,s);
    g.imgW = s->w; g.imgH = s->h;
    g.fullSrc = {0,0,g.imgW,g.imgH};
    int W=s->w,H=s->h; SDL_FreeSurface(s);

    g.tiles.clear(); int tw=W/N, th=H/N;

    int rw=0, rh=0; SDL_GetRendererOutputSize(g.ren, &rw, &rh);
    int side = (rw>0 && rh>0) ? std::min(rw, rh) : winSize;

    g.cell = side / N;
    for(int r=0;r<N;r++)for(int c=0;c<N;c++){
        if(r==N-1 && c==N-1) continue;
        Tile t; t.src={c*tw,r*th,tw,th}; t.dst={c*g.cell,r*g.cell,g.cell,g.cell}; g.tiles.push_back(t);
    }
    pu_init(p,N);
    g.startTicks = SDL_GetTicks(); // cronómetro desde que cargamos
    g.isPaused=false; g.pauseAccumMs=0; g.pauseStart=0;
    return true;
}

static void gfx_draw(Gfx& g,const Puzzle& p,bool solved){
    SDL_SetRenderDrawColor(g.ren,15,15,20,255); SDL_RenderClear(g.ren);

    // --- PUZZLE ---
    for(int r=0;r<p.N;r++){
        for(int c=0;c<p.N;c++){
            int v=p.t[IX(r,c,p.N)];
            SDL_Rect dst{c*g.cell, r*g.cell, g.cell, g.cell};
            if(v==p.N*p.N-1){
                SDL_SetRenderDrawColor(g.ren,60,60,80,255);
                SDL_RenderDrawRect(g.ren,&dst);
                continue;
            }
            const Tile& t=g.tiles[v];
            SDL_RenderCopy(g.ren,g.tex,&t.src,&dst);
            SDL_SetRenderDrawColor(g.ren,25,25,35,255);
            SDL_RenderDrawRect(g.ren,&dst);
        }
    }
    if(solved){
        SDL_SetRenderDrawColor(g.ren,0,180,0,255);
        SDL_Rect r{2,2,g.cell*p.N-4,g.cell*p.N-4};
        SDL_RenderDrawRect(g.ren,&r);
    }

    // --- PANEL DERECHO: referencia 75% + cronómetro ---
    int winW=0, winH=0; SDL_GetRendererOutputSize(g.ren,&winW,&winH);
    const int puzzleW = g.cell * p.N;
    const int margin  = 10;

    int xStart = std::max(winW/2, puzzleW + margin);
    int availW = std::max(0, winW - xStart);
    int availH = winH;

    if (g.tex && availW > 32 && availH > 32){
        // Reservamos espacio para el cronómetro
        int targetScaleText = std::max(2, std::min(24, availW/60)); // escala base del font
        int textH = 5 * targetScaleText;
        int textGap = 10;
        int spaceForImgH = std::max(0, availH - textH - textGap - margin);

        // Escala de la referencia con 75% del “fit” y centrada
        float scFit = std::min(spaceForImgH / float(g.imgH), availW / float(g.imgW));
        scFit = std::max(0.0f, scFit) * 0.75f; // 75%
        int dW = std::max(1, int(g.imgW * scFit));
        int dH = std::max(1, int(g.imgH * scFit));
        SDL_Rect dstRef{
            xStart + (availW - dW)/2,
            margin + (spaceForImgH - dH)/2,
            dW, dH
        };
        SDL_RenderCopy(g.ren, g.tex, &g.fullSrc, &dstRef);
        SDL_SetRenderDrawColor(g.ren,40,40,55,255);
        SDL_RenderDrawRect(g.ren, &dstRef);

        // Cronómetro mm:ss centrado debajo (respeta pausa)
        uint32_t now = SDL_GetTicks();
        uint32_t base = g.isPaused ? g.pauseStart : now;
        uint32_t elapsed = base - g.startTicks - g.pauseAccumMs;
        int totalSec = int(elapsed/1000u);
        int mm = (totalSec/60) % 100;
        int ss = totalSec%60;
        char buf[6]; std::snprintf(buf,sizeof(buf),"%02d:%02d",mm,ss);

        int nchars=5;
        int maxScaleByW = std::max(2, availW / (nchars*4 + 1)); // ancho aprox
        int scale = std::min(targetScaleText, maxScaleByW);
        int textWpx = bitmap_text_width(nchars, scale);
        int xText = xStart + (availW - textWpx)/2;
        int yText = dstRef.y + dstRef.h + textGap;
        if (yText + 5*scale > winH - margin) yText = winH - margin - 5*scale;

        SDL_Color fg{230,230,245,255};
        draw_bitmap_text(g.ren, buf, xText, yText, scale, fg);
    }

    // Overlay de PAUSA (suave)
    if(g.isPaused){
        SDL_BlendMode prev; SDL_GetRenderDrawBlendMode(g.ren, &prev);
        SDL_SetRenderDrawBlendMode(g.ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g.ren, 0, 0, 0, 140);
        SDL_Rect full{0,0,winW,winH};
        SDL_RenderFillRect(g.ren,&full);

        const char* word="PAUSA";
        int nchars=5, scale=12;
        int w = bitmap_text_width(nchars, scale);
        int x = (winW - w)/2;
        int y = (winH - 5*scale)/2;
        SDL_Color fg{240,240,255,255};
        draw_bitmap_text(g.ren, word, x, y, scale, fg);
        SDL_SetRenderDrawBlendMode(g.ren, prev);
    }

    SDL_RenderPresent(g.ren);
}

// ===================== Kinect + DEBUG (polling) =====================
enum class Dir{None,Left,Right,Up,Down,Pause,Quit};

static const char* dirName(Dir d){
    switch(d){
        case Dir::Left: return "Left"; case Dir::Right: return "Right";
        case Dir::Up: return "Up"; case Dir::Down: return "Down";
        case Dir::Pause: return "Pause"; case Dir::Quit: return "Quit";
        default: return "None";
    }
}

// Contexto Kinect + parámetros de gestos
struct KinectCtx{
    bool ready=false, enabled=true;
    bool mirrorX=false;

    // EMA de posición de mano (derecha)
    struct EMA2 { bool has=false; float x=0,y=0;
        void reset(){has=false;}
        void push(float nx,float ny,float a=0.35f){
            if(!has){ x=nx; y=ny; has=true; return; }
            x = a*nx + (1-a)*x; y = a*ny + (1-a)*y;
        }
    } hand;

    // Neutral y referencias
    bool  hasNeutral=false; float nx=0, ny=0;
    float lastScx=0, lastScy=0;

    // Sensibilidades / dominancia / armado (ligeramente más estrictos)
    float minPos   = 0.115f;
    float scaleH   = 0.39f;
    float scaleV   = 0.44f;

    float axisRatioH = 1.25f;
    float axisRatioV = 1.22f;

    // Velocidad mínima / EMA
    float vAlpha   = 0.42f;
    float vx = 0.f, vy = 0.f;
    float prev_fx = 0.f, prev_fy = 0.f; bool havePrev=false;
    float vMinFrac = 0.14f;   // vertical

    // Histéresis (flanco + release)
    float trigInH  = 1.00f, trigOutH = 0.60f;
    float trigInV  = 1.00f, trigOutV = 0.60f;
    bool  latchedH = false, latchedV = false;
    int   dirLockH = 0,     dirLockV = 0;
    const int dirLockFrames = 8;

    float prev_ndx = 0.f, prev_ndy = 0.f;

    // Gesto (estado)
    struct Gest {
        bool armed=true; int neutralCnt=0; const int neutralFrames=3;
        Dir  cand=Dir::None; int sustain=0; const int sustainNeed=1; // disparo inmediato
        int  cooldown=0;      const int cooldownFrames=10;
    } g;

    // ===== Gestos globales (pausa / salir) =====
    struct Hold { bool counting=false; uint32_t start=0; uint32_t coolUntil=0; } pauseHold, quitHold;
    // parámetros
    uint32_t pauseMs = 3000;   // manos arriba 3 s
    uint32_t quitMs  = 1500;   // brazos extendidos 1.5 s
    uint32_t holdCooldownMs = 1000;
    float pauseYFrac = 0.90f;      // por encima de 90% de la distancia hombro-centro -> cabeza
    float armsOutFracX = 0.85f;    // distancia mínima horizontal desde hombro-centro (en múltiplos de shoulderW)
    float armsYAlignFrac = 0.35f;  // tolerancia vertical respecto a scy en múltiplos de headSpan

    // Debug
    struct DebugSpam{ bool enabled=true; uint32_t last=0; uint32_t everyMs=120; } dbg;

#ifdef _WIN32
    NUI_TRANSFORM_SMOOTH_PARAMETERS sp{0.5f,0.1f,0.5f,0.05f,0.04f};
#endif
};

static void kinect_shutdown(KinectCtx& k){
#ifdef _WIN32
    NuiShutdown();
#endif
    k.ready=false;
}
static bool kinect_init(KinectCtx& k){
#ifdef _WIN32
    HRESULT hr=NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
    if(FAILED(hr)){ std::fprintf(stderr,"[Kinect] NuiInitialize fail\n"); return false; }
    hr = NuiSkeletonTrackingEnable(NULL, NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT);
    if(FAILED(hr)){ NuiShutdown(); return false; }
    k.ready=true; std::fprintf(stderr,"[Kinect] OK (polling). (K on/off, M mirror, R recalib, N neutral, D debug)\n");
    return true;
#else
    (void)k; return false;
#endif
}

static inline bool hold_eval(bool cond, KinectCtx::Hold& h, uint32_t now, uint32_t needMs, uint32_t cooldownMs){
    if(now < h.coolUntil){ h.counting=false; return false; }
    if(cond){
        if(!h.counting){ h.counting=true; h.start=now; }
        if(now - h.start >= needMs){ h.counting=false; h.coolUntil = now + cooldownMs; return true; }
    }else{
        h.counting=false;
    }
    return false;
}

static Dir kinect_step(KinectCtx& k){
#ifdef _WIN32
    uint32_t nowTicks = SDL_GetTicks();

    if(!k.ready || !k.enabled){
        if(k.dbg.enabled){
            uint32_t now=nowTicks; 
            if(now-k.dbg.last>=k.dbg.everyMs){ k.dbg.last=now; std::printf("(kinect %s)\n",k.enabled?"ON":"OFF"); }
        }
        return Dir::None;
    }
    if(k.g.cooldown>0){ k.g.cooldown--; }
    if(k.dirLockH>0) k.dirLockH--;
    if(k.dirLockV>0) k.dirLockV--;

    NUI_SKELETON_FRAME fr={0};
    HRESULT hr=NuiSkeletonGetNextFrame(0,&fr);
    if(FAILED(hr)){
        if(k.dbg.enabled){
            uint32_t now=nowTicks; 
            if(now-k.dbg.last>=k.dbg.everyMs){ k.dbg.last=now; std::printf("(sin frame)\n"); }
        }
        return Dir::None;
    }
    NuiTransformSmooth(&fr,&k.sp);

    for(int i=0;i<NUI_SKELETON_COUNT;i++){
        const NUI_SKELETON_DATA* s=&fr.SkeletonData[i];
        if(s->eTrackingState!=NUI_SKELETON_TRACKED) continue;
        if(s->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT]   != NUI_SKELETON_POSITION_TRACKED ||
           s->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER]!= NUI_SKELETON_POSITION_TRACKED)
            continue;

        // Posiciones clave
        float rx=s->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x;
        float ry=s->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y;
        float lx=0.f, ly=0.f; bool leftOK=false;
        if(s->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT]==NUI_SKELETON_POSITION_TRACKED){
            lx=s->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x;
            ly=s->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y;
            leftOK=true;
        }
        float scx=s->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].x;
        float scy=s->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].y;

        if(k.mirrorX){ rx=-rx; lx=-lx; scx=-scx; }

        // EMA posición (mano derecha) para gestos de movimiento
        k.hand.push(rx,ry,0.35f);

        // Velocidad (EMA)
        if(!k.havePrev){ k.prev_fx=k.hand.x; k.prev_fy=k.hand.y; k.havePrev=true; }
        float inst_vx = k.hand.x - k.prev_fx;
        float inst_vy = k.hand.y - k.prev_fy;
        k.vx = k.vAlpha*inst_vx + (1.f - k.vAlpha)*k.vx;
        k.vy = k.vAlpha*inst_vy + (1.f - k.vAlpha)*k.vy;
        k.prev_fx = k.hand.x; k.prev_fy = k.hand.y;

        k.lastScx=scx; k.lastScy=scy;

        // Neutral inicial
        if(!k.hasNeutral){ k.nx=scx; k.ny=scy; k.hasNeutral=true; }

        // Medidas de escala por cuerpo
        float shoulderW=0.f, headSpan=0.f;
        if(s->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_LEFT]==NUI_SKELETON_POSITION_TRACKED &&
           s->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_RIGHT]==NUI_SKELETON_POSITION_TRACKED){
            shoulderW = std::fabs(
                s->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x -
                s->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].x);
        }
        if(s->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD]==NUI_SKELETON_POSITION_TRACKED){
            headSpan = std::fabs(s->SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y - scy);
        }

        // ================== GESTOS GLOBALES ==================
        if(leftOK && shoulderW>0 && headSpan>0){
            // 1) Pausa: ambas manos arriba 3 s
            bool bothUp = (ry - scy) > k.pauseYFrac*headSpan && (ly - scy) > k.pauseYFrac*headSpan;
            if(hold_eval(bothUp, k.pauseHold, nowTicks, k.pauseMs, k.holdCooldownMs)){
                if(k.dbg.enabled) std::printf("[GESTO] PAUSE/TOGGLE\n");
                return Dir::Pause;
            }

            // 2) Salir: brazos estirados a los lados (T) 1.5 s
            bool extR = (rx - scx) >= k.armsOutFracX * shoulderW;   // derecha hacia +X
            bool extL = (scx - lx) >= k.armsOutFracX * shoulderW;   // izquierda hacia -X
            bool yAlign = std::fabs(ry - scy) <= k.armsYAlignFrac * headSpan &&
                          std::fabs(ly - scy) <= k.armsYAlignFrac * headSpan;

            if(hold_eval(extR && extL && yAlign, k.quitHold, nowTicks, k.quitMs, k.holdCooldownMs)){
                if(k.dbg.enabled) std::printf("[GESTO] QUIT (T-pose)\n");
                return Dir::Quit;
            }
        }

        // ================== GESTOS DE MOVIMIENTO (lo de antes) ==================
        // Umbrales por eje
        float thH = std::max(k.minPos, k.scaleH*(shoulderW>0? shoulderW:0.40f));
        float thV = std::max(k.minPos, k.scaleV*(headSpan  >0? headSpan  :0.35f));

        // Offset desde neutral (mano derecha)
        float fx=k.hand.x, fy=k.hand.y;
        float dx = fx - k.nx;
        float dy = fy - k.ny;

        // Rearmado suave por quietud
        float quiet = std::fabs(dx) + std::fabs(dy);
        float vmag  = std::fabs(k.vx) + std::fabs(k.vy);
        float stillThresh = 0.60f*std::min(thH,thV);

        if (quiet < stillThresh && vmag < 0.05f*std::min(thH,thV)) {
            if (k.g.neutralCnt < 1000) k.g.neutralCnt++;
            if (k.g.neutralCnt >= k.g.neutralFrames) k.g.armed = true;
        } else if (k.g.cooldown == 0) {
            k.g.armed = true;
            k.g.neutralCnt = 0;
        } else {
            k.g.neutralCnt = 0;
        }

        // Normalizados + dominancia
        float ndx = std::fabs(dx)/thH;
        float ndy = std::fabs(dy)/thV;

        bool domH = (ndx >= 1.0f) && (ndx >= k.axisRatioH * std::max(1e-6f, ndy));
        bool domV = (ndy >= 1.0f) && (ndy >= k.axisRatioV * std::max(1e-6f, ndx));

        // Histéresis por eje
        if (k.latchedH && ndx < k.trigOutH) k.latchedH = false;
        if (k.latchedV && ndy < k.trigOutV) k.latchedV = false;

        bool crossH = (!k.latchedH && (k.prev_ndx < k.trigInH) && (ndx >= k.trigInH));
        bool crossV = (!k.latchedV && (k.prev_ndy < k.trigInV) && (ndy >= k.trigInV));

        // Velocidad mínima
        float vMinH = 0.035f * thH;
        float vMinV = k.vMinFrac * thV;

        Dir cand = Dir::None;
        if (k.g.armed){
            if (domV && crossV && k.dirLockV==0 && std::fabs(k.vy) >= vMinV){
                cand = (dy>0)?Dir::Up:Dir::Down;
            } else if (domH && crossH && k.dirLockH==0 && std::fabs(k.vx) >= vMinH){
                cand = (dx>0)?Dir::Right:Dir::Left;
            }
        }

        if(k.dbg.enabled){
            uint32_t now=nowTicks;
            if(now-k.dbg.last>=k.dbg.everyMs){
                k.dbg.last=now;
                std::printf("dx=%.3f dy=%.3f | ndx=%.2f ndy=%.2f | vx=%.3f vy=%.3f | H[L=%d] V[L=%d] | cand=%s\n",
                    dx,dy, ndx,ndy, k.vx,k.vy, k.latchedH?1:0, k.latchedV?1:0, dirName(cand));
            }
        }

        // Disparo inmediato por flanco
        if (k.g.armed && cand!=Dir::None){
            k.g.armed=false;
            k.g.cooldown=k.g.cooldownFrames;

            if (cand==Dir::Left || cand==Dir::Right){ k.latchedH = true; k.dirLockH = k.dirLockFrames; }
            else { k.latchedV = true; k.dirLockV = k.dirLockFrames; }

            // re-centrar neutral
            k.nx = k.lastScx; 
            k.ny = k.lastScy;

            k.prev_ndx = ndx; k.prev_ndy = ndy;
            return cand;
        }

        k.prev_ndx = ndx; k.prev_ndy = ndy;
        return Dir::None;
    }
#endif
    return Dir::None;
}

// ===================== API embebida =====================
int run_puzzle_game(const char* imagePath, int grid, int size,
                    SDL_Window* existing_window, SDL_Renderer* existing_renderer)
{
    Args a; a.imagePath = imagePath ? imagePath : ""; 
    a.grid = (grid==2||grid==3||grid==4)?grid:3; 
    a.winSize = std::max(300,size);

    Gfx g;
    if(!gfx_init(g, a.winSize, existing_window, existing_renderer)) return 1;

    Puzzle p; 
    if(!gfx_load(g, a.imagePath, a.grid, a.winSize, p)){ 
        gfx_shutdown(g); 
        return 1; 
    }
    pu_shuffle(p,(unsigned)SDL_GetTicks(),250);
    g.startTicks = SDL_GetTicks();
    g.isPaused=false; g.pauseAccumMs=0; g.pauseStart=0;

    KinectCtx k; 
    kinect_init(k);

    bool running=true, announced=false;

    auto request_exit = [&](){ running = false; };
    auto toggle_pause = [&](){
        uint32_t now = SDL_GetTicks();
        if(!g.isPaused){ g.isPaused=true; g.pauseStart=now; }
        else { g.isPaused=false; g.pauseAccumMs += now - g.pauseStart; }
    };

    while(running){
        SDL_Event e; 
        while(SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT) { request_exit(); break; }
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) { request_exit(); break; }

            if(e.type==SDL_KEYDOWN){
                switch(e.key.keysym.sym){
                    case SDLK_ESCAPE: request_exit(); break;

                    case SDLK_LEFT:  if(!announced && !g.isPaused) pu_move(p,0,-1); break;
                    case SDLK_RIGHT: if(!announced && !g.isPaused) pu_move(p,0, 1); break;
                    case SDLK_UP:    if(!announced && !g.isPaused) pu_move(p,-1,0); break;
                    case SDLK_DOWN:  if(!announced && !g.isPaused) pu_move(p, 1,0); break;

                    case SDLK_s:     pu_shuffle(p,(unsigned)SDL_GetTicks(),250); announced=false; g.startTicks=SDL_GetTicks(); g.pauseAccumMs=0; g.isPaused=false; break;

                    case SDLK_k:     k.enabled=!k.enabled; std::fprintf(stderr,"[Kinect] %s\n",k.enabled?"ON":"OFF"); break;
                    case SDLK_m:     k.mirrorX=!k.mirrorX; std::fprintf(stderr,"[Kinect] mirror=%s\n",k.mirrorX?"true":"false"); break;
                    case SDLK_r:     k.hasNeutral=false; k.g.armed=true; k.g.neutralCnt=0; std::fprintf(stderr,"[Kinect] recalibrar neutral\n"); break;
                    case SDLK_n:     k.nx=k.lastScx; k.ny=k.lastScy; std::fprintf(stderr,"[Kinect] neutral re-centrado\n"); break;
                    case SDLK_d:     k.dbg.enabled=!k.dbg.enabled; std::fprintf(stderr,"[DBG] %s\n",k.dbg.enabled?"ON":"OFF"); break;

                    case SDLK_z:
                        k.g.cooldown = 0; k.latchedH = k.latchedV = false; k.dirLockH = k.dirLockV = 0;
                        k.g.armed = true; k.g.neutralCnt = k.g.neutralFrames;
                        k.nx = k.lastScx; k.ny = k.lastScy;
                        std::fprintf(stderr, "[Kinect] Z: reset latch/locks y neutral re-centrado\n");
                        break;

                    case SDLK_p: // atajo manual de pausa
                        toggle_pause();
                        break;

                    case SDLK_SPACE:
                        if(announced){
                            pu_shuffle(p,(unsigned)SDL_GetTicks(),250);
                            announced=false;
                            g.startTicks=SDL_GetTicks(); g.pauseAccumMs=0; g.isPaused=false;
                        }
                        break;
                }
            }
        }
        if(!running) break;

        // Leer Kinect SIEMPRE para permitir pausar/salir por gesto aunque esté en pausa
        Dir d = kinect_step(k);
        if(d==Dir::Pause){ toggle_pause(); }
        else if(d==Dir::Quit){ request_exit(); }
        else if(!announced && !g.isPaused){
            switch(d){
                case Dir::Left:  pu_move(p,0,-1); break;
                case Dir::Right: pu_move(p,0, 1); break;
                case Dir::Up:    pu_move(p,-1,0); break;
                case Dir::Down:  pu_move(p, 1,0); break;
                default: break;
            }
        }

        bool solved=pu_solved(p);
        gfx_draw(g,p,solved);

        if(solved && !announced){
            announced=true;
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION, "¡Felicidades!",
                "Has completado el rompecabezas.\n\n"
                "ESPACIO = barajar y seguir\n"
                "ESC = salir y regresar al menú",
                g.win
            );
        }

        SDL_Delay(16);
    }

    kinect_shutdown(k); 
    gfx_shutdown(g); 
    return 0;
}

// ===================== main (solo cuando NO se compila como librería) =====================
#ifndef BUILD_AS_LIB
int main(int argc, char** argv){
    Args a; if(!parse_args(argc,argv,a)){ usage(argv[0]); return 0; }
    return run_puzzle_game(a.imagePath.c_str(), a.grid, a.winSize, nullptr, nullptr);
}
#endif
