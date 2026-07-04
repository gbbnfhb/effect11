#ifndef D3D11USER_H
#define D3D11USER_H

#define D3D11USER_FORMAT        DXGI_FORMAT_R8G8B8A8_UNORM

// 深度ステンシルビューとシェーダーリソースビューのどちらとしても使用できるようにするため型を指定しない
// フォーマットについてはDXGI_FORMAT参照
//#define D3D11USER_DEPTH_FORMAT  DXGI_FORMAT_R8_TYPELESS  // Direct3D11のサンプルでは設定されているが使ってみるとサポートしてないってエラーになる
#define D3D11USER_DEPTH_FORMAT  DXGI_FORMAT_R16_TYPELESS
//#define D3D11USER_DEPTH_FORMAT  DXGI_FORMAT_R32_TYPELESS
//#define D3D11USER_DEPTH_FORMAT  DXGI_FORMAT_R24G8_TYPELESS

#define D3D11USER_F_WINDOWSTYLE WS_POPUP
#define D3D11USER_W_WINDOWSTYLE WS_BORDER | WS_CAPTION | WS_SYSMENU

class D3D11USER
{
public:
   HWND                    m_hWnd;
   D3D_FEATURE_LEVEL       m_FeatureLevel;            // Direct3D 初期化後、実際に採用されたフィーチャーレベル。
   ID3D11Device*           m_D3DDevice;               // Direct3D11 デバイス。
   ID3D11DeviceContext*    m_D3DDeviceContext;        // Direct3D11 デバイスコンテキスト。
   IDXGIAdapter*           m_Adapter;                 // アダプター
   IDXGISwapChain*         m_SwapChain;               // スワップチェーン
   ID3D11RenderTargetView* m_RenderTargetView;        // レンダリングターゲットビュー
   ID3D11DepthStencilView* m_DepthStencilView;
   std::vector<DXGI_MODE_DESC> m_DisplayModeDesc;
   BOOL m_UseDepthBuffer;

private:
   void OutputMsg( const TCHAR* pMsg1, const TCHAR* pMsg2, const TCHAR* pMsg3 );

   // ウィンドウ作成
   HRESULT InitWindow( TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode );

   // アダプターを取得
   HRESULT CreateAdapter();

   // デバイスの作成
   HRESULT CreateDevice( BOOL DirectX11Only );

   // スワップチェーンの作成
   HRESULT CreateSwapChain( DXGI_MODE_DESC* pDisplayMode, BOOL MultisampleEnabled );

   // レンダリングターゲットビューの作成
   HRESULT CreateRenderTargetView();

   // 深度ステンシルビューの作成
   HRESULT CreateDepthStencilView();

   // ビューポートの作成
   HRESULT CreateViewport();

   // ウィンドウアソシエーション
   HRESULT SetWindowAssociation();

   // デフォルトのラスタライザを設定
   HRESULT SetDefaultRasterize();

   // デフォルトの深度ステンシルステートを設定
   HRESULT SetDefaultDepthStencilState();

   // 頂点、インデックス、定数バッファを作成する
   HRESULT CreateBuffer( ID3D11Buffer** pBuffer, void* pData, size_t size, UINT CPUAccessFlag, D3D11_BIND_FLAG BindFlag );

public:
   D3D11USER();
   ~D3D11USER();
   // メモリ開放
   void Invalidate();

   // ディスプレイモード一覧を作成
   HRESULT GetDisplayMode();

   // Direct3Dの初期化
   HRESULT InitD3D11( TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, DXGI_MODE_DESC* pDisplayMode,
                      BOOL Windowed, BOOL DirectX11Only, BOOL MultisampleEnabled, BOOL UseDepthBuffer );

   // 表示モードの切り替えを最適化
   HRESULT ChangeWindowModeOptimization( UINT Width, UINT Height );
   // 表示モードを切り替える
   HRESULT ChangeWindowMode();

   // 頂点バッファを作成する
   HRESULT CreateVertexBuffer( ID3D11Buffer** pBuffer, void* pData, size_t size, UINT CPUAccessFlag );
   // インデックスバッファを作成する
   HRESULT CreateIndexBuffer( ID3D11Buffer** pBuffer, void* pData, size_t size, UINT CPUAccessFlag );

   // 定数バッファを作成する
   HRESULT CreateConstantBuffer( ID3D11Buffer** pBuffer, void* pData, size_t size, UINT CPUAccessFlag );

   // テクスチャーファイルからシェーダーリソースビュー配列を作成する
   HRESULT CreateShaderResourceViewArray( TCHAR* pTextureFileNames[], UINT NumTextures, ID3D11ShaderResourceView** pSRVView );

   // 頂点シェーダーをファイルから作成する
   HRESULT CreateVertexShaderFromFile( OUT ID3D11VertexShader** pVertexShader,
                                       IN TCHAR pSrcFile[], IN CHAR pFunctionName[], CHAR pProfile[],
                                       OUT ID3D11InputLayout** g_pLayout, 
                                       IN D3D11_INPUT_ELEMENT_DESC pInputElementDescs[], UINT NumElements );
   // 頂点シェーダーをメモリから作成する
   HRESULT CreateVertexShaderFromMemory( OUT ID3D11VertexShader** pVertexShader,
                                         IN const BYTE* pShader, size_t size,
                                         OUT ID3D11InputLayout** g_pLayout, 
                                         IN D3D11_INPUT_ELEMENT_DESC pInputElementDescs[], UINT NumElements );

   // ハルシェーダーをファイルから作成する
   HRESULT CreateHullShaderFromFile( ID3D11HullShader** pHullShader, TCHAR pSrcFile[], CHAR pFunctionName[], CHAR pProfile[] );
   // ハルシェーダーをメモリから作成する
   HRESULT CreateHullShaderFromMemory( ID3D11HullShader** pHullShader, const BYTE* pShader, size_t size );

   // ドメインシェーダーをファイルから作成する
   HRESULT CreateDomainShaderFromFile( ID3D11DomainShader** pDomainShader, TCHAR pSrcFile[], CHAR pFunctionName[], CHAR pProfile[] );
   // ドメインシェーダーをメモリから作成する
   HRESULT CreateDomainShaderFromMemory( ID3D11DomainShader** pDomainShader, const BYTE* pShader, size_t size );

   // ジオメトリシェーダーをファイルから作成する
   HRESULT CreateGeometryShaderFromFile( ID3D11GeometryShader** pGeometryShader, TCHAR pSrcFile[], CHAR pFunctionName[], CHAR pProfile[] );
   // ジオメトリシェーダーをメモリから作成する
   HRESULT CreateGeometryShaderFromMemory( ID3D11GeometryShader** pGeometryShader, const BYTE* pShader, size_t size );

   // ストリーミング出力バッファーへの書き込みが可能なジオメトリ シェーダーをファイルから作成する
   HRESULT CreateGeometryShaderWithStreamOutputFromFile(
                                          ID3D11GeometryShader** pGeometryShader,
                                          TCHAR pSrcFile[],   CHAR pFunctionName[], CHAR pProfile[],
                                          const D3D11_SO_DECLARATION_ENTRY *pSODeclaration,
                                          UINT NumEntries,
                                          const UINT *pBufferStrides,
                                          UINT NumStrides );
   // ストリーミング出力バッファーへの書き込みが可能なジオメトリ シェーダーをメモリから作成する
   HRESULT CreateGeometryShaderWithStreamOutputFromMemory(
                                          ID3D11GeometryShader** pGeometryShader,
                                          const BYTE* pShader, size_t size,
                                          const D3D11_SO_DECLARATION_ENTRY *pSODeclaration,
                                          UINT NumEntries,
                                          const UINT *pBufferStrides,
                                          UINT NumStrides );

   // ピクセルシェーダーをファイルから作成する
   HRESULT CreatePixelShaderFromFile( ID3D11PixelShader** pPixelShader, TCHAR pSrcFile[], CHAR pFunctionName[], CHAR pProfile[] );
   // ピクセルシェーダーをメモリから作成する
   HRESULT CreatePixelShaderFromMemory( ID3D11PixelShader** pPixelShader, const BYTE* pShader, size_t size );

   // スクリーンショット作成
   HRESULT CreateScreenShot();

   // ブレンド ステートを無効にするための設定を取得する
   D3D11_RENDER_TARGET_BLEND_DESC GetDefaultBlendDesc();
   // 線形合成用ブレンド ステートのためのを設定を取得する
   D3D11_RENDER_TARGET_BLEND_DESC GetAlignmentBlendDesc();
   // 加算合成用ブレンド ステートのためのを設定を取得する
   D3D11_RENDER_TARGET_BLEND_DESC GetAddBlendDesc();
   // 減算合成用ブレンド ステートのためのを設定を取得する
   D3D11_RENDER_TARGET_BLEND_DESC GetSubtractBlendDesc();
   // 積算合成用ブレンド ステートのためのを設定を取得する
   D3D11_RENDER_TARGET_BLEND_DESC GetMultipleBlendDesc();
   // ブレンドステートを設定する
   HRESULT SetBlendState( D3D11_RENDER_TARGET_BLEND_DESC BlendStateArray[], UINT NumBlendState, BOOL AlphaToCoverageEnable );

   // サンプラーステートを作成する
   HRESULT CreateSamplerState( ID3D11SamplerState** ppSamplerState, D3D11_FILTER Filter, UINT MaxAnisotropy );

   // 深度ステンシルステートの中で設定変更頻度が高そうなパラメータを設定する
   HRESULT SetDepthStencilState( BOOL DepthEnable, D3D11_DEPTH_WRITE_MASK DepthWriteEnabled );

   // 深度ステンシルビューからシェーダーリソースビューを取得
   ID3D11ShaderResourceView* GetSRViewFromDepthStencil();

   // シェーダーにバインド可能なレンダーターゲットビューを作成する
   HRESULT CreateRenderTargetView( ID3D11RenderTargetView** ppRTView, D3DXVECTOR2* pRTViewSize, DXGI_FORMAT Format );

   // シェーダーにバインド可能なレンダーターゲットビューを作成する
   HRESULT CreateRenderTargetView( ID3D11RenderTargetView** ppRTView, D3DXVECTOR2* pRTViewSize, DXGI_FORMAT Format, UINT MipLevels );

   // レンダーターゲットビューからシェーダーリソースビューを取得する
   ID3D11ShaderResourceView* GetSRViewFromRTView( ID3D11RenderTargetView* pRTView );
};

#endif