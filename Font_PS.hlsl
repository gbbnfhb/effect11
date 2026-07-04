#include "Font_Shader.hlsli"

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

// ピクセルシェーダメイン
float4 Font_PS_Main( VS_OUT In ) : SV_TARGET
{
   return g_Tex.Sample( g_Sampler, In.texel + g_Offset.xy ) * In.color;
}
