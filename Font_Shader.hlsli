// 頂点シェーダーの出力 兼 ピクセルシェーダーの入力パラメータ
struct VS_OUT
{
   float4 pos   : SV_POSITION;
   float4 color : COLOR0;
   float2 texel : TEXCOORD0;
};
