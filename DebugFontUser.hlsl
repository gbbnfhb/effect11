cbuffer cbMatrixWVP : register( b0 )
{
   // 列優先行列
   column_major  float4x4 g_matWVP : packoffset( c0 );
   // テクセルのオフセット値
   float4 g_Offset : packoffset( c4 );
};

// テクスチャー
Texture2D g_Tex : register( t0 );

// サンプラーステート
SamplerState  g_Sampler : register( s0 );

// 頂点シェーダーの入力パラメータ
struct VS_IN
{
   float3 pos   : POSITION;   // 頂点座標
   float4 color : COLOR;      // 頂点カラー
   float2 texel : TEXCOORD;   // テクセル
};

// 頂点シェーダーの出力パラメータ
struct VS_OUT
{
   float4 pos   : SV_POSITION;
   float4 color : COLOR0;
   float2 texel : TEXCOORD0;
};

// 頂点シェーダー
VS_OUT Font_VS_Main( VS_IN In )
{
   VS_OUT Out;

   Out.pos   = mul( float4( In.pos, 1.0f ), g_matWVP );
   Out.color = In.color;
   Out.texel = In.texel;

   return Out;
}

// ピクセルシェーダ
float4 Font_PS_Main( VS_OUT In ) : SV_TARGET
{
   return g_Tex.Sample( g_Sampler, In.texel + g_Offset.xy ) * In.color;
}