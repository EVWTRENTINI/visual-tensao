#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include "imgui.h"
#include "rlImGui.h"
#include "rlImGuiColors.h"
#include "seguisb.h"
#include "segoeuisl.h" 

constexpr const char* APP_VERSION = "0.5.1";

// Globais
float escalaTensao = 0.000001f;
float escalaCarga = 0.00001f;
float escalaForca = 0.00001f;
float escalaMomento = 0.00001875;

float myFontSize = 32;

const float fadeVel = 200.0f; // px de alfa que caem por segundo (≈1,3 s de fade)
float overlayAlpha = 255.0f;   // visível ao iniciar
bool  fadeStarted  = false;    // começa parado


// Função para rotacionar um ponto em torno de uma origem, com ângulo em radianos
Vector2 rotate(Vector2 ponto, Vector2 origem, float angulo)
{
    float radianos = -angulo;

    float x = ponto.x - origem.x;
    float y = ponto.y - origem.y;

    Vector2 pontoRotacionado;
    pontoRotacionado.x = x * cosf(radianos) + y * sinf(radianos);
    pontoRotacionado.y = -x * sinf(radianos) + y * cosf(radianos);

    pontoRotacionado.x += origem.x;
    pontoRotacionado.y += origem.y;

    return pontoRotacionado;
}

Color DARKRED = {76, 63, 47, 255};

void drawSeta(Vector2 ini, Vector2 fim, float esp, Color cor, float larguraCabeça)
{
    float comprimentoCabeça = esp * 5;
    float dx = fim.x - ini.x;
    float dy = fim.y - ini.y;
    float T = sqrt(dx * dx + dy * dy);
    if (T < comprimentoCabeça)
    {
        larguraCabeça = larguraCabeça * T / comprimentoCabeça;
        comprimentoCabeça = T;
    }

    // Calcula o ângulo entre o início e o fim da linha
    float angulo = atan2f(fim.y - ini.y, fim.x - ini.x);

    // Novo fim da linha, encurtado para dar espaço à cabeça da seta
    Vector2 novofim = {fim.x - cosf(angulo) * comprimentoCabeça, fim.y - sinf(angulo) * comprimentoCabeça};

    // Desenha a linha da seta
    DrawLineEx(ini, novofim, esp, cor);

    // Pontos iniciais do triângulo da cabeça da seta
    Vector2 ponto1 = {fim.x - comprimentoCabeça, fim.y - larguraCabeça};
    Vector2 ponto2 = {fim.x - comprimentoCabeça, fim.y + larguraCabeça};

    // Rotaciona os pontos para alinhar a cabeça com a direção da linha
    ponto1 = rotate(ponto1, fim, angulo);
    ponto2 = rotate(ponto2, fim, angulo);

    // Desenha o triângulo da cabeça da seta
    DrawTriangle(ponto1, ponto2, fim, cor);
}

void drawMomento(float r, int numeroSeguimentos, bool antihorario, float xm, float ym,float alfa1, float alfa2,float arcoEspessura, float larguraCabeca, Color arcoCor)
{
    if (numeroSeguimentos < 1) numeroSeguimentos = 1;
    float comprimentoCabeca = arcoEspessura * 5;
    float anguloSeta = comprimentoCabeca / r; // em radianos
    float theta1 = alfa1 * (PI / 180.0f);
    float theta2 = alfa2 * (PI / 180.0f);
    float thetavar = ((theta2 + anguloSeta) - theta1) / numeroSeguimentos; // Variação do ângulo por segmento
    if (thetavar > 0) thetavar = 0;
    if (abs(theta2 - theta1) < abs(anguloSeta)) anguloSeta = -(theta2 - theta1);
    if (abs(theta2 - theta1)*r < comprimentoCabeca){
        larguraCabeca = larguraCabeca * abs(theta2 - theta1)*r / comprimentoCabeca;
        comprimentoCabeca = abs(theta2 - theta1)*r;
    }

if (!antihorario){
    thetavar = -((theta1 - theta2 + anguloSeta)- 2 * PI)/ numeroSeguimentos;
    //theta2 = theta2 + 180 * DEG2RAD;
    anguloSeta = - anguloSeta - 2*PI;
    anguloSeta = fmodf(fmodf(anguloSeta, 2 * PI) + 2 * PI, 2 * PI);
    if (abs(theta2 - theta1) > abs(anguloSeta)) anguloSeta = -(theta2 - theta1);
    if (thetavar < 0) thetavar = 0;
    if (abs(-((theta1 - theta2) - 2 * PI))*r < comprimentoCabeca){
        larguraCabeca = larguraCabeca * abs(-((theta1 - theta2) - 2 * PI))*r / comprimentoCabeca;
        comprimentoCabeca = abs(-((theta1 - theta2) - 2 * PI))*r;
    }
}
    // Ponto inicial do primeiro segmento do arc/o
    Vector2 p1_esq = {xm + r * cosf(theta1), ym + r * sinf(theta1)};

    // Itera sobre o número de segmentos para desenhar o arco
    for (int i = 1; i <= numeroSeguimentos; i++)
    {
        // Calcula o ângulo do ponto final para o segmento atual
        float thetai = theta1 + thetavar * i;

        // Calcula o ponto final do segmento atual
        Vector2 p2_esq = {xm + r * cosf(thetai), ym + r * sinf(thetai)};

        // Desenha o segmento do arco como uma linha
        DrawLineEx(p1_esq, p2_esq, arcoEspessura , arcoCor);

        // O ponto final do segmento atual se torna o ponto inicial do próximo segmento
        p1_esq = p2_esq;
    }


    Vector2 p1T = {0, 0};
    Vector2 p2T = {p1T.x - comprimentoCabeca, p1T.y - larguraCabeca};
    Vector2 p3T = {p1T.x - comprimentoCabeca, p1T.y + larguraCabeca};

    //p2T = rotate(p2T, p1T, theta1 + 180 * DEG2RAD - thetavar * 2);
    //p3T = rotate(p3T, p1T, theta1 + 180 * DEG2RAD - thetavar * 2);
    p2T = rotate(p2T, p1T, theta2 + 270 * DEG2RAD + anguloSeta/2);
    p3T = rotate(p3T, p1T, theta2 + 270 * DEG2RAD + anguloSeta/2);
    Vector2 pFinal = {xm + r * cosf(theta2), ym + r * sinf(theta2)};
    p1T = Vector2Add(p1T, pFinal);
    p2T = Vector2Add(p2T, pFinal);
    p3T = Vector2Add(p3T, pFinal);
    DrawTriangle(p1T, p2T, p3T, arcoCor);
}



int main()
{


    int larguraJanela = 1500;
    int alturaJanela = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(larguraJanela, alturaJanela, "Visualisador de Tensão");
    SetTargetFPS(60);

    Font fontTtf = LoadFontFromMemory(
        ".ttf",          // extensão do tipo da fonte (pode ser ".ttf")
        seguisb_ttf,     // ponteiro para o array de bytes
        seguisb_ttf_len, // tamanho da fonte em bytes
        myFontSize,         // tamanho da fonte (em px)
        NULL,            // caracteres (NULL para padrão)
        250              // número de glifos a carregar
    );

    rlImGuiBeginInitImGui(); // em vez de rlImGuiSetup
    ImGuiIO &io = ImGui::GetIO();

    io.Fonts->Clear(); // se quiser remover a padrão
    /*io.FontDefault = io.Fonts->AddFontFromMemoryTTF(
        (void *)seguisb_ttf,
        seguisb_ttf_len,
        18.0f);*/

    io.FontDefault = io.Fonts->AddFontFromMemoryTTF(
        (void *)segoeuisl_ttf,
        segoeuisl_ttf_len,
        18.0f);

    ImGui::StyleColorsLight(); // <-- tema claro
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    rlImGuiEndInitImGui(); // constrói o atlas e faz upload





    /*
    // Estilo - Colorido
    Color corLinha = Color{211, 077, 044, 255};
    Color retanguloCor = Color{044, 178, 211, 255};
    Color corCarga = Color{160, 44, 211, 255};
    Color corReacao = Color{95, 211, 044, 255};
    */ 

    
    // Estilo - Beer
    Color corLinha = Color{210, 15, 125, 255};
    Color retanguloCor = Color{145, 145, 145, 255};
    Color corCarga = Color{210, 15, 125, 255};
    Color corReacao = Color{210, 15, 125, 255};
    
    /*
    // Estilo - Vermelhinho
    Color corLinha = Color{210, 015, 015, 255};
    Color retanguloCor = Color{145, 145, 145, 255};
    Color corCarga = Color{210, 015, 015, 255};
    Color corReacao = Color{210, 015, 015, 255};
    */ 
    float espessuraLinha = 4.0f;
    float comprimentoSeta = 1.2f;

    int n = 11;

    float x = 1;
    float xAlvo = 1; 

    float w = 30000.0f; // Carregamento distribuido
    float wAlvo = 30000.0f; // Carregamento distribuido

    float N = 0.0f; // Força normal
    float NAlvo = 0.0f; // Força normal

    float L = 2.0f;    // Comprimento da viga (em metros)
    float LAlvo = 2.0f;    // Comprimento da viga (em metros)

    float b_viga = 2.0f;      // Base da viga (em metros)
    float b_vigaAlvo = 2.0f;      // Base da viga (em metros)

    float h_viga = 0.5f;      // Altura da viga (em metros)
    float h_vigaAlvo = 0.5f;      // Altura da viga (em metros)

    float folga = 0.0f;       // continua sendo a folga que o desenho usa
    float folgaAlvo = 0.0f;   // valor que o slider controla

    const float rFol = 1e-7f; // mesmo “r” que você usa no zoom

    int displayTensao = 0;

    int larguraMenu = 265;



    // Definindo a câmera 2D
    Camera2D camera = {0};
    camera.zoom = 300.0f;
    camera.offset = Vector2{static_cast<float>(larguraJanela + larguraMenu) / 2, static_cast<float>(alturaJanela) / 2};
    camera.target = Vector2{0, h_viga/2 - .3f};
    float bZoom = camera.zoom;
    


    // Cálculo do momento de inércia da seção retangular

    // Loop principal
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        folga = (folga - folgaAlvo) * powf(rFol, dt) + folgaAlvo;
        x = (x - xAlvo) * powf(rFol, dt) + xAlvo;
        w = (w - wAlvo) * powf(rFol, dt) + wAlvo;
        N = (N - NAlvo) * powf(rFol, dt) + NAlvo;
        L = (L - LAlvo) * powf(rFol, dt) + LAlvo;
        b_viga = (b_viga - b_vigaAlvo) * powf(rFol, dt) + b_vigaAlvo;
        h_viga = (h_viga - h_vigaAlvo) * powf(rFol, dt) + h_vigaAlvo;
        
        if (folga < 0.001f) folga = 0;
        if (w < 10.0f) w = 0;

        if (!fadeStarted) camera.offset = Vector2{static_cast<float>(GetScreenWidth() + larguraMenu) / 2, static_cast<float>(alturaJanela) / 2};;

        if (fadeStarted && overlayAlpha > 0.0f)
        {
            overlayAlpha -= fadeVel * dt; // fade
            if (overlayAlpha < 0.0f)
                overlayAlpha = 0.0f;
        }

        // Translate based on mouse right click
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        // Zoom based on mouse wheel
        float wheel = GetMouseWheelMove();
        float r = .0000001f;
        if (wheel != 0)
        {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;
            float scaleFactor = 1.0f + (0.25f * fabsf(wheel));
            if (wheel < 0)
                scaleFactor = 1.0f / scaleFactor;
            bZoom = Clamp(bZoom * scaleFactor, 10.0f, 10000.0f);
        }
        camera.zoom = (camera.zoom-bZoom)*pow(r,GetFrameTime())+bZoom;

        // Desenho da cena
        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(camera); // Aplicar a câmera 2D

        // Desenha o retângulo fixo

        // numero de setas
        if (IsKeyPressed(KEY_UP)) n = n + 1;
        if (IsKeyPressed(KEY_DOWN)) n = n - 1;

        // velocidade de variação quando a tecla fica segurada (m/s)
        const float passoTecla = 0.5f; // experimente outro valor se quiser

        if (IsKeyDown(KEY_RIGHT))
            xAlvo += passoTecla * dt;
        if (IsKeyDown(KEY_LEFT))
            xAlvo -= passoTecla * dt;

        // garante que o corte permanece dentro do vão
        xAlvo = Clamp(xAlvo, 0.0f, LAlvo);

        // tipo de visualização
        if (IsKeyPressed(KEY_ONE)) displayTensao = 0;
        if (IsKeyPressed(KEY_TWO)) displayTensao = 1;
        if (IsKeyPressed(KEY_THREE)) displayTensao = 2;

        float posXEsq = -x - folga / 2; // Canto  esquerdo superior da viga
        float posYEsq = 0;              // Canto  esquerdo superior da viga
        float posXDir = posXEsq + x + folga;
        float posYDir = posYEsq;
        
        float cg_viga = h_viga / 2; // Centroide da viga, de cima para baixo. Esta /2 pq considerando retangulo, se não foir, mudar. (em metros)




        

        DrawRectangleV(Vector2{posXEsq, posYEsq}, Vector2{x, h_viga}, retanguloCor);
        DrawRectangleV(Vector2{posXDir, posYDir}, Vector2{(L - x), h_viga}, retanguloCor);


        float M = (w * L / 2) * x - (w * x * x / 2);
        float V = (w * L / 2) - (w * x);
        float I = (b_viga * h_viga * h_viga * h_viga / 12);
        float A = h_viga * b_viga;



        // Desenha Carga distribuida
        {
            float distanciaEntreSetasIdeal = 0.2f;
            int numeroSetas = L/distanciaEntreSetasIdeal;
            float distanciaEntreSetas;
            int nSetasEsq;
            int nSetasDir;
            if (numeroSetas == 0){
                distanciaEntreSetas=0;
                nSetasEsq = 1;
                nSetasDir = 1;
            }else{
                distanciaEntreSetas = L/numeroSetas;
                nSetasEsq = (int)(x / distanciaEntreSetas)+1;
                nSetasDir = (int)((L - x) / distanciaEntreSetas)+1;
            }
            
            // Desenha as setas na primeira parte da barra
            for (int i = 0; i < nSetasEsq; i++)
            {
                float posXCarga;
                if (i == 0){
                    posXCarga = 0;
                }else{
                    posXCarga= i * distanciaEntreSetas;
                }
                Vector2 pontoInicio = {posXCarga - x - (folga * 0.5f), posYEsq - w * escalaCarga};
                Vector2 pontoFim = {posXCarga - x - (folga * 0.5f), posYEsq};
                drawSeta(pontoInicio, pontoFim, espessuraLinha / camera.zoom, corCarga, espessuraLinha / camera.zoom * comprimentoSeta);
            }


            if (folga > 0)
            {
            drawSeta(Vector2{x - x - (folga * 0.5f), posYEsq - w * escalaCarga}, Vector2{x - x - (folga * 0.5f), posYEsq}, espessuraLinha / camera.zoom, corCarga, espessuraLinha / camera.zoom * comprimentoSeta);
            }
            // Desenha as setas na segunda parte da barra (da direita para a esquerda)
            for (int i = 0; i < nSetasDir; i++)
            {
                float posXCarga;
                if (nSetasDir - i == 0){
                    posXCarga = 0;
                }else{
                    posXCarga = (nSetasDir - i) * distanciaEntreSetas - distanciaEntreSetas;
                }

                Vector2 pontoInicio = {-posXCarga + (L - x) + (folga * 0.5f), posYEsq - w * escalaCarga};
                Vector2 pontoFim = {-posXCarga + (L - x) + (folga * 0.5f), posYEsq};
                drawSeta(pontoInicio, pontoFim, espessuraLinha / camera.zoom, corCarga, espessuraLinha / camera.zoom * comprimentoSeta);
            }
            if (folga > 0)
            {
            drawSeta({x - x + (folga * 0.5f), posYEsq - w * escalaCarga}, {x - x + (folga * 0.5f), posYEsq}, espessuraLinha / camera.zoom, corCarga, espessuraLinha / camera.zoom * comprimentoSeta);
            }
            if (w>0){
                // Desenha a linha reta que vai de uma extremidade da barra até a outra, acima das setas
                Vector2 pontoInicialLinha = {-x - (folga * 0.5f) - ((espessuraLinha / camera.zoom) * 0.5f), posYEsq - w * escalaCarga}; // Começa acima da altura das setas
                Vector2 pontoFinalLinha = {-(folga * 0.5f) + ((espessuraLinha / camera.zoom) * 0.5f), posYEsq - w * escalaCarga};       // Termina na outra extremidade da barra

                DrawLineEx(pontoInicialLinha, pontoFinalLinha, espessuraLinha / camera.zoom, corCarga); // Linha vermelha

                Vector2 pontoInicialLinhaB = {+(folga * 0.5f) - ((espessuraLinha / camera.zoom) * 0.5f), posYEsq - w * escalaCarga};        // Começa acima da altura das setas
                Vector2 pontoFinalLinhaB = {(L - x) + (folga * 0.5f) + ((espessuraLinha / camera.zoom) * 0.5f), posYEsq - w * escalaCarga}; // Termina na outra extremidade da barra

                DrawLineEx(pontoInicialLinhaB, pontoFinalLinhaB, espessuraLinha / camera.zoom, corCarga); // Linha vermelha
            }
        }

        // Desenha Reação de apoio
        {
        // Reação de apoio da barra biapoiada
        float reacaoApoio = w * escalaForca * L / 2.0f; // Cálculo da reação de apoio

        Vector2 pontoFimSetaEsquerda = {-x - (folga * 0.5f), posYEsq + h_viga};                  // Extremo inferior esquerdo da barra
        Vector2 pontoInicioSetaEsquerda = {-x - (folga * 0.5f), posYEsq + h_viga + reacaoApoio}; // Ponto final, altura proporcional à reação de apoio

        Vector2 pontoFimSetaDireita = {(L - x) + (folga * 0.5f), posYEsq + h_viga};                  // Extremo inferior direito da barra
        Vector2 pontoInicioSetaDireita = {(L - x) + (folga * 0.5f), posYEsq + h_viga + reacaoApoio}; // Ponto final, altura proporcional à reação de apoio

        drawSeta(pontoInicioSetaEsquerda, pontoFimSetaEsquerda, espessuraLinha / camera.zoom, corReacao, espessuraLinha / camera.zoom * comprimentoSeta);
        drawSeta(pontoInicioSetaDireita, pontoFimSetaDireita, espessuraLinha / camera.zoom, corReacao, espessuraLinha / camera.zoom * comprimentoSeta);
        }

        // Desenha Força normal
        {
        Vector2 pontoInicioSetaEsq;
        Vector2 pontoFimSetaEsq;
        Vector2 pontoInicioSetaDir;
        Vector2 pontoFimSetaDir;
        if (N>0){ //TODO() Virar seta quando o normal é negativo
            pontoInicioSetaEsq = {posXEsq, posYEsq + h_viga/2}; 
            pontoFimSetaEsq = {posXEsq - N * escalaForca, posYEsq + h_viga/2};         

            pontoInicioSetaDir = {posXDir + (L-x), posYDir + h_viga/2}; 
            pontoFimSetaDir = {posXDir + N * escalaForca + (L-x), posYDir + h_viga/2};                
        }else{
            pontoInicioSetaEsq = {posXEsq + N * escalaForca, posYEsq + h_viga/2}; 
            pontoFimSetaEsq = {posXEsq, posYEsq + h_viga/2};       

            pontoInicioSetaDir = {posXDir - N * escalaForca + (L-x), posYDir + h_viga/2};     
            pontoFimSetaDir =  {posXDir + (L-x), posYDir + h_viga/2}; 
        }

        drawSeta(pontoInicioSetaEsq, pontoFimSetaEsq, espessuraLinha / camera.zoom, corReacao, espessuraLinha / camera.zoom * comprimentoSeta);
        drawSeta(pontoInicioSetaDir, pontoFimSetaDir, espessuraLinha / camera.zoom, corReacao, espessuraLinha / camera.zoom * comprimentoSeta);
        }


        if (folga > 0)
        {
            // Desenha Tensão Resultante
            if (displayTensao == 0)
            {

                for (int i = 0; i <= n; i++)
                {

                    float posYi = i * h_viga / n;
                    float y = cg_viga - (posYi - posYDir);
                    float tensaoNormal = -M * y / I + N / A;
                    float tensaoCisalhante = -((3 / 2 * (V / (A))) * (1 - ((y * y) / ((h_viga / 2) * (h_viga / 2)))));

                    Vector2 pontoInicio;
                    Vector2 pontoFim;

                    // Esquerda
                    if (tensaoNormal > 0)
                    { // Original
                        pontoInicio = {posXEsq + x, posYEsq + posYi + tensaoCisalhante * escalaTensao / 2};
                        pontoFim = {posXEsq + x + tensaoNormal * escalaTensao, posYEsq + posYi - tensaoCisalhante * escalaTensao / 2};
                    }
                    else
                    { // Trocado
                        pontoInicio = {posXEsq + x - tensaoNormal * escalaTensao, posYEsq + posYi + tensaoCisalhante * escalaTensao / 2};
                        pontoFim = {posXEsq + x, posYEsq + posYi - tensaoCisalhante * escalaTensao / 2};
                    }
                    drawSeta(pontoInicio, pontoFim, espessuraLinha / camera.zoom, corLinha, espessuraLinha / camera.zoom * comprimentoSeta);

                    // Direita
                    if (tensaoNormal > 0)
                    { // Original
                        pontoInicio = {posXDir, posYDir + posYi - tensaoCisalhante * escalaTensao / 2};
                        pontoFim = {posXDir - tensaoNormal * escalaTensao, posYDir + posYi + tensaoCisalhante * escalaTensao / 2};
                    }
                    else
                    { // Original
                        pontoInicio = {posXDir + tensaoNormal * escalaTensao, posYDir + posYi - tensaoCisalhante * escalaTensao / 2};
                        pontoFim = {posXDir, posYDir + posYi + tensaoCisalhante * escalaTensao / 2};
                    }
                    drawSeta(pontoInicio, pontoFim, espessuraLinha / camera.zoom, corLinha, espessuraLinha / camera.zoom * comprimentoSeta);
                }
            }
            else if (displayTensao == 1)
            {

                for (int i = 0; i <= n; i++)
                {

                    float posYi = i * h_viga / n;
                    float y = cg_viga - (posYi - posYDir);
                    float tensaoNormal = -M * y / I + N / A;
                    float tensaoCisalhante = -((3 / 2 * (V / (A))) * (1 - ((y * y) / ((h_viga / 2) * (h_viga / 2)))));

                    Vector2 pontoInicioN;
                    Vector2 pontoFimN;
                    Vector2 pontoInicioC;
                    Vector2 pontoFimC;

                    // Esquerda
                    if (tensaoNormal > 0)
                    { // Original
                        pontoInicioN = {posXEsq + x, posYEsq + posYi};
                        pontoFimN = {posXEsq + x + tensaoNormal * escalaTensao, posYDir + posYi};
                        pontoInicioC = {posXEsq + x, posYi + tensaoCisalhante * escalaTensao / 2};
                        pontoFimC = {posXEsq + x, posYi - tensaoCisalhante * escalaTensao / 2};
                    }
                    else
                    { // Trocado
                        pontoInicioN = {posXEsq + x - tensaoNormal * escalaTensao, posYEsq + posYi};
                        pontoFimN = {posXEsq + x, posYEsq + posYi};
                        pontoInicioC = {posXEsq + x, posYEsq + posYi + tensaoCisalhante * escalaTensao / 2};
                        pontoFimC = {posXEsq + x, posYEsq + posYi - tensaoCisalhante * escalaTensao / 2};
                    }
                    drawSeta(pontoInicioN, pontoFimN, espessuraLinha / camera.zoom, corLinha, espessuraLinha / camera.zoom * comprimentoSeta);
                    drawSeta(pontoInicioC, pontoFimC, espessuraLinha / camera.zoom, corLinha, espessuraLinha / camera.zoom * comprimentoSeta);

                    // Direita
                    if (tensaoNormal > 0)
                    { // Original
                        pontoInicioN = {posXDir, posYDir + posYi};
                        pontoFimN = {posXDir - tensaoNormal * escalaTensao, posYDir + posYi};
                        pontoInicioC = {posXDir, posYDir + posYi - tensaoCisalhante * escalaTensao / 2};
                        pontoFimC = {posXDir, posYDir + posYi + tensaoCisalhante * escalaTensao / 2};
                    }
                    else
                    { // Original
                        pontoInicioN = {posXDir + tensaoNormal * escalaTensao, posYDir + posYi};
                        pontoFimN = {posXDir, posYDir + posYi};
                        pontoInicioC = {posXDir, posYDir + posYi - tensaoCisalhante * escalaTensao / 2};
                        pontoFimC = {posXDir, posYDir + posYi + tensaoCisalhante * escalaTensao / 2};
                    }
                    drawSeta(pontoInicioN, pontoFimN, espessuraLinha / camera.zoom, corLinha, espessuraLinha / camera.zoom * comprimentoSeta);
                    drawSeta(pontoInicioC, pontoFimC, espessuraLinha / camera.zoom, corLinha, espessuraLinha / camera.zoom * comprimentoSeta);
                }
            }
            else if (displayTensao == 2)
            {

                Vector2 pontoInicioNormalEsq;
                Vector2 pontoFimNormalEsq;
                Vector2 pontoInicioNormalDir;
                Vector2 pontoFimNormalDir;

                if (N > 0) // tração
                {
                    pontoInicioNormalEsq = {posXEsq + x, posYEsq + h_viga / 2};
                    pontoFimNormalEsq = {posXEsq + x + N * escalaForca, posYEsq + h_viga / 2};

                    pontoInicioNormalDir = {posXDir, posYDir + h_viga / 2};
                    pontoFimNormalDir = {posXDir - N * escalaForca, posYDir + h_viga / 2};
                }
                else // compressão  (N < 0)
                {
                    pontoInicioNormalEsq = {posXEsq + x - N * escalaForca, posYEsq + h_viga / 2};
                    pontoFimNormalEsq = {posXEsq + x, posYEsq + h_viga / 2};

                    pontoInicioNormalDir = {posXDir + N * escalaForca, posYDir + h_viga / 2};
                    pontoFimNormalDir = {posXDir, posYDir + h_viga / 2};
                }

                // desenha as setas
                drawSeta(pontoInicioNormalEsq, pontoFimNormalEsq,
                         espessuraLinha / camera.zoom, corLinha,
                         espessuraLinha / camera.zoom * comprimentoSeta);

                drawSeta(pontoInicioNormalDir, pontoFimNormalDir,
                         espessuraLinha / camera.zoom, corLinha,
                         espessuraLinha / camera.zoom * comprimentoSeta);

                Vector2 pontoInicioCortanteEsq = Vector2{posXEsq + x + espessuraLinha / camera.zoom * 2, posYEsq + h_viga / 2 - V * escalaForca / 2};
                Vector2 pontoFimCortanteEsq = Vector2{posXEsq + x + espessuraLinha / camera.zoom * 2, posYEsq + h_viga / 2 + V * escalaForca /2};
                drawSeta(pontoInicioCortanteEsq, pontoFimCortanteEsq, espessuraLinha / camera.zoom, corLinha, espessuraLinha / camera.zoom * comprimentoSeta);

                Vector2 pontoInicioCortanteDir = Vector2{posXDir - espessuraLinha / camera.zoom * 2, posYDir + h_viga / 2 + V * escalaForca / 2};
                Vector2 pontoFimCortanteDir = Vector2{posXDir - espessuraLinha / camera.zoom * 2, posYDir + h_viga / 2 - V * escalaForca /2};
                drawSeta(pontoInicioCortanteDir, pontoFimCortanteDir, espessuraLinha / camera.zoom, corLinha, espessuraLinha / camera.zoom * comprimentoSeta);


                float aberturaArco = 80; // Abertura do arco do comento em graus
                float r = M * escalaMomento; // raio em metros
                float r_px = M * escalaMomento * camera.zoom; // raio em px
                float arcoEspessura = espessuraLinha / camera.zoom;
                float arcoComprimentoSeta = espessuraLinha / camera.zoom * comprimentoSeta;
                float r_px_min = 40;
                 if (r_px < r_px_min) {
                    arcoEspessura = arcoEspessura * r_px / r_px_min;
                    arcoComprimentoSeta = arcoComprimentoSeta * r_px / r_px_min;
                }
                float comprimentoCabeca = arcoEspessura * 5;
                float anguloSeta = comprimentoCabeca / r; // em radianos
                
                float aberturaArcoRealPx = (aberturaArco * DEG2RAD - anguloSeta) * r_px;

                int nSeguimentos = static_cast<int>((aberturaArcoRealPx)/7);
                if (nSeguimentos < 3) nSeguimentos = 3;






                const float meiaAbertura = 0.5f * aberturaArco * DEG2RAD;
                const float dx =  cosf(meiaAbertura) * r;   // r·cosθ
                float xm_esquerda = (posXEsq + x) - dx + 30 / camera.zoom;
                float xm_direita  =  posXDir + dx - 30 / camera.zoom; 
                
                float ym_esquerda = posYEsq + h_viga / 2;
                float ym_direita = posYDir + h_viga / 2; 

                drawMomento(r, nSeguimentos, true, xm_esquerda, ym_esquerda, aberturaArco/2, -aberturaArco/2, arcoEspessura, arcoComprimentoSeta, corLinha);
                drawMomento(r, nSeguimentos, false, xm_direita, ym_direita, (90-aberturaArco/2)*2 + aberturaArco/2, -(90-aberturaArco/2)*2 - aberturaArco/2, arcoEspessura, arcoComprimentoSeta, corLinha);
            }
        }

        EndMode2D();

        if (overlayAlpha > 0.0f)
        {
            unsigned char a = (unsigned char)overlayAlpha;

            const char *linha1 = "Bem vindo. Esta é uma ferramenta para visualização das tensões em uma simples viga biapoiada.";
            const char *linha2 = "Para iniciar altere o valor da folga no menu à esquerda.";

            float spacing = 0.0f;

            // Calcula a largura de cada linha na fonte que você já carregou
            Vector2 sz1 = MeasureTextEx(fontTtf, linha1, myFontSize, spacing);
            Vector2 sz2 = MeasureTextEx(fontTtf, linha2, myFontSize, spacing);

            // Posição-y onde você quer começar a desenhar
            float y1 = roundf(75.0f);                  // linha 1
            float y2 = roundf(y1 + myFontSize + 6.0f); // linha 2 (6 px de espaço extra)

            // Calcula o x centralizado para cada linha (usa a largura de tela atual)
            float x1 = roundf((GetScreenWidth() - larguraMenu - sz1.x) * 0.5f + larguraMenu);
            float x2 = roundf((GetScreenWidth() - larguraMenu - sz2.x) * 0.5f + larguraMenu);

            Color textoCor = {30, 30, 30, a};                      // mesmo cinza, com alpha variável
            Color backCor = {0, 0, 0, (unsigned char)(a * 0.2f)}; // ~120 quando a=255

            // Desenha já centralizado
            DrawTextEx(fontTtf, linha1, {x1, y1}, myFontSize, spacing, textoCor);
            DrawTextEx(fontTtf, linha2, {x2, y2}, myFontSize, spacing, textoCor);

            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), backCor); // retangulo transparente
        }

        rlImGuiBegin();
        /*DoMainMenu();
        if (ImGuiDemoOpen)
            ImGui::ShowDemoWindow(&ImGuiDemoOpen);*/
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(larguraMenu, (float)GetScreenHeight()));
        if (ImGui::Begin("##Controles", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar ))//| ImGuiWindowFlags_NoResize
        {
            larguraMenu = static_cast<int>(ImGui::GetWindowWidth());
            if (ImGui::BeginTabBar("Tabs"))
            {
                if (ImGui::BeginTabItem("Controles"))
                {
                    if (ImGui::SliderFloat("Folga", &folgaAlvo, 0.f, 5.0f, "f = %.3f"))
                    {
                        if (!fadeStarted)       // primeira vez
                            fadeStarted = true; // começa a sumir
                    }
                    ImGui::SliderFloat("Seção", &xAlvo, 0.00001f, L, "x = %.2f");
                    ImGui::SliderFloat("Base", &b_vigaAlvo, 0.1f, 2.f, "b = %.2f");
                    ImGui::SliderFloat("Altura", &h_vigaAlvo, 0.1f, 2.f, "h = %.2f");
                    ImGui::SliderFloat("Carga", &wAlvo, 0.0f, 100000.0f, "w = %.0f");
                    ImGui::SliderFloat("Normal", &NAlvo, -100000.0f, 100000.0f, "N = %.0f");
                    float fracao_x = (LAlvo > 0.0f) ? xAlvo / LAlvo : 0.0f;

                    // slider passa a controlar SOMENTE o alvo
                    if (ImGui::SliderFloat("Vão", &LAlvo, 0.001f, 10.0f, "L = %.2f"))
                    {
                        // mantém o corte na mesma proporção depois que o usuário mudou L
                        xAlvo = fracao_x * LAlvo;          // <-- agora mexe em xAlvo
                        xAlvo = Clamp(xAlvo, 0.0f, LAlvo); // evita passar do novo comprimento
                    }
                    ImGui::SeparatorText("Tipos de Visualização");
                    ImGui::RadioButton("Modo Tensão Resultante", &displayTensao, 0);
                    ImGui::RadioButton("Modo Tensão Normal e Cisalhante", &displayTensao, 1);
                    ImGui::RadioButton("Modo Esforços Internos", &displayTensao, 2);

                    ImGui::EndTabItem();
                }
            }
            if (ImGui::BeginTabItem("Aparência")) {
                ImGui::SliderFloat("Esp Linha", &espessuraLinha, 0.0f, 10.f, "Esp = %.3f");
                ImGui::SliderFloat("Lar Seta", &comprimentoSeta, 0.0f, 10.f, "Larg = %.3f");
                static ImVec4 imCorLinha = rlImGuiColors::Convert(corLinha);
                static ImVec4 imRetanguloCor = rlImGuiColors::Convert(retanguloCor);
                static ImVec4 imCorCarga = rlImGuiColors::Convert(corCarga);
                static ImVec4 imCorReacao = rlImGuiColors::Convert(corReacao);
                ImGui::ColorPicker4("Viga", (float *)&imRetanguloCor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
                retanguloCor = rlImGuiColors::Convert(imRetanguloCor);
                ImGui::ColorPicker4("Carga", (float *)&imCorCarga, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
                corCarga = rlImGuiColors::Convert(imCorCarga);
                ImGui::ColorPicker4("Reação", (float *)&imCorReacao, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
                corReacao = rlImGuiColors::Convert(imCorReacao);
                ImGui::ColorPicker4("Tensão", (float *)&imCorLinha, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
                corLinha = rlImGuiColors::Convert(imCorLinha);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Sobre")) {
                ImGui::Text("UNIVERSIDADE FEDERAL DE UBERLÂNDIA");
                ImGui::Text(" ");
                ImGui::Text("Visualizador de Tensão v%s", APP_VERSION);
                ImGui::Text("Desenvolvido por:");
                ImGui::Text(" - João Vitor Ferreira Figueiredo");
                ImGui::Text(" - Victor Martins Oliveira");
                ImGui::Text(" ");
                ImGui::Text("Orientado por:");
                ImGui::Text(" - Eduardo Vicente Wolf Trentini");
                ImGui::Text(" - Email: etrentini@ufu.br");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();

        rlImGuiEnd(); 

        EndDrawing();
    }

    rlImGuiShutdown(); 

    CloseWindow();

    return 0;
}