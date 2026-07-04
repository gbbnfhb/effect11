#include "Font_Shader.hlsli"

cbuffer cbMatrixWVP : register( b0 )
{
   // 列優先行列
   column_major  float4x4 g_matWVP : packoffset( c0 );
   // テクセルのオフセット値
   float4 g_Offset : packoffset( c4 );
};

// 頂点シェーダーの入力パラメータ
struct VS_IN
{
   float3 pos   : POSITION;   // 頂点座標
   float4 color : COLOR;      // 頂点カラー
   float2 texel : TEXCOORD;   // テクセル
};

// 頂点シェーダーメイン
VS_OUT Font_VS_Main( VS_IN In )
{
   VS_OUT Out;

   Out.pos   = mul( float4( In.pos, 1.0f ), g_matWVP );
   Out.color = In.color;
   Out.texel = In.texel;

   return Out;
}
