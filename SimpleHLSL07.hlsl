// ワールド行列 × ビュー × 射影行列
cbuffer cbMatrixWVP : register( b0 )
{
   // 列優先
   column_major  float4x4 g_matWVP : packoffset( c0 );
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
VS_OUT VS_Main( VS_IN In )
{
   VS_OUT Out;

   Out.pos = mul( float4( In.pos, 1 ), g_matWVP );

   Out.color = In.color;

   Out.texel = In.texel;

   return Out;
}

// ピクセルシェーダ
float4 PS_Main( VS_OUT In ) : SV_TARGET
{
	float4 final = g_Tex.Sample( g_Sampler, In.texel );
	float gray = dot(final, float3(0.299, 0.587, 0.114)); 
	return float4(gray, gray, gray, gray);
   // サンプラーを使用してサンプリング
   //return g_Tex.Sample( g_Sampler, In.texel );
}

// エントリーポイントとバージョンを指定する
technique11 TShader
{
   pass P0
   {
      SetVertexShader( CompileShader( vs_5_0, VS_Main() ) );
      SetGeometryShader( NULL );
      SetHullShader( NULL );
      SetDomainShader( NULL );
      SetPixelShader(  CompileShader( ps_5_0, PS_Main() ) );
      SetComputeShader( NULL );
   }
}
