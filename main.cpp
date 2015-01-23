#include <stdio.h>
#include <string.h>

#include "ShaderEditor.h"
#include "Renderer.h"
#include "FFT.h"
#include "Timer.h"

void main()
{
  RENDERER_SETTINGS settings;
  settings.nWidth = 1280;
  settings.nHeight = 720;
  settings.windowMode = RENDERER_WINDOWMODE_WINDOWED;

  if (!Renderer::Open( &settings ))
    return;

  if (!FFT::Open())
    return;

  printf("Loading textures...\n");

  printf("* textures/checker.png...\n");
  Renderer::Texture * texChecker = Renderer::CreateRGBA8TextureFromFile("textures/checker.png");

  printf("* textures/noise.png...\n");
  Renderer::Texture * texNoise = Renderer::CreateRGBA8TextureFromFile("textures/noise.png");

  Renderer::Texture * tex[8];
  for(int i=0; i<8; i++)
  {
    char sz[] = "textures/tex1.jpg";
    sz[12] = '1' + i;
    printf("* %s...\n",sz);
    tex[i] = Renderer::CreateRGBA8TextureFromFile(sz);
  }

  Renderer::Texture * texFFT = Renderer::Create1DR32Texture( FFT_SIZE );

  char szError[4096];
  if (!Renderer::ReloadShader( Renderer::defaultShader, strlen(Renderer::defaultShader), szError, 4096 ))
  {
    return;
  }

#ifdef SCI_LEXER
  Scintilla_LinkLexers();
#endif
  Scintilla::Surface * surface = Scintilla::Surface::Allocate( SC_TECHNOLOGY_DEFAULT );

  ShaderEditor mShaderEditor( surface );
  mShaderEditor.Initialise();
  mShaderEditor.SetText( Renderer::defaultShader );

  Timer::Start();
  float fNextTick = 0.1;
  while (!Renderer::WantsToQuit())
  {
    Renderer::StartFrame();

    float time = Timer::GetTime() / 1000.0; // seconds
    Renderer::SetShaderConstant( "fGlobalTime", time );

    Renderer::SetShaderConstant( "v2Resolution", settings.nWidth, settings.nHeight );

    Renderer::SetShaderTexture( "texChecker", texChecker );
    Renderer::SetShaderTexture( "texNoise", texNoise );

    static float fftData[FFT_SIZE];
    if (FFT::GetFFT(fftData))
      Renderer::UpdateR32Texture( texFFT, fftData );

    Renderer::SetShaderTexture( "texFFT", texFFT );

    for(int i=0; i<8; i++)
    {
      char sz[] = "texTex1";
      sz[6] = '1' + i;
      Renderer::SetShaderTexture( sz, tex[i] );
    }

    Renderer::RenderFullscreenQuad();

    if (time > fNextTick)
    {
      mShaderEditor.Tick();
      fNextTick = time + 0.1;
    }

    mShaderEditor.Paint( );

    Renderer::EndFrame();
  }

  delete surface;

  FFT::Close();

  Renderer::Close();
}