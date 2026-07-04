#include "DX11User.h"
#include "D3D11User.h"
#include "DebugFontUser.h"
#include "DebugFontUser_vs.h"
#include "DebugFontUser_ps.h"

CDebugFont::CDebugFont()
{
   m_pVertexBuffer = NULL;
   m_pVertexShader = NULL;
   m_pLayout = NULL;
   m_pPixelShader = NULL;
   m_pResourceView = NULL;
   m_pConstantBuffers = NULL;
   m_pSamplerState = NULL;
   m_pBlendState = NULL;
   m_pDepthStencilState = NULL;

   Invalidate();
}

void CDebugFont::Invalidate()
{
   SAFE_RELEASE( m_pDepthStencilState );
   SAFE_RELEASE( m_pBlendState );
   SAFE_RELEASE( m_pSamplerState );
   SAFE_RELEASE( m_pConstantBuffers );
   SAFE_RELEASE( m_pResourceView );
   SAFE_RELEASE( m_pPixelShader );
   SAFE_RELEASE( m_pLayout );
   SAFE_RELEASE( m_pVertexShader );
   SAFE_RELEASE( m_pVertexBuffer );

   m_FontWidth = 0;
   m_FontHeight = 0;
   m_FontCnt = 95.0f;

   m_Frame = 0;
   m_FrameDisplay = 0;
}

CDebugFont::~CDebugFont()
{
   Invalidate();
}

HRESULT CDebugFont::Create( ID3D11Device* pD3DDevice, float FontWidth, float FontHeight )
{
   HRESULT hr = E_FAIL;
   float TU = 1.0f / m_FontCnt;   // テクスチャー上での１フォントのサイズ
   // 頂点のデータ
   FONT_VERTEX v[] = {
	   //D3DXVECTOR3( -m_FontWidth,  m_FontHeight, 0 ), 0xFFFFFFFF, D3DXVECTOR2( 0,  0 ),
	   //D3DXVECTOR3(  m_FontWidth,  m_FontHeight, 0 ), 0xFFFFFFFF, D3DXVECTOR2( TU, 0 ),
	   //D3DXVECTOR3( -m_FontWidth, -m_FontHeight, 0 ), 0xFFFFFFFF, D3DXVECTOR2( 0,  1 ),
	   //D3DXVECTOR3(  m_FontWidth, -m_FontHeight, 0 ), 0xFFFFFFFF, D3DXVECTOR2( TU, 1 )
	   D3DXVECTOR3(  m_FontWidth,  m_FontHeight, 0 ), 0xFFFFFFFF, D3DXVECTOR2( TU, 0 ),
	   D3DXVECTOR3( -m_FontWidth,  m_FontHeight, 0 ), 0xFFFFFFFF, D3DXVECTOR2(  0, 0 ),
	   D3DXVECTOR3(  m_FontWidth, -m_FontHeight, 0 ), 0xFFFFFFFF, D3DXVECTOR2( TU, 1 ),
	   D3DXVECTOR3( -m_FontWidth, -m_FontHeight, 0 ), 0xFFFFFFFF, D3DXVECTOR2(  0, 1 )
   };
   // 入力レイアウトを作成する。
   D3D11_INPUT_ELEMENT_DESC layout[] = {
	   { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	   { "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	   { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
   };

   Invalidate();

   // ファイルからシェーダーリソースビューを作成する
   D3DX11_IMAGE_LOAD_INFO info;
   ::ZeroMemory( &info, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
   info.Width = D3DX11_DEFAULT; 
   info.Height = D3DX11_DEFAULT; 
   info.Depth = D3DX11_DEFAULT; 
   info.FirstMipLevel = D3DX11_DEFAULT;          // テクスチャーの最高解像度のミップマップ レベル。
   info.MipLevels = 1;                           // ミップマップ数を１個のみ。
   info.Usage = D3D11_USAGE_DEFAULT; 
   info.BindFlags = D3D11_BIND_SHADER_RESOURCE;
   info.CpuAccessFlags = 0;
   info.MiscFlags = 0;
   info.Format = DXGI_FORMAT_FROM_FILE;
   info.Filter = D3DX11_FILTER_POINT;            // テクスチャー読み込み時に使用するフィルター
   info.MipFilter = D3DX11_FILTER_POINT;         // ミップマップ作成時に使用するフィルター
   info.pSrcInfo = NULL;
   hr = D3DX11CreateShaderResourceViewFromFile( pD3DDevice, _T("res\\Font.dds"), &info, NULL, &m_pResourceView, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // 頂点バッファ作成
   m_FontWidth  = FontWidth;      // ポリゴンの横幅のスケーリング値
   m_FontHeight = FontHeight;     // ポリゴンの縦幅のスケーリング値

   // バッファー リソース
   D3D11_BUFFER_DESC BufferDesc;
   ::ZeroMemory( &BufferDesc, sizeof( BufferDesc ) );
   BufferDesc.ByteWidth             = sizeof( v );               // バッファサイズ
   BufferDesc.Usage                 = D3D11_USAGE_DEFAULT;       // リソース使用法を特定する
   BufferDesc.BindFlags             = D3D11_BIND_VERTEX_BUFFER;  // バッファの種類
   BufferDesc.CPUAccessFlags        = 0;                         // CPU アクセス
   BufferDesc.MiscFlags             = 0;                         // その他のフラグも設定しない
   // サブリソース( 初期値 )
   D3D11_SUBRESOURCE_DATA resource;
   resource.pSysMem = (void*)v;
   resource.SysMemPitch = 0;
   resource.SysMemSlicePitch = 0;
   // バッファを作成する
   hr = pD3DDevice->CreateBuffer( &BufferDesc, &resource, &m_pVertexBuffer );
   if( FAILED( hr ) ) goto EXIT;

   // 定数バッファを作成
   ::ZeroMemory( &BufferDesc, sizeof( BufferDesc ) );
   BufferDesc.ByteWidth             = sizeof( CBUFFER );         // バッファサイズ
   BufferDesc.Usage                 = D3D11_USAGE_DYNAMIC;       // リソース使用法を特定する
   BufferDesc.BindFlags             = D3D11_BIND_CONSTANT_BUFFER;// バッファの種類
   BufferDesc.CPUAccessFlags        = D3D11_CPU_ACCESS_WRITE;    // CPU アクセス
   BufferDesc.MiscFlags             = 0;                         // その他のフラグも設定しない
   // バッファを作成する
   hr = pD3DDevice->CreateBuffer( &BufferDesc, NULL, &m_pConstantBuffers );
   if( FAILED( hr ) ) goto EXIT;

   // 頂点シェーダー作成
   hr = pD3DDevice->CreateVertexShader( g_Font_VS_Main, sizeof( g_Font_VS_Main ), NULL, &m_pVertexShader );
   if( FAILED( hr ) ) goto EXIT;


   hr = pD3DDevice->CreateInputLayout( layout, _countof( layout ), g_Font_VS_Main, sizeof( g_Font_VS_Main ), &m_pLayout );
   if( FAILED( hr ) ) goto EXIT;

   // ピクセルシェーダー作成
   hr = pD3DDevice->CreatePixelShader( g_Font_PS_Main, sizeof( g_Font_PS_Main ), NULL, &m_pPixelShader );
   if( FAILED( hr ) ) goto EXIT;

   // サンプラーステートを作成する
   D3D11_SAMPLER_DESC samplerDesc;
   ::ZeroMemory( &samplerDesc, sizeof( samplerDesc ) );
   samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // サンプリング時に使用するフィルタ。ここでは異方性フィルターを使用する。
   samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある u テクスチャー座標の描画方法
   samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある v テクスチャー座標の描画方法
   samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある w テクスチャー座標の描画方法
   samplerDesc.MipLODBias = 0;                            // 計算されたミップマップ レベルからのバイアス
   samplerDesc.MaxAnisotropy = 0;                         // サンプリングに異方性補間を使用している場合の限界値。有効な値は 1 ～ 16 。
   samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;  // 比較オプション。
   ::CopyMemory( samplerDesc.BorderColor, D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f ), sizeof( D3DXVECTOR4 ) ); // 境界色
   samplerDesc.MinLOD = 0;                                // アクセス可能なミップマップの下限値
   samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // アクセス可能なミップマップの上限値
   hr = pD3DDevice->CreateSamplerState( &samplerDesc, &m_pSamplerState );
   if( FAILED( hr ) ) goto EXIT;

   // ブレンドステートを作成する( ddsファイルを使用するため線形合成 )
   // D3D11_BLEND_DESC
   D3D11_BLEND_DESC BlendDesc;
   ::ZeroMemory( &BlendDesc, sizeof( BlendDesc ) );
   BlendDesc.AlphaToCoverageEnable = FALSE;
   BlendDesc.IndependentBlendEnable = FALSE;
   BlendDesc.RenderTarget[0].BlendEnable = TRUE;
   BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
   BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
   BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
   BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
   BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
   BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
   BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
   // ID3D11Device::CreateBlendState
   hr = pD3DDevice->CreateBlendState( &BlendDesc, &m_pBlendState );
   if( FAILED( hr ) ) goto EXIT;

   // 深度ステンシルステートを作成する
   // D3D11_DEPTH_STENCIL_DESC
   D3D11_DEPTH_STENCIL_DESC ddsDesc;
   ::ZeroMemory( &ddsDesc, sizeof( ddsDesc ) );
   ddsDesc.DepthEnable = FALSE;                                     // 深度テストを使用しない
   ddsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
   ddsDesc.DepthFunc = D3D11_COMPARISON_LESS;
   ddsDesc.StencilEnable = FALSE;
   // ID3D11Device::CreateDepthStencilState
   hr = pD3DDevice->CreateDepthStencilState( &ddsDesc, &m_pDepthStencilState );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:

   return hr;
}

HRESULT CDebugFont::RenderDebugText( ID3D11DeviceContext* pD3DDeviceContext, char* pStr, float X, float Y )
{
   HRESULT hr = E_FAIL;

   // 頂点シェーダーをデバイスに設定する。
   pD3DDeviceContext->VSSetShader( m_pVertexShader, NULL, 0 );

   // ハルシェーダーを無効にする。
   pD3DDeviceContext->HSSetShader( NULL, NULL, 0 );

   // ドメインシェーダーを無効にする。
   pD3DDeviceContext->DSSetShader( NULL, NULL, 0 );

   // ジオメトリシェーダーを無効にする。
   pD3DDeviceContext->GSSetShader( NULL, NULL, 0 );

   // ピクセルシェーダーをデバイスに設定する。
   pD3DDeviceContext->PSSetShader( m_pPixelShader, NULL, 0 );
   pD3DDeviceContext->PSSetSamplers( 0, 1, &m_pSamplerState );
   pD3DDeviceContext->PSSetShaderResources( 0, 1, &m_pResourceView );

   // コンピュートシェーダーを無効にする。
   pD3DDeviceContext->CSSetShader( NULL, NULL, 0 );

   // ブレンドステート( ddsファイルを使用するため線形合成 )
   // ID3D11DeviceContext::OMSetBlendState
   pD3DDeviceContext->OMSetBlendState( m_pBlendState, 0, 0xffffffff );

   // 深度ステンシルステートをセット.最前面に描画するため深度テスト無効
   // ID3D11DeviceContext::OMSetDepthStencilState
   pD3DDeviceContext->OMSetDepthStencilState( m_pDepthStencilState, 0 );

   // 頂点バッファ設定
   UINT stride = sizeof( FONT_VERTEX );
   UINT offset = 0;
   pD3DDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );

   // レイアウト設定
   pD3DDeviceContext->IASetInputLayout( m_pLayout );

   // プリミティブタイプを設定
   pD3DDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

   // 描画
   D3D11_MAPPED_SUBRESOURCE mappedResource;
   CBUFFER* cBuffer;
   D3DXMATRIX matTranslation;
   // 文字数分ループ
   for( int i=0; i<(int)strlen( pStr ); i++ )
   {
      hr = pD3DDeviceContext->Map( m_pConstantBuffers, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
      if( FAILED( hr ) ) goto EXIT;
      cBuffer = (CBUFFER*)(mappedResource.pData);
      // 行列
      D3DXMatrixTranslation(
         &matTranslation,
         -1.0f + m_FontWidth * ( 1.0f + 2.0f * (float)i ) + X,
         1.0f - m_FontHeight - Y,
         0.0f
         );
      D3DXMatrixTranspose( &(cBuffer->matWVP), &matTranslation );
      // テクセル
      cBuffer->Offset.x = (float)( pStr[i] - 32 ) / m_FontCnt;// TU
      cBuffer->Offset.y = 0.0f;                               // TV
      cBuffer->Offset.z = 0.0f;                               // 未使用
      cBuffer->Offset.w = 0.0f;                               // 未使用
      pD3DDeviceContext->Unmap( m_pConstantBuffers, 0 );

      // 頂点シェーダーに定数バッファを設定する。
      pD3DDeviceContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffers );

      // ピクセルシェーダーに定数バッファを設定する。
      pD3DDeviceContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffers );

      // 描画
      pD3DDeviceContext->Draw( 4, 0 );
   }
 
   hr = S_OK;
EXIT:
   return hr;
}

// FPS 出力
HRESULT CDebugFont::RenderFPS( ID3D11DeviceContext* pD3DDeviceContext, float X, float Y )
{
   HRESULT hr = E_FAIL;

   DWORD now = ::timeGetTime();
   char s[20];

   if( now - m_Timer > 1000 )
   {
      m_Timer = now;
      m_FrameDisplay = m_Frame;
      m_Frame = 0;
   }
   else
      m_Frame++;
   sprintf_s( s, "FPS : %d", m_FrameDisplay );

   // フォント描画
   hr = RenderDebugText( pD3DDeviceContext, s, X, Y );

   hr = S_OK;
//EXIT:
   return hr;
}