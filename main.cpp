#include "DX11User.h"
#include "D3D11User.h"
#include "DebugFontUser.h"
// コンパイル済みエフェクト
#include "SimpleHLSL07_fx.h"

#include <d3dx11effect.h>                 // エフェクトのインクルードファイル
#pragma comment( lib, "Effects11d.lib" )   // エフェクトのライブラリ
#pragma comment( lib, "d3dcompiler.lib" ) // Effects11.lib を使用するのに必要

// アプリケーション名
TCHAR* AppName = (TCHAR*)L"DX11_Tutrial07";

// Direct3D関連の自作クラス
D3D11USER* g_pD3D11User = NULL;

// デバッグ専用のテキスト描画する自作クラス
CDebugFont* g_pDebugFontUser = NULL;

// 頂点バッファ
ID3D11Buffer* g_pVertexBuffer = NULL;
// インデックスバッファ
ID3D11Buffer* g_pIndexBuffer = NULL;
// 入力レイアウト
ID3D11InputLayout* g_pLayout = NULL;
// 深度ステンシルステート
ID3D11DepthStencilState*  g_pDepthStencilState = NULL;

// シェーダーリソースビュー
ID3D11ShaderResourceView* g_pSRView = NULL;

// レンダリング エフェクト
// ID3DX11Effect
ID3DX11Effect* g_pEffect = NULL;
// パスの集合
// ID3DX11EffectTechnique
ID3DX11EffectTechnique* g_pEffectTechnique = NULL;
// パス
// ID3DX11EffectPass
ID3DX11EffectPass* g_pEffectPass = NULL;
// 定数バッファ
// ID3DX11EffectMatrixVariable
ID3DX11EffectMatrixVariable* g_pMatWVP = NULL;
// シェーダーリソース
// ID3DX11EffectShaderResourceVariable
ID3DX11EffectShaderResourceVariable* g_pTex = NULL;
// サンプラーステート
// ID3DX11EffectSamplerVariable
ID3DX11EffectSamplerVariable* g_pSampler = NULL;

// 頂点定義
struct VERTEX
{
   // 頂点座標
   D3DXVECTOR3   pos;
   // 頂点カラー
   D3DXCOLOR   color;
   // テクセル
   D3DXVECTOR2 texel;
};

// 節電モードの制御に使用する変数
bool Activate = true;    // ウィンドウがアクティブか
bool StandBy = false;    // スタンバイ状態か

bool ScreenShot = false; // スクリーンショットを作成するかフラグ

// リソースの初期化
HRESULT Init()
{
   HRESULT hr = E_FAIL;
   ID3D10Blob* pBlob = NULL;
   D3DX11_IMAGE_LOAD_INFO info;
   // 頂点の実データを設定
   VERTEX v[] = {
                  D3DXVECTOR3(  50.0f,  50.0f, 0.0f ), 0xFFFFFFFF, D3DXVECTOR2( 1.0f, 0.0f ),
                  D3DXVECTOR3( -50.0f,  50.0f, 0.0f ), 0xFFFFFFFF, D3DXVECTOR2( 0.0f, 0.0f ),
                  D3DXVECTOR3(  50.0f, -50.0f, 0.0f ), 0xFFFFFFFF, D3DXVECTOR2( 1.0f, 1.0f ),
                  D3DXVECTOR3( -50.0f, -50.0f, 0.0f ), 0xFFFFFFFF, D3DXVECTOR2( 0.0f, 1.0f ),
               };

   ID3D11SamplerState* pSamplerState = NULL;
   UINT Index[] = { 0, 1, 2, 3};
   D3D11_INPUT_ELEMENT_DESC layout[] = {
	   { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	   { "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	   { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
   };
   ID3DX11EffectVariable* pVariable;

   // 頂点バッファを作成する
   hr = g_pD3D11User->CreateVertexBuffer( &g_pVertexBuffer, v, sizeof( v ), 0 );
   if( FAILED( hr ) ) goto EXIT;

   // インデックスバッファを作成する。
   hr = g_pD3D11User->CreateIndexBuffer( &g_pIndexBuffer, Index, sizeof( Index ), 0 );
   if( FAILED( hr ) ) goto EXIT;

   // ファイルからシェーダーリソースビューを作成する
   ::ZeroMemory( &info, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
   info.Width = D3DX11_DEFAULT; 
   info.Height = D3DX11_DEFAULT; 
   info.Depth = D3DX11_DEFAULT; 
   info.FirstMipLevel = D3DX11_DEFAULT;          // テクスチャーの最高解像度のミップマップ レベル。実際の使用方法不明
   info.MipLevels = 0;                           // ミップマップ数。0 または D3DX11_DEFAULT を使用するとすべてのミップマップ チェーンを作成する
   info.Usage = D3D11_USAGE_DEFAULT; 
   info.BindFlags = D3D11_BIND_SHADER_RESOURCE;
   info.CpuAccessFlags = 0;
   info.MiscFlags = 0;
   info.Format = DXGI_FORMAT_FROM_FILE;
   info.Filter = D3DX11_FILTER_POINT;            // テクスチャー読み込み時に使用するフィルター
   info.MipFilter = D3DX11_FILTER_POINT;         // ミップマップ作成時に使用するフィルター
   info.pSrcInfo = NULL;
   hr = D3DX11CreateShaderResourceViewFromFile( g_pD3D11User->m_D3DDevice, _T("res\\01.jpg"), &info, NULL, &g_pSRView, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // バイナリ エフェクトまたはバイナリ ファイルからエフェクトを作成
   // D3DX11CreateEffectFromMemory
   hr = D3DX11CreateEffectFromMemory( (void*)g_effect01, sizeof( g_effect01 ), 0, g_pD3D11User->m_D3DDevice, &g_pEffect );
   if( FAILED( hr ) ) goto EXIT;

   // 深度ステンシルステートを作成する
   D3D11_DEPTH_STENCIL_DESC ddsDesc;
   ::ZeroMemory( &ddsDesc, sizeof( ddsDesc ) );
   ddsDesc.DepthEnable = TRUE;                                     // 深度テストを使用する
   ddsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
   ddsDesc.DepthFunc = D3D11_COMPARISON_LESS;
   ddsDesc.StencilEnable = FALSE;
   hr = g_pD3D11User->m_D3DDevice->CreateDepthStencilState( &ddsDesc, &g_pDepthStencilState );
   if( FAILED( hr ) ) goto EXIT;

   // 名前によってテクニックを取得
   // ID3DX11Effect::GetTechniqueByName
   g_pEffectTechnique = g_pEffect->GetTechniqueByName( "TShader" );
   if( g_pEffectTechnique->IsValid() == FALSE ){ hr = E_FAIL; goto EXIT; }

   // 取得したテクニックから名前によってパスを取得
   // ID3DX11EffectTechnique::GetPassByName
   g_pEffectPass = g_pEffectTechnique->GetPassByName( "P0" );
   if( g_pEffectPass->IsValid() == FALSE ){ hr = E_FAIL; goto EXIT; }
   // D3DX11_PASS_DESC
   D3DX11_PASS_DESC passDesc;
   g_pEffectPass->GetDesc( &passDesc );

   // 入力レイアウトを作成する
   hr = g_pD3D11User->m_D3DDevice->CreateInputLayout( layout, _countof( layout ), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &g_pLayout );
   if( FAILED( hr ) ) goto EXIT;

   // ID3DX11EffectVariable
   pVariable = NULL;

   // 名前によって定数バッファを取得
   // ID3DX11Effect::GetVariableByName
   pVariable = g_pEffect->GetVariableByName( "g_matWVP" );
   if( pVariable->IsValid() == FALSE ){ hr = E_FAIL; goto EXIT; }
   // ID3DX11EffectVariable::AsMatrix
   g_pMatWVP = pVariable->AsMatrix();

   // 名前によってシェーダーリソースを取得
   pVariable = g_pEffect->GetVariableByName( "g_Tex" );
   if( pVariable->IsValid() == FALSE ){ hr = E_FAIL; goto EXIT; }
   // ID3DX11EffectVariable::AsShaderResource
   g_pTex = pVariable->AsShaderResource();

   // 名前によってサンプラーステートを取得
   pVariable = g_pEffect->GetVariableByName( "g_Sampler" );
   if( pVariable->IsValid() == FALSE ){ hr = E_FAIL; goto EXIT; }
   // ID3DX11EffectVariable::AsSampler
   g_pSampler = pVariable->AsSampler();
   // サンプラーステートの作成
   D3D11_SAMPLER_DESC samplerDesc;
   samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;         // サンプリング時に使用するフィルタ。ここでは異方性フィルターを使用する。
   samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある u テクスチャー座標の描画方法
   samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある v テクスチャー座標の描画方法
   samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある w テクスチャー座標の描画方法
   samplerDesc.MipLODBias = 0;                            // 計算されたミップマップ レベルからのバイアス
   samplerDesc.MaxAnisotropy = 16;                        // サンプリングに異方性補間を使用している場合の限界値。有効な値は 1 ～ 16 。
   samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;  // 比較オプション。
   ::CopyMemory( samplerDesc.BorderColor, D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f ), sizeof( D3DXVECTOR4 ) ); // 境界色
   samplerDesc.MinLOD = 0;                                // アクセス可能なミップマップの下限値
   samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // アクセス可能なミップマップの上限値
   hr = g_pD3D11User->m_D3DDevice->CreateSamplerState( &samplerDesc, &pSamplerState );
   if( FAILED( hr ) ) goto EXIT;
   // サンプラーステートは同一のエフェクト内で変更する必要することはあまりないと思うのでここで設定する。
   g_pSampler->SetSampler( 0, pSamplerState );

   hr = S_OK;

EXIT:
   SAFE_RELEASE( pSamplerState );
   SAFE_RELEASE( pBlob );
   return hr;
}

// メモリ開放
void Invalidate()
{
   SAFE_RELEASE( g_pEffect );
   SAFE_RELEASE( g_pDepthStencilState );
   SAFE_RELEASE( g_pLayout );
   SAFE_RELEASE( g_pIndexBuffer );
   SAFE_RELEASE( g_pVertexBuffer );
   SAFE_RELEASE( g_pSRView );

   SAFE_DELETE( g_pDebugFontUser );
   SAFE_DELETE( g_pD3D11User );
}

// 描画処理
HRESULT Render()
{
   HRESULT hr = E_FAIL;
   D3DXMATRIX matWorld, matView, matProj, matWVP;

   float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

   // バックバッファをクリア
   g_pD3D11User->m_D3DDeviceContext->ClearRenderTargetView( g_pD3D11User->m_RenderTargetView, ClearColor ); 

   // 深度バッファをクリア
   if( g_pD3D11User->m_DepthStencilView )
      g_pD3D11User->m_D3DDeviceContext->ClearDepthStencilView( g_pD3D11User->m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

   // 頂点バッファ設定
   UINT stride = sizeof( VERTEX );
   UINT offset = 0;
   g_pD3D11User->m_D3DDeviceContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

   // インデックスバッファ設定
   g_pD3D11User->m_D3DDeviceContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

   // レイアウト設定
   g_pD3D11User->m_D3DDeviceContext->IASetInputLayout( g_pLayout );

   // プリミティブ タイプおよびデータの順序に関する情報を設定
   g_pD3D11User->m_D3DDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

   // 深度テストを有効にする
   g_pD3D11User->m_D3DDeviceContext->OMSetDepthStencilState( g_pDepthStencilState, 0 );

   // アルファブレンディングを無効
   D3D11_RENDER_TARGET_BLEND_DESC BlendDesc;
   BlendDesc = g_pD3D11User->GetDefaultBlendDesc();
   g_pD3D11User->SetBlendState( &BlendDesc, 1, FALSE );

   // 射影行列
   D3DXMatrixPerspectiveFovLH( &matProj, 3.1415926f / 2.0f, 4.0f / 3.0f, 1.0f, 250.0f );
   // ビュー行列
   D3DXVECTOR3 eye( 0.0f, 0.0f, -60.0f );
   D3DXVECTOR3 at( 0.0f, 0.0f, 0.0f );
   D3DXVECTOR3 up( 0.0f, 1.0f, 0.0f );
   D3DXMatrixLookAtLH( &matView,
						&eye,
                        &at,
                        &up );
   // ワールド行列
   D3DXMatrixTranslation( &matWorld, 0.0f, 0.0f, 20.0f );
   matWVP = matWorld * matView * matProj;

   // 行列をエフェクトに設定
   // ID3DX11EffectMatrixVariable::SetMatrixTranspose
   g_pMatWVP->SetMatrixTranspose( (float*)( matWVP.m ) );

   // シェーダーリソースをエフェクトに設定
   // ID3DX11EffectShaderResourceVariable::SetResource
   g_pTex->SetResource( g_pSRView );

   // パスに含まれているステートをデバイスに設定
   // ID3DX11EffectPass::Apply
   g_pEffectPass->Apply( 0, g_pD3D11User->m_D3DDeviceContext );
   
   // 描画
   g_pD3D11User->m_D3DDeviceContext->DrawIndexed( 4, 0, 0 );

   // デバッグ専用フォント描画
   if( g_pDebugFontUser )
   {
      g_pDebugFontUser->RenderFPS( g_pD3D11User->m_D3DDeviceContext, 0, 0 );
   }

   // レンダリングされたイメージをユーザーに表示。
   hr = g_pD3D11User->m_SwapChain->Present( 0, 0 );

   if( ScreenShot )
   {
      // スクリーンショット作成
      g_pD3D11User->CreateScreenShot();
      ScreenShot = false;
   }

   return hr;
}

// 節電処理および描画処理
HRESULT PowerSavingAndRender()
{
   HRESULT hr = E_FAIL;

   switch( StandBy )
   {
   // スタンバイモード
   case  true:
      // テストのみ行い、描画処理は行わない。
      hr = g_pD3D11User->m_SwapChain->Present( 0, DXGI_PRESENT_TEST );
      switch( hr )
      {
      // いまだスタンバイ中。。。
      case DXGI_STATUS_OCCLUDED:
         // 電源管理によるスリープ状態の場合ここにくる。
         // フルスクリーンモード時にスクリーンセーバーが起動時した場合は、表示モードが強制的にウィンドウモードに変更されるためここにこない。
         goto EXIT;
         break;
      case S_OK:
         // フルスクリーンモード時にスクリーンセーバーが起動時した場合は表示モードが強制的にウィンドウモードに変更される。
         // ウィンドウモードの場合スタンバイから復帰してしまうため、ウィンドウがアクティブになったときに復帰するようにする。
         if( Activate == true )
         {
            // たまにウィンドウが表示されないときがあるので表示するようにする
            ::ShowWindow( g_pD3D11User->m_hWnd, SW_SHOW );
            StandBy = false;
         }
         break;
      default:
         goto EXIT;
         break;
      }
      break;
   // スタンバイモードでない
   case false:
      // 描画処理
      hr = Render();
      if( FAILED( hr ) ) goto EXIT;

      switch( hr )
      {
      case DXGI_STATUS_OCCLUDED:
         // スタンバイモードへ移行
         // フルスクリーンモード時のスクリーンセーバー起動時、
         // スリープ状態に移行した時に発生する。
         StandBy = true;
         goto EXIT;
         break;
      case S_OK:
         break;
      default:
         goto EXIT;
         break;
      }
      break;
   }

   hr = S_OK;

EXIT:

   return hr;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM  wParam, LPARAM  lParam )
{
   switch( msg )
   {
   case WM_KEYUP:
      // アプリ終了
      if( wParam == VK_ESCAPE )
         ::DestroyWindow( hWnd );

      // F2キーを押すと、ウィンドウモードを切り替える。
      // 自動的にウィンドウモードを切り替える機能もあるが、ウィンドウスタイルを自由に変更するために自分で実装することにした。
      if( wParam == VK_F2 )
      {
         g_pD3D11User->ChangeWindowMode();
      }
      // スクリーンショットを作成する
      if( wParam == VK_SNAPSHOT )
         ScreenShot = true;
      break;

   case WM_ACTIVATE:
      Activate = true;
      break;

// フルスクリーンからウィンドウモードに切り替えるとき WM_SIZE イベントが発生せず、結果的に IDXGISwapChain::ResizeBuffers がコールされない。
// 環境にもよるようだが、画面上に何も表示されない現象が発生する可能性があるので
// D3D11USER::ChangeWindowModeOptimization() は D3D11USER::ChangeWindowMode() 内でコールするように修正し、ここでの処理は無効にする
//   case WM_SIZE:
//      g_pD3D11User->ChangeWindowModeOptimization();
//      break;

   case WM_DESTROY:
      Invalidate();
      ::PostQuitMessage(0);
      break;

   default:
      return ::DefWindowProc( hWnd, msg, (UINT)wParam, lParam );
   }

   return 0L;
}

// メイン関数
int APIENTRY _tWinMain( HINSTANCE hInstance,
                        HINSTANCE /*hPrevInstance*/,
                        LPTSTR    /*lpCmpLine*/,
                        INT       /*nCmdShow*/ )
{
   HRESULT hr = E_FAIL;
   MSG msg;
   ::ZeroMemory(&msg, sizeof(MSG));

   // 表示モードを記述するための構造体。
   DXGI_MODE_DESC sd;

   // Direct3D 関連自作クラスのインスタンスを作成
   g_pD3D11User = new D3D11USER();

   // ディスプレイモード一覧を取得する。
   // 取得した値はクラス内部に保持される。
   hr = g_pD3D11User->GetDisplayMode();
   if( FAILED( hr ) )
   {
      ::MessageBox( NULL, _T("ディスプレイモード取得エラー"), _T("初期化エラー"), MB_OK );
      goto EXIT;
   }
   // とりあえず最初に見つかったディスプレイモードを選択する
   CopyMemory( &sd, &g_pD3D11User->m_DisplayModeDesc[0], sizeof( DXGI_MODE_DESC ) );

   // ウィンドウの作成およびDirect3D の初期化
   hr = g_pD3D11User->InitD3D11( AppName, hInstance, WndProc, &sd, TRUE, TRUE, TRUE, TRUE );
   if( FAILED( hr ) )
   {
      ::MessageBoxW( NULL, L"Direct3D 11.0 初期化エラー", L"初期化エラー", MB_OK );
      goto EXIT;
   }

   // デバッグ専用フォント出力クラスの作成処理
   // デバックコンパイル時のみ使用する
#if defined(DEBUG) || defined(_DEBUG)
   g_pDebugFontUser = new CDebugFont();
   hr = g_pDebugFontUser->Create( g_pD3D11User->m_D3DDevice, 0.015f, 0.05f );
   if( FAILED( hr ) )
   {
      ::MessageBox( NULL, _T("デバックフォントクラス初期化エラー"), _T("初期化エラー"), MB_OK );
      goto EXIT;
   }
#endif

   // リソースの初期化
   hr = Init();
   if( FAILED( hr ) )
   {
      ::MessageBox( NULL, _T("リソース初期化エラー"), _T("初期化エラー"), MB_OK );
      goto EXIT;
   }
   
   ::ShowWindow(g_pD3D11User->m_hWnd, SW_SHOW);
   ::UpdateWindow(g_pD3D11User->m_hWnd);

   // メッセージループ
   do
   { 
      if( ::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
      {
         ::TranslateMessage(&msg); 
         ::DispatchMessage(&msg); 
      }
      else
      {
         hr = PowerSavingAndRender();
         if( FAILED( hr ) )
            ::DestroyWindow( g_pD3D11User->m_hWnd );
      }
   }while( msg.message != WM_QUIT );

EXIT:
   if( g_pD3D11User && g_pD3D11User->m_hWnd )
      ::DestroyWindow( g_pD3D11User->m_hWnd );

   ::UnregisterClass( AppName, hInstance );

   return msg.wParam;
}
