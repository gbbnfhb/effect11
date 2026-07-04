#ifndef DEBUGFONTUSER_H
#define DEBUGFONTUSER_H

class CDebugFont
{
private:
   // 頂点定義
   typedef struct _FONT_VERTEX
   {
      D3DXVECTOR3 pos;     // 頂点の座標
      D3DXCOLOR   color;   // 頂点カラー( 白固定 )
      D3DXVECTOR2 texel;   // テクセル座標
   }FONT_VERTEX;

   // 定数バッファの定義
   typedef struct _CBUFFER
   {
      D3DXMATRIX  matWVP;  // フォントの表示位置、サイズを指定するための行列
      D3DXVECTOR4 Offset;  // テクスチャー上のテクセル位置を指定するオフセット値
   }CBUFFER;

   ID3D11Buffer*             m_pVertexBuffer;
   ID3D11VertexShader*       m_pVertexShader;
   ID3D11InputLayout*        m_pLayout;
   ID3D11PixelShader*        m_pPixelShader;
   ID3D11ShaderResourceView* m_pResourceView;
   ID3D11Buffer*             m_pConstantBuffers;
   ID3D11SamplerState*       m_pSamplerState;
   ID3D11BlendState*         m_pBlendState;
   ID3D11DepthStencilState*  m_pDepthStencilState;

   float m_FontWidth;   // ポリゴンの横幅( テクスチャーのサイズから取得する )
   float m_FontHeight;  // ポリゴンの縦幅( テクスチャーのサイズから取得する )
   float m_FontCnt;     // フォント数

   // FPS計算用変数
   DWORD m_Timer;         // 前回時間
   DWORD m_Frame;         // FPS計測値
   DWORD m_FrameDisplay;  // 表示用のFPS計測値

public:
   CDebugFont();
   virtual ~CDebugFont();
   void Invalidate();

   // 作成処理
   // フォントサイズ( 実際はポリゴンサイズ )の単位は 1 を設定したときスクリーンいっぱいに描画される感じ.
   // フォントサイズは作成時に設定した値を後で変更できない.
   HRESULT Create( ID3D11Device* pD3DDevice, float FontWidth, float FontHeight );
   // デバック用のテキストを描画する
   // 文字列の最後は必ず \n を指定すること.
   // 文字は半角英数字と半角記号のみサポートする.それ以外の文字をパラメータに渡した場合の動作は保証しない.
   // フォントの基準位置はスクリーン上の左上隅.( 2, 2 )で右下隅という微妙な仕様.
   HRESULT RenderDebugText( ID3D11DeviceContext* pD3DDeviceContext, char* pStr, float X, float Y );
   // FPS 出力
   HRESULT RenderFPS(  ID3D11DeviceContext* pD3DDeviceContext, float X, float Y );
};

#endif