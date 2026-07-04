#include <tchar.h>
#include "DX11User.h"
#include "D3D11User.h"

// デバックウィンドウに出力
void D3D11USER::OutputMsg( const TCHAR* pMsg1,const  TCHAR* pMsg2, const TCHAR* pMsg3 )
{
#if defined(DEBUG) || defined(_DEBUG)
   TCHAR s[512];
   _stprintf_s( s, _T("■□■ %s [ %s ] ⇒ %s\n"), pMsg1, pMsg2, pMsg3 );
   OutputDebugString( s );
#endif
}

D3D11USER::D3D11USER()
{
   m_hWnd = NULL;
   m_D3DDevice = NULL;
   m_D3DDeviceContext = NULL;
   m_Adapter = NULL;
   m_SwapChain = NULL;
   m_RenderTargetView = NULL;
   m_DepthStencilView = NULL;
   m_UseDepthBuffer = TRUE;
}

D3D11USER::~D3D11USER()
{
   Invalidate();
}

// メモリ開放
void D3D11USER::Invalidate()
{
   ULONG cnt;
   TCHAR s[256];

   m_DisplayModeDesc.clear();

   if( m_DepthStencilView )
   {
      cnt = m_DepthStencilView->Release();
      if( cnt != 0 )
      {
         _stprintf_s( s, _T("%d"), cnt );
         OutputMsg( _T("ID3D11DepthStencilViewの参照カウンタが不正です。"), s, _T("ＮＧ") );
      }
      m_DepthStencilView = NULL;
   }


   if( m_RenderTargetView )
   {
      cnt = m_RenderTargetView->Release();
      if( cnt != 0 )
      {
         _stprintf_s( s, _T("%d"), cnt );
         OutputMsg( _T("ID3D11RenderTargetViewの参照カウンタが不正です。"), s, _T("ＮＧ") );
      }
      m_RenderTargetView = NULL;
   }

   if( m_SwapChain )
   {
      // スワップ チェーンをフルスクリーン モードで解放できないことがあるらしい。
      // フルスクリーンの場合ウィンドウモードに切り替えてから開放する必要があるらしい。
      // DXGI の概要
      BOOL FullScreen;
      m_SwapChain->GetFullscreenState( &FullScreen, NULL );
      // フルスクリーンのとき
      if( FullScreen == TRUE ) m_SwapChain->SetFullscreenState( FALSE, NULL );

      cnt = m_SwapChain->Release();
      if( cnt != 0 )
      {
         _stprintf_s( s, _T("%d"), cnt );
         OutputMsg( _T("IDXGISwapChainの参照カウンタが不正です。"), s, _T("ＮＧ") );
      }
      m_SwapChain = NULL;
   }

   if( m_D3DDeviceContext )
   {
      cnt = m_D3DDeviceContext->Release();
      if( cnt != 0 )
      {
         _stprintf_s( s, _T("%d"), cnt );
         OutputMsg( _T("ID3D11DeviceContextの参照カウンタが不正です。"), s, _T("ＮＧ") );
      }
      m_D3DDeviceContext = NULL;
   }
   if( m_D3DDevice )
   {
      cnt = m_D3DDevice->Release();
      if( cnt != 0 )
      {
         _stprintf_s( s, _T("%d"), cnt );
         OutputMsg( _T("ID3D11Deviceの参照カウンタが不正です。"), s, _T("ＮＧ") );
      }
      m_D3DDevice = NULL;
   }

   if( m_Adapter )
   {
      cnt = m_Adapter->Release();
      if( cnt != 0 )
      {
         _stprintf_s( s, _T("%d"), cnt );
         OutputMsg( _T("IDXGIAdapterの参照カウンタが不正です。"), s, _T("ＮＧ") );
      }
      m_Adapter = NULL;
   }
}

// デフォルトアダプターのインターフェースを作成する
HRESULT D3D11USER::CreateAdapter()
{
   HRESULT hr = E_FAIL;
   IDXGIFactory* pFactory = NULL;

   if( m_Adapter == NULL )
   {
      // ファクトリを作成する。
      // CreateDXGIFactory
      hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory) );
      if( FAILED( hr ) ) goto EXIT;

      // デフォルトアダプターを取得
      // IDXGIFactory::EnumAdapters
      hr = pFactory->EnumAdapters( 0, &m_Adapter );
      if( FAILED( hr ) ) goto EXIT;
   }

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pFactory );

   return hr;
}

// デフォルトディスプレイのモードを列挙する
HRESULT D3D11USER::GetDisplayMode()
{
   HRESULT hr = E_FAIL;
   IDXGIOutput* pOutput = NULL;
   DXGI_MODE_DESC* desc = NULL;

   TCHAR s[256];

   OutputMsg( _T("ディスプレイモード一覧の列挙"), _T(""), _T("開始") );

   m_DisplayModeDesc.clear();

   // アダプターを取得。
   hr = CreateAdapter();
   if( FAILED( hr ) ) goto EXIT;

   // アダプターに接続されているプライマリディスプレイを取得。
   // IDXGIAdapter::EnumOutputs
   hr = m_Adapter->EnumOutputs( 0, &pOutput );
   if( FAILED( hr ) ) goto EXIT;

   UINT num;
   // ディスプレイモード一覧の配列数を取得
   // IDXGIOutput::GetDisplayModeList
   hr = pOutput->GetDisplayModeList( D3D11USER_FORMAT, 0, &num, NULL );
   if( FAILED( hr ) ) goto EXIT;

   desc = new DXGI_MODE_DESC[num];
   // ディスプレイモードの一覧を取得
   hr = pOutput->GetDisplayModeList( D3D11USER_FORMAT, 0, &num, desc );
   if( FAILED( hr ) ) goto EXIT;

   for( UINT i=0; i<num; i++ )
   {
      m_DisplayModeDesc.push_back( desc[i] );
      _stprintf_s( s, _T( "解像度( %d : %d )　リフレッシュレート( %d / %d )" ),
       m_DisplayModeDesc[i].Width, m_DisplayModeDesc[i].Height, m_DisplayModeDesc[i].RefreshRate.Denominator, m_DisplayModeDesc[i].RefreshRate.Numerator );
      OutputMsg( _T("ディスプレイモード"), s, _T("リストに追加") );      
   }

   OutputMsg( _T("ディスプレイモード一覧の列挙"), _T(""), _T("完了") );

   hr = S_OK;

EXIT:
   SAFE_DELETE_ARRAY( desc );
   SAFE_RELEASE( pOutput );

   return hr;
}

// ウィンドウの作成 および Direct3Dの初期化
HRESULT D3D11USER::InitD3D11( TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode,
                              BOOL Windowed, BOOL DirectX11Only, BOOL MultisampleEnabled, BOOL UseDepthBuffer )
{
   HRESULT hr = E_FAIL;

   m_UseDepthBuffer = UseDepthBuffer;

   // ウィンドウを作成
   hr = InitWindow( AppName, hInstance, WndProc, pDisplayMode );
   if( FAILED( hr ) ) goto EXIT;

   // アダプターを取得。
   hr = CreateAdapter();
   if( FAILED( hr ) ) goto EXIT;

   // デバイスの作成
   hr = CreateDevice( DirectX11Only );
   if( FAILED( hr ) ) goto EXIT;

   // スワップチェーンの作成
   hr = CreateSwapChain( pDisplayMode, MultisampleEnabled );
   if( FAILED( hr ) ) goto EXIT;

   // レンダリングターゲットビューの作成
   hr = CreateRenderTargetView();
   if( FAILED( hr ) ) goto EXIT;

   // 深度ステンシルビューの作成
   hr = CreateDepthStencilView();
   if( FAILED( hr ) ) goto EXIT;

   // ターゲットビューの設定
   // ID3D11DeviceContext::OMSetRenderTargets
   m_D3DDeviceContext->OMSetRenderTargets( 1, &m_RenderTargetView, m_DepthStencilView );

   // ビューポート作成
   hr = CreateViewport();
   if( FAILED( hr ) ) goto EXIT;

   // 表示モードの自動切換えを無効にする。
   // 自動切換えを有効にする場合はこの関数をコメントアウトする。
   // しかしコメントアウトした場合、初期起動をフルスクリーンモードにしたとき正しく動作してくれない。
   hr = SetWindowAssociation();
   if( FAILED( hr ) ) goto EXIT;

   // デフォルトのラスタライザを設定する
   hr = SetDefaultRasterize();
   if( FAILED( hr ) ) goto EXIT;

   // デフォルトの深度ステンシルステートを設定する
   hr = SetDefaultDepthStencilState();
   if( FAILED( hr ) ) goto EXIT;

   // ウィンドウ作成やスワップチェーンの初期化時にはウィンドウモードで作成する。
   // 初回起動をフルスクリーンにする場合は、初期化終了後に表示モードを変更するようにする。
   if( Windowed == FALSE )
   {
      hr = ChangeWindowMode();
      if( FAILED( hr ) ) goto EXIT;
   }

   OutputMsg( _T("初期化完了"), _T(""), _T("(^。^)") );

   hr = S_OK;

EXIT:
   SAFE_RELEASE( m_Adapter );

   return hr;
}

// ウィンドウ作成
HRESULT D3D11USER::InitWindow( TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode )
{
   HRESULT hr = E_FAIL;
   // ウィンドウスタイル
   DWORD WindowStyle = 0;

   OutputMsg( _T("ウィンドウ作成"), _T(""), _T("開始") );

   WNDCLASSEX wc;
   wc.cbSize        = sizeof(WNDCLASSEX);
   wc.style         = CS_VREDRAW | CS_HREDRAW;
   wc.lpfnWndProc   = WndProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = sizeof(DWORD);
   wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
   wc.hIcon         = NULL;
   wc.hIconSm       = NULL;
   wc.lpszMenuName  = NULL;
   wc.lpszClassName = AppName;
   wc.hbrBackground = (HBRUSH)::GetStockObject( BLACK_BRUSH );
   wc.hInstance     = hInstance;
   ::RegisterClassEx(&wc);

   // 初期起動時の表示モードはウィンドウモードに固定とする。
   WindowStyle = D3D11USER_W_WINDOWSTYLE;

   RECT rc;
   ::SetRect( &rc, 0, 0, pDisplayMode->Width, pDisplayMode->Height );
   // ウィンドウモードのときタイトルバーを考慮して表示領域を設定するために必要なサイズを取得する
   // AdjustWindowRect
   ::AdjustWindowRect( &rc, WindowStyle, FALSE );

   // ウィンドウを作成
   m_hWnd = ::CreateWindow(
                           wc.lpszClassName,
                           wc.lpszClassName,
                           WindowStyle,
                           0,
                           0,
                           rc.right - rc.left,
                           rc.bottom - rc.top,
                           NULL,
                           NULL,
                           hInstance,
                           NULL
                        );

   if( m_hWnd == NULL )
   {
      hr = E_FAIL;
      goto EXIT;
   }

   OutputMsg( _T("ウィンドウ作成"), _T(""), _T("完了") );

   hr = S_OK;
EXIT:
   return hr;
}

// デバイスの作成
HRESULT D3D11USER::CreateDevice( BOOL DirectX11Only )
{
   HRESULT hr = E_FAIL;

   OutputMsg( _T("デバイス作成"), _T(""), _T("開始") );

   // デバイス作成時に使用するビデオカードの性能一覧
   // D3D_FEATURE_LEVEL
   D3D_FEATURE_LEVEL FeatureLevel[] =  {
	   D3D_FEATURE_LEVEL_11_1,  // Direct3D 11.0 SM 5
	   D3D_FEATURE_LEVEL_11_0,  // Direct3D 11.0 SM 5
	   D3D_FEATURE_LEVEL_10_1,  // Direct3D 10.1 SM 4
                                          D3D_FEATURE_LEVEL_10_0,  // Direct3D 10.0 SM 4
                                          D3D_FEATURE_LEVEL_9_3,   // Direct3D 9.3  SM 3
                                          D3D_FEATURE_LEVEL_9_2,   // Direct3D 9.2  SM 2
                                          D3D_FEATURE_LEVEL_9_1,   // Direct3D 9.1  SM 2
                                       };
   int featureCnt = 0;

   // ビデオカードがDirectX11およびシェーダーモデル5をサポートする環境でのみ実行可能
   if( DirectX11Only == TRUE )
      featureCnt = 1;
   // ビデオカードがサポートするDirect3Dおよびシェーダーモデルのバージョンを上位から自動選択
   else
      featureCnt = sizeof( FeatureLevel ) / sizeof( D3D_FEATURE_LEVEL );

#if defined(DEBUG) || defined(_DEBUG)
   // デバッグコンパイルの場合、デバッグレイヤーを有効にする。
   // ソフトウェア レイヤー
   UINT createDeviceFlag = D3D11_CREATE_DEVICE_DEBUG;
#else
   UINT createDeviceFlag = 0;
#endif

   // デバイスを作成する。
   // D3D11CreateDevice
   hr = D3D11CreateDevice(
            m_Adapter,                  // 使用するアダプターを設定。NULLの場合はデフォルトのアダプター。
            D3D_DRIVER_TYPE_UNKNOWN,    // D3D_DRIVER_TYPEのいずれか。ドライバーの種類。pAdapterが NULL 以外の場合は、D3D_DRIVER_TYPE_UNKNOWNを指定する。
            NULL,                       // ソフトウェアラスタライザを実装するDLLへのハンドル。D3D_DRIVER_TYPE を D3D_DRIVER_TYPE_SOFTWARE に設定している場合は NULL にできない。
            createDeviceFlag,           // D3D11_CREATE_DEVICE_FLAGの組み合わせ。デバイスを作成時に使用されるパラメータ。
            FeatureLevel,               // D3D_FEATURE_LEVELのポインタ
            featureCnt,                 // D3D_FEATURE_LEVEL配列の要素数
            D3D11_SDK_VERSION,          // DirectX SDKのバージョン。この値は固定。
            &m_D3DDevice,               // 初期化されたデバイス
            &m_FeatureLevel,            // 採用されたフィーチャーレベル
            &m_D3DDeviceContext         // 初期化されたデバイスコンテキスト
                           );
   if( FAILED( hr ) ) goto EXIT;

   // アダプターの情報を取得
   // DXGI_ADAPTER_DESC
   DXGI_ADAPTER_DESC desc;
   m_Adapter->GetDesc( &desc );

   // DirectXのバージョン、シェーダモデルのバージョンと採用したアダプターの名前をデバックウィンドウに出力
   TCHAR s[512];
   switch( m_FeatureLevel )
   {
   case D3D_FEATURE_LEVEL_11_1:
	   _stprintf_s( s, _T("Direct3D(11.1) SM(5)　%s"), desc.Description );
	   break;
   case D3D_FEATURE_LEVEL_11_0:
      _stprintf_s( s, _T("Direct3D(11.0) SM(5)　%s"), desc.Description );
      break;
   case D3D_FEATURE_LEVEL_10_1:
      _stprintf_s( s, _T("Direct3D(10.1) SM(4)　%s"), desc.Description );
      break;
   case D3D_FEATURE_LEVEL_10_0:
      _stprintf_s( s, _T("Direct3D(10.0) SM(4)　%s"), desc.Description );
      break;
   case D3D_FEATURE_LEVEL_9_3:
      _stprintf_s( s, _T("Direct3D(9.3) SM(3)　%s"), desc.Description );
      break;
   case D3D_FEATURE_LEVEL_9_2:
      _stprintf_s( s, _T("Direct3D(9.2) SM(2)　%s"), desc.Description );
      break;
   case D3D_FEATURE_LEVEL_9_1:
      _stprintf_s( s, _T("Direct3D(9.1) SM(2)　%s"), desc.Description );
      break;
   }
   if( SUCCEEDED( hr ) )
      OutputMsg( _T("ビデオカード"), s, _T("ОＫ") );
   else
      OutputMsg( _T("ビデオカード"), s, _T("ＮＧ") );

   OutputMsg( _T("デバイス作成"), _T(""), _T("完了") );

   hr = S_OK;
EXIT:

   return hr;
}

// スワップチェーンの作成
HRESULT D3D11USER::CreateSwapChain( DXGI_MODE_DESC* pDisplayMode, BOOL MultisampleEnabled )
{
   HRESULT hr = E_FAIL;
   TCHAR s[256];
   IDXGIFactory* pFactory = NULL;
   UINT Quality = 0;

   OutputMsg( _T("スワップチェーン作成"), _T(""), _T("開始") );

   // DXGI_SWAP_CHAIN_DESC
   DXGI_SWAP_CHAIN_DESC sd;
   ZeroMemory( &sd, sizeof(sd) );

   // DXGI_MODE_DESC
   ::CopyMemory( &(sd.BufferDesc), pDisplayMode, sizeof( DXGI_MODE_DESC ) );
   sd.BufferDesc.Format = D3D11USER_FORMAT;                         // バックバッファフォーマット
   // バックバッファのサーフェス使用法および CPU アクセス オプションを表す DXGI_USAGE 列挙型のメンバー
   sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;                // バックバッファにレンダリング可能にする。
   //スワップ チェーンのバッファー数を表す値。
   sd.BufferCount = 1;
   // 出力ウィンドウへの HWND ハンドル。このメンバーを NULL にはできない。
   sd.OutputWindow = m_hWnd;
   // 出力がウィンドウ モードの場合は TRUE。それ以外の場合は FALSE。
   // 初期起動時の表示モードはウィンドウモードに固定とする。
   sd.Windowed = TRUE;
   // サーフェスのスワップ処理後にバックバッファの内容を保持するか。
   sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;                        // バックバッファの内容を保持しない。
   // スワップ チェーンの動作のオプション。
   sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;               // フルスクリーンモードに切り替えると、アプリケーション ウィンドウのサイズに合わせて解像度が変更される。

   // ファクトリーを作成する
   // CreateDXGIFactoryで作成したファクトリーを使用すると実行できるがワーニングエラーになるので IDXGIAdapter から作成する。
   hr = m_Adapter->GetParent( __uuidof(IDXGIFactory), (void**)&pFactory );
   if( FAILED( hr ) ) goto EXIT;

   // マルチサンプリングの設定
   // マルチサンプルを使用しない場合の設定
   sd.SampleDesc.Count = 1;
   sd.SampleDesc.Quality = 0;

   int startMultisample;

   // サンプリング数が最大のものを採用する
   if( MultisampleEnabled == TRUE )
      startMultisample = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
   // マルチサンプルを無効にする。
   else
      startMultisample = 1;

   for( int i=startMultisample; i>=0; i-- )
   {
      // サポートするクォリティレベルの最大値を取得する
      // CheckMultisampleQualityLevels
      hr = m_D3DDevice->CheckMultisampleQualityLevels( pDisplayMode->Format, (UINT)i, &Quality );
      if( FAILED( hr ) ) goto EXIT;

      // 0 以外のときフォーマットとサンプリング数の組み合わせをサポートする
      // オンラインヘルプではCheckMultisampleQualityLevelsの戻り値が 0 のときサポートされないとかかれているが
      // pNumQualityLevels の戻り値が 0 かで判断する。
      // Direct3D 10 版 のオンラインヘルプにはそうかかれているので、オンラインヘルプの間違い。
      if( Quality != 0 )
      {
         sd.SampleDesc.Count = i;
         sd.SampleDesc.Quality = Quality - 1;

         // スワップチェーンを作成する。
         // CreateSwapChain
         hr = pFactory->CreateSwapChain( m_D3DDevice, &sd, &m_SwapChain );
         _stprintf_s( s, _T("Count(%d) Quality(%d)"), sd.SampleDesc.Count, sd.SampleDesc.Quality );
         if( SUCCEEDED( hr ) )
         {
            OutputMsg( _T("マルチサンプル"), s, _T("ОＫ") );
            break;
         }
         else
            OutputMsg( _T("マルチサンプル"), s, _T("ＮＧ") );
      }
   }
   if( m_SwapChain == NULL )
   {
      hr = E_FAIL;
      goto EXIT;
   }

   _stprintf_s( s, _T( "解像度( %d : %d )　リフレッシュレート( %d / %d )" ),
    pDisplayMode->Width, pDisplayMode->Height, pDisplayMode->RefreshRate.Denominator, pDisplayMode->RefreshRate.Numerator );
   OutputMsg( _T("ディスプレイモード"), s, _T("選択") );

   OutputMsg( _T("スワップチェーン作成"), _T(""), _T("完了") );

   hr = S_OK;

EXIT:
   SAFE_RELEASE( pFactory );

   return hr;
}

// レンダリングターゲットビューの作成
HRESULT D3D11USER::CreateRenderTargetView()
{
   HRESULT hr = E_FAIL;
   ID3D11Texture2D* pBackBuffer = NULL;

   OutputMsg( _T("レンダリングターゲットビュー作成"), _T(""), _T("開始") );

   // スワップチェーンからバックバッファを取得する
   hr = m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer );
   if( FAILED( hr ) ) goto EXIT;

   // レンダリングターゲットビューの作成
   // CreateRenderTargetView
   hr = m_D3DDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_RenderTargetView );
   if( FAILED( hr ) ) goto EXIT;
   
   OutputMsg( _T("レンダリングターゲットビュー作成"), _T(""), _T("完了") );

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pBackBuffer );

   return hr;
}

// 深度ステンシルビューの作成
HRESULT D3D11USER::CreateDepthStencilView()
{
   HRESULT hr = E_FAIL;
   ID3D11Texture2D* pBackBuffer = NULL;
   ID3D11Texture2D* pDepthBuffer = NULL;

   // D3D11_TEXTURE2D_DESC
   D3D11_TEXTURE2D_DESC descDS;
   
   // D3D11_DEPTH_STENCIL_VIEW_DESC
   D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;

   if( m_UseDepthBuffer == FALSE )
   {
      hr = S_OK;
      goto EXIT;
   }

   if( m_SwapChain == NULL ) goto EXIT;

   OutputMsg( _T("深度バッファビュー作成"), _T(""), _T("開始") );

   DXGI_SWAP_CHAIN_DESC chainDesc;
   // スワップチェーンの設定を取得する
   // GetDesc
   hr = m_SwapChain->GetDesc( &chainDesc );
   if( FAILED( hr ) ) goto EXIT;

   ::ZeroMemory( &descDS, sizeof( D3D11_TEXTURE2D_DESC ) );
   descDS.Width              = chainDesc.BufferDesc.Width;   // バックバッファと同じにする。
   descDS.Height             = chainDesc.BufferDesc.Height;  // バックバッファと同じにする。
   descDS.MipLevels          = 1;                            // ミップマップを作成しない
   descDS.ArraySize          = 1;                            // テクスチャーの配列数
   descDS.Format             = D3D11USER_DEPTH_FORMAT;       // フォーマット
   descDS.SampleDesc.Count   = chainDesc.SampleDesc.Count;   // バックバッファと同じにする。
   descDS.SampleDesc.Quality = chainDesc.SampleDesc.Quality; // バックバッファと同じにする。
   descDS.Usage              = D3D11_USAGE_DEFAULT;          // GPU による読み取りおよび書き込みアクセスを必要とするリソース。
//   descDS.BindFlags          = D3D11_BIND_DEPTH_STENCIL;     // ステンシルバッファ
   descDS.BindFlags          = D3D11_BIND_DEPTH_STENCIL |    // 深度ステンシルバッファとして作成する
                               D3D11_BIND_SHADER_RESOURCE;   // シェーダーリソースビューとして作成する
   descDS.CPUAccessFlags     = 0;                            // CPU アクセスが不要。
   descDS.MiscFlags          = 0;                            // その他のフラグも設定しない。
   // 深度バッファ用のテクスチャー作成
   // CreateTexture2D
   hr = m_D3DDevice->CreateTexture2D( &descDS, NULL, &pDepthBuffer );
   if( FAILED( hr ) ) goto EXIT;

   ::ZeroMemory( &descDSV, sizeof( D3D11_DEPTH_STENCIL_VIEW_DESC ) );

// テクスチャー作成時に指定したフォーマットと互換性があり、深度ステンシルビューとして指定できるフォーマットを指定する
//   descDSV.Format = descDS.Format;
   switch( descDS.Format )
   {
   // 8ビットフォーマットは使用できない？
   case DXGI_FORMAT_R8_TYPELESS:
      descDSV.Format = DXGI_FORMAT_R8_UNORM;
      break;
   case DXGI_FORMAT_R16_TYPELESS:
      descDSV.Format = DXGI_FORMAT_D16_UNORM;
      break;
   case DXGI_FORMAT_R32_TYPELESS:
      descDSV.Format = DXGI_FORMAT_D32_FLOAT;
      break;
   case DXGI_FORMAT_R24G8_TYPELESS:
      descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
      break;
   default:
      descDSV.Format = descDS.Format;
   }

   // マルチサンプルを使用している場合
   if( chainDesc.SampleDesc.Count > 1 ) descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
   // マルチサンプルを使用していない場合
   else                              descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

   descDSV.Texture2D.MipSlice = 0;
   // 深度ステンシルビューを作成
   // CreateDepthStencilView
   hr = m_D3DDevice->CreateDepthStencilView( pDepthBuffer, &descDSV, &m_DepthStencilView );
   if( FAILED( hr ) ) goto EXIT;

   OutputMsg( _T("深度バッファビュー作成"), _T(""), _T("完了") );

   hr = S_OK;

EXIT:
   SAFE_RELEASE( pDepthBuffer );
   SAFE_RELEASE( pBackBuffer );

   return hr;
}

// ビューポートの作成
HRESULT D3D11USER::CreateViewport()
{
   HRESULT hr = E_FAIL;

   if( m_SwapChain == NULL ) goto EXIT;

   OutputMsg( _T("ビューポート作成"), _T(""), _T("開始") );

   DXGI_SWAP_CHAIN_DESC chainDesc;
   // スワップチェーンの設定を取得する
   hr = m_SwapChain->GetDesc( &chainDesc );
   if( FAILED( hr ) ) goto EXIT;

   // D3D11_VIEWPORT
   D3D11_VIEWPORT Viewport[1];   
   Viewport[0].TopLeftX = 0;
   Viewport[0].TopLeftY = 0;
   Viewport[0].Width    = (FLOAT)chainDesc.BufferDesc.Width;
   Viewport[0].Height   = (FLOAT)chainDesc.BufferDesc.Height;
   Viewport[0].MinDepth = 0.0f;
   Viewport[0].MaxDepth = 1.0f;
   // RSSetViewports
   m_D3DDeviceContext->RSSetViewports( 1, Viewport );

   OutputMsg( _T("ビューポート作成"), _T(""), _T("完了") );

   hr = S_OK;
EXIT:
   return hr;
}

// ウィンドウアソシエーション
HRESULT D3D11USER::SetWindowAssociation()
{
   HRESULT hr = E_FAIL;
   IDXGIFactory* pFactory = NULL;

   OutputMsg( _T("ウィンドウアソシエーション設定"), _T(""), _T("開始") );

   // ファクトリーを作成する
   // CreateDXGIFactoryで作成したファクトリーを使用すると実行できるがワーニングエラーになるので IDXGIAdapter から作成する。
   hr = m_Adapter->GetParent( __uuidof(IDXGIFactory), (void**)&pFactory );
   if( FAILED( hr ) ) goto EXIT;
   
   // 表示モードの自動切換えを無効にする。
   // MakeWindowAssociation
   hr = pFactory->MakeWindowAssociation( m_hWnd, DXGI_MWA_NO_WINDOW_CHANGES );
   if( FAILED( hr ) ) goto EXIT;

   OutputMsg( _T("ウィンドウアソシエーション設定"), _T(""), _T("完了") );

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pFactory );

   return hr;
}

// デフォルトのラスタライザを設定
HRESULT D3D11USER::SetDefaultRasterize()
{
   HRESULT hr = E_FAIL;

   OutputMsg( _T("デフォルトのラスタライザを設定"), _T(""), _T("開始") );

   // ID3D11RasterizerState
   ID3D11RasterizerState* g_pRasterState = NULL;
   // D3D11_RASTERIZER_DESC
   D3D11_RASTERIZER_DESC rsState;
   rsState.FillMode = D3D11_FILL_SOLID;    // ポリゴン面描画
   rsState.CullMode = D3D11_CULL_BACK;     // 裏面を描画しない
//   rsState.FrontCounterClockwise = FALSE;  // 時計回りを表面
   rsState.FrontCounterClockwise = TRUE;   // 反時計回りを表面
   rsState.DepthBias = 0;
   rsState.DepthBiasClamp = 0;
   rsState.SlopeScaledDepthBias = 0;
   rsState.DepthClipEnable = TRUE;
   rsState.ScissorEnable = FALSE;          // シザー矩形無効

   // スワップチェーンのマルチサンプリングの設定にあわせる
   DXGI_SWAP_CHAIN_DESC swapDesc;
   m_SwapChain->GetDesc( &swapDesc );
   if( swapDesc.SampleDesc.Count != 1 )
      rsState.MultisampleEnable = TRUE;
   else
      rsState.MultisampleEnable = FALSE;

   rsState.AntialiasedLineEnable = FALSE;
   // CreateRasterizerState
   hr = m_D3DDevice->CreateRasterizerState( &rsState, &g_pRasterState );
   if( FAILED( hr ) ) goto EXIT;

   // RSSetState
   m_D3DDeviceContext->RSSetState( g_pRasterState );
   
   hr = S_OK;

   OutputMsg( _T("デフォルトのラスタライザを設定"), _T(""), _T("完了") );

EXIT:
   SAFE_RELEASE( g_pRasterState );
   return hr;
}

// デフォルトの深度ステンシルステートを設定
HRESULT D3D11USER::SetDefaultDepthStencilState()
{
   HRESULT hr = E_FAIL;

   OutputMsg( _T("デフォルトの深度ステンシルステートを設定"), _T(""), _T("開始") );

   ID3D11DepthStencilState* pDepthStencilState = NULL;
   // D3D11_RASTERIZER_DESC
   D3D11_DEPTH_STENCIL_DESC dsState;
   ::ZeroMemory( &dsState, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
   dsState.DepthEnable = TRUE;
   dsState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
   dsState.DepthFunc = D3D11_COMPARISON_LESS;
   dsState.StencilEnable = FALSE;

   hr = m_D3DDevice->CreateDepthStencilState( &dsState, &pDepthStencilState );
   if( FAILED( hr ) ) goto EXIT;

   m_D3DDeviceContext->OMSetDepthStencilState( pDepthStencilState, 0 );
   
   hr = S_OK;

   OutputMsg( _T("デフォルトの深度ステンシルステートを設定"), _T(""), _T("完了") );

EXIT:
   SAFE_RELEASE( pDepthStencilState );
   return hr;
}

// 表示モードの切り替えを最適化
// DXGI の概要
HRESULT D3D11USER::ChangeWindowModeOptimization( UINT Width, UINT Height )
{
   HRESULT hr = E_FAIL;

   if( m_SwapChain == NULL ) goto EXIT;

   OutputMsg( _T("表示モード変更の最適化処理"), _T(""), _T("開始") );

   DXGI_SWAP_CHAIN_DESC desc;
   hr = m_SwapChain->GetDesc( &desc );
   if( FAILED( hr ) ) goto EXIT;

   // ターゲットビューを解除
   OutputMsg( _T("ターゲットビューを解除"), _T(""), _T("開始") );

   // OMSetRenderTargets
   m_D3DDeviceContext->OMSetRenderTargets( 0, NULL, NULL );
   SAFE_RELEASE( m_RenderTargetView );
   SAFE_RELEASE( m_DepthStencilView );
   OutputMsg( _T("ターゲットビューを解除"), _T(""), _T("完了") );

   // スワップ チェーンのバック バッファー サイズ、フォーマット、およびバッファー数を変更する。
   // アプリケーション ウィンドウのサイズが変更されたときに呼び出す必要がある。
   // ResizeBuffers
   hr = m_SwapChain->ResizeBuffers( desc.BufferCount, Width, Height, desc.BufferDesc.Format, desc.Flags );
   if( FAILED( hr ) ) goto EXIT;

   // レンダリングターゲットビューの作成
   hr = CreateRenderTargetView();
   if( FAILED( hr ) ) goto EXIT;

   // 深度ステンシルビューの作成
   hr = CreateDepthStencilView();
   if( FAILED( hr ) ) goto EXIT;

   // ターゲットビューの設定
   m_D3DDeviceContext->OMSetRenderTargets( 1, &m_RenderTargetView, m_DepthStencilView );

   OutputMsg( _T("表示モード変更の最適化処理"), _T(""), _T("完了") );

   hr = S_OK;
EXIT:
   return hr;
}

// 表示モードを切り替える
HRESULT D3D11USER::ChangeWindowMode()
{
   HRESULT hr = E_FAIL;

   if( m_SwapChain == NULL ) goto EXIT;

   OutputMsg( _T("表示モード変更"), _T(""), _T("開始") );

   DXGI_SWAP_CHAIN_DESC desc;
   hr = m_SwapChain->GetDesc( &desc );
   if( FAILED( hr ) ) goto EXIT;

   BOOL FullScreen;

   // GetFullscreenState
   hr = m_SwapChain->GetFullscreenState( &FullScreen, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // SetFullscreenState
   hr = m_SwapChain->SetFullscreenState( !FullScreen, NULL );
   if( FAILED( hr ) ) goto EXIT;

   switch( FullScreen )
   {
   case TRUE:
      OutputMsg( _T("表示モード変更"), _T("ウィンドウモードに変更"), _T("完了") );
      break;
   case FALSE:
      OutputMsg( _T("表示モード変更"), _T("フルスクリーンに変更"), _T("完了") );
      break;
   }

   // 初期起動をフルスクリーンモードにした場合、ウィンドウモードに変更すると
   // ウィンドウがアクティブにならないので表示させる。
   ::ShowWindow(m_hWnd, SW_SHOW);

   // フルスクリーンからウィンドウモードに切り替えるとき WM_SIZE イベントが発生せず、結果的に IDXGISwapChain::ResizeBuffers がコールされない。
   // 環境にもよるようだが、画面上に何も表示されない現象が発生する可能性があるので
   // D3D11USER::ChangeWindowModeOptimization() は D3D11USER::ChangeWindowMode() 内でコールするように修正し、ここでの処理は無効にする
   hr = ChangeWindowModeOptimization( 0, 0 );

   hr = S_OK;
EXIT:
   return hr;
}

// 頂点、インデックス、定数バッファを作成する
HRESULT D3D11USER::CreateBuffer( ID3D11Buffer** pBuffer, void* pData, size_t size, UINT CPUAccessFlag, D3D11_BIND_FLAG BindFlag )
{
   HRESULT hr = E_FAIL;

   // バッファー リソース。
   // D3D11_BUFFER_DESC
   D3D11_BUFFER_DESC BufferDesc;

   // サブリソース
   // D3D11_SUBRESOURCE_DATA
   D3D11_SUBRESOURCE_DATA* resource = NULL;

   D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
   UINT CPUAccessFlags = 0;

   switch( CPUAccessFlag )
   {
   // CPUアクセスを許可しない
   case 0:
      Usage = D3D11_USAGE_DEFAULT;
      CPUAccessFlags = CPUAccessFlag;
      break;
   // CPUアクセスを許可する
   default:
      Usage = D3D11_USAGE_DYNAMIC;
      CPUAccessFlags = CPUAccessFlag;
      break;
   }

   // 初期値を設定する
   if( pData )
   {
      resource = new D3D11_SUBRESOURCE_DATA();
      resource->pSysMem = pData;
      resource->SysMemPitch = 0;
      resource->SysMemSlicePitch = 0;
   }

   // バッファの設定
   ::ZeroMemory( &BufferDesc, sizeof( BufferDesc ) );
   BufferDesc.ByteWidth             = size;                      // バッファサイズ
   BufferDesc.Usage                 = Usage;                     // リソース使用法を特定する
   BufferDesc.BindFlags             = BindFlag;                  // バッファの種類
   BufferDesc.CPUAccessFlags        = CPUAccessFlags;            // CPU アクセス
   BufferDesc.MiscFlags             = 0;                         // その他のフラグも設定しない

   // バッファを作成する
   // ID3D11Device::CreateBuffer
   hr = m_D3DDevice->CreateBuffer( &BufferDesc, resource, pBuffer );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;

EXIT:
   SAFE_DELETE( resource );
   return hr;
}

// 頂点バッファを作成する
HRESULT D3D11USER::CreateVertexBuffer( ID3D11Buffer** pBuffer, void* pData, size_t size, UINT CPUAccessFlag )
{
   return CreateBuffer( pBuffer, pData, size, CPUAccessFlag, D3D11_BIND_VERTEX_BUFFER );
}

// インデックスバッファを作成する
HRESULT D3D11USER::CreateIndexBuffer( ID3D11Buffer** pBuffer, void* pData, size_t size, UINT CPUAccessFlag )
{
   return CreateBuffer( pBuffer, pData, size, CPUAccessFlag, D3D11_BIND_INDEX_BUFFER );
}

// 定数バッファを作成する
HRESULT D3D11USER::CreateConstantBuffer( ID3D11Buffer** pBuffer, void* pData, size_t size, UINT CPUAccessFlag )
{
   return CreateBuffer( pBuffer, pData, size, CPUAccessFlag, D3D11_BIND_CONSTANT_BUFFER );
}

// テクスチャーファイルからシェーダーリソースビュー配列を作成する
HRESULT D3D11USER::CreateShaderResourceViewArray( TCHAR* pTextureFileNames[], UINT NumTextures, ID3D11ShaderResourceView** pSRVView )
{
   HRESULT hr = E_FAIL;

   ID3D11Texture2D* pTexture = NULL;

   D3DX11_IMAGE_LOAD_INFO info;
   ::ZeroMemory( &info, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
   info.Width = D3DX11_DEFAULT; 
   info.Height = D3DX11_DEFAULT; 
   info.Depth = D3DX11_DEFAULT; 
   info.FirstMipLevel = D3DX11_DEFAULT;          // テクスチャーの最高解像度のミップマップ レベル。
   info.MipLevels = 1;                           // ミップマップ数。ここでは1つのみとする。
   info.Usage = D3D11_USAGE_STAGING;             // GPU から CPU へのデータ転送 (コピー) をサポート
   info.BindFlags = 0;
   info.CpuAccessFlags = D3D11_CPU_ACCESS_READ;  // リソースをマップ可能にし、CPU がそのリソースの内容を読み取れるようにする
   info.MiscFlags = 0;
   info.Format = DXGI_FORMAT_FROM_FILE;
   info.Filter = D3DX11_FILTER_POINT;            // テクスチャー読み込み時に使用するフィルター
   info.MipFilter = D3DX11_FILTER_POINT;         // ミップマップ作成時に使用するフィルター

   D3D11_TEXTURE2D_DESC desc;
   ::ZeroMemory( &desc, sizeof( D3D11_TEXTURE2D_DESC ) );

   for( UINT i=0; i<NumTextures; i++ )
   {
      ID3D11Resource* pLoadedRes = NULL;
      ID3D11Texture2D* pLoadedTexture2D = NULL;

      // 画像ファイルを読み込んでテクスチャーを作成する
      hr = D3DX11CreateTextureFromFile( m_D3DDevice, pTextureFileNames[i], &info, NULL, &pLoadedRes, NULL );
      if( FAILED( hr ) )
         goto EXIT;

      hr = pLoadedRes->QueryInterface( __uuidof( ID3D11Texture2D ), reinterpret_cast <void**>(&pLoadedTexture2D) );
      if( FAILED( hr ) )
         goto EXIT;

      // 空テクスチャー作成
      if( i == 0 )
      {
         // 画像ファイルを読み込んで作成したテクスチャーの情報を取得する
         pLoadedTexture2D->GetDesc( &desc );

         desc.Usage = D3D11_USAGE_DEFAULT;
         desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
         desc.CPUAccessFlags = 0;
         desc.ArraySize = NumTextures;
         hr = m_D3DDevice->CreateTexture2D( &desc, NULL, &pTexture );
         if( FAILED( hr ) )
            goto EXIT;
      }

      D3D11_MAPPED_SUBRESOURCE mappedResource;

      // ファイルから画像ファイルを読み込んで作成したテクスチャーを、読み込みモードでマップする
      hr = m_D3DDeviceContext->Map( pLoadedTexture2D, NULL, D3D11_MAP_READ, 0, &mappedResource );
      if( FAILED( hr ) )
         goto EXIT;

      // 空テクスチャーのサブリソースをファイルから画像ファイルを読み込んで作成したテクスチャーの内容で更新する
      for( UINT iMip = 0; iMip < desc.MipLevels; iMip++ )
      {
         m_D3DDeviceContext->UpdateSubresource( pTexture,
                                                D3D11CalcSubresource( iMip, i, desc.MipLevels ),
                                                NULL,
                                                mappedResource.pData,
                                                mappedResource.RowPitch,
                                                0
                                              );
      }
      m_D3DDeviceContext->Unmap( pLoadedTexture2D, NULL );

      SAFE_RELEASE( pLoadedTexture2D );
      SAFE_RELEASE( pLoadedRes );
   }

   D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
   ZeroMemory( &SRVDesc, sizeof( SRVDesc ) );
   SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
   SRVDesc.Texture2DArray.MipLevels = desc.MipLevels;
   SRVDesc.Texture2DArray.ArraySize = NumTextures;
   // シェーダーリソースビューを作成する
   hr = m_D3DDevice->CreateShaderResourceView( pTexture, &SRVDesc, pSRVView );
   if( FAILED( hr ) )
      goto EXIT;

   hr = S_OK;

EXIT:
   SAFE_RELEASE( pTexture );

   return hr;
}

// 頂点シェーダーをファイルから作成する
HRESULT D3D11USER::CreateVertexShaderFromFile( OUT ID3D11VertexShader** pVertexShader,
                                               IN TCHAR pSrcFile[], IN CHAR pFunctionName[], IN CHAR pProfile[],
                                               OUT ID3D11InputLayout** g_pLayout,
                                               IN D3D11_INPUT_ELEMENT_DESC pLayoutDesc[], UINT NumElements )
{
   HRESULT hr = E_FAIL;
   CHAR Profile[256];

   ID3D10Blob* pBlob = NULL;
   BYTE* vsbuffer = NULL;
   size_t vsbuffersize = NULL;

   // 行列を列優先で設定し、古い形式の記述を許可しないようにする
   UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
   // 最適化レベルを設定する
#if defined(DEBUG) || defined(_DEBUG)
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

   // Direct3D 9 モードで実行中の場合は、バージョンを変更する必要がある
   switch( m_FeatureLevel )
   {
   case D3D_FEATURE_LEVEL_9_1:
   case D3D_FEATURE_LEVEL_9_2:
      strcpy_s( Profile, "vs_4_0_level_9_1" );
      break;
   case D3D_FEATURE_LEVEL_9_3:
      strcpy_s( Profile, "vs_4_0_level_9_3" );
      break;
   default:
      strcpy_s( Profile, pProfile );
      break;
   }

   // ファイルを元にエフェクトをコンパイルする
   hr = D3DX11CompileFromFile( pSrcFile, NULL, NULL, pFunctionName, Profile, Flag1, 0, NULL, &pBlob, NULL, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // コンパイル済みシェーダーから、頂点シェーダー オブジェクトを作成する
   hr = m_D3DDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pVertexShader );
   if( FAILED( hr ) ) goto EXIT;

    vsbuffer = (LPBYTE)pBlob->GetBufferPointer();
    vsbuffersize = pBlob->GetBufferSize();

   // シェーダーで使用される入力バッファーを記述するための入力レイアウトオブジェクトを作成する。
   hr = m_D3DDevice->CreateInputLayout( pLayoutDesc, NumElements, vsbuffer, vsbuffersize, g_pLayout );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pBlob );
   return hr;
}

// 頂点シェーダーをヘッダファイルから作成する
HRESULT D3D11USER::CreateVertexShaderFromMemory( ID3D11VertexShader** pVertexShader,
                                                 const BYTE* pShader, size_t size,
                                                 OUT ID3D11InputLayout** g_pLayout,
                                                 IN D3D11_INPUT_ELEMENT_DESC pLayoutDesc[], UINT NumElements )
{
   HRESULT hr = E_FAIL;

   // コンパイル済みシェーダーから、頂点シェーダー オブジェクトを作成する
   hr = m_D3DDevice->CreateVertexShader( pShader, size, NULL, pVertexShader );
   if( FAILED( hr ) ) goto EXIT;

   // シェーダーで使用される入力バッファーを記述するための入力レイアウトオブジェクトを作成する。
   hr = m_D3DDevice->CreateInputLayout( pLayoutDesc, NumElements, pShader, size, g_pLayout );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   return hr;
}

// ハルシェーダーをファイルから作成する
HRESULT D3D11USER::CreateHullShaderFromFile( ID3D11HullShader** pHullShader,
                                             TCHAR pSrcFile[],
                                             CHAR pFunctionName[],
                                             CHAR pProfile[] )
{
   HRESULT hr = E_FAIL;
   CHAR Profile[256];

   ID3D10Blob* pBlob = NULL;

   UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

   strcpy_s( Profile, pProfile );

   // ファイルを元にエフェクトをコンパイルする
   hr = D3DX11CompileFromFile( pSrcFile, NULL, NULL, pFunctionName, Profile, Flag1, 0, NULL, &pBlob, NULL, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // コンパイル済みシェーダーから、ハルシェーダー オブジェクトを作成する
   hr = m_D3DDevice->CreateHullShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pHullShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pBlob );
   return hr;
}

// ハルシェーダーをメモリから作成する
HRESULT D3D11USER::CreateHullShaderFromMemory( ID3D11HullShader** pHullShader, const BYTE* pShader, size_t size )
{
   HRESULT hr = E_FAIL;

   // コンパイル済みシェーダーから、ハルシェーダー オブジェクトを作成する
   hr = m_D3DDevice->CreateHullShader( pShader, size, NULL, pHullShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   return hr;
}

// ドメインシェーダーをファイルから作成する
HRESULT D3D11USER::CreateDomainShaderFromFile( ID3D11DomainShader** pDomainShader,
                                               TCHAR pSrcFile[],
                                               CHAR pFunctionName[],
                                               CHAR pProfile[] )
{
   HRESULT hr = E_FAIL;
   CHAR Profile[256];

   ID3D10Blob* pBlob = NULL;

   UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

   strcpy_s( Profile, pProfile );

   // ファイルを元にエフェクトをコンパイルする
   hr = D3DX11CompileFromFile( pSrcFile, NULL, NULL, pFunctionName, Profile, Flag1, 0, NULL, &pBlob, NULL, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // コンパイル済みシェーダーから、ドメインシェーダー オブジェクトを作成する
   hr = m_D3DDevice->CreateDomainShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pDomainShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pBlob );
   return hr;
}

// ドメインシェーダーをメモリから作成する
HRESULT D3D11USER::CreateDomainShaderFromMemory( ID3D11DomainShader** pDomainShader, const BYTE* pShader, size_t size )
{
   HRESULT hr = E_FAIL;

   // コンパイル済みシェーダーから、ドメインシェーダー オブジェクトを作成する
   hr = m_D3DDevice->CreateDomainShader( pShader, size, NULL, pDomainShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   return hr;
}

// ジオメトリシェーダーをファイルから作成する
HRESULT D3D11USER::CreateGeometryShaderFromFile( ID3D11GeometryShader** pGeometryShader,
                                                 TCHAR pSrcFile[],
                                                 CHAR pFunctionName[],
                                                 CHAR pProfile[] )
{
   HRESULT hr = E_FAIL;
   CHAR Profile[256];

   ID3D10Blob* pBlob = NULL;

   UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

   //// Direct3D 9 モードで実行中の場合はエラーとする
   //switch( m_FeatureLevel )
   //{
   //case D3D_FEATURE_LEVEL_9_1:
   //case D3D_FEATURE_LEVEL_9_2:
   //case D3D_FEATURE_LEVEL_9_3:
   //   goto EXIT;
   //default:
      strcpy_s( Profile, pProfile );
   //   break;
   //}

   // ファイルを元にエフェクトをコンパイルする
   // D3DX11CompileFromFile
   hr = D3DX11CompileFromFile( pSrcFile, NULL, NULL, pFunctionName, Profile, Flag1, 0, NULL, &pBlob, NULL, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // コンパイル済みシェーダーから、ジオメトリシェーダー オブジェクトを作成する
   // ID3D11Device::CreateGeometryShader
   hr = m_D3DDevice->CreateGeometryShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pGeometryShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pBlob );
   return hr;
}

// ジオメトリシェーダーをメモリから作成する
HRESULT D3D11USER::CreateGeometryShaderFromMemory( ID3D11GeometryShader** pGeometryShader, const BYTE* pShader, size_t size )
{
   HRESULT hr = E_FAIL;

   // コンパイル済みシェーダーから、ジオメトリシェーダー オブジェクトを作成する
   hr = m_D3DDevice->CreateGeometryShader( pShader, size, NULL, pGeometryShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   return hr;
}

// ストリーミング出力バッファーへの書き込みが可能なジオメトリ シェーダーをファイルから作成する
HRESULT D3D11USER::CreateGeometryShaderWithStreamOutputFromFile(
                                                ID3D11GeometryShader** pGeometryShader,
                                                TCHAR pSrcFile[],   CHAR pFunctionName[], CHAR pProfile[],
                                                const D3D11_SO_DECLARATION_ENTRY *pSODeclaration,
                                                UINT NumEntries,
                                                const UINT *pBufferStrides,
                                                UINT NumStrides )
{
   HRESULT hr = E_FAIL;
   CHAR Profile[256];

   ID3D10Blob* pBlob = NULL;

   UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif
   strcpy_s( Profile, pProfile );

   // ファイルを元にエフェクトをコンパイルする
   hr = D3DX11CompileFromFile( pSrcFile, NULL, NULL, pFunctionName, Profile, Flag1, 0, NULL, &pBlob, NULL, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // ストリーム出力するジオメトリシェーダーを作成
   // ID3D11Device::CreateGeometryShaderWithStreamOutput
   hr = m_D3DDevice->CreateGeometryShaderWithStreamOutput(
                                             pBlob->GetBufferPointer(),
                                             pBlob->GetBufferSize(),
                                             pSODeclaration,
                                             NumEntries,
                                             pBufferStrides,
                                             NumStrides,
                                             D3D11_SO_NO_RASTERIZED_STREAM,
                                             NULL,
                                             pGeometryShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pBlob );
   return hr;
}

// ストリーミング出力バッファーへの書き込みが可能なジオメトリ シェーダーをメモリから作成する
HRESULT D3D11USER::CreateGeometryShaderWithStreamOutputFromMemory( 
                                          ID3D11GeometryShader** pGeometryShader,
                                          const BYTE* pShader, size_t size,
                                          const D3D11_SO_DECLARATION_ENTRY *pSODeclaration,
                                          UINT NumEntries,
                                          const UINT *pBufferStrides,
                                          UINT NumStrides )
{
   HRESULT hr = E_FAIL;

   // ストリーム出力するジオメトリシェーダーを作成
   // ID3D11Device::CreateGeometryShaderWithStreamOutput
   hr = m_D3DDevice->CreateGeometryShaderWithStreamOutput(
                                             pShader,
                                             size,
                                             pSODeclaration,
                                             NumEntries,
                                             pBufferStrides,
                                             NumStrides,
                                             D3D11_SO_NO_RASTERIZED_STREAM,
                                             NULL,
                                             pGeometryShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   return hr;
}

// ピクセルシェーダーをファイルから作成する
HRESULT D3D11USER::CreatePixelShaderFromFile( ID3D11PixelShader** pPixelShader,
                                              TCHAR pSrcFile[],
                                              CHAR  pFunctionName[],
                                              CHAR  pProfile[]
)
{
   HRESULT hr = E_FAIL;
   CHAR Profile[256];

   ID3D10Blob* pBlob = NULL;

   UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
   Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

   // Direct3D 9 モードで実行中の場合は、バージョンを変更する必要がある
   switch( m_FeatureLevel )
   {
   case D3D_FEATURE_LEVEL_9_1:
   case D3D_FEATURE_LEVEL_9_2:
      strcpy_s( Profile, "ps_4_0_level_9_1" );
      break;
   case D3D_FEATURE_LEVEL_9_3:
      strcpy_s( Profile, "ps_4_0_level_9_3" );
      break;
   default:
      strcpy_s( Profile, pProfile );
      break;
   }

   // ファイルを元にエフェクトをコンパイルする
   // D3DX11CompileFromFile
   hr = D3DX11CompileFromFile( pSrcFile, NULL, NULL, pFunctionName, Profile, Flag1, 0, NULL, &pBlob, NULL, NULL );
   if( FAILED( hr ) ) goto EXIT;

   // コンパイル済みシェーダーから、ピクセルシェーダー オブジェクトを作成する
   // ID3D11Device::CreatePixelShader
   hr = m_D3DDevice->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pPixelShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pBlob );
   return hr;
}

// ピクセルシェーダーをヘッダファイルから作成する
HRESULT D3D11USER::CreatePixelShaderFromMemory( ID3D11PixelShader** pPixelShader, const BYTE* pShader, size_t size )
{
   HRESULT hr = E_FAIL;

   // コンパイル済みシェーダーから、ピクセルシェーダー オブジェクトを作成する
   hr = m_D3DDevice->CreatePixelShader( pShader, size, NULL, pPixelShader );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;
EXIT:
   return hr;
}

// スクリーンショット作成
HRESULT D3D11USER::CreateScreenShot()
{
   HRESULT hr = E_FAIL;
   TCHAR root[] = _T("ScreenShot");
   TCHAR path[256];
   int maxFileCnt = 20;
   int i = 0;

   ID3D11Texture2D* pBackBuffer = NULL;
   ID3D11Texture2D* pTexture = NULL;

   OutputMsg( _T("スクリーンショット作成"), _T(""), _T("開始") );

   // ScreenShot フォルダがありますか？
   if( ::PathFileExists( root ) == false )
   {
      // ないので ScreenShotフォルダを作成する
      if( ::CreateDirectory( root, NULL ) == FALSE ) goto EXIT;
   }

   //ファイルを作成しすぎてディスクがパンクすることがないようにするため、ファイルの作成可能数を限定する。
   while( i < maxFileCnt )
   {      
      _stprintf_s( path, _T("%s\\%02d.jpg"), root, i );

      //ファイルがありますか？
      if( ::PathFileExists( path ) == FALSE ) break;

      i++;
   }   
   // スクリーンショットの作成可能数をオーバーしているため作成できません。
   if( i >= maxFileCnt )
   {
      OutputMsg( _T("スクリーンショット作成"), _T(""), _T("作成数オーバー") );
      goto EXIT;
   }
  
   // スワップチェーンからバックバッファを取得する
   hr = m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer );
   if( FAILED( hr ) ) goto EXIT;

   DXGI_SWAP_CHAIN_DESC swapchainDesc;
   m_SwapChain->GetDesc( &swapchainDesc );

   // バックバッファがマルチサンプル無効で作成されている場合
   if( swapchainDesc.SampleDesc.Count == 1 && swapchainDesc.SampleDesc.Quality == 0 )
   {
      // この場合はそのままファイル出力する
      // D3DX11SaveTextureToFile
      hr = D3DX11SaveTextureToFile( m_D3DDeviceContext, pBackBuffer, D3DX11_IFF_JPG, path );
      if( FAILED( hr ) ) goto EXIT;
   }
   // バックバッファがマルチサンプルが有効で作成されている場合
   else
   {
      D3D11_TEXTURE2D_DESC texDesc;
      pBackBuffer->GetDesc( &texDesc );
      texDesc.Usage = D3D11_USAGE_DEFAULT;
      texDesc.SampleDesc.Count   = 1;
      texDesc.SampleDesc.Quality = 0;
      // コピー先用のテクスチャーを作成する。
      hr = m_D3DDevice->CreateTexture2D( &texDesc, NULL, &pTexture );
      if( FAILED( hr ) ) goto EXIT;

      // マルチサンプリングされたリソースをマルチサンプリングされていないリソースにコピーする
      // ID3D11DeviceContext::ResolveSubresource
      m_D3DDeviceContext->ResolveSubresource( pTexture, NULL, pBackBuffer, NULL, texDesc.Format );

      // ファイル出力
      hr = D3DX11SaveTextureToFile( m_D3DDeviceContext, pTexture, D3DX11_IFF_JPG, path );
      if( FAILED( hr ) ) goto EXIT;
   }

   OutputMsg( _T("スクリーンショット作成"), path, _T("完了") );

   hr = S_OK;

EXIT:
   SAFE_RELEASE( pTexture );
   SAFE_RELEASE( pBackBuffer );

   return hr;
}

// ブレンド ステートを無効にするための設定を取得する
D3D11_RENDER_TARGET_BLEND_DESC D3D11USER::GetDefaultBlendDesc()
{
   D3D11_RENDER_TARGET_BLEND_DESC RenderTarget;

   RenderTarget.BlendEnable = FALSE;
   RenderTarget.SrcBlend = D3D11_BLEND_ONE;
   RenderTarget.DestBlend = D3D11_BLEND_ZERO;
   RenderTarget.BlendOp = D3D11_BLEND_OP_ADD;
   RenderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
   RenderTarget.DestBlendAlpha = D3D11_BLEND_ZERO;
   RenderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
   RenderTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

   return RenderTarget;
}

// 線形合成用ブレンド ステートのためのを設定を取得する
D3D11_RENDER_TARGET_BLEND_DESC D3D11USER::GetAlignmentBlendDesc()
{
   D3D11_RENDER_TARGET_BLEND_DESC RenderTarget;

   RenderTarget.BlendEnable = TRUE;
   RenderTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
   RenderTarget.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
   RenderTarget.BlendOp = D3D11_BLEND_OP_ADD;
   RenderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
   RenderTarget.DestBlendAlpha = D3D11_BLEND_ZERO;
   RenderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
   RenderTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

   return RenderTarget;
}

// 加算合成用ブレンド ステートのためのを設定を取得する
D3D11_RENDER_TARGET_BLEND_DESC D3D11USER::GetAddBlendDesc()
{
   D3D11_RENDER_TARGET_BLEND_DESC RenderTarget;

   RenderTarget.BlendEnable = TRUE;
   RenderTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
   RenderTarget.DestBlend = D3D11_BLEND_ONE;
   RenderTarget.BlendOp = D3D11_BLEND_OP_ADD;
   RenderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
   RenderTarget.DestBlendAlpha = D3D11_BLEND_ZERO;
   RenderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
   RenderTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

   return RenderTarget;
}

// 減算合成用ブレンド ステートのためのを設定を取得する
D3D11_RENDER_TARGET_BLEND_DESC D3D11USER::GetSubtractBlendDesc()
{
   D3D11_RENDER_TARGET_BLEND_DESC RenderTarget;

   RenderTarget.BlendEnable = TRUE;
   RenderTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
   RenderTarget.DestBlend = D3D11_BLEND_ONE;
   RenderTarget.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
   RenderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
   RenderTarget.DestBlendAlpha = D3D11_BLEND_ZERO;
   RenderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
   RenderTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

   return RenderTarget;
}

// 積算合成用ブレンド ステートのためのを設定を取得する
D3D11_RENDER_TARGET_BLEND_DESC D3D11USER::GetMultipleBlendDesc()
{
   D3D11_RENDER_TARGET_BLEND_DESC RenderTarget;

   RenderTarget.BlendEnable = TRUE;
   RenderTarget.SrcBlend = D3D11_BLEND_ZERO;
   RenderTarget.DestBlend = D3D11_BLEND_SRC_COLOR;
   RenderTarget.BlendOp = D3D11_BLEND_OP_ADD;
   RenderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
   RenderTarget.DestBlendAlpha = D3D11_BLEND_ZERO;
   RenderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
   RenderTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

   return RenderTarget;
}

// ブレンドステートを設定する
HRESULT D3D11USER::SetBlendState( D3D11_RENDER_TARGET_BLEND_DESC BlendStateArray[], UINT NumBlendState, BOOL AlphaToCoverageEnable )
{
   HRESULT hr = E_FAIL;

   ID3D11BlendState* pBlendState = NULL;

   if( NumBlendState > 8 )
      goto EXIT;

   D3D11_BLEND_DESC BlendDesc;
   ::ZeroMemory( &BlendDesc, sizeof( BlendDesc ) );
   BlendDesc.AlphaToCoverageEnable = AlphaToCoverageEnable;
   // TRUEの場合、マルチレンダーターゲットで各レンダーターゲットのブレンドステートの設定を個別に設定できる
   // FALSEの場合、0番目のみが使用される
   BlendDesc.IndependentBlendEnable = TRUE;

   for( UINT i=0; i<NumBlendState; i++ )
   {
      ::CopyMemory( &BlendDesc.RenderTarget[i], &BlendStateArray[i], sizeof( D3D11_RENDER_TARGET_BLEND_DESC ) );
   }

   hr = m_D3DDevice->CreateBlendState( &BlendDesc, &pBlendState );
   if( FAILED( hr ) )
      goto EXIT;

   m_D3DDeviceContext->OMSetBlendState( pBlendState, 0, 0xffffffff );

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pBlendState );
   return hr;
}

// サンプラーステートを作成する
HRESULT D3D11USER::CreateSamplerState( ID3D11SamplerState** ppSamplerState, D3D11_FILTER Filter, UINT MaxAnisotropy )
{
   HRESULT hr = E_FAIL;

   D3D11_SAMPLER_DESC samplerDesc;
   samplerDesc.Filter = Filter;                           // サンプリング時に使用するフィルタ
   samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある u テクスチャー座標の描画方法
   samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある v テクスチャー座標
   samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある w テクスチャー座標
   samplerDesc.MipLODBias = 0;                            // 計算されたミップマップ レベルからのバイアス
   samplerDesc.MaxAnisotropy = MaxAnisotropy;             // サンプリングに異方性補間を使用している場合の限界値。有効な値は 1 ～ 16 。
   samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;  // 比較オプション。
   ::CopyMemory( samplerDesc.BorderColor, D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f ), sizeof( D3DXVECTOR4 ) ); // 境界色
   samplerDesc.MinLOD = 0;                                // アクセス可能なミップマップの下限値
   samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // アクセス可能なミップマップの上限値
   hr = m_D3DDevice->CreateSamplerState( &samplerDesc, ppSamplerState );
   if( FAILED( hr ) )
      goto EXIT;

   hr = S_OK;
EXIT:
   return hr;
}

// 深度ステンシルステートの中で設定変更頻度が高そうなパラメータを設定する
HRESULT D3D11USER::SetDepthStencilState( BOOL DepthEnable, D3D11_DEPTH_WRITE_MASK DepthWriteEnabled )
{
   HRESULT hr = E_FAIL;

   // 深度ステンシルステート
   ID3D11DepthStencilState*  pDepthStencilState = NULL;

   // 深度ステンシルステートを作成する
   D3D11_DEPTH_STENCIL_DESC ddsDesc;
   ::ZeroMemory( &ddsDesc, sizeof( ddsDesc ) );
   ddsDesc.DepthEnable = DepthEnable;           // 深度テストを行うか
   ddsDesc.DepthWriteMask = DepthWriteEnabled;  // 深度バッファへの書き込みを行うか
   ddsDesc.DepthFunc = D3D11_COMPARISON_LESS;
   ddsDesc.StencilEnable = FALSE;
   hr = m_D3DDevice->CreateDepthStencilState( &ddsDesc, &pDepthStencilState );
   if( FAILED( hr ) )
      goto EXIT;

   m_D3DDeviceContext->OMSetDepthStencilState( pDepthStencilState, 0 );

   hr = S_OK;
EXIT:
   SAFE_RELEASE( pDepthStencilState );
   return hr;
}

// 深度ステンシルビューのシェーダーリソースビューを取得
ID3D11ShaderResourceView* D3D11USER::GetSRViewFromDepthStencil()
{
   HRESULT hr = E_FAIL;
   ID3D11ShaderResourceView* pSRView = NULL;
   ID3D11Resource* pResource = NULL;

   if( m_DepthStencilView == NULL ) goto EXIT;

   // 深度ステンシルビューの設定を取得する
   D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
   m_DepthStencilView->GetDesc( &DSVDesc );

   D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
   ::ZeroMemory( &SRVDesc, sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ) );
   // リソースにビューを適用する場合、リソース ビューの記述で、リソース フォーマットと互換性のある型のフォーマットを指定する必要がある
   switch( DSVDesc.Format )
   {
   // 8ビットフォーマットは使用できるのか？
   case DXGI_FORMAT_R8_UNORM:
      SRVDesc.Format = DXGI_FORMAT_R8_UNORM;
      break;
   case DXGI_FORMAT_D16_UNORM:
      SRVDesc.Format = DXGI_FORMAT_R16_UNORM;
      break;
   case DXGI_FORMAT_D32_FLOAT:
      SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
      break;
   case DXGI_FORMAT_D24_UNORM_S8_UINT:
      SRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
      break;
   }
   DXGI_SWAP_CHAIN_DESC SCDesc;
   // スワップチェーンの設定を取得する
   m_SwapChain->GetDesc( &SCDesc );
   // マルチサンプリングを使用しない
   if( SCDesc.SampleDesc.Count == 1 )
      SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   // マルチサンプリングを使用する
   else
      SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
   SRVDesc.Texture2D.MostDetailedMip = 0;
   SRVDesc.Texture2D.MipLevels = 1;   

   // 深度ステンシルビューからリソースを取得
   m_DepthStencilView->GetResource( &pResource );
   // シェーダーリソースビューを作成する
   hr = m_D3DDevice->CreateShaderResourceView( pResource, &SRVDesc, &pSRView );
   if( FAILED( hr ) ) goto EXIT;

   hr = S_OK;

EXIT:
   SAFE_RELEASE( pResource );
   return pSRView;
}

// シェーダーにバインド可能なレンダーターゲットビューを作成する
HRESULT D3D11USER::CreateRenderTargetView( ID3D11RenderTargetView** ppRTView, D3DXVECTOR2* pRTViewSize, DXGI_FORMAT Format )
{
   return CreateRenderTargetView( ppRTView, pRTViewSize, Format, 1 );
}

// シェーダーにバインド可能なレンダーターゲットビューを作成する
HRESULT D3D11USER::CreateRenderTargetView( ID3D11RenderTargetView** ppRTView, D3DXVECTOR2* pRTViewSize, DXGI_FORMAT Format, UINT MipLevels )
{
   HRESULT hr = E_FAIL;

   ID3D11Texture2D* pTexture2D = NULL;

   // スワップチェーンの情報を取得する
   DXGI_SWAP_CHAIN_DESC SCDesc;
   m_SwapChain->GetDesc( &SCDesc );

   UINT Width, Height;
   if( pRTViewSize == NULL )
   {
      Width = SCDesc.BufferDesc.Width;
      Height = SCDesc.BufferDesc.Height;
   }
   else
   {
      Width = (UINT)pRTViewSize->x;
      Height = (UINT)pRTViewSize->y;
   }

   D3D11_TEXTURE2D_DESC Tex2DDesc;
   D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;

   ::ZeroMemory( &Tex2DDesc, sizeof( D3D11_TEXTURE2D_DESC ) );
   Tex2DDesc.ArraySize        = 1;
   Tex2DDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
   Tex2DDesc.Usage            = D3D11_USAGE_DEFAULT;
   Tex2DDesc.Format           = Format;
   Tex2DDesc.Width            = Width;
   Tex2DDesc.Height           = Height;
   Tex2DDesc.MipLevels        = MipLevels;
   Tex2DDesc.SampleDesc.Count = SCDesc.SampleDesc.Count;
   Tex2DDesc.SampleDesc.Quality = SCDesc.SampleDesc.Quality;

   ::ZeroMemory( &RTVDesc, sizeof( D3D11_RENDER_TARGET_VIEW_DESC ) );
   RTVDesc.Format = Tex2DDesc.Format;
   if( Tex2DDesc.SampleDesc.Count == 1 )
      RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
   else
      RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
   RTVDesc.Texture2D.MipSlice = 0;

   // ミップマップチェーンの作成を許可する
   if( MipLevels != 1 )
      Tex2DDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

   hr = m_D3DDevice->CreateTexture2D( &Tex2DDesc, NULL, &pTexture2D );
   if( FAILED( hr ) )
      goto EXIT;

   hr = m_D3DDevice->CreateRenderTargetView( pTexture2D, &RTVDesc, ppRTView );
   if( FAILED( hr ) )
      goto EXIT;
   
   SAFE_RELEASE( pTexture2D );

   hr = S_OK;
EXIT:
   return hr;
}

// レンダーターゲットビューからシェーダーリソースビューを取得する
ID3D11ShaderResourceView* D3D11USER::GetSRViewFromRTView( ID3D11RenderTargetView* pRTView )
{
   ID3D11Resource* pResource = NULL;
   ID3D11ShaderResourceView* pSRView = NULL;

   pRTView->GetResource( &pResource );
   m_D3DDevice->CreateShaderResourceView( pResource, NULL, &pSRView );
   SAFE_RELEASE( pResource );

   return pSRView;
}
