// Native GLSL pilot path for the simple + bumpmapped material family.
// See rt_glslpilot.h.
//
// The shader math replicates the Cg sources shipped in the piggs:
//   shaders/cgfx/modulatefp.cg              (modulate fragment)
//   shaders/cgfx/multiplyRegfp.cg           (multiply fragment)
//   shaders/cgfx/colorBlendDualfp.cg        (dual tint fragment)
//   shaders/cgfx/addglowfp.cg               (window-glow fragment)
//   shaders/cgfx/alphaDetailfp.cg           (alpha detail fragment)
//   shaders/cgfx/bumpmapColorblendDualfp.cg (bumped dual tint fragment,
//                                            default + BIT_HIGH_QUALITY)
//   shaders/cgfx/vp_master_vp.cg            (vertex: DUALTEX + bump_dual variants)
//   shaders/cgfx/functions.cgh              (fog, tinting, tangent-space, lighting)
//   shaders/cgfx/effects/*.cg               (post-processing family; fullscreen
//                                            quads under the DRAWMODE_SPRITE
//                                            dualtex vertex variant)
// The GLSL builds on compatibility-profile built-ins (gl_ModelViewMatrix,
// gl_TextureMatrix, gl_LightSource[0], gl_Fog, gl_Color, gl_FogFragCoord)
// which read the same GL server state the Cg `state.*` semantics read, so
// the engine needs no new parameter plumbing beyond the mirrored
// program-local constants (g_ReflectionParamVP, g_Env0/1FP, g_GlowParamFP,
// and the bump lighting constants + tangent attribute for the bump
// materials; the HQ bump fragment reads g_LightDirFP instead of a
// vertex-interpolated light vector; the effects materials read the
// per-program g_Effects_* locals).

#include "render/thread/ogl.h"
#include "render/thread/rt_glslpilot.h"
#include "render/thread/rt_shaderMgr.h"
#include "render/thread/rt_effects.h"
#include "cmdparse/cmdgame.h"
#include <stdio.h>
#include <string.h>

// ogl.h #undef's the GLEW macro names for GL 2.0 shader entry points as a
// "do not use" policy for the fixed pipelines; the underlying GLEW function
// pointers (GLEW_GET_FUN targets, gl-prefix stripped) remain the supported
// way to reach them.

static const char s_pilotVertexSource[] =
"#version 120\n"
"\n"
"uniform vec4 g_ReflectionParamVP;   // engine constant, see header\n"
"uniform int  g_VertexLitMode;       // variants.cgh: VERT_COLOR=1, FF_LIT_GL=4, FF_UNLIT_GL=5\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position_vs = ( gl_ModelViewMatrix * gl_Vertex ).xyz;\n"
"    gl_Position = gl_ProjectionMatrix * vec4( position_vs, 1.0 );\n"
"\n"
"    // fog coordinate: eye-radial distance (vp_master_vp.cg)\n"
"    gl_FogFragCoord = sqrt( dot( position_vs, position_vs ) + 1e-16 );\n"
"\n"
"    // texture coordinates through the GL texture matrices (TC_XFORM == TC_MATRIX)\n"
"    vec2 uv0 = ( gl_TextureMatrix[0] * gl_MultiTexCoord0 ).xy;\n"
"    vec2 uv1 = ( gl_TextureMatrix[1] * gl_MultiTexCoord1 ).xy;\n"
"\n"
"    // legacy faux spheremap reflection uv and per-material selector\n"
"    // (calc_faux_reflection_uv in functions.cgh)\n"
"    vec3 normal = normalize( gl_NormalMatrix * gl_Normal );\n"
"    vec3 Rr = reflect( position_vs, normal );\n"
"    vec3 Ro = normalize( Rr + vec3( 0.0, 0.0, 1.0 ) );\n"
"    vec2 uvFaux = ( 0.5 * Ro.xy ) + 0.5;\n"
"    uv0 = ( g_ReflectionParamVP.z * uv0 ) + ( uvFaux * g_ReflectionParamVP.x );\n"
"    uv1 = ( g_ReflectionParamVP.w * uv1 ) + ( uvFaux * g_ReflectionParamVP.y );\n"
"    gl_TexCoord[0] = vec4( uv0, uv1 );\n"
"\n"
"    if ( g_VertexLitMode == 4 ) // FF_LIT_GL: OpenGL fixed function lighting\n"
"    {\n"
"        vec3 light = gl_LightSource[0].position.xyz;\n"
"        vec3 view = normalize( -position_vs );\n"
"        vec3 half_vs = normalize( light + view );\n"
"        float n_dot_l = dot( normal, light );\n"
"        float n_dot_h = dot( normal, half_vs );\n"
"        float specular = pow( clamp( n_dot_h, 0.0, 1.0 ), gl_FrontMaterial.shininess );\n"
"        vec3 diffuse = clamp( n_dot_l * gl_LightSource[0].diffuse.rgb, 0.0, 1.0 );\n"
"        vec3 rgb = ( diffuse + gl_LightSource[0].ambient.rgb ) * gl_Color.rgb;\n"
"        rgb += specular * gl_LightSource[0].specular.rgb;\n"
"        gl_FrontColor = vec4( rgb, gl_Color.a );\n"
"    }\n"
"    else // VERT_COLOR / FF_UNLIT_GL: pass the vertex color through\n"
"    {\n"
"        gl_FrontColor = gl_Color;\n"
"    }\n"
"}\n";

// Fixed-function vertex path for the pbuffer effects passes (rt_effects.c):
// those draws run with vertex programs disabled — positions through the
// fixed-function modelview/projection, texture coordinates through the GL
// texture matrices, exactly what a vertex-program-less pipeline does.
static const char s_ffVertexSource[] =
"#version 120\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = ftransform();\n"
"    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
"    gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;\n"
"    gl_FrontColor = gl_Color;\n"
"}\n";

static const char s_modulateFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base;   // TEXUNIT0\n"
"uniform sampler2D sampler_blend;  // TEXUNIT1\n"
"\n"
"void main()\n"
"{\n"
"    vec4 out_color = gl_Color\n"
"        * texture2D( sampler_base, gl_TexCoord[0].xy )\n"
"        * texture2D( sampler_blend, gl_TexCoord[0].zw );\n"
"\n"
"    // calc_fogged_color reads state.fog.params / state.fog.color, which\n"
"    // map to the same GL fog state exposed by the gl_Fog built-in\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// multiplyRegfp.cg, default variant (no BIT_HIGH_QUALITY / cubemap / shadow):
// straight modulation of base by blend texture (alpha included), then
// out_color.rgb *= 8*IN.color.rgb and out_color.a *= g_Env0FP.a.
static const char s_multiplyFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base;   // TEXUNIT0\n"
"uniform sampler2D sampler_blend;  // TEXUNIT1\n"
"uniform vec4 g_Env0FP;            // engine constColor0 (TIE(ENV8) -> fragment program.env[8])\n"
"\n"
"void main()\n"
"{\n"
"    vec4 out_color = texture2D( sampler_base, gl_TexCoord[0].xy )\n"
"        * texture2D( sampler_blend, gl_TexCoord[0].zw );\n"
"\n"
"    // modulate by vertex color and scale (x8 to match the multiplyReg\n"
"    // register combiner program and old assets)\n"
"    out_color.rgb *= 8.0 * gl_Color.rgb;\n"
"    // modulate by lod alpha\n"
"    out_color.a *= g_Env0FP.a;\n"
"\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// colorBlendDualfp.cg, default variant (no BIT_HIGH_QUALITY / shadow):
// CoV-style dual color tinting (calc_dual_tint in functions.cgh) driven by
// the two engine tint constants, then out_color.rgb *= 4*IN.color.rgb.
static const char s_colorBlendDualFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base;   // TEXUNIT0\n"
"uniform sampler2D sampler_dual;   // TEXUNIT1\n"
"uniform vec4 g_Env0FP;            // engine constColor0 (TIE(ENV8) -> fragment program.env[8])\n"
"uniform vec4 g_Env1FP;            // engine constColor1 (TIE(ENV9) -> fragment program.env[9])\n"
"\n"
"void main()\n"
"{\n"
"    vec4 tex_base = texture2D( sampler_base, gl_TexCoord[0].xy );\n"
"    vec4 tex_dual = texture2D( sampler_dual, gl_TexCoord[0].zw );\n"
"\n"
"    // calc_dual_tint: lerp between the two tint colors by the dual\n"
"    // texture, mask back toward white by base alpha, modulate by base\n"
"    vec4 out_color;\n"
"    out_color.rgb = mix( g_Env1FP.rgb, g_Env0FP.rgb, tex_dual.rgb );\n"
"    out_color.rgb = mix( out_color.rgb, vec3( 1.0 ), tex_base.a );\n"
"    out_color.rgb *= tex_base.rgb;\n"
"    out_color.a = tex_dual.a * g_Env0FP.a;\n"
"\n"
"    // modulate with vertex color * 4 (matches old assets and reg\n"
"    // combiner programs)\n"
"    out_color.rgb *= 4.0 * gl_Color.rgb;\n"
"\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// addglowfp.cg, default variant (no BIT_HIGH_QUALITY / shadow): old-style
// tinting of the base texture (calc_old_tint with g_Env0FP), then the
// random window-glow add (has_glow/add_glow_orig with g_GlowParamFP).
static const char s_addGlowFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base;       // TEXUNIT0\n"
"uniform sampler2D sampler_glow;       // TEXUNIT1\n"
"uniform sampler2D sampler_glow_mask;  // TEXUNIT2 (e.g. buildingLightPattern, 128x128)\n"
"uniform vec4 g_Env0FP;                // engine constColor0 (TIE(ENV8) -> fragment program.env[8])\n"
"uniform vec4 g_GlowParamFP;           // .x glow threshold, .y per-model random seed\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv0 = gl_TexCoord[0].xy;\n"
"    vec2 uv1 = gl_TexCoord[0].zw;\n"
"\n"
"    // calc_old_tint: lerp from the tint color by base alpha; alpha from tint\n"
"    vec4 tex_base = texture2D( sampler_base, uv0 );\n"
"    vec4 out_color;\n"
"    out_color.rgb = mix( g_Env0FP.rgb, tex_base.rgb, tex_base.a );\n"
"    out_color.a = g_Env0FP.a;\n"
"\n"
"    // modulate with vertex color * 4 (matches old assets and reg\n"
"    // combiner programs)\n"
"    out_color.rgb *= 4.0 * gl_Color.rgb;\n"
"\n"
"    // has_glow: each 1x1 tile of the base texture shares one glow-mask\n"
"    // texel (0.0078125 == 1/128); glow fires when mask < threshold\n"
"    vec2 maskUv = ( floor( uv0 ) * 0.0078125 ) + g_GlowParamFP.yw;\n"
"    float mask = texture2D( sampler_glow_mask, maskUv ).r;\n"
"    if ( mask < g_GlowParamFP.x )\n"
"    {\n"
"        out_color.rgb += texture2D( sampler_glow, uv1 ).rgb;\n"
"    }\n"
"\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// alphaDetailfp.cg, default variant (no BIT_HIGH_QUALITY / shadow): base
// alpha blends between the blend and base rgb; blend alpha modulates the
// constant (g_Env0FP) alpha.
static const char s_alphaDetailFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base;   // TEXUNIT0\n"
"uniform sampler2D sampler_blend;  // TEXUNIT1\n"
"uniform vec4 g_Env0FP;            // engine constColor0 (TIE(ENV8) -> fragment program.env[8])\n"
"\n"
"void main()\n"
"{\n"
"    vec4 tex_base = texture2D( sampler_base, gl_TexCoord[0].xy );\n"
"    vec4 tex_blend = texture2D( sampler_blend, gl_TexCoord[0].zw );\n"
"\n"
"    // texture 0 (base) alpha blends between texture 0 and 1 rgb;\n"
"    // texture 1 (blend) alpha modulates the current constant alpha\n"
"    vec4 out_color;\n"
"    out_color.rgb = mix( tex_blend.rgb, tex_base.rgb, tex_base.a );\n"
"    out_color.rgb *= 4.0 * gl_Color.rgb;\n"
"    out_color.a = g_Env0FP.a * tex_blend.a;\n"
"\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// vp_master_vp.cg "bump_dual" (SKIN=0) and "skin_bump" (SKIN=1) variants,
// both LIGHT_SPACE=VIEW VERTEX_LIT=NONE TC_XFORM=NONE PIXEL_LIT=BUMP_ALL
// REFLECT=NONE, non-HQ: transforms the light direction and view position
// into tangent space per vertex (calc_tangent_space_light_and_position in
// functions.cgh) using the tangent vertex attribute (generic attribute 7,
// binormal sign in w). The skinned branch replicates the two-bone blending
// (blend_bone_positions/blend_bone_normals) with the bone matrix array the
// engine pushes via loadBoneMatrices; bone weights arrive on generic
// attribute 1 and bone indices (premultiplied by 3) on attribute 5.
static const char s_bumpDualVertexSource[] =
"#version 120\n"
"\n"
"attribute vec4 attr_tangent;      // generic vertex attribute 7 (see rt_model.c/rt_bonedmodel.c)\n"
"attribute vec4 attr_boneweights;  // generic vertex attribute 1 (skinned draws only)\n"
"attribute vec4 attr_boneindices;  // generic vertex attribute 5 (skinned draws only; x/y = bone*3)\n"
"uniform vec4 g_LightDirVP;        // engine constant, view-space light direction\n"
"uniform vec4 g_BoneMatrixArrVP[48]; // engine bone matrices, 3 vec4 rows per bone\n"
"uniform int  g_Skinned;           // 1 = skin through the bone matrices (skin_bump variant)\n"
"\n"
"varying vec3 vLightTs;            // OUT.light_ts (TEXCOORD2 in the Cg program)\n"
"varying vec3 vViewTs;             // OUT.view_ts (TEXCOORD1); = -position in tangent space\n"
"\n"
"// Cg mul(boneMatrix, v): the three array vec4s are the matrix rows\n"
"vec3 boneXform( int bone, vec3 v, float w )\n"
"{\n"
"    return vec3( dot( g_BoneMatrixArrVP[bone + 0], vec4( v, w ) ),\n"
"                 dot( g_BoneMatrixArrVP[bone + 1], vec4( v, w ) ),\n"
"                 dot( g_BoneMatrixArrVP[bone + 2], vec4( v, w ) ) );\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position_vs;\n"
"    vec3 normal;\n"
"    vec3 tangent;\n"
"\n"
"    if ( g_Skinned != 0 )\n"
"    {\n"
"        // two-bone skinning directly to view space; the weight comes in on\n"
"        // boneWeights.y and the indices are premultiplied by 3 (ARB layout)\n"
"        int bone0 = int( attr_boneindices.x );\n"
"        int bone1 = int( attr_boneindices.y );\n"
"        float weight = attr_boneweights.y;\n"
"\n"
"        position_vs = mix( boneXform( bone0, gl_Vertex.xyz, 1.0 ),\n"
"                           boneXform( bone1, gl_Vertex.xyz, 1.0 ), weight );\n"
"        normal = normalize( mix( boneXform( bone0, gl_Normal, 0.0 ),\n"
"                                 boneXform( bone1, gl_Normal, 0.0 ), weight ) );\n"
"        tangent = normalize( mix( boneXform( bone0, attr_tangent.xyz, 0.0 ),\n"
"                                  boneXform( bone1, attr_tangent.xyz, 0.0 ), weight ) );\n"
"    }\n"
"    else\n"
"    {\n"
"        position_vs = ( gl_ModelViewMatrix * gl_Vertex ).xyz;\n"
"        // Cg uses the modelview rows, not the normal matrix; identical for\n"
"        // the rigid transforms these draw with\n"
"        normal = normalize( mat3( gl_ModelViewMatrix ) * gl_Normal );\n"
"        tangent = normalize( mat3( gl_ModelViewMatrix ) * attr_tangent.xyz );\n"
"    }\n"
"\n"
"    gl_Position = gl_ProjectionMatrix * vec4( position_vs, 1.0 );\n"
"\n"
"    // fog coordinate: eye-radial distance (vp_master_vp.cg)\n"
"    gl_FogFragCoord = sqrt( dot( position_vs, position_vs ) + 1e-16 );\n"
"\n"
"    // TC_XFORM == NONE: texture coordinates pass through untransformed\n"
"    gl_TexCoord[0] = vec4( gl_MultiTexCoord0.xy, gl_MultiTexCoord1.xy );\n"
"\n"
"    // calc_tangent_space_light_and_position: change of basis into tangent\n"
"    // space via (tangent, bitangent, normal) rows; Cg mul(M_ts, v) computes\n"
"    // dot(row, v), which is the transpose of GLSL's column-major mat3\n"
"    vec3 binormal = cross( tangent, normal ) * sign( attr_tangent.w );\n"
"    vec3 light_vs = g_LightDirVP.xyz;\n"
"    vLightTs = vec3( dot( light_vs, tangent ), dot( light_vs, binormal ), dot( light_vs, normal ) );\n"
"    vec3 position_ts = vec3( dot( position_vs, tangent ), dot( position_vs, binormal ), dot( position_vs, normal ) );\n"
"    vViewTs = -position_ts;\n"
"}\n";

// vp_master_vp.cg "bump_dual"/"skin_bump" variants compiled with
// BIT_HIGH_QUALITY: instead of the tangent-space light/view vectors, the HQ
// vertex program passes the view-space tangent (binormal sign in w), normal,
// and position through as interpolants — the HQ fragment shader builds the
// tangent basis and lighting vectors per pixel. The skinned branch is the
// same two-bone blend as the low-quality shader; g_LightDirVP is dead code
// in the HQ Cg variants (the fragment reads g_LightDirFP instead), so it is
// not declared here.
static const char s_bumpDualHQVertexSource[] =
"#version 120\n"
"\n"
"attribute vec4 attr_tangent;      // generic vertex attribute 7 (see rt_model.c/rt_bonedmodel.c)\n"
"attribute vec4 attr_boneweights;  // generic vertex attribute 1 (skinned draws only)\n"
"attribute vec4 attr_boneindices;  // generic vertex attribute 5 (skinned draws only; x/y = bone*3)\n"
"uniform vec4 g_BoneMatrixArrVP[48]; // engine bone matrices, 3 vec4 rows per bone\n"
"uniform int  g_Skinned;           // 1 = skin through the bone matrices (skin_bump variant)\n"
"\n"
"varying vec3 vNormalVs;           // OUT.normal_vs (TEXCOORD semantics in the Cg program)\n"
"varying vec4 vTangentVs;          // OUT.tangent_vs; .w = binormal sign, already sign()'d\n"
"varying vec3 vPositionVs;         // OUT.position_vs\n"
"\n"
"// Cg mul(boneMatrix, v): the three array vec4s are the matrix rows\n"
"vec3 boneXform( int bone, vec3 v, float w )\n"
"{\n"
"    return vec3( dot( g_BoneMatrixArrVP[bone + 0], vec4( v, w ) ),\n"
"                 dot( g_BoneMatrixArrVP[bone + 1], vec4( v, w ) ),\n"
"                 dot( g_BoneMatrixArrVP[bone + 2], vec4( v, w ) ) );\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position_vs;\n"
"    vec3 normal;\n"
"    vec3 tangent;\n"
"\n"
"    if ( g_Skinned != 0 )\n"
"    {\n"
"        // two-bone skinning directly to view space; the weight comes in on\n"
"        // boneWeights.y and the indices are premultiplied by 3 (ARB layout)\n"
"        int bone0 = int( attr_boneindices.x );\n"
"        int bone1 = int( attr_boneindices.y );\n"
"        float weight = attr_boneweights.y;\n"
"\n"
"        position_vs = mix( boneXform( bone0, gl_Vertex.xyz, 1.0 ),\n"
"                           boneXform( bone1, gl_Vertex.xyz, 1.0 ), weight );\n"
"        normal = normalize( mix( boneXform( bone0, gl_Normal, 0.0 ),\n"
"                                 boneXform( bone1, gl_Normal, 0.0 ), weight ) );\n"
"        tangent = normalize( mix( boneXform( bone0, attr_tangent.xyz, 0.0 ),\n"
"                                  boneXform( bone1, attr_tangent.xyz, 0.0 ), weight ) );\n"
"    }\n"
"    else\n"
"    {\n"
"        position_vs = ( gl_ModelViewMatrix * gl_Vertex ).xyz;\n"
"        // Cg uses the modelview rows, not the normal matrix; identical for\n"
"        // the rigid transforms these draw with\n"
"        normal = normalize( mat3( gl_ModelViewMatrix ) * gl_Normal );\n"
"        tangent = normalize( mat3( gl_ModelViewMatrix ) * attr_tangent.xyz );\n"
"    }\n"
"\n"
"    gl_Position = gl_ProjectionMatrix * vec4( position_vs, 1.0 );\n"
"\n"
"    // fog coordinate: eye-radial distance (vp_master_vp.cg)\n"
"    gl_FogFragCoord = sqrt( dot( position_vs, position_vs ) + 1e-16 );\n"
"\n"
"    // TC_XFORM == NONE: texture coordinates pass through untransformed\n"
"    gl_TexCoord[0] = vec4( gl_MultiTexCoord0.xy, gl_MultiTexCoord1.xy );\n"
"\n"
"    // HQ interpolants (the fragment shader renormalizes the basis)\n"
"    vNormalVs = normal;\n"
"    vTangentVs = vec4( tangent, sign( attr_tangent.w ) );\n"
"    vPositionVs = position_vs;\n"
"}\n";

// bumpmapColorblendDualfp.cg, default variant (no BIT_HIGH_QUALITY /
// cubemap / shadow): the dual-tint material color from colorBlendDual, lit
// per pixel with the tangent-space vectors from the bump_dual vertex
// variant (renormalized), a normal-map perturbed normal with gloss in its
// alpha, and the engine's bump lighting constants.
static const char s_bumpColorBlendDualFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base_1;              // TEXUNIT0\n"
"uniform sampler2D sampler_dual_1;              // TEXUNIT1\n"
"uniform sampler2D sampler_normal_and_gloss_1;  // TEXUNIT2\n"
"uniform vec4 g_Env0FP;                         // engine constColor0 (TIE(ENV8))\n"
"uniform vec4 g_Env1FP;                         // engine constColor1 (TIE(ENV9))\n"
"uniform vec4 g_AmbientColorFP;                 // TIE(ENV0), setupBumpPixelShader ambient*2\n"
"uniform vec4 g_DiffuseColorFP;                 // TIE(ENV1), setupBumpPixelShader diffuse*4\n"
"uniform vec4 g_GlossParamFP;                   // TIE(ENV2), .w = glossConst\n"
"uniform vec4 g_Specular1ColorAndExponentFP;    // TIE(ENV5), rgb spec color, a exponent\n"
"uniform vec4 g_ModernMaterialParamsFP;         // .x = opt-in modern response\n"
"\n"
"varying vec3 vLightTs;\n"
"varying vec3 vViewTs;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv0 = gl_TexCoord[0].xy;\n"
"    vec2 uv1 = gl_TexCoord[0].zw;\n"
"\n"
"    // renormalize the interpolated lighting vectors (low-quality path);\n"
"    // half vector = view + light, both in tangent space\n"
"    vec3 light_ts = normalize( vLightTs );\n"
"    vec3 view_ts = normalize( vViewTs );\n"
"    vec3 half_ts = normalize( normalize( vViewTs ) + light_ts );\n"
"\n"
"    // map_color_to_normal: expand [0,1] to [-1,1] and renormalize; the\n"
"    // alpha channel piggybacks the gloss map\n"
"    vec4 normal_gloss = texture2D( sampler_normal_and_gloss_1, uv1 );\n"
"    normal_gloss.xyz = normalize( normal_gloss.xyz * 2.0 - 1.0 );\n"
"\n"
"    // base material color: calc_dual_tint with the engine tint constants\n"
"    vec4 tex_base = texture2D( sampler_base_1, uv0 );\n"
"    vec4 tex_dual = texture2D( sampler_dual_1, uv1 );\n"
"    vec4 out_color;\n"
"    out_color.rgb = mix( g_Env1FP.rgb, g_Env0FP.rgb, tex_dual.rgb );\n"
"    out_color.rgb = mix( out_color.rgb, vec3( 1.0 ), tex_base.a );\n"
"    out_color.rgb *= tex_base.rgb;\n"
"    out_color.a = tex_dual.a * g_Env0FP.a;\n"
"\n"
"    // calc_lighting_factors (shade_factor = 1 without shadowmaps):\n"
"    // (1, saturate(NdotL), specular) with gloss-masked specular\n"
"    float n_dot_l = dot( normal_gloss.xyz, light_ts );\n"
"    float n_dot_h = dot( normal_gloss.xyz, half_ts );\n"
"    float specular = pow( clamp( n_dot_h, 0.0, 1.0 ), g_Specular1ColorAndExponentFP.a );\n"
"    specular *= normal_gloss.w;\n"
"\n"
"    // apply_lighting with get_default_light_properties\n"
"    vec3 ambient = g_AmbientColorFP.rgb;\n"
"    vec3 diffuse = clamp( n_dot_l, 0.0, 1.0 ) * g_DiffuseColorFP.rgb;\n"
"    vec3 gloss;\n"
"    if ( g_ModernMaterialParamsFP.x > 0.5 )\n"
"    {\n"
"        // Normalized Blinn-Phong with Schlick Fresnel. The legacy exponent\n"
"        // remains the lobe-width authoring clue; no new material texture is\n"
"        // introduced, and the gloss mask/constant remain the strength.\n"
"        float legacyExponent = clamp( g_Specular1ColorAndExponentFP.a, 1.0, 128.0 );\n"
"        float roughness = clamp( sqrt( 2.0 / ( legacyExponent + 2.0 ) ), 0.18, 0.95 );\n"
"        float lobeExponent = clamp( ( 2.0 / ( roughness * roughness ) ) - 2.0, 1.0, 128.0 );\n"
"        float glossStrength = clamp( normal_gloss.w * max( g_GlossParamFP.w, 0.0 ), 0.0, 1.0 );\n"
"        vec3 authoredSpecular = clamp( g_Specular1ColorAndExponentFP.rgb, 0.0, 1.0 );\n"
"        float n_dot_v = clamp( dot( normal_gloss.xyz, view_ts ), 0.0, 1.0 );\n"
"        float n_dot_h_modern = clamp( dot( normal_gloss.xyz, normalize( view_ts + light_ts ) ), 0.0, 1.0 );\n"
"        float fresnelFactor = pow( 1.0 - n_dot_v, 5.0 );\n"
"        vec3 f0 = clamp( authoredSpecular * ( 0.04 + 0.16 * glossStrength ), 0.0, 0.75 );\n"
"        vec3 fresnel = mix( f0, vec3( 1.0 ), fresnelFactor );\n"
"        float normalizedLobe = pow( n_dot_h_modern, lobeExponent ) * ( ( lobeExponent + 2.0 ) * 0.15915494 );\n"
"        float specularEnergy = clamp( normalizedLobe * 0.25 * glossStrength * clamp( n_dot_l, 0.0, 1.0 ), 0.0, 1.0 );\n"
"        gloss = fresnel * specularEnergy;\n"
"        float maxFresnel = max( fresnel.r, max( fresnel.g, fresnel.b ) );\n"
"        diffuse *= clamp( 1.0 - maxFresnel * glossStrength * 0.5, 0.65, 1.0 );\n"
"        out_color.rgb = out_color.rgb * ( ambient + diffuse ) + gloss;\n"
"    }\n"
"    else\n"
"    {\n"
"        gloss = clamp( specular * g_GlossParamFP.w * g_Specular1ColorAndExponentFP.rgb, 0.0, 1.0 );\n"
"        out_color.rgb = out_color.rgb * ( ambient + diffuse ) + gloss;\n"
"    }\n"
"\n"
"    // calc_fogged_color (same GL fog state as the other materials)\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// bumpmapColorblendDualfp.cg, BIT_HIGH_QUALITY variant (no cubemap /
// shadow): same material color and lighting model as the default variant,
// but the tangent-space basis and lighting vectors are computed per pixel
// from the HQ vertex interpolants (populate_lighting_vectors_hq in
// functions.cgh) and the light direction comes from the g_LightDirFP
// fragment constant instead of a vertex-interpolated vector. Note the Cg
// source deliberately does NOT renormalize half_ts after the basis change
// (the original ARB programs don't either; renormalizing changes the
// specular response).
static const char s_bumpColorBlendDualHQFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base_1;              // TEXUNIT0\n"
"uniform sampler2D sampler_dual_1;              // TEXUNIT1\n"
"uniform sampler2D sampler_normal_and_gloss_1;  // TEXUNIT2\n"
"uniform vec4 g_Env0FP;                         // engine constColor0 (TIE(ENV8))\n"
"uniform vec4 g_Env1FP;                         // engine constColor1 (TIE(ENV9))\n"
"uniform vec4 g_LightDirFP;                     // TIE(ENV11), view-space light dir (HQ only)\n"
"uniform vec4 g_AmbientColorFP;                 // TIE(ENV0), setupBumpPixelShader ambient*2\n"
"uniform vec4 g_DiffuseColorFP;                 // TIE(ENV1), setupBumpPixelShader diffuse*4\n"
"uniform vec4 g_GlossParamFP;                   // TIE(ENV2), .w = glossConst\n"
"uniform vec4 g_Specular1ColorAndExponentFP;    // TIE(ENV5), rgb spec color, a exponent\n"
"uniform vec4 g_ModernMaterialParamsFP;         // .x = opt-in modern response\n"
"\n"
"varying vec3 vNormalVs;\n"
"varying vec4 vTangentVs;\n"
"varying vec3 vPositionVs;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv0 = gl_TexCoord[0].xy;\n"
"    vec2 uv1 = gl_TexCoord[0].zw;\n"
"\n"
"    // populate_lighting_vectors_hq: renormalize the interpolated basis,\n"
"    // rebuild the binormal from the sign in tangent.w\n"
"    vec3 normal_vs = normalize( vNormalVs );\n"
"    vec3 tangent_vs = normalize( vTangentVs.xyz );\n"
"    vec3 binormal_vs = cross( tangent_vs, normal_vs ) * vTangentVs.w;\n"
"    vec3 light_vs = g_LightDirFP.xyz;\n"
"\n"
"    vec3 u_to_eye = normalize( -vPositionVs );\n"
"    // toBasis: dot(row, v) per basis row (the transpose of GLSL's\n"
"    // column-major mat3); light_ts IS renormalized...\n"
"    vec3 light_ts = normalize( vec3( dot( light_vs, tangent_vs ),\n"
"                                     dot( light_vs, binormal_vs ),\n"
"                                     dot( light_vs, normal_vs ) ) );\n"
"    vec3 h_vs = normalize( u_to_eye + light_vs );\n"
"    // ...but half_ts deliberately is not (see comment above)\n"
"    vec3 half_ts = vec3( dot( h_vs, tangent_vs ),\n"
"                         dot( h_vs, binormal_vs ),\n"
"                         dot( h_vs, normal_vs ) );\n"
"    vec3 view_ts = normalize( vec3( dot( u_to_eye, tangent_vs ),\n"
"                                    dot( u_to_eye, binormal_vs ),\n"
"                                    dot( u_to_eye, normal_vs ) ) );\n"
"\n"
"    // map_color_to_normal: expand [0,1] to [-1,1] and renormalize; the\n"
"    // alpha channel piggybacks the gloss map\n"
"    vec4 normal_gloss = texture2D( sampler_normal_and_gloss_1, uv1 );\n"
"    normal_gloss.xyz = normalize( normal_gloss.xyz * 2.0 - 1.0 );\n"
"\n"
"    // base material color: calc_dual_tint with the engine tint constants\n"
"    vec4 tex_base = texture2D( sampler_base_1, uv0 );\n"
"    vec4 tex_dual = texture2D( sampler_dual_1, uv1 );\n"
"    vec4 out_color;\n"
"    out_color.rgb = mix( g_Env1FP.rgb, g_Env0FP.rgb, tex_dual.rgb );\n"
"    out_color.rgb = mix( out_color.rgb, vec3( 1.0 ), tex_base.a );\n"
"    out_color.rgb *= tex_base.rgb;\n"
"    out_color.a = tex_dual.a * g_Env0FP.a;\n"
"\n"
"    // calc_lighting_factors (shade_factor = 1 without shadowmaps):\n"
"    // (1, saturate(NdotL), specular) with gloss-masked specular\n"
"    float n_dot_l = dot( normal_gloss.xyz, light_ts );\n"
"    float n_dot_h = dot( normal_gloss.xyz, half_ts );\n"
"    float specular = pow( clamp( n_dot_h, 0.0, 1.0 ), g_Specular1ColorAndExponentFP.a );\n"
"    specular *= normal_gloss.w;\n"
"\n"
"    // apply_lighting with get_default_light_properties\n"
"    vec3 ambient = g_AmbientColorFP.rgb;\n"
"    vec3 diffuse = clamp( n_dot_l, 0.0, 1.0 ) * g_DiffuseColorFP.rgb;\n"
"    vec3 gloss;\n"
"    if ( g_ModernMaterialParamsFP.x > 0.5 )\n"
"    {\n"
"        // Match the LQ material model while reconstructing V/H from the\n"
"        // HQ view-space position and tangent basis.\n"
"        float legacyExponent = clamp( g_Specular1ColorAndExponentFP.a, 1.0, 128.0 );\n"
"        float roughness = clamp( sqrt( 2.0 / ( legacyExponent + 2.0 ) ), 0.18, 0.95 );\n"
"        float lobeExponent = clamp( ( 2.0 / ( roughness * roughness ) ) - 2.0, 1.0, 128.0 );\n"
"        float glossStrength = clamp( normal_gloss.w * max( g_GlossParamFP.w, 0.0 ), 0.0, 1.0 );\n"
"        vec3 authoredSpecular = clamp( g_Specular1ColorAndExponentFP.rgb, 0.0, 1.0 );\n"
"        float n_dot_v = clamp( dot( normal_gloss.xyz, view_ts ), 0.0, 1.0 );\n"
"        float n_dot_h_modern = clamp( dot( normal_gloss.xyz, normalize( view_ts + light_ts ) ), 0.0, 1.0 );\n"
"        float fresnelFactor = pow( 1.0 - n_dot_v, 5.0 );\n"
"        vec3 f0 = clamp( authoredSpecular * ( 0.04 + 0.16 * glossStrength ), 0.0, 0.75 );\n"
"        vec3 fresnel = mix( f0, vec3( 1.0 ), fresnelFactor );\n"
"        float normalizedLobe = pow( n_dot_h_modern, lobeExponent ) * ( ( lobeExponent + 2.0 ) * 0.15915494 );\n"
"        float specularEnergy = clamp( normalizedLobe * 0.25 * glossStrength * clamp( n_dot_l, 0.0, 1.0 ), 0.0, 1.0 );\n"
"        gloss = fresnel * specularEnergy;\n"
"        float maxFresnel = max( fresnel.r, max( fresnel.g, fresnel.b ) );\n"
"        diffuse *= clamp( 1.0 - maxFresnel * glossStrength * 0.5, 0.65, 1.0 );\n"
"        out_color.rgb = out_color.rgb * ( ambient + diffuse ) + gloss;\n"
"    }\n"
"    else\n"
"    {\n"
"        gloss = clamp( specular * g_GlossParamFP.w * g_Specular1ColorAndExponentFP.rgb, 0.0, 1.0 );\n"
"        out_color.rgb = out_color.rgb * ( ambient + diffuse ) + gloss;\n"
"    }\n"
"\n"
"    // calc_fogged_color (same GL fog state as the other materials)\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// bumpmapMultiplyfp.cg, default variant (no BIT_HIGH_QUALITY / shadow): the
// model-space bump material. Vertex lighting (diffuse+ambient baked per
// vertex, or prelit instance lighting x4) modulates a straight base*blend
// texture multiply, scaled x8 (legacy register-combiner assets), with per
// pixel bumped SPECULAR lighting only (the historical behavior of this blend
// mode); alpha keeps base.a*blend.a and is modulated by the lod alpha
// (g_Env0FP.a). Verified against the cgc-compiled ARB: the specular dot is
// saturated BEFORE the pow, and the ambient/diffuse fragment constants are
// dead in this variant (only env5 specular1 and env8 are read).
static const char s_bumpMultiplyFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base;              // TEXUNIT0\n"
"uniform sampler2D sampler_blend;             // TEXUNIT1\n"
"uniform sampler2D sampler_normal_gloss;      // TEXUNIT2\n"
"uniform vec4 g_Env0FP;                       // engine constColor0 (TIE(ENV8)); .a = lod alpha\n"
"uniform vec4 g_Specular1ColorAndExponentFP;  // TIE(ENV5), rgb spec color, a exponent\n"
"\n"
"varying vec3 vLightTs;\n"
"varying vec3 vViewTs;\n"
"varying vec4 vColor;                         // IN.color (vertex-lit)\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv0 = gl_TexCoord[0].xy;\n"
"    vec2 uv1 = gl_TexCoord[0].zw;\n"
"\n"
"    // renormalize the interpolated lighting vectors, half = view + light\n"
"    vec3 light_ts = normalize( vLightTs );\n"
"    vec3 half_ts = normalize( normalize( vViewTs ) + light_ts );\n"
"\n"
"    // map_color_to_normal: expand [0,1] to [-1,1] and renormalize; the\n"
"    // alpha channel piggybacks the gloss map\n"
"    vec4 normal_gloss = texture2D( sampler_normal_gloss, uv1 );\n"
"    normal_gloss.xyz = normalize( normal_gloss.xyz * 2.0 - 1.0 );\n"
"\n"
"    // base material color: straight modulation, vertex color on rgb only\n"
"    // (alpha keeps base.a*blend.a), legacy x8 rgb scale\n"
"    vec4 tex_base = texture2D( sampler_base, uv0 );\n"
"    vec4 tex_blend = texture2D( sampler_blend, uv1 );\n"
"    vec4 out_color = tex_base * tex_blend;\n"
"    out_color.rgb *= vColor.rgb;\n"
"    out_color.rgb *= 8.0;\n"
"    out_color.a *= g_Env0FP.a;\n"
"\n"
"    // calc_lighting_factors, LQ branch: only the specular factor is used\n"
"    float n_dot_h = dot( normal_gloss.xyz, half_ts );\n"
"    float specular = pow( clamp( n_dot_h, 0.0, 1.0 ), g_Specular1ColorAndExponentFP.a );\n"
"    specular *= normal_gloss.w;\n"
"    vec3 gloss = clamp( specular * g_Specular1ColorAndExponentFP.rgb, 0.0, 1.0 );\n"
"    out_color.rgb = out_color.rgb + gloss;\n"
"\n"
"    // calc_fogged_color (same GL fog state as the other materials)\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// vp_master_vp.cg "bump.vp"/"bump_rgb.vp" variants: the model-space lighting
// path (LIGHT_SPACE=MODEL, PIXEL_LIT=BUMP_SPEC, TC_XFORM=TC_OFFSET). The
// engine pushes a model-space light POSITION as g_LightDirVP (sun or the
// dummy ambient light transformed into model space, rt_model.c), and the
// vertex program derives the per-vertex light direction from it. The
// model-space normal/tangent are used RAW (deliberately not normalized —
// matches the compiled ARB, unlike the view-space bump_dual variants), and
// the tangent-space position passed to the fragment is the VIEW-space
// position against that model-space basis (verified against the cgc ARB
// output; the mixed-space expression is what shipped). The two engine
// variants differ only in the vertex color: DIFFUSE (bump.vp, outdoor
// DRAWMODE_BUMPMAP_NORMALS) computes saturate(NdotL)*diffuse+ambient;
// PRELIT (bump_rgb.vp, ambient-group DRAWMODE_BUMPMAP_RGBS) reads the baked
// instance lighting on ATTR11 and scales it x4 — mode-switched here by
// g_Prelit like the skinning switch on the other bump vertex shader.
static const char s_bumpModelVertexSource[] =
"#version 120\n"
"\n"
"attribute vec4 attr_tangent;      // generic vertex attribute 7 (see rt_model.c)\n"
"attribute vec4 attr_prelit_color; // generic vertex attribute 11 (RGBS variant draws)\n"
"uniform vec4 g_LightDirVP;        // model-space light position (rt_model.c)\n"
"uniform vec4 g_TexScroll0VP;      // TC_OFFSET texcoord scrolls\n"
"uniform vec4 g_TexScroll1VP;\n"
"uniform vec4 g_AmbientParameterVP;  // VERTEX_LIT == DIFFUSE terms (drawLoopBump)\n"
"uniform vec4 g_DiffuseParameterVP;\n"
"uniform int  g_Prelit;            // 0 = bump.vp (DIFFUSE), 1 = bump_rgb.vp (PRELIT)\n"
"\n"
"varying vec3 vLightTs;\n"
"varying vec3 vViewTs;\n"
"varying vec4 vColor;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position_vs = ( gl_ModelViewMatrix * gl_Vertex ).xyz;\n"
"    gl_Position = gl_ProjectionMatrix * vec4( position_vs, 1.0 );\n"
"\n"
"    // fog coordinate: eye-radial distance (vp_master_vp.cg)\n"
"    gl_FogFragCoord = sqrt( dot( position_vs, position_vs ) + 1e-16 );\n"
"\n"
"    // TC_XFORM == TC_OFFSET\n"
"    gl_TexCoord[0] = vec4( gl_MultiTexCoord0.xy + g_TexScroll0VP.xy,\n"
"                           gl_MultiTexCoord1.xy + g_TexScroll1VP.xy );\n"
"\n"
"    // LIGHT_SPACE == MODEL: raw model-space basis and per-vertex light\n"
"    // direction from the model-space light position\n"
"    vec3 normal = gl_Normal;\n"
"    vec3 tangent = attr_tangent.xyz;\n"
"    vec3 light = normalize( g_LightDirVP.xyz - gl_Vertex.xyz );\n"
"\n"
"    if ( g_Prelit != 0 )\n"
"        vColor = attr_prelit_color * 4.0;\n"
"    else\n"
"        vColor = clamp( dot( normal, light ), 0.0, 1.0 ) * g_DiffuseParameterVP\n"
"               + g_AmbientParameterVP;\n"
"\n"
"    // calc_tangent_space_light_and_position: change of basis via\n"
"    // (tangent, binormal, normal) rows; the position is view-space against\n"
"    // this model-space basis, faithful to the shipped ARB program\n"
"    vec3 binormal = cross( tangent, normal ) * sign( attr_tangent.w );\n"
"    vLightTs = vec3( dot( light, tangent ), dot( light, binormal ), dot( light, normal ) );\n"
"    vec3 position_ts = vec3( dot( position_vs, tangent ), dot( position_vs, binormal ), dot( position_vs, normal ) );\n"
"    vViewTs = -position_ts;\n"
"}\n";

// vp_master_vp.cg \"bump_dual_multi\" variant (DRAWMODE_BUMPMAP_MULTITEX:
// SKIN=0 LIGHT_SPACE=VIEW VERTEX_LIT=PRELIT_WHITE TC_XFORM=NONE
// PIXEL_LIT=BUMP_ALL REFLECT=FAUX_MULTI): the vertex pairing of the water
// material (and, unported, multi9). Same tangent-space lighting setup as
// bump_dual, but TEXCOORD0 carries (uv0, faux spheremap reflection uv) and
// the vertex color is the constant white of VERTEX_LIT=PRELIT_WHITE.
static const char s_bumpMultiVertexSource[] =
"#version 120\n"
"\n"
"attribute vec4 attr_tangent;      // generic vertex attribute 7 (see rt_model.c)\n"
"uniform vec4 g_LightDirVP;        // engine constant, view-space light direction\n"
"\n"
"varying vec3 vLightTs;            // OUT.light_ts (TEXCOORD2 in the Cg program)\n"
"varying vec3 vViewTs;             // OUT.view_ts (TEXCOORD1); = -position in tangent space\n"
"varying vec3 vPositionVs;         // OUT.position_vs (TEXCOORD3); shadow CSM input\n"
"varying vec4 vUv0Faux;            // OUT.uv0_uv1 (TEXCOORD0); zw = faux reflection uv\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position_vs = ( gl_ModelViewMatrix * gl_Vertex ).xyz;\n"
"    vec3 normal = normalize( mat3( gl_ModelViewMatrix ) * gl_Normal );\n"
"    vec3 tangent = normalize( mat3( gl_ModelViewMatrix ) * attr_tangent.xyz );\n"
"    gl_Position = gl_ProjectionMatrix * vec4( position_vs, 1.0 );\n"
"\n"
"    // fog coordinate: eye-radial distance (vp_master_vp.cg)\n"
"    gl_FogFragCoord = sqrt( dot( position_vs, position_vs ) + 1e-16 );\n"
"\n"
"    // VERTEX_LIT == PRELIT_WHITE: constant vertex color\n"
"    gl_FrontColor = vec4( 1.0 );\n"
"\n"
"    // calc_faux_reflection_uv: legacy sphere-map uv from the view-space\n"
"    // normal and the incident (position) vector\n"
"    vec3 rr = reflect( position_vs, normal );\n"
"    vec3 ro = normalize( rr + vec3( 0.0, 0.0, 1.0 ) );\n"
"    vUv0Faux = vec4( gl_MultiTexCoord0.xy, 0.5 * ro.xy + 0.5 );\n"
"\n"
"    // calc_tangent_space_light_and_position (see bump_dual vertex shader)\n"
"    vec3 binormal = cross( tangent, normal ) * sign( attr_tangent.w );\n"
"    vec3 light_vs = g_LightDirVP.xyz;\n"
"    vLightTs = vec3( dot( light_vs, tangent ), dot( light_vs, binormal ), dot( light_vs, normal ) );\n"
"    vec3 position_ts = vec3( dot( position_vs, tangent ), dot( position_vs, binormal ), dot( position_vs, normal ) );\n"
"    vViewTs = -position_ts;\n"
"    vPositionVs = position_vs;\n"
"}\n";

// waterfp.cg, variant 0 (no BIT_PLANAR_REFLECTION / BIT_SHADOWMAP / HQ):
// the fancy-water material. Dual scrolling normal maps are averaged into a
// tangent-space normal; the base color is 2*vertexColor*multiply1*base1 (the
// vertex color is the constant white of the PRELIT_WHITE pairing) tinted by
// the two material constants blended on the view-normal term and base1.a,
// lit without gloss, then blended toward the depth-skewed refraction sample
// by g_GlossParamFP.w, with per-pixel bumped gloss added on top. The depth
// test clamps the normal skew so the refraction never samples geometry in
// front of the water surface (two depth taps, ARB-faithful). Alpha comes
// from g_Env0FP.a, optionally modulated by base1.a. scroll_scale(i) is the
// per-texlayer uv scroll/scale array; the selector bits (faux reflection uv
// for multiply1, water alpha) live in g_BumpMultiFlagsFP.x as integer-valued
// floats tested with the Cg frac(x/(bit*2)) >= 0.5 idiom.
static const char s_waterFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D sampler_base_1;              // TEXUNIT0\n"
"uniform sampler2D sampler_multiply_1;          // TEXUNIT1\n"
"uniform sampler2D sampler_normal_and_gloss_1;  // TEXUNIT2\n"
"uniform sampler2D sampler_reflection;           // TEXUNIT3 (water reflection pbuffer)\n"
"uniform sampler2D sampler_refraction;          // TEXUNIT5 (rt_water.c pbuffer)\n"
"uniform sampler2D sampler_refraction_depth;    // TEXUNIT6 (depth for the skew clamp)\n"
"uniform sampler2D sampler_normal_and_gloss_2;  // TEXUNIT7\n"
"uniform sampler2DShadow sampler_shadow;        // TEXUNIT11 (shadow atlas)\n"
"uniform vec4 g_Env0FP;                         // TIE(ENV8); .a = water alpha\n"
"uniform vec4 g_AmbientColorFP;                 // TIE(ENV0), setupBumpPixelShader ambient*2\n"
"uniform vec4 g_DiffuseColorFP;                 // TIE(ENV1), setupBumpPixelShader diffuse*4\n"
"uniform vec4 g_GlossParamFP;                   // TIE(ENV2); .x = glossConst, .w = refraction blend\n"
"uniform vec4 g_Specular1ColorAndExponentFP;    // TIE(ENV5), rgb spec color, a exponent\n"
"uniform vec4 g_ConstColor0FP;                  // TIE(ENV3), material tint color 0\n"
"uniform vec4 g_ConstColor1FP;                  // TIE(ENV4), material tint color 1\n"
"uniform vec4 g_WaterRefractionTransformFP;     // TIE(ENV7); .xy = 1/refraction texture size\n"
"uniform vec4 g_WaterRefractionParamsFP;        // TIE(ENV22); skew normal/depth scale, min skew\n"
"uniform vec4 g_WaterReflectionTransformFP;      // TIE(ENV6); .xy = 1/screen size\n"
"uniform vec4 g_WaterReflectionParamsFP;         // TIE(ENV23); .x = skew, .y = reflectivity\n"
"uniform vec4 g_WaterFresnelParamsFP;            // TIE(ENV24); bias, scale, power\n"
"uniform vec4 g_BumpMultiFlagsFP;               // TIE(ENV10); bit0 = reflect uv, bit3 = alpha water\n"
"uniform vec4 g_ScrollScaleArrFP[10];           // TIE(ENV12..21); xy scroll, zw scale per layer\n"
"uniform bool g_UseShadowMap;                   // pilot target selector\n"
"uniform int g_ShadowFilterMode;                // NONE/FAST=1, MEDIUM=2, HIGH=3\n"
"uniform vec4 g_ShadowMap1MatrixFP[4];           // TIE(ENV32..35)\n"
"uniform vec4 g_ShadowMap2MatrixFP[4];           // TIE(ENV36..39)\n"
"uniform vec4 g_ShadowMap3MatrixFP[4];           // TIE(ENV40..43)\n"
"uniform vec4 g_ShadowMap4MatrixFP[4];           // TIE(ENV44..47)\n"
"uniform vec4 g_ShadowParamsFP;                 // TIE(ENV48)\n"
"uniform vec4 g_ShadowSplitsFP;                 // TIE(ENV49)\n"
"uniform vec4 g_ShadowParams2FP;                // TIE(ENV50)\n"
"uniform vec4 g_ShadowParams3FP;                // TIE(ENV51)\n"
"uniform bool g_UsePlanarReflection;            // pilot target selector\n"
"\n"
"varying vec3 vLightTs;\n"
"varying vec3 vViewTs;\n"
"varying vec3 vPositionVs;\n"
"varying vec4 vUv0Faux;\n"
"\n"
"// isBitSet idiom from functions.cgh (integer-valued float bits)\n"
"bool isBitSet( float bits, float bit )\n"
"{\n"
"    return fract( bits / ( bit * 2.0 ) ) >= 0.5;\n"
"}\n"
"\n"
"// scroll_scale from functions.cgh: uv * scale + scroll\n"
"vec2 scrollScale( int layer, vec2 uv )\n"
"{\n"
"    return uv * g_ScrollScaleArrFP[layer].zw + g_ScrollScaleArrFP[layer].xy;\n"
"}\n"

"// The Cg wrapper uploads each shadow matrix as four row vec4s and the\n"
"// legacy source intentionally uses mul(position, matrix). Dotting against\n"
"// those rows reproduces that row-vector multiply without relying on GLSL\n"
"// matrix packing rules.\n"
"vec4 shadowCoord1( vec3 p )\n"
"{\n"
"    vec4 q = vec4( p, 1.0 );\n"
"    return vec4( dot( q, g_ShadowMap1MatrixFP[0] ), dot( q, g_ShadowMap1MatrixFP[1] ),\n"
"                 dot( q, g_ShadowMap1MatrixFP[2] ), dot( q, g_ShadowMap1MatrixFP[3] ) );\n"
"}\n"
"vec4 shadowCoord2( vec3 p )\n"
"{\n"
"    vec4 q = vec4( p, 1.0 );\n"
"    return vec4( dot( q, g_ShadowMap2MatrixFP[0] ), dot( q, g_ShadowMap2MatrixFP[1] ),\n"
"                 dot( q, g_ShadowMap2MatrixFP[2] ), dot( q, g_ShadowMap2MatrixFP[3] ) );\n"
"}\n"
"vec4 shadowCoord3( vec3 p )\n"
"{\n"
"    vec4 q = vec4( p, 1.0 );\n"
"    return vec4( dot( q, g_ShadowMap3MatrixFP[0] ), dot( q, g_ShadowMap3MatrixFP[1] ),\n"
"                 dot( q, g_ShadowMap3MatrixFP[2] ), dot( q, g_ShadowMap3MatrixFP[3] ) );\n"
"}\n"
"vec4 shadowCoord4( vec3 p )\n"
"{\n"
"    vec4 q = vec4( p, 1.0 );\n"
"    return vec4( dot( q, g_ShadowMap4MatrixFP[0] ), dot( q, g_ShadowMap4MatrixFP[1] ),\n"
"                 dot( q, g_ShadowMap4MatrixFP[2] ), dot( q, g_ShadowMap4MatrixFP[3] ) );\n"
"}\n"
"\n"
"float shadowLookup( vec4 coord, vec2 offset )\n"
"{\n"
"    vec4 sampleCoord = vec4( coord.xy + offset * g_ShadowParams2FP.w * coord.w, coord.z, coord.w );\n"
"    return shadow2DProj( sampler_shadow, sampleCoord ).x;\n"
"}\n"
"\n"
"float shadowPcf2x2( vec4 coord, vec2 fragmentXY )\n"
"{\n"
"    vec2 offset = step( vec2( 0.25 ), fract( fragmentXY * 0.5 ) );\n"
"    offset.y += offset.x;\n"
"    if ( offset.y > 1.1 ) offset.y = 0.0;\n"
"    float result = shadowLookup( coord, offset + vec2( -1.5, 0.5 ) );\n"
"    result += shadowLookup( coord, offset + vec2( 0.5, 0.5 ) );\n"
"    result += shadowLookup( coord, offset + vec2( -1.5, -1.5 ) );\n"
"    result += shadowLookup( coord, offset + vec2( 0.5, -1.5 ) );\n"
"    return result * 0.25;\n"
"}\n"
"\n"
"float shadowPcf4x4( vec4 coord )\n"
"{\n"
"    float result = 0.0;\n"
"    for ( int y = 0; y < 4; y++ )\n"
"        for ( int x = 0; x < 4; x++ )\n"
"            result += shadowLookup( coord, vec2( float(x) - 1.5, float(y) - 1.5 ) );\n"
"    return result * 0.0625;\n"
"}\n"
"\n"
"float shadowFiltered( vec4 coord, vec2 fragmentXY )\n"
"{\n"
"    if ( g_ShadowFilterMode == 3 ) return shadowPcf4x4( coord );\n"
"    if ( g_ShadowFilterMode == 2 ) return shadowPcf2x2( coord, fragmentXY );\n"
"    return shadowLookup( coord, vec2( 0.0 ) );\n"
"}\n"
"\n"
"float shadowApplyFade( float z, float lightAmount )\n"
"{\n"
"    float shadowStrength = clamp( ( g_ShadowParams2FP.x - z ) * g_ShadowParams2FP.y, 0.0, 1.0 );\n"
"    return mix( 1.0, lightAmount, shadowStrength );\n"
"}\n"
"\n"
"float shadowPlain( vec3 positionVS, vec2 fragmentXY )\n"
"{\n"
"    vec4 coord;\n"
"    float zEye = abs( positionVS.z );\n"
"    if ( zEye <= g_ShadowSplitsFP.x ) coord = shadowCoord1( positionVS );\n"
"    else if ( zEye <= g_ShadowSplitsFP.y ) coord = shadowCoord2( positionVS );\n"
"    else if ( zEye <= g_ShadowSplitsFP.z ) coord = shadowCoord3( positionVS );\n"
"    else if ( zEye <= g_ShadowSplitsFP.w ) coord = shadowCoord4( positionVS );\n"
"    else return 1.0;\n"
"    return shadowApplyFade( zEye, shadowFiltered( coord, fragmentXY ) );\n"
"}\n"
"\n"
"float shadowBlend( vec4 coordA, vec4 coordB, float factor )\n"
"{\n"
"    float result = 0.0;\n"
"    for ( int y = 0; y < 4; y++ )\n"
"    {\n"
"        vec4 coord = ( float(y) < factor * 4.0 ) ? coordB : coordA;\n"
"        for ( int x = 0; x < 4; x++ )\n"
"            result += shadowLookup( coord, vec2( float(x) - 1.5, float(y) - 1.5 ) );\n"
"    }\n"
"    return result * 0.0625;\n"
"}\n"
"\n"
"float shadowCsmBlend( vec3 positionVS )\n"
"{\n"
"    vec4 coord1 = shadowCoord1( positionVS );\n"
"    vec4 coord2 = shadowCoord2( positionVS );\n"
"    vec4 coord3 = shadowCoord3( positionVS );\n"
"    vec4 coord4 = shadowCoord4( positionVS );\n"
"    vec4 coordA = coord4;\n"
"    vec4 coordB = coord4;\n"
"    float zEye = abs( positionVS.z );\n"
"    float nextSplit = g_ShadowSplitsFP.w;\n"
"    float blendDelta = 0.0;\n"
"    if ( zEye <= g_ShadowSplitsFP.x ) { coordA = coord1; coordB = coord2; nextSplit = g_ShadowSplitsFP.x; blendDelta = g_ShadowParams3FP.y; }\n"
"    else if ( zEye <= g_ShadowSplitsFP.y ) { coordA = coord2; coordB = coord3; nextSplit = g_ShadowSplitsFP.y; blendDelta = g_ShadowParams3FP.z; }\n"
"    else if ( zEye <= g_ShadowSplitsFP.z ) { coordA = coord3; coordB = coord4; nextSplit = g_ShadowSplitsFP.z; blendDelta = g_ShadowParams3FP.w; }\n"
"    else if ( zEye <= g_ShadowSplitsFP.w ) { coordA = coord4; coordB = coord4; }\n"
"    else return 1.0;\n"
"    float blendNext = ( blendDelta > 0.0 ) ? 1.0 - clamp( ( nextSplit - zEye ) / blendDelta, 0.0, 1.0 ) : 0.0;\n"
"    return shadowApplyFade( zEye, shadowBlend( coordA, coordB, blendNext ) );\n"
"}\n"
"\n"
"float shadowCsm( vec3 positionVS, vec2 fragmentXY )\n"
"{\n"
"    if ( g_ShadowFilterMode == 3 ) return shadowCsmBlend( positionVS );\n"
"    return shadowPlain( positionVS, fragmentXY );\n"
"}\n"
"\n"
"vec3 shadowLightFactors( vec3 lightFactors, float rawFactor )\n"
"{\n"
"    float diffuseReduction = clamp( lightFactors.y, g_ShadowParams2FP.z, 1.0 );\n"
"    float adjusted = clamp( rawFactor * diffuseReduction, 0.0, 1.0 );\n"
"    vec3 shadowFactors = vec3( clamp( adjusted, g_ShadowParamsFP.x, 1.0 ),\n"
"                               clamp( adjusted, g_ShadowParamsFP.y, 1.0 ),\n"
"                               clamp( rawFactor, g_ShadowParamsFP.z, 1.0 ) );\n"
"    return lightFactors * shadowFactors;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv0 = vUv0Faux.xy;\n"
"    vec2 uv_reflect = vUv0Faux.zw;\n"
"    vec2 uv_multiply1 = isBitSet( g_BumpMultiFlagsFP.x, 1.0 ) ? uv_reflect : uv0;\n"
"\n"
"    // renormalize the interpolated lighting vectors; half = view + light\n"
"    vec3 light_ts = normalize( vLightTs );\n"
"    vec3 view_ts = normalize( vViewTs );\n"
"    vec3 half_ts = normalize( view_ts + light_ts );\n"
"\n"
"    vec4 texBase1 = texture2D( sampler_base_1, scrollScale( 0, uv0 ) );\n"
"    vec4 texMultiply1 = texture2D( sampler_multiply_1, scrollScale( 1, uv_multiply1 ) );\n"
"    vec4 texNormal1 = texture2D( sampler_normal_and_gloss_1, scrollScale( 2, uv0 ) );\n"
"    vec4 texNormal2 = texture2D( sampler_normal_and_gloss_2, scrollScale( 7, uv0 ) );\n"
"\n"
"    // map_color_to_normal on both normal maps, averaged (gloss alphas too)\n"
"    vec4 n1 = texNormal1;  n1.xyz = normalize( n1.xyz * 2.0 - 1.0 );\n"
"    vec4 n2 = texNormal2;  n2.xyz = normalize( n2.xyz * 2.0 - 1.0 );\n"
"    vec4 normal_ts = 0.5 * ( n1 + n2 );\n"
"\n"
"    float nDOTv = clamp( dot( normal_ts.xyz, view_ts ), 0.0, 1.0 );\n"
"\n"
"    // calc_lighting_factors (shade_factor = 1): (1, saturate(NdotL), gloss-masked spec)\n"
"    float n_dot_l = clamp( dot( normal_ts.xyz, light_ts ), 0.0, 1.0 );\n"
"    float specular = pow( clamp( dot( normal_ts.xyz, half_ts ), 0.0, 1.0 ),\n"
"                          g_Specular1ColorAndExponentFP.a ) * normal_ts.w;\n"
"    vec3 light_factors = vec3( 1.0, n_dot_l, specular );\n"
"    if ( g_UseShadowMap )\n"
"        light_factors = shadowLightFactors( light_factors, shadowCsm( vPositionVs, gl_FragCoord.xy ) );\n"
"\n"
"    // base color: 2 * vertexColor(=1) * multiply1 * base1\n"
"    vec3 out_color = 2.0 * gl_Color.rgb * texMultiply1.rgb * texBase1.rgb;\n"
"\n"
"    // refraction: screen-space uv, normal-derived skew clamped by two\n"
"    // depth taps so we never sample geometry in front of the surface\n"
"    vec2 texCoord_raw = gl_FragCoord.xy * g_WaterRefractionTransformFP.xy;\n"
"    vec2 skewFromNormal = normal_ts.xy * g_WaterRefractionTransformFP.xy * 1024.0;\n"
"    float z = gl_FragCoord.z;\n"
"    float w = gl_FragCoord.w;\n"
"    float depth1 = clamp( g_WaterRefractionParamsFP.y *\n"
"                          ( texture2D( sampler_refraction_depth, texCoord_raw ).x - z ) / w, 0.0, 1.0 );\n"
"    vec2 texCoord = skewFromNormal * g_WaterRefractionParamsFP.x + texCoord_raw;\n"
"    float depth2 = clamp( g_WaterRefractionParamsFP.y *\n"
"                          ( texture2D( sampler_refraction_depth, texCoord ).x - z ) / w, 0.0, 1.0 );\n"
"    float skewAmount = min( g_WaterRefractionParamsFP.z,\n"
"                            depth2 * depth1 * g_WaterRefractionParamsFP.x );\n"
"    texCoord = skewFromNormal * skewAmount + texCoord_raw;\n"
"    vec3 refracted = texture2D( sampler_refraction, texCoord ).xyz;\n"
"\n"
"    // surface tint blends the two material constants on the view-normal term\n"
"    vec3 surfaceColor = mix( g_ConstColor1FP.rgb, g_ConstColor0FP.rgb, nDOTv );\n"
"    out_color = out_color * surfaceColor * texBase1.a;\n"
"\n"
"    // apply_lighting_no_gloss\n"
"    out_color = out_color * ( light_factors.x * g_AmbientColorFP.rgb\n"
"                              + light_factors.y * g_DiffuseColorFP.rgb );\n"
"\n"
"    // refraction blend, then apply_lighting_gloss_only\n"
"    out_color = mix( out_color, refracted, g_GlossParamFP.w );\n"
"    vec3 gloss = clamp( light_factors.z * g_GlossParamFP.x\n"
"                        * g_Specular1ColorAndExponentFP.rgb, 0.0, 1.0 );\n"
"    if ( g_UsePlanarReflection )\n"
"    {\n"
"        // waterfp.cg BIT_PLANAR_REFLECTION: WPOS screen uv, Y-flipped for\n"
"        // the reflection pbuffer, then normal skew in screen pixels.\n"
"        vec2 reflectionTexCoord = gl_FragCoord.xy * g_WaterReflectionTransformFP.xy;\n"
"        reflectionTexCoord.y = ( reflectionTexCoord.y * -1.0 ) + 1.0;\n"
"        reflectionTexCoord += skewFromNormal * g_WaterReflectionParamsFP.x;\n"
"        vec3 reflection = texture2D( sampler_reflection, reflectionTexCoord ).rgb;\n"
"\n"
"        // calc_reflectivity_hq(view_ts, normal_ts, fresnel):\n"
"        // saturate(bias + scale * pow(1 - saturate(NdotV), power)).\n"
"        float fresnelTerm = clamp( g_WaterFresnelParamsFP.x\n"
"                                   + g_WaterFresnelParamsFP.y\n"
"                                   * pow( 1.0 - clamp( dot( view_ts, normal_ts.xyz ), 0.0, 1.0 ),\n"
"                                          g_WaterFresnelParamsFP.z ), 0.0, 1.0 );\n"
"        vec3 reflectionGloss = ( 1.0 - gloss )\n"
"                              * ( g_WaterReflectionParamsFP.y * fresnelTerm );\n"
"        out_color = mix( out_color, reflection, reflectionGloss );\n"
"        out_color += gloss;\n"
"    }\n"
"    else\n"
"    {\n"
"        out_color += gloss;\n"
"    }\n"
"\n"
"    // calc_fogged_color (same GL fog state as the other materials)\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color = mix( gl_Fog.color.rgb, out_color, fogAmount );\n"
"\n"
"    float alpha = g_Env0FP.a;\n"
"    if ( isBitSet( g_BumpMultiFlagsFP.x, 8.0 ) )\n"
"        alpha *= texBase1.a;\n"
"\n"
"    gl_FragColor = vec4( out_color, alpha );\n"
"}\n";

// vp_master_vp.cg "bump_dual_multi" compiled with BIT_HIGH_QUALITY: same
// FAUX_MULTI static pairing as the LQ variant (constant-white vertex color,
// (uv0, faux spheremap uv) on TEXCOORD0), but like the HQ bump_dual variant
// the tangent (binormal sign in w), normal and position pass through as
// interpolants — the HQ multi9 fragment builds the tangent basis per pixel
// and reads g_LightDirFP instead of a vertex-interpolated light vector.
static const char s_bumpMultiHQVertexSource[] =
"#version 120\n"
"\n"
"attribute vec4 attr_tangent;      // generic vertex attribute 7 (see rt_model.c)\n"
"\n"
"varying vec3 vNormalVs;           // OUT.normal_vs (TEXCOORD2 in the Cg program)\n"
"varying vec4 vTangentVs;          // OUT.tangent_vs (TEXCOORD1); .w = binormal sign\n"
"varying vec3 vPositionVs;         // OUT.position_vs (TEXCOORD3)\n"
"varying vec4 vUv0Faux;            // OUT.uv0_uv1 (TEXCOORD0); zw = faux reflection uv\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position_vs = ( gl_ModelViewMatrix * gl_Vertex ).xyz;\n"
"    vec3 normal = normalize( mat3( gl_ModelViewMatrix ) * gl_Normal );\n"
"    vec3 tangent = normalize( mat3( gl_ModelViewMatrix ) * attr_tangent.xyz );\n"
"    gl_Position = gl_ProjectionMatrix * vec4( position_vs, 1.0 );\n"
"\n"
"    // fog coordinate: eye-radial distance (vp_master_vp.cg)\n"
"    gl_FogFragCoord = sqrt( dot( position_vs, position_vs ) + 1e-16 );\n"
"\n"
"    // VERTEX_LIT == PRELIT_WHITE: constant vertex color\n"
"    gl_FrontColor = vec4( 1.0 );\n"
"\n"
"    // calc_faux_reflection_uv: legacy sphere-map uv from the view-space\n"
"    // normal and the incident (position) vector\n"
"    vec3 rr = reflect( position_vs, normal );\n"
"    vec3 ro = normalize( rr + vec3( 0.0, 0.0, 1.0 ) );\n"
"    vUv0Faux = vec4( gl_MultiTexCoord0.xy, 0.5 * ro.xy + 0.5 );\n"
"\n"
"    vNormalVs = normal;\n"
"    vTangentVs = vec4( tangent, sign( attr_tangent.w ) );\n"
"    vPositionVs = position_vs;\n"
"}\n";

// ---------------------------------------------------------------------------
// multi9 (BLENDMODE_MULTI) fragment variants: the dual-material 'new style'
// material. Five variants are ported — the ones that bind on static maps
// without cubemap/shadowmap support: variant 0 (dual material LQ),
// BIT_HIGH_QUALITY (dual material, per-pixel tangent basis), and the same two
// for BIT_SINGLE_MATERIAL, plus BIT_BUILDING. The shared math replicates the
// functions.cgh helpers (calc_dual_tint/calc_old_tint for the material-1
// g_Env0/1 tint and the material-2 g_ConstColor0/1 tint, mix_material_colors
// driven by the g_GlossParamFP.z/.w mask-blend selectors, has_glow with
// g_MiscParamFP, per-material calc_lighting_factors/apply_lighting with
// glossConst .x (material 1) / .y (material 2) and the specular1/specular2
// constants). The RGBS (baked instance lighting) vertex pairing, the cubemap/
// planar-reflection/shadow variants and the skinned multitex draws stay on
// the ARB path (the pilot declines their vertex programs).
// ---------------------------------------------------------------------------

// shared declarations/helpers for the multi9 fragment variants; the bodies
// below append their main() to this preamble
#define MULTI9_FP_HELPERS \
"uniform sampler2D sampler_base_1;              // TEXUNIT0\n" \
"uniform sampler2D sampler_multiply_1;          // TEXUNIT1\n" \
"uniform sampler2D sampler_normal_and_gloss_1;  // TEXUNIT2\n" \
"uniform sampler2D sampler_dual_1;              // TEXUNIT3\n" \
"uniform sampler2D sampler_mask;                // TEXUNIT4\n" \
"uniform sampler2D sampler_base_2;              // TEXUNIT5\n" \
"uniform sampler2D sampler_multiply_2;          // TEXUNIT6\n" \
"uniform sampler2D sampler_normal_and_gloss_2;  // TEXUNIT7\n" \
"uniform sampler2D sampler_dual_2;              // TEXUNIT8\n" \
"uniform sampler2D sampler_glow;                // TEXUNIT9\n" \
"uniform sampler2D sampler_glow_mask;           // TEXUNIT15\n" \
"uniform vec4 g_Env0FP;                         // TIE(ENV8), material 1 tint 0\n" \
"uniform vec4 g_Env1FP;                         // TIE(ENV9), material 1 tint 1\n" \
"uniform vec4 g_AmbientColorFP;                 // TIE(ENV0), setupBumpPixelShader ambient*2\n" \
"uniform vec4 g_DiffuseColorFP;                 // TIE(ENV1), setupBumpPixelShader diffuse*4\n" \
"uniform vec4 g_GlossParamFP;                   // TIE(ENV2); .x/.y gloss consts, .z/.w mask blend\n" \
"uniform vec4 g_Specular1ColorAndExponentFP;    // TIE(ENV5), material 1 spec color + exponent\n" \
"uniform vec4 g_Specular2ColorAndExponentFP;    // TIE(ENV6), material 2 spec color + exponent\n" \
"uniform vec4 g_ConstColor0FP;                  // TIE(ENV3), material 2 tint 0\n" \
"uniform vec4 g_ConstColor1FP;                  // TIE(ENV4), material 2 tint 1\n" \
"uniform vec4 g_MiscParamFP;                    // TIE(ENV7); .x glow threshold, .y seed\n" \
"uniform vec4 g_BumpMultiFlagsFP;               // TIE(ENV10); .x/.y selector bits\n" \
"uniform vec4 g_ScrollScaleArrFP[10];           // TIE(ENV12..21); xy scroll, zw scale per layer\n" \
"\n" \
"varying vec4 vUv0Faux;            // IN.uv0_uv1; zw = faux spheremap reflection uv\n" \
"\n" \
"// isBitSet idiom from functions.cgh (integer-valued float bits)\n" \
"bool isBitSet( float bits, float bit )\n" \
"{\n" \
"    return fract( bits / ( bit * 2.0 ) ) >= 0.5;\n" \
"}\n" \
"\n" \
"// scroll_scale from functions.cgh: uv * scale + scroll\n" \
"vec2 scrollScale( int layer, vec2 uv )\n" \
"{\n" \
"    return uv * g_ScrollScaleArrFP[layer].zw + g_ScrollScaleArrFP[layer].xy;\n" \
"}\n" \
"\n" \
"// map_color_to_normal: expand [0,1] to [-1,1] and renormalize; the alpha\n" \
"// channel piggybacks the gloss map\n" \
"vec4 mapColorToNormal( vec4 t )\n" \
"{\n" \
"    t.xyz = normalize( t.xyz * 2.0 - 1.0 );\n" \
"    return t;\n" \
"}\n" \
"\n" \
"// calc_dual_tint: lerp between the two tint colors by the dual texture,\n" \
"// mask back toward white by base alpha, modulate by base\n" \
"vec4 calcDualTint( vec4 c0, vec4 c1, vec4 tex0, vec4 tex1 )\n" \
"{\n" \
"    vec4 d;\n" \
"    d.rgb = mix( c1.rgb, c0.rgb, tex1.rgb );\n" \
"    d.rgb = mix( d.rgb, vec3( 1.0 ), tex0.a );\n" \
"    d.rgb *= tex0.rgb;\n" \
"    d.a = tex1.a * c0.a;\n" \
"    return d;\n" \
"}\n" \
"\n" \
"// calc_old_tint: lerp from the tint color by base alpha; alpha from tint\n" \
"vec4 calcOldTint( vec4 color, vec4 tex )\n" \
"{\n" \
"    return vec4( mix( color.rgb, tex.rgb, tex.a ), color.a );\n" \
"}\n" \
"\n" \
"// has_glow with g_MiscParamFP: each 1x1 tile of the base texture shares one\n" \
"// 128x128 glow-mask texel; glow fires when mask < threshold\n" \
"bool hasGlow( vec2 uv )\n" \
"{\n" \
"    vec2 maskUv = ( floor( uv ) * 0.0078125 ) + g_MiscParamFP.yw;\n" \
"    return texture2D( sampler_glow_mask, maskUv ).r < g_MiscParamFP.x;\n" \
"}\n" \
"\n" \
"// add_glow_multi: glow texture on the scroll-scaled uv, optionally tinted\n" \
"// by g_Env1FP.rgb, added to material 1 or 2 as the flags direct\n" \
"void addGlowMulti( vec2 uv0, inout vec4 color1, inout vec4 color2 )\n" \
"{\n" \
"    if ( hasGlow( uv0 ) )\n" \
"    {\n" \
"        vec3 texAddGlow = texture2D( sampler_glow, scrollScale( 9, uv0 ) ).rgb;\n" \
"        if ( isBitSet( g_BumpMultiFlagsFP.x, 16.0 ) )\n" \
"            texAddGlow *= g_Env1FP.rgb;\n" \
"        if ( isBitSet( g_BumpMultiFlagsFP.x, 4.0 ) )\n" \
"            color2.rgb += texAddGlow;\n" \
"        else\n" \
"            color1.rgb += texAddGlow;\n" \
"    }\n" \
"}\n" \
"\n" \
"// mix_material_colors: the mask texture blends the two materials; the\n" \
"// g_GlossParamFP.z selector lerps the mask toward its alpha channel and\n" \
"// g_GlossParamFP.w gates the mask entirely (1 = mask, 0 = material 1)\n" \
"vec4 mixMaterialColors( vec4 mixmask, vec4 color1, vec4 color2 )\n" \
"{\n" \
"    vec4 lerpValue = mix( mixmask, vec4( mixmask.a ), g_GlossParamFP.z );\n" \
"    lerpValue = mix( vec4( 1.0 ), lerpValue, g_GlossParamFP.w );\n" \
"    return mix( color2, color1, lerpValue );\n" \
"}\n" \
"\n" \
"// calc_lighting_factors + apply_lighting for one sub-material: (1, NdotL,\n" \
"// gloss-masked specular) against the shared ambient/diffuse and the\n" \
"// sub-material specular constant and gloss constant\n" \
"vec3 applyMultiLighting( vec3 color_in, vec4 normal_gloss, vec3 light_ts, vec3 half_ts,\n" \
"                         vec4 spec_color_exp, float glossConst )\n" \
"{\n" \
"    float specular = pow( clamp( dot( normal_gloss.xyz, half_ts ), 0.0, 1.0 ),\n" \
"                          spec_color_exp.a ) * normal_gloss.w;\n" \
"    vec3 diffuse = clamp( dot( normal_gloss.xyz, light_ts ), 0.0, 1.0 ) * g_DiffuseColorFP.rgb;\n" \
"    vec3 gloss = clamp( specular * glossConst * spec_color_exp.rgb, 0.0, 1.0 );\n" \
"    return color_in * ( g_AmbientColorFP.rgb + diffuse ) + gloss;\n" \
"}\n"

// multi9fp.cg, variant 0 (MULTI_FULL, no HQ/cubemap/shadow): both
// sub-materials are dual-tinted, multiplied, vertex-colored and lit with
// their own specular/gloss, addglow is applied to either, and the mask
// texture blends the result.
static const char s_multi9FullFragmentSource[] =
"#version 120\n"
"\n"
MULTI9_FP_HELPERS
"varying vec3 vLightTs;\n"
"varying vec3 vViewTs;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv0 = vUv0Faux.xy;\n"
"    vec2 uv_reflect = vUv0Faux.zw;\n"
"    vec2 uv_mult1 = isBitSet( g_BumpMultiFlagsFP.x, 1.0 ) ? uv_reflect : uv0;\n"
"    vec2 uv_mult2 = isBitSet( g_BumpMultiFlagsFP.x, 2.0 ) ? uv_reflect : uv0;\n"
"    bool oldTint = isBitSet( g_BumpMultiFlagsFP.y, 1.0 );\n"
"\n"
"    vec3 light_ts = normalize( vLightTs );\n"
"    vec3 half_ts = normalize( normalize( vViewTs ) + light_ts );\n"
"\n"
"    vec4 n1 = mapColorToNormal( texture2D( sampler_normal_and_gloss_1, scrollScale( 2, uv0 ) ) );\n"
"    vec4 n2 = mapColorToNormal( texture2D( sampler_normal_and_gloss_2, scrollScale( 7, uv0 ) ) );\n"
"\n"
"    // material 1: dual tint (g_Env0/1), multiply, vertex color, lighting\n"
"    vec4 texBase1 = texture2D( sampler_base_1, scrollScale( 0, uv0 ) );\n"
"    vec4 color1 = oldTint ? calcOldTint( g_Env0FP, texBase1 )\n"
"        : calcDualTint( g_Env0FP, g_Env1FP, texBase1,\n"
"                        texture2D( sampler_dual_1, scrollScale( 3, uv0 ) ) );\n"
"    color1 *= texture2D( sampler_multiply_1, scrollScale( 1, uv_mult1 ) );\n"
"    color1.rgb *= gl_Color.rgb;\n"
"    color1.rgb = applyMultiLighting( color1.rgb, n1, light_ts, half_ts,\n"
"                                     g_Specular1ColorAndExponentFP, g_GlossParamFP.x );\n"
"\n"
"    // material 2: dual tint (g_ConstColor0/1), multiply, vertex color, its\n"
"    // own specular/gloss\n"
"    vec4 texBase2 = texture2D( sampler_base_2, scrollScale( 5, uv0 ) );\n"
"    vec4 color2 = oldTint ? calcOldTint( g_ConstColor0FP, texBase2 )\n"
"        : calcDualTint( g_ConstColor0FP, g_ConstColor1FP, texBase2,\n"
"                        texture2D( sampler_dual_2, scrollScale( 8, uv0 ) ) );\n"
"    color2 *= texture2D( sampler_multiply_2, scrollScale( 6, uv_mult2 ) );\n"
"    color2.rgb *= gl_Color.rgb;\n"
"    color2.rgb = applyMultiLighting( color2.rgb, n2, light_ts, half_ts,\n"
"                                     g_Specular2ColorAndExponentFP, g_GlossParamFP.y );\n"
"\n"
"    addGlowMulti( uv0, color1, color2 );\n"
"\n"
"    vec4 out_color = mixMaterialColors( texture2D( sampler_mask, scrollScale( 4, uv0 ) ),\n"
"                                        color1, color2 );\n"
"\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// multi9fp.cg, BIT_HIGH_QUALITY (MULTI_FULL): identical material math, but
// the tangent-space basis and lighting vectors are built per pixel from the
// HQ vertex interpolants and the light direction comes from the g_LightDirFP
// fragment constant (light_ts renormalized after the basis change, half_ts
// deliberately not — matches the Cg/ARB specular response).
static const char s_multi9FullHQFragmentSource[] =
"#version 120\n"
"\n"
MULTI9_FP_HELPERS
"uniform vec4 g_LightDirFP;                    // TIE(ENV11), view-space light dir (HQ only)\n"
"\n"
"varying vec3 vNormalVs;\n"
"varying vec4 vTangentVs;\n"
"varying vec3 vPositionVs;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv0 = vUv0Faux.xy;\n"
"    vec2 uv_reflect = vUv0Faux.zw;\n"
"    vec2 uv_mult1 = isBitSet( g_BumpMultiFlagsFP.x, 1.0 ) ? uv_reflect : uv0;\n"
"    vec2 uv_mult2 = isBitSet( g_BumpMultiFlagsFP.x, 2.0 ) ? uv_reflect : uv0;\n"
"    bool oldTint = isBitSet( g_BumpMultiFlagsFP.y, 1.0 );\n"
"\n"
"    // populate_lighting_vectors_hq (see the HQ bump dual fragment)\n"
"    vec3 normal_vs = normalize( vNormalVs );\n"
"    vec3 tangent_vs = normalize( vTangentVs.xyz );\n"
"    vec3 binormal_vs = cross( tangent_vs, normal_vs ) * vTangentVs.w;\n"
"    vec3 light_vs = g_LightDirFP.xyz;\n"
"    vec3 u_to_eye = normalize( -vPositionVs );\n"
"    vec3 light_ts = normalize( vec3( dot( light_vs, tangent_vs ),\n"
"                                     dot( light_vs, binormal_vs ),\n"
"                                     dot( light_vs, normal_vs ) ) );\n"
"    vec3 h_vs = normalize( u_to_eye + light_vs );\n"
"    vec3 half_ts = vec3( dot( h_vs, tangent_vs ),\n"
"                         dot( h_vs, binormal_vs ),\n"
"                         dot( h_vs, normal_vs ) );\n"
"\n"
"    vec4 n1 = mapColorToNormal( texture2D( sampler_normal_and_gloss_1, scrollScale( 2, uv0 ) ) );\n"
"    vec4 n2 = mapColorToNormal( texture2D( sampler_normal_and_gloss_2, scrollScale( 7, uv0 ) ) );\n"
"\n"
"    vec4 texBase1 = texture2D( sampler_base_1, scrollScale( 0, uv0 ) );\n"
"    vec4 color1 = oldTint ? calcOldTint( g_Env0FP, texBase1 )\n"
"        : calcDualTint( g_Env0FP, g_Env1FP, texBase1,\n"
"                        texture2D( sampler_dual_1, scrollScale( 3, uv0 ) ) );\n"
"    color1 *= texture2D( sampler_multiply_1, scrollScale( 1, uv_mult1 ) );\n"
"    color1.rgb *= gl_Color.rgb;\n"
"    color1.rgb = applyMultiLighting( color1.rgb, n1, light_ts, half_ts,\n"
"                                     g_Specular1ColorAndExponentFP, g_GlossParamFP.x );\n"
"\n"
"    vec4 texBase2 = texture2D( sampler_base_2, scrollScale( 5, uv0 ) );\n"
"    vec4 color2 = oldTint ? calcOldTint( g_ConstColor0FP, texBase2 )\n"
"        : calcDualTint( g_ConstColor0FP, g_ConstColor1FP, texBase2,\n"
"                        texture2D( sampler_dual_2, scrollScale( 8, uv0 ) ) );\n"
"    color2 *= texture2D( sampler_multiply_2, scrollScale( 6, uv_mult2 ) );\n"
"    color2.rgb *= gl_Color.rgb;\n"
"    color2.rgb = applyMultiLighting( color2.rgb, n2, light_ts, half_ts,\n"
"                                     g_Specular2ColorAndExponentFP, g_GlossParamFP.y );\n"
"\n"
"    addGlowMulti( uv0, color1, color2 );\n"
"\n"
"    vec4 out_color = mixMaterialColors( texture2D( sampler_mask, scrollScale( 4, uv0 ) ),\n"
"                                        color1, color2 );\n"
"\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";

// multi9fp.cg, BIT_SINGLE_MATERIAL (and its BIT_HIGH_QUALITY sibling): only
// sub-material 1 is calculated (dual tint, multiply, lighting) plus the
// addglow; no mask texture, no material 2. The LQ and HQ variants differ only
// in the lighting-vector setup, so they share a body macro.
#define MULTI9_SINGLE_BODY( LIGHTING_SETUP ) \
"void main()\n" \
"{\n" \
"    vec2 uv0 = vUv0Faux.xy;\n" \
"    vec2 uv_reflect = vUv0Faux.zw;\n" \
"    vec2 uv_mult1 = isBitSet( g_BumpMultiFlagsFP.x, 1.0 ) ? uv_reflect : uv0;\n" \
"    bool oldTint = isBitSet( g_BumpMultiFlagsFP.y, 1.0 );\n" \
"\n" \
LIGHTING_SETUP \
"\n" \
"    vec4 n1 = mapColorToNormal( texture2D( sampler_normal_and_gloss_1, scrollScale( 2, uv0 ) ) );\n" \
"\n" \
"    // material 1: dual tint (g_Env0/1), multiply, vertex color, lighting\n" \
"    vec4 texBase1 = texture2D( sampler_base_1, scrollScale( 0, uv0 ) );\n" \
"    vec4 out_color = oldTint ? calcOldTint( g_Env0FP, texBase1 )\n" \
"        : calcDualTint( g_Env0FP, g_Env1FP, texBase1,\n" \
"                        texture2D( sampler_dual_1, scrollScale( 3, uv0 ) ) );\n" \
"    out_color *= texture2D( sampler_multiply_1, scrollScale( 1, uv_mult1 ) );\n" \
"    out_color.rgb *= gl_Color.rgb;\n" \
"    out_color.rgb = applyMultiLighting( out_color.rgb, n1, light_ts, half_ts,\n" \
"                                        g_Specular1ColorAndExponentFP, g_GlossParamFP.x );\n" \
"\n" \
"    // add_glow (single-material flavor): same mask/glow uvs and tint as\n" \
"    // add_glow_multi but applied straight to the result\n" \
"    if ( hasGlow( uv0 ) )\n" \
"    {\n" \
"        vec3 texAddGlow = texture2D( sampler_glow, scrollScale( 9, uv0 ) ).rgb;\n" \
"        if ( isBitSet( g_BumpMultiFlagsFP.x, 16.0 ) )\n" \
"            texAddGlow *= g_Env1FP.rgb;\n" \
"        out_color.rgb += texAddGlow;\n" \
"    }\n" \
"\n" \
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n" \
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n" \
"\n" \
"    gl_FragColor = out_color;\n" \
"}\n"

static const char s_multi9SingleFragmentSource[] =
"#version 120\n"
"\n"
MULTI9_FP_HELPERS
"varying vec3 vLightTs;\n"
"varying vec3 vViewTs;\n"
"\n"
MULTI9_SINGLE_BODY(
"    vec3 light_ts = normalize( vLightTs );\n"
"    vec3 half_ts = normalize( normalize( vViewTs ) + light_ts );\n" );

static const char s_multi9SingleHQFragmentSource[] =
"#version 120\n"
"\n"
MULTI9_FP_HELPERS
"uniform vec4 g_LightDirFP;                    // TIE(ENV11), view-space light dir (HQ only)\n"
"\n"
"varying vec3 vNormalVs;\n"
"varying vec4 vTangentVs;\n"
"varying vec3 vPositionVs;\n"
"\n"
MULTI9_SINGLE_BODY(
"    // populate_lighting_vectors_hq (see the HQ bump dual fragment)\n"
"    vec3 normal_vs = normalize( vNormalVs );\n"
"    vec3 tangent_vs = normalize( vTangentVs.xyz );\n"
"    vec3 binormal_vs = cross( tangent_vs, normal_vs ) * vTangentVs.w;\n"
"    vec3 light_vs = g_LightDirFP.xyz;\n"
"    vec3 u_to_eye = normalize( -vPositionVs );\n"
"    vec3 light_ts = normalize( vec3( dot( light_vs, tangent_vs ),\n"
"                                     dot( light_vs, binormal_vs ),\n"
"                                     dot( light_vs, normal_vs ) ) );\n"
"    vec3 h_vs = normalize( u_to_eye + light_vs );\n"
"    vec3 half_ts = vec3( dot( h_vs, tangent_vs ),\n"
"                         dot( h_vs, binormal_vs ),\n"
"                         dot( h_vs, normal_vs ) );\n" );

// multi9fp.cg, BIT_BUILDING: material 1 is calculated and lit as usual (but
// without scroll/scale on its base/dual/mask uvs), and material 2 is just
// material 1's color multiplied by the multiply-2 texture (no material-2
// lighting); the unscrolled mask blends the two. Low-quality only (the
// HQ|BUILDING variant is not known to bind on static maps).
static const char s_multi9BuildingFragmentSource[] =
"#version 120\n"
"\n"
MULTI9_FP_HELPERS
"varying vec3 vLightTs;\n"
"varying vec3 vViewTs;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv0 = vUv0Faux.xy;\n"
"    vec2 uv_reflect = vUv0Faux.zw;\n"
"    vec2 uv_mult1 = isBitSet( g_BumpMultiFlagsFP.x, 1.0 ) ? uv_reflect : uv0;\n"
"    vec2 uv_mult2 = isBitSet( g_BumpMultiFlagsFP.x, 2.0 ) ? uv_reflect : uv0;\n"
"    bool oldTint = isBitSet( g_BumpMultiFlagsFP.y, 1.0 );\n"
"\n"
"    vec3 light_ts = normalize( vLightTs );\n"
"    vec3 half_ts = normalize( normalize( vViewTs ) + light_ts );\n"
"\n"
"    // building variant: normal map and base/dual uvs unscrolled\n"
"    vec4 n1 = mapColorToNormal( texture2D( sampler_normal_and_gloss_1, uv0 ) );\n"
"\n"
"    vec4 texBase1 = texture2D( sampler_base_1, uv0 );\n"
"    vec4 color1 = oldTint ? calcOldTint( g_Env0FP, texBase1 )\n"
"        : calcDualTint( g_Env0FP, g_Env1FP, texBase1,\n"
"                        texture2D( sampler_dual_1, uv0 ) );\n"
"    color1 *= texture2D( sampler_multiply_1, scrollScale( 1, uv_mult1 ) );\n"
"    color1.rgb *= gl_Color.rgb;\n"
"    color1.rgb = applyMultiLighting( color1.rgb, n1, light_ts, half_ts,\n"
"                                     g_Specular1ColorAndExponentFP, g_GlossParamFP.x );\n"
"\n"
"    // material 2 is material 1's lit color times the multiply-2 texture\n"
"    vec4 color2 = color1 * texture2D( sampler_multiply_2, scrollScale( 6, uv_mult2 ) );\n"
"\n"
"    addGlowMulti( uv0, color1, color2 );\n"
"\n"
"    vec4 out_color = mixMaterialColors( texture2D( sampler_mask, uv0 ),\n"
"                                        color1, color2 );\n"
"\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
"}\n";


// ---------------------------------------------------------------------------
// effects/ post-processing family (rt_effects.c). Fullscreen quads drawn
// under the 2D rendering setup, which force-binds the DRAWMODE_SPRITE
// vertex program — the pilot's dualtex vertex shader covers it, so these
// materials pair with kPilotVertexKind_DualTex. The fragments read
// gl_TexCoord[0].xy (uv0 through the same texture-matrix path the Cg
// programs see). Note utilFinalColor always writes alpha 1; several passes
// rely on that even when the intermediate math carries alpha.
// ---------------------------------------------------------------------------

// shrinkfp.cg, HIGH_RANGE (SHADER_SHRINK_EXTEND): 4-tap box downsample
// (utilShrinkSample) with the [0,1] -> extended-range conversion
// saturate(rgb*3-2).
static const char s_fxShrinkExtendFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_TextTransformFP; // per-pass half-texel offsets\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec4 t = g_Effects_TextTransformFP;\n"
"    // utilShrinkSample: the Cg swizzles xyzz/zyzz/xwzz/zwzz + texCoord\n"
"    vec4 c = texture2D( texSampler0, t.xy + tc )\n"
"           + texture2D( texSampler0, t.zy + tc )\n"
"           + texture2D( texSampler0, t.xw + tc )\n"
"           + texture2D( texSampler0, t.zw + tc );\n"
"    c *= 0.25;\n"
"\n"
"    // utilFinalColor with HIGH_RANGE\n"
"    gl_FragColor = vec4( clamp( c.rgb * 3.0 - 2.0, 0.0, 1.0 ), 1.0 );\n"
"}\n";

// shrinkfp.cg without defines (SHADER_SHRINK2 and SHADER_SHRINK2DOF — the
// DOF branch is #undef'd in the source, so both compile identically).
static const char s_fxShrinkPlainFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_TextTransformFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec4 t = g_Effects_TextTransformFP;\n"
"    vec4 c = texture2D( texSampler0, t.xy + tc )\n"
"           + texture2D( texSampler0, t.zy + tc )\n"
"           + texture2D( texSampler0, t.xw + tc )\n"
"           + texture2D( texSampler0, t.zw + tc );\n"
"    c *= 0.25;\n"
"\n"
"    gl_FragColor = vec4( c.rgb, 1.0 );\n"
"}\n";

// blurfp.cg BLUR_HOR / BLUR_VER (SHADER_HBLUR/SHADER_VBLUR): SMALL_FILTER
// 7-tap gaussian. Alpha accumulates like the Cg (oColor = H2).
static const char s_fxHBlurFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_TextTransformFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 t = g_Effects_TextTransformFP;\n"
"    vec2 base = gl_TexCoord[0].xy;\n"
"    vec4 h2 = vec4( 0.0 );\n"
"    // horizontal: offsets along TextTransform.xz\n"
"    h2 += ( 0.06963716 * texture2D( texSampler0, ( t.xz * -3.0 ) + base ) );\n"
"    h2 += ( 0.13009916 * texture2D( texSampler0, ( t.xz * -2.0 ) + base ) );\n"
"    h2 += ( 0.18929282 * texture2D( texSampler0, ( t.xz * -1.0 ) + base ) );\n"
"    h2 += ( 0.22194172 * texture2D( texSampler0, ( t.xz *  0.0 ) + base ) );\n"
"    h2 += ( 0.18929282 * texture2D( texSampler0, ( t.xz *  1.0 ) + base ) );\n"
"    h2 += ( 0.13009916 * texture2D( texSampler0, ( t.xz *  2.0 ) + base ) );\n"
"    h2 += ( 0.06963716 * texture2D( texSampler0, ( t.xz *  3.0 ) + base ) );\n"
"    gl_FragColor = h2;\n"
"}\n";

static const char s_fxVBlurFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_TextTransformFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 t = g_Effects_TextTransformFP;\n"
"    vec2 base = gl_TexCoord[0].xy;\n"
"    vec4 h2 = vec4( 0.0 );\n"
"    // vertical: offsets along TextTransform.zy\n"
"    h2 += ( 0.06963716 * texture2D( texSampler0, ( t.zy * -3.0 ) + base ) );\n"
"    h2 += ( 0.13009916 * texture2D( texSampler0, ( t.zy * -2.0 ) + base ) );\n"
"    h2 += ( 0.18929282 * texture2D( texSampler0, ( t.zy * -1.0 ) + base ) );\n"
"    h2 += ( 0.22194172 * texture2D( texSampler0, ( t.zy *  0.0 ) + base ) );\n"
"    h2 += ( 0.18929282 * texture2D( texSampler0, ( t.zy *  1.0 ) + base ) );\n"
"    h2 += ( 0.13009916 * texture2D( texSampler0, ( t.zy *  2.0 ) + base ) );\n"
"    h2 += ( 0.06963716 * texture2D( texSampler0, ( t.zy *  3.0 ) + base ) );\n"
"    gl_FragColor = h2;\n"
"}\n";

// tonemapfp.cg (SHADER_TONEMAP): the nVidia-demo tonemapper; the
// blur/exposure uniforms are constants (1.5 / 1.0) in the shipped source —
// the engine's BlurAmtAndExposure push is read by no live shader.
static const char s_fxTonemapFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform sampler2D texSampler1;        // TEXUNIT1 (blurred)\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec4 orig = texture2D( texSampler0, tc );\n"
"    vec4 blurred = texture2D( texSampler1, tc );\n"
"\n"
"    vec3 LUM = vec3( 0.35, 0.45, 0.20 );\n"
"    float brightness = clamp( 1.5 * dot( blurred.rgb, LUM ), 0.0, 1.0 );\n"
"    vec4 blursat = clamp(( blurred * 0.25 ) + 0.75, 0.0, 1.0 );\n"
"    vec4 a = mix( orig, blursat, brightness );\n"
"    a *= 1.0; // exposure\n"
"\n"
"    gl_FragColor = vec4( a.rgb, 1.0 );\n"
"}\n";

// shrink4xfp.cg (SHADER_SHRINK4 / SHRINK4LUM / SHRINK4EXP): 16-tap
// downsample; the LUM variant reduces to luminance, the EXP variant to
// exp2 of the red channel (the luminance chain's averaging stages).
static const char s_fxShrink4FragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_TextTransformFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec2 s = g_Effects_TextTransformFP.xy;\n"
"    vec4 c = vec4( 0.0 );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0,  3.0 ) ) + tc );\n"
"    c *= 0.0625;\n"
"\n"
"    gl_FragColor = vec4( c.rgb, 1.0 );\n"
"}\n";

static const char s_fxShrink4LumFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_TextTransformFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec2 s = g_Effects_TextTransformFP.xy;\n"
"    vec4 c = vec4( 0.0 );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0,  3.0 ) ) + tc );\n"
"    c *= 0.0625;\n"
"\n"
"    // USE_LUMINANCE: reduce to the luminance scalar (broadcast like Cg)\n"
"    float lum = dot( c.rgb, vec3( 0.35, 0.45, 0.20 ) );\n"
"    gl_FragColor = vec4( lum, lum, lum, 1.0 );\n"
"}\n";

static const char s_fxShrink4ExpFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_TextTransformFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec2 s = g_Effects_TextTransformFP.xy;\n"
"    vec4 c = vec4( 0.0 );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0,  3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0, -3.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0, -1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0,  1.0 ) ) + tc );\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0,  3.0 ) ) + tc );\n"
"    c *= 0.0625;\n"
"\n"
"    // USE_EXP: exp2 of the red channel, broadcast like Cg\n"
"    float e = exp2( c.x );\n"
"    gl_FragColor = vec4( e, e, e, 1.0 );\n"
"}\n";

// lightAdaptationfp.cg (SHADER_LIGHTADAPTATION): the eye-adaptation pass on
// the 1x1-class targets. texCoord1 reads the quad's unit-1 texture
// coordinates — with no vertex program bound, fixed function provides
// texcoord[1] from glMultiTexCoord(GL_TEXTURE1), and the pilot's
// fixed-function vertex shader writes the same gl_TexCoord[1].
static const char s_fxLightAdaptationFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0 (avgLum)\n"
"uniform sampler2D texSampler1;        // TEXUNIT1 (lastLum)\n"
"uniform vec4 g_TimeStepFP;            // .x frame time\n"
"\n"
"void main()\n"
"{\n"
"    vec4 avgLum = texture2D( texSampler0, gl_TexCoord[0].xy );\n"
"    vec4 lastLum = texture2D( texSampler1, gl_TexCoord[1].xy );\n"
"\n"
"    float scaleLum = 1.0 - pow( 0.99, g_TimeStepFP.x );\n"
"    vec4 deltaLum = ( avgLum - lastLum ) * scaleLum;\n"
"    float initVar = ( -lastLum.x >= 0.0 ) ? 1.0 : 0.0;\n"
"    vec3 color = mix( lastLum + deltaLum, avgLum, initVar ).rgb;\n"
"    gl_FragColor = vec4( color, 1.0 );\n"
"}\n";

// logfp.cg (SHADER_LOG): 16-tap luminance log2 sampling for the adaptation
// chain (useLogSampling).
static const char s_fxLogFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_TextTransformFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec2 s = g_Effects_TextTransformFP.xy;\n"
"    vec3 c = vec3( 0.0 );\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0, -3.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0, -1.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0,  1.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2( -3.0,  3.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0, -3.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0, -1.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0,  1.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2( -1.0,  3.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0, -3.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0, -1.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0,  1.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2(  1.0,  3.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0, -3.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0, -1.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0,  1.0 ) ) + tc ).rgb;\n"
"    c += texture2D( texSampler0, ( s * vec2(  3.0,  3.0 ) ) + tc ).rgb;\n"
"    c *= 0.0625;\n"
"\n"
"    float cDotL = dot( c, vec3( 0.2125, 0.7154, 0.0721 ) );\n"
"    float logSample = log2( cDotL + 0.01 );\n"
"    gl_FragColor = vec4( logSample, logSample, logSample, 1.0 );\n"
"}\n";

// brightpassfp.cg (SHADER_BRIGHTPASS): exposure-style tone scale toward the
// adapted luminance (the pass is currently disabled in the engine flow).
static const char s_fxBrightpassFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform sampler2D texSampler1;        // TEXUNIT1 (avgLum, 1x1)\n"
"uniform vec4 g_Effects_ExpectedLumFP; // .x middleGray .y tonemap weight\n"
"\n"
"void main()\n"
"{\n"
"    vec4 sample = texture2D( texSampler0, gl_TexCoord[0].xy );\n"
"    vec4 avgLum = texture2D( texSampler1, vec2( 0.5, 0.0 ) );\n"
"\n"
"    // DoToneMapping\n"
"    float lum = 1.0 / ( avgLum.x + 0.1 );\n"
"    float effMiddleGray = ( g_Effects_ExpectedLumFP.x - 0.5 ) * 0.4 + 0.5;\n"
"    float scale = ( effMiddleGray * lum ) - 1.0;\n"
"    sample *= ( scale * g_Effects_ExpectedLumFP.y * 0.7 ) + 1.0;\n"
"\n"
"    gl_FragColor = vec4( sample.rgb, 1.0 );\n"
"}\n";

// tonemap2fp.cg (SHADER_TONEMAP2, optional DESATURATE): the default bloom
// final pass — tone scale against the adapted 1x1 luminance, then bloom
// glow from the blurred buffer (utilBlurredToneMap + utilBloomGlow).
static const char s_fxTonemap2FragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0 (frame)\n"
"uniform sampler2D texSampler1;        // TEXUNIT1 (adapted avgLum, 1x1)\n"
"uniform sampler2D texSampler2;        // TEXUNIT2 (blurred)\n"
"uniform vec4 g_Effects_ExpectedLumFP; // .x middleGray .y weight .z glow weight\n"
"uniform vec4 g_Effects_PresentationFP; // .x presentation, .y modern bloom enable\n"
"\n"
"vec3 applyModernPresentation( vec3 color, float avgLum )\n"
"{\n"
"    float scaleCalc = ( g_Effects_ExpectedLumFP.x - avgLum ) * g_Effects_ExpectedLumFP.y * 0.4;\n"
"    float exposure = max( 0.0, 1.0 + scaleCalc );\n"
"    vec3 exposed = color * exposure;\n"
"    float expectedLum = max( g_Effects_ExpectedLumFP.x, 0.01 );\n"
"    vec3 shoulder = max( exposed - vec3( expectedLum ), vec3( 0.0 ) );\n"
"    vec3 filmic = exposed / ( vec3( 1.0 ) + shoulder * 0.35 );\n"
"    float curveWeight = clamp( g_Effects_ExpectedLumFP.y * 0.75, 0.0, 1.0 );\n"
"    return mix( exposed, filmic, curveWeight );\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec3 LUM = vec3( 0.35, 0.45, 0.20 );\n"
"    vec4 sample = texture2D( texSampler0, tc );\n"
"    vec4 avgLum = clamp( texture2D( texSampler1, vec2( 0.5, 0.0 ) ), 0.01, 1.0 );\n"
"    vec4 blurred = texture2D( texSampler2, tc );\n"
"    vec3 bloomSource = blurred.rgb;\n"
"    vec4 blurredTonemapped;\n"
"\n"
"    if ( g_Effects_PresentationFP.x > 0.5 )\n"
"    {\n"
"        sample.rgb = applyModernPresentation( sample.rgb, avgLum.x );\n"
"        blurred.rgb = applyModernPresentation( blurred.rgb, avgLum.x );\n"
"        blurredTonemapped = blurred;\n"
"    }\n"
"    else\n"
"    {\n"
"        // utilBlurredToneMap (DO_TONEMAP, scaleScale = 0.4)\n"
"        float scaleCalc = ( g_Effects_ExpectedLumFP.x - avgLum.x ) * g_Effects_ExpectedLumFP.y * 0.4;\n"
"        sample += sample * abs( scaleCalc );\n"
"        sample += scaleCalc * (( scaleCalc < 0.0 ) ? 1.0 : 0.0 );\n"
"        blurredTonemapped = ( blurred * abs( scaleCalc ) ) + blurred;\n"
"        blurredTonemapped += scaleCalc;\n"
"    }\n"
"\n"
"    if ( g_Effects_PresentationFP.y > 0.5 )\n"
"    {\n"
"        // Modern Bloom v1: scalar soft-knee extraction from the original\n"
"        // blurred scene, then hue/chroma-preserving RGB contribution.\n"
"        const float bloomThreshold = 0.62;\n"
"        const float bloomKnee = 0.22;\n"
"        float sourceLum = dot( max( bloomSource, vec3( 0.0 ) ), LUM );\n"
"        float soft = clamp( sourceLum - bloomThreshold + bloomKnee, 0.0, 2.0 * bloomKnee );\n"
"        float bloomEnergy = ( soft * soft ) / ( 4.0 * bloomKnee );\n"
"        bloomEnergy = max( bloomEnergy, max( sourceLum - bloomThreshold, 0.0 ) );\n"
"        float bloomGate = clamp( ( bloomEnergy / max( sourceLum, 0.001 ) ) * 2.50, 0.0, 1.0 );\n"
"        float bloomWeight = clamp( g_Effects_ExpectedLumFP.z, 0.0, 1.0 );\n"
"        vec3 glow = blurredTonemapped.rgb * bloomGate * bloomWeight;\n"
"        sample.rgb += glow * max( vec3( 0.0 ), vec3( 1.0 ) - sample.rgb );\n"
"    }\n"
"    else\n"
"    {\n"
"        // utilBloomGlow (blur = 1.5, glow weight = ExpectedLum.z)\n"
"        vec3 blurBiased = ( bloomSource * 3.0 ) - 2.0;\n"
"        float blurBloom = clamp( 1.5 * dot( blurBiased, LUM ), 0.0, 1.0 );\n"
"        blurBloom = clamp( blurBloom * g_Effects_ExpectedLumFP.z, 0.0, 1.0 );\n"
"        sample = max( sample, mix( sample, blurredTonemapped, blurBloom ) );\n"
"    }\n"
"\n"
"    gl_FragColor = vec4( sample.rgb, 1.0 );\n"
"}\n";

static const char s_fxTonemap2DesatFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0 (frame)\n"
"uniform sampler2D texSampler1;        // TEXUNIT1 (adapted avgLum, 1x1)\n"
"uniform sampler2D texSampler2;        // TEXUNIT2 (blurred)\n"
"uniform vec4 g_Effects_ExpectedLumFP;\n"
"uniform vec4 g_Effects_PresentationFP; // .x presentation, .y modern bloom enable\n"
"uniform vec4 g_Effects_DesaturateParamFP; // .x desaturate amount\n"
"\n"
"vec3 applyModernPresentation( vec3 color, float avgLum )\n"
"{\n"
"    float scaleCalc = ( g_Effects_ExpectedLumFP.x - avgLum ) * g_Effects_ExpectedLumFP.y * 0.4;\n"
"    float exposure = max( 0.0, 1.0 + scaleCalc );\n"
"    vec3 exposed = color * exposure;\n"
"    float expectedLum = max( g_Effects_ExpectedLumFP.x, 0.01 );\n"
"    vec3 shoulder = max( exposed - vec3( expectedLum ), vec3( 0.0 ) );\n"
"    vec3 filmic = exposed / ( vec3( 1.0 ) + shoulder * 0.35 );\n"
"    float curveWeight = clamp( g_Effects_ExpectedLumFP.y * 0.75, 0.0, 1.0 );\n"
"    return mix( exposed, filmic, curveWeight );\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec3 LUM = vec3( 0.35, 0.45, 0.20 );\n"
"    vec4 sample = texture2D( texSampler0, tc );\n"
"    vec4 avgLum = clamp( texture2D( texSampler1, vec2( 0.5, 0.0 ) ), 0.01, 1.0 );\n"
"    vec4 blurred = texture2D( texSampler2, tc );\n"
"    vec3 bloomSource = blurred.rgb;\n"
"    vec4 blurredTonemapped;\n"
"\n"
"    if ( g_Effects_PresentationFP.x > 0.5 )\n"
"    {\n"
"        sample.rgb = applyModernPresentation( sample.rgb, avgLum.x );\n"
"        blurred.rgb = applyModernPresentation( blurred.rgb, avgLum.x );\n"
"        blurredTonemapped = blurred;\n"
"    }\n"
"    else\n"
"    {\n"
"        float scaleCalc = ( g_Effects_ExpectedLumFP.x - avgLum.x ) * g_Effects_ExpectedLumFP.y * 0.4;\n"
"        sample += sample * abs( scaleCalc );\n"
"        sample += scaleCalc * (( scaleCalc < 0.0 ) ? 1.0 : 0.0 );\n"
"        blurredTonemapped = ( blurred * abs( scaleCalc ) ) + blurred;\n"
"        blurredTonemapped += scaleCalc;\n"
"    }\n"
"\n"
"    if ( g_Effects_PresentationFP.y > 0.5 )\n"
"    {\n"
"        const float bloomThreshold = 0.62;\n"
"        const float bloomKnee = 0.22;\n"
"        float sourceLum = dot( max( bloomSource, vec3( 0.0 ) ), LUM );\n"
"        float soft = clamp( sourceLum - bloomThreshold + bloomKnee, 0.0, 2.0 * bloomKnee );\n"
"        float bloomEnergy = ( soft * soft ) / ( 4.0 * bloomKnee );\n"
"        bloomEnergy = max( bloomEnergy, max( sourceLum - bloomThreshold, 0.0 ) );\n"
"        float bloomGate = clamp( ( bloomEnergy / max( sourceLum, 0.001 ) ) * 2.50, 0.0, 1.0 );\n"
"        float bloomWeight = clamp( g_Effects_ExpectedLumFP.z, 0.0, 1.0 );\n"
"        vec3 glow = blurredTonemapped.rgb * bloomGate * bloomWeight;\n"
"        sample.rgb += glow * max( vec3( 0.0 ), vec3( 1.0 ) - sample.rgb );\n"
"    }\n"
"    else\n"
"    {\n"
"        vec3 blurBiased = ( bloomSource * 3.0 ) - 2.0;\n"
"        float blurBloom = clamp( 1.5 * dot( blurBiased, LUM ), 0.0, 1.0 );\n"
"        blurBloom = clamp( blurBloom * g_Effects_ExpectedLumFP.z, 0.0, 1.0 );\n"
"        sample = max( sample, mix( sample, blurredTonemapped, blurBloom ) );\n"
"    }\n"
"\n"
"    // DoDesaturate\n"
"    vec4 desat = vec4( 1.0, 0.91, 0.65, 0.0 ) * dot( sample.rgb, LUM );\n"
"    sample = mix( sample, desat, g_Effects_DesaturateParamFP.x );\n"
"\n"
"    gl_FragColor = vec4( sample.rgb, 1.0 );\n"
"}\n";

// dofFinalfp.cg (SHADER_DOF_FINAL, optional DESATURATE): depth-of-field
// composite — distance from the depth buffer through the 1D blur lookup,
// lerping frame vs blurred.
static const char s_fxDofFinalFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0 (frame)\n"
"uniform sampler2D texSampler2;        // TEXUNIT2 (blurred)\n"
"uniform sampler2D texSampler3;        // TEXUNIT3 (depth)\n"
"uniform sampler1D texSampler4;        // TEXUNIT4 (blur-distance lookup)\n"
"uniform vec4 g_Effects_DofParam2FP;   // .x minDistance .z 1/range\n"
"uniform vec4 g_Effects_DofProjectFP;  // projection terms\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec4 sample = texture2D( texSampler0, tc );\n"
"    vec4 blurred = texture2D( texSampler2, tc );\n"
"    float depth = texture2D( texSampler3, tc ).x;\n"
"\n"
"    // utilDistanceFromDepth\n"
"    float distXY = ( g_Effects_DofProjectFP.x * depth ) - g_Effects_DofProjectFP.y;\n"
"    float distZW = ( g_Effects_DofProjectFP.z * depth ) - g_Effects_DofProjectFP.w;\n"
"    float scaled = (( distXY / distZW ) - g_Effects_DofParam2FP.x ) * g_Effects_DofParam2FP.z;\n"
"    float distance = texture1D( texSampler4, scaled ).x;\n"
"\n"
"    vec4 lerpSample = mix( sample, blurred, distance );\n"
"    gl_FragColor = vec4( lerpSample.rgb, 1.0 );\n"
"}\n";

static const char s_fxDofFinalDesatFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0 (frame)\n"
"uniform sampler2D texSampler2;        // TEXUNIT2 (blurred)\n"
"uniform sampler2D texSampler3;        // TEXUNIT3 (depth)\n"
"uniform sampler1D texSampler4;        // TEXUNIT4 (blur-distance lookup)\n"
"uniform vec4 g_Effects_DofParam2FP;\n"
"uniform vec4 g_Effects_DofProjectFP;\n"
"uniform vec4 g_Effects_DesaturateParamFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec3 LUM = vec3( 0.35, 0.45, 0.20 );\n"
"    vec4 sample = texture2D( texSampler0, tc );\n"
"    vec4 blurred = texture2D( texSampler2, tc );\n"
"    float depth = texture2D( texSampler3, tc ).x;\n"
"\n"
"    float distXY = ( g_Effects_DofProjectFP.x * depth ) - g_Effects_DofProjectFP.y;\n"
"    float distZW = ( g_Effects_DofProjectFP.z * depth ) - g_Effects_DofProjectFP.w;\n"
"    float scaled = (( distXY / distZW ) - g_Effects_DofParam2FP.x ) * g_Effects_DofParam2FP.z;\n"
"    float distance = texture1D( texSampler4, scaled ).x;\n"
"\n"
"    vec4 lerpSample = mix( sample, blurred, distance );\n"
"\n"
"    vec4 desat = vec4( 1.0, 0.91, 0.65, 0.0 ) * dot( lerpSample.rgb, LUM );\n"
"    lerpSample = mix( lerpSample, desat, g_Effects_DesaturateParamFP.x );\n"
"\n"
"    gl_FragColor = vec4( lerpSample.rgb, 1.0 );\n"
"}\n";

// dofBloomFinalfp.cg (SHADER_DOF_BLOOM_FINAL, optional DESATURATE): DOF
// composite followed by the tonemap2 bloom chain (applied to the lerped
// sample, as in the Cg).
static const char s_fxDofBloomFinalFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0 (frame)\n"
"uniform sampler2D texSampler1;        // TEXUNIT1 (adapted avgLum, 1x1)\n"
"uniform sampler2D texSampler2;        // TEXUNIT2 (blurred)\n"
"uniform sampler2D texSampler3;        // TEXUNIT3 (depth)\n"
"uniform sampler1D texSampler4;        // TEXUNIT4 (blur-distance lookup)\n"
"uniform vec4 g_Effects_ExpectedLumFP;\n"
"uniform vec4 g_Effects_PresentationFP; // .x presentation, .y modern bloom enable\n"
"uniform vec4 g_Effects_DofParam2FP;\n"
"uniform vec4 g_Effects_DofProjectFP;\n"
"\n"
"vec3 applyModernPresentation( vec3 color, float avgLum )\n"
"{\n"
"    float scaleCalc = ( g_Effects_ExpectedLumFP.x - avgLum ) * g_Effects_ExpectedLumFP.y * 0.4;\n"
"    float exposure = max( 0.0, 1.0 + scaleCalc );\n"
"    vec3 exposed = color * exposure;\n"
"    float expectedLum = max( g_Effects_ExpectedLumFP.x, 0.01 );\n"
"    vec3 shoulder = max( exposed - vec3( expectedLum ), vec3( 0.0 ) );\n"
"    vec3 filmic = exposed / ( vec3( 1.0 ) + shoulder * 0.35 );\n"
"    float curveWeight = clamp( g_Effects_ExpectedLumFP.y * 0.75, 0.0, 1.0 );\n"
"    return mix( exposed, filmic, curveWeight );\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec3 LUM = vec3( 0.35, 0.45, 0.20 );\n"
"    vec4 sample = texture2D( texSampler0, tc );\n"
"    vec4 avgLum = clamp( texture2D( texSampler1, vec2( 0.5, 0.0 ) ), 0.01, 1.0 );\n"
"    vec4 blurred = texture2D( texSampler2, tc );\n"
"    float depth = texture2D( texSampler3, tc ).x;\n"
"\n"
"    float distXY = ( g_Effects_DofProjectFP.x * depth ) - g_Effects_DofProjectFP.y;\n"
"    float distZW = ( g_Effects_DofProjectFP.z * depth ) - g_Effects_DofProjectFP.w;\n"
"    float scaled = (( distXY / distZW ) - g_Effects_DofParam2FP.x ) * g_Effects_DofParam2FP.z;\n"
"    float dist = texture1D( texSampler4, scaled ).x;\n"
"    sample = mix( sample, blurred, dist );\n"
"    vec3 bloomSource = blurred.rgb;\n"
"    vec4 blurredTonemapped;\n"
"\n"
"    if ( g_Effects_PresentationFP.x > 0.5 )\n"
"    {\n"
"        sample.rgb = applyModernPresentation( sample.rgb, avgLum.x );\n"
"        blurred.rgb = applyModernPresentation( blurred.rgb, avgLum.x );\n"
"        blurredTonemapped = blurred;\n"
"    }\n"
"    else\n"
"    {\n"
"        float scaleCalc = ( g_Effects_ExpectedLumFP.x - avgLum.x ) * g_Effects_ExpectedLumFP.y * 0.4;\n"
"        sample += sample * abs( scaleCalc );\n"
"        sample += scaleCalc * (( scaleCalc < 0.0 ) ? 1.0 : 0.0 );\n"
"        blurredTonemapped = ( blurred * abs( scaleCalc ) ) + blurred;\n"
"        blurredTonemapped += scaleCalc;\n"
"    }\n"
"\n"
"    if ( g_Effects_PresentationFP.y > 0.5 )\n"
"    {\n"
"        const float bloomThreshold = 0.62;\n"
"        const float bloomKnee = 0.22;\n"
"        float sourceLum = dot( max( bloomSource, vec3( 0.0 ) ), LUM );\n"
"        float soft = clamp( sourceLum - bloomThreshold + bloomKnee, 0.0, 2.0 * bloomKnee );\n"
"        float bloomEnergy = ( soft * soft ) / ( 4.0 * bloomKnee );\n"
"        bloomEnergy = max( bloomEnergy, max( sourceLum - bloomThreshold, 0.0 ) );\n"
"        float bloomGate = clamp( ( bloomEnergy / max( sourceLum, 0.001 ) ) * 2.50, 0.0, 1.0 );\n"
"        float bloomWeight = clamp( g_Effects_ExpectedLumFP.z, 0.0, 1.0 );\n"
"        vec3 glow = blurredTonemapped.rgb * bloomGate * bloomWeight;\n"
"        sample.rgb += glow * max( vec3( 0.0 ), vec3( 1.0 ) - sample.rgb );\n"
"    }\n"
"    else\n"
"    {\n"
"        vec3 blurBiased = ( bloomSource * 3.0 ) - 2.0;\n"
"        float blurBloom = clamp( 1.5 * dot( blurBiased, LUM ), 0.0, 1.0 );\n"
"        blurBloom = clamp( blurBloom * g_Effects_ExpectedLumFP.z, 0.0, 1.0 );\n"
"        sample = max( sample, mix( sample, blurredTonemapped, blurBloom ) );\n"
"    }\n"
"\n"
"    gl_FragColor = vec4( sample.rgb, 1.0 );\n"
"}\n";

static const char s_fxDofBloomFinalDesatFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0 (frame)\n"
"uniform sampler2D texSampler1;        // TEXUNIT1 (adapted avgLum, 1x1)\n"
"uniform sampler2D texSampler2;        // TEXUNIT2 (blurred)\n"
"uniform sampler2D texSampler3;        // TEXUNIT3 (depth)\n"
"uniform sampler1D texSampler4;        // TEXUNIT4 (blur-distance lookup)\n"
"uniform vec4 g_Effects_ExpectedLumFP;\n"
"uniform vec4 g_Effects_PresentationFP; // .x presentation, .y modern bloom enable\n"
"uniform vec4 g_Effects_DofParam2FP;\n"
"uniform vec4 g_Effects_DofProjectFP;\n"
"uniform vec4 g_Effects_DesaturateParamFP;\n"
"\n"
"vec3 applyModernPresentation( vec3 color, float avgLum )\n"
"{\n"
"    float scaleCalc = ( g_Effects_ExpectedLumFP.x - avgLum ) * g_Effects_ExpectedLumFP.y * 0.4;\n"
"    float exposure = max( 0.0, 1.0 + scaleCalc );\n"
"    vec3 exposed = color * exposure;\n"
"    float expectedLum = max( g_Effects_ExpectedLumFP.x, 0.01 );\n"
"    vec3 shoulder = max( exposed - vec3( expectedLum ), vec3( 0.0 ) );\n"
"    vec3 filmic = exposed / ( vec3( 1.0 ) + shoulder * 0.35 );\n"
"    float curveWeight = clamp( g_Effects_ExpectedLumFP.y * 0.75, 0.0, 1.0 );\n"
"    return mix( exposed, filmic, curveWeight );\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tc = gl_TexCoord[0].xy;\n"
"    vec3 LUM = vec3( 0.35, 0.45, 0.20 );\n"
"    vec4 sample = texture2D( texSampler0, tc );\n"
"    vec4 avgLum = clamp( texture2D( texSampler1, vec2( 0.5, 0.0 ) ), 0.01, 1.0 );\n"
"    vec4 blurred = texture2D( texSampler2, tc );\n"
"    float depth = texture2D( texSampler3, tc ).x;\n"
"\n"
"    float distXY = ( g_Effects_DofProjectFP.x * depth ) - g_Effects_DofProjectFP.y;\n"
"    float distZW = ( g_Effects_DofProjectFP.z * depth ) - g_Effects_DofProjectFP.w;\n"
"    float scaled = (( distXY / distZW ) - g_Effects_DofParam2FP.x ) * g_Effects_DofParam2FP.z;\n"
"    float dist = texture1D( texSampler4, scaled ).x;\n"
"    sample = mix( sample, blurred, dist );\n"
"    vec3 bloomSource = blurred.rgb;\n"
"    vec4 blurredTonemapped;\n"
"\n"
"    if ( g_Effects_PresentationFP.x > 0.5 )\n"
"    {\n"
"        sample.rgb = applyModernPresentation( sample.rgb, avgLum.x );\n"
"        blurred.rgb = applyModernPresentation( blurred.rgb, avgLum.x );\n"
"        blurredTonemapped = blurred;\n"
"    }\n"
"    else\n"
"    {\n"
"        float scaleCalc = ( g_Effects_ExpectedLumFP.x - avgLum.x ) * g_Effects_ExpectedLumFP.y * 0.4;\n"
"        sample += sample * abs( scaleCalc );\n"
"        sample += scaleCalc * (( scaleCalc < 0.0 ) ? 1.0 : 0.0 );\n"
"        blurredTonemapped = ( blurred * abs( scaleCalc ) ) + blurred;\n"
"        blurredTonemapped += scaleCalc;\n"
"    }\n"
"\n"
"    if ( g_Effects_PresentationFP.y > 0.5 )\n"
"    {\n"
"        const float bloomThreshold = 0.62;\n"
"        const float bloomKnee = 0.22;\n"
"        float sourceLum = dot( max( bloomSource, vec3( 0.0 ) ), LUM );\n"
"        float soft = clamp( sourceLum - bloomThreshold + bloomKnee, 0.0, 2.0 * bloomKnee );\n"
"        float bloomEnergy = ( soft * soft ) / ( 4.0 * bloomKnee );\n"
"        bloomEnergy = max( bloomEnergy, max( sourceLum - bloomThreshold, 0.0 ) );\n"
"        float bloomGate = clamp( ( bloomEnergy / max( sourceLum, 0.001 ) ) * 2.50, 0.0, 1.0 );\n"
"        float bloomWeight = clamp( g_Effects_ExpectedLumFP.z, 0.0, 1.0 );\n"
"        vec3 glow = blurredTonemapped.rgb * bloomGate * bloomWeight;\n"
"        sample.rgb += glow * max( vec3( 0.0 ), vec3( 1.0 ) - sample.rgb );\n"
"    }\n"
"    else\n"
"    {\n"
"        vec3 blurBiased = ( bloomSource * 3.0 ) - 2.0;\n"
"        float blurBloom = clamp( 1.5 * dot( blurBiased, LUM ), 0.0, 1.0 );\n"
"        blurBloom = clamp( blurBloom * g_Effects_ExpectedLumFP.z, 0.0, 1.0 );\n"
"        sample = max( sample, mix( sample, blurredTonemapped, blurBloom ) );\n"
"    }\n"
"\n"
"    vec4 desat = vec4( 1.0, 0.91, 0.65, 0.0 ) * dot( sample.rgb, LUM );\n"
"    sample = mix( sample, desat, g_Effects_DesaturateParamFP.x );\n"
"\n"
"    gl_FragColor = vec4( sample.rgb, 1.0 );\n"
"}\n";

// simple_desaturatefp.cg (SHADER_SIMPLE_DESATURATE): desaturate-only final.
static const char s_fxSimpleDesaturateFragmentSource[] =
"#version 120\n"
"\n"
"uniform sampler2D texSampler0;        // TEXUNIT0\n"
"uniform vec4 g_Effects_DesaturateParamFP;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 LUM = vec3( 0.35, 0.45, 0.20 );\n"
"    vec4 sample = texture2D( texSampler0, gl_TexCoord[0].xy );\n"
"    vec4 desat = vec4( 1.0, 0.91, 0.65, 0.0 ) * dot( sample.rgb, LUM );\n"
"    sample = mix( sample, desat, g_Effects_DesaturateParamFP.x );\n"
"    gl_FragColor = vec4( sample.rgb, 1.0 );\n"
"}\n";

// variants.cgh values for g_VertexLitMode
enum {
    kPilotVertLit_VertColor = 1,
    kPilotVertLit_FF_Lit    = 4,
    kPilotVertLit_FF_Unlit  = 5,
};

typedef struct tPilotVertexEntry
{
    GLuint                pgmId;
    tPilotVertexKind    kind;
    int                    vertexLitMode;    // variants.cgh VERTEX_LIT (NONE=0 for bump_dual)
} tPilotVertexEntry;

typedef struct tPilotSampler
{
    const char*    name;                // GLSL sampler uniform name
    int                unit;                // fixed texture unit (TEXUNITn)
} tPilotSampler;

#define kPilotMaxBoneVec4s 48    // matches g_BoneMatrixArrVP (16 bones x 3 rows)

typedef struct tPilotMaterial
{
    GLuint                arbFragmentId;        // target ARB/Cg program id (0 = none)
    const char*            name;                // for logging
    const char*            fragmentSource;      // GLSL fragment source
    const tPilotSampler* samplers;            // NULL-terminated sampler/unit map
    bool                usesEnv0;            // fragment consumes the g_Env0FP mirror
    bool                usesEnv1;            // fragment consumes the g_Env1FP mirror
    bool                usesGlowParam;        // fragment consumes the g_GlowParamFP mirror
    bool                usesBumpConstants;    // fragment consumes the bump lighting mirrors
    bool                usesWaterConstants;    // fragment consumes the water/multitex mirrors
    unsigned            vertexKindMask;        // ePilotVertexKind bits this material pairs with
    const char*            vertexSource;        // GLSL vertex source for those variants
    GLuint                program;                // compiled GLSL program (0 = not yet)
    GLint                locReflectionParam;
    GLint                locVertexLitMode;
    GLint                locEnv0;
    GLint                locEnv1;
    GLint                locGlowParam;
    GLint                locLightDir;            // g_LightDirVP (bump vertex shader)
    GLint                locLightDirFP;            // g_LightDirFP (HQ bump fragment shader)
    GLint                locAmbient;            // g_AmbientColorFP (bump fragment)
    GLint                locDiffuse;            // g_DiffuseColorFP (bump fragment)
    GLint                locGloss;                // g_GlossParamFP (bump fragment)
    GLint                locSpecular1;            // g_Specular1ColorAndExponentFP (bump fragment)
    GLint                locSkinned;            // g_Skinned (bump vertex shader skinning switch)
    GLint                locBoneMatrices;        // g_BoneMatrixArrVP[0] (bump vertex shader)
    GLint                locTexScroll0;        // g_TexScroll0VP (model-space bump vertex)
    GLint                locTexScroll1;        // g_TexScroll1VP (model-space bump vertex)
    GLint                locAmbientVP;            // g_AmbientParameterVP (VERTEX_LIT=DIFFUSE)
    GLint                locDiffuseVP;            // g_DiffuseParameterVP (VERTEX_LIT=DIFFUSE)
    GLint                locPrelit;                // g_Prelit (model-space bump: PRELIT vs DIFFUSE)
    GLint                locConstColor0FP;        // g_ConstColor0FP (water fragment)
    GLint                locConstColor1FP;        // g_ConstColor1FP (water fragment)
    GLint                locWaterRefractTransform;// g_WaterRefractionTransformFP (water fragment)
    GLint                locWaterRefractParams;    // g_WaterRefractionParamsFP (water fragment)
    GLint                locBumpMultiFlags;        // g_BumpMultiFlagsFP (water fragment)
    GLint                locScrollScaleArr;        // g_ScrollScaleArrFP[0] (water fragment)
    unsigned            fxConstMask;            // kPilotFxConst bits (effects materials)
    GLint                locFx[kPilotFxConst_Count]; // effects constant uniform locations
    GLuint                programFF;                // effects: program linked with the fixed-function vertex shader
    GLint                locFxFor[kPilotFxConst_Count]; // effects constants on programFF
    bool                failedFF;
    bool                activeFF;                // which program object is in use (effects materials)
    bool                failed;
    bool                activationLogged;    // one-time activation evidence for logs
    bool                declineLogged;        // one-time unregistered-vertex diagnostic
    // multi9 (BLENDMODE_MULTI) constants: the selector flags, scroll/scale
    // array and 'lights on' glow params all five variants consume, plus the
    // material-2 set (tint colors + specular2) the dual-material variants
    // consume. Appended fields so the existing positional initializers stay
    // valid; the location fields are assigned by pilotInit before any push
    // (guarded by usesMultiConstants, which only the multi9 materials set).
    bool                usesMultiConstants;
    bool                usesMultiMat2;        // material-2 tints + specular2
    GLint                locSpecular2;        // g_Specular2ColorAndExponentFP (multi9 fragment)
    GLint                locMiscParam;        // g_MiscParamFP (multi9 fragment)
    // water BIT_SHADOWMAP constants. Appended so the existing positional
    // initializers remain valid; the shadow-water row enables this block in
    // pilotInit before resolving its locations.
    bool                usesShadowConstants;
    GLint               locUseShadowMap;
    GLint               locShadowFilterMode;
    GLint               locShadowMap[4];
    GLint               locShadowParams;
    GLint               locShadowSplits;
    GLint               locShadowParams2;
    GLint               locShadowParams3;
    // water BIT_PLANAR_REFLECTION constants. Appended so the existing
    // positional material initializers remain valid; the planar row enables
    // this block in pilotInit before resolving its locations.
    bool                usesWaterReflectionConstants;
    GLint               locWaterReflectionTransform;
    GLint               locWaterReflectionParams;
    GLint               locWaterFresnelParams;
    GLint               locUsePlanarReflection;
} tPilotMaterial;

#define kPilotKindBit( kind ) ( 1u << (kind) )
#define kPilotBumpKindMask ( kPilotKindBit( kPilotVertexKind_BumpDual ) | \
                             kPilotKindBit( kPilotVertexKind_SkinBump ) )
#define kPilotBumpHQKindMask ( kPilotKindBit( kPilotVertexKind_BumpDualHQ ) | \
                               kPilotKindBit( kPilotVertexKind_SkinBumpHQ ) )
#define kPilotBumpModelKindMask ( kPilotKindBit( kPilotVertexKind_BumpNormals ) | \
                                  kPilotKindBit( kPilotVertexKind_BumpRGBS ) )
#define kPilotBumpMultiKindMask ( kPilotKindBit( kPilotVertexKind_BumpMulti ) )
#define kPilotBumpMultiHQKindMask ( kPilotKindBit( kPilotVertexKind_BumpMultiHQ ) )

#define kPilotMaxVertexEntries 16

static tPilotVertexEntry    s_vertexEntries[kPilotMaxVertexEntries];
static int                    s_vertexEntryCount = 0;

// sampler names mirror the Cg sources; units are the TEXUNITn semantics
static const tPilotSampler s_dualSamplers[] = {
    { "sampler_base",  0 },
    { "sampler_blend", 1 },
    { NULL, -1 },
};

static const tPilotSampler s_dualTintSamplers[] = {
    { "sampler_base", 0 },
    { "sampler_dual", 1 },
    { NULL, -1 },
};

static const tPilotSampler s_addGlowSamplers[] = {
    { "sampler_base",      0 },
    { "sampler_glow",      1 },
    { "sampler_glow_mask", 2 },
    { NULL, -1 },
};

static const tPilotSampler s_bumpDualTintSamplers[] = {
    { "sampler_base_1",              0 },
    { "sampler_dual_1",              1 },
    { "sampler_normal_and_gloss_1",  2 },
    { NULL, -1 },
};

static const tPilotSampler s_bumpMultiplySamplers[] = {
    { "sampler_base",         0 },
    { "sampler_blend",        1 },
    { "sampler_normal_gloss", 2 },
    { NULL, -1 },
};

// water sampler/unit map (names mirror waterfp.cg; unit 3 is the water
// reflection pbuffer, units 5/6 are the refraction pbuffer color/depth bound
// by rt_water.c, and unit 7 is the second scrolling normal map)
static const tPilotSampler s_waterSamplers[] = {
    { "sampler_base_1",              0 },
    { "sampler_multiply_1",          1 },
    { "sampler_normal_and_gloss_1",  2 },
    { "sampler_reflection",           3 },
    { "sampler_refraction",          5 },
    { "sampler_refraction_depth",    6 },
    { "sampler_normal_and_gloss_2",  7 },
    { "sampler_shadow",              11 },
    { NULL, -1 },
};

// multi9 sampler/unit map (names mirror multi9fp.cg; unit 15 is the
// non-standard glow-mask texture layer)
static const tPilotSampler s_multi9Samplers[] = {
    { "sampler_base_1",              0 },
    { "sampler_multiply_1",          1 },
    { "sampler_normal_and_gloss_1",  2 },
    { "sampler_dual_1",              3 },
    { "sampler_mask",                4 },
    { "sampler_base_2",              5 },
    { "sampler_multiply_2",          6 },
    { "sampler_normal_and_gloss_2",  7 },
    { "sampler_dual_2",              8 },
    { "sampler_glow",                9 },
    { "sampler_glow_mask",           15 },
    { NULL, -1 },
};

// effects sampler/unit maps (names mirror the Cg sources; texSampler4 is
// the 1D blur-distance lookup on unit 4)
static const tPilotSampler s_fxTex0Samplers[] = {
    { "texSampler0", 0 },
    { NULL, -1 },
};

static const tPilotSampler s_fxTex01Samplers[] = {
    { "texSampler0", 0 },
    { "texSampler1", 1 },
    { NULL, -1 },
};

static const tPilotSampler s_fxTex012Samplers[] = {
    { "texSampler0", 0 },
    { "texSampler1", 1 },
    { "texSampler2", 2 },
    { NULL, -1 },
};

static const tPilotSampler s_fxDofSamplers[] = {
    { "texSampler0", 0 },
    { "texSampler2", 2 },
    { "texSampler3", 3 },
    { "texSampler4", 4 },
    { NULL, -1 },
};

static const tPilotSampler s_fxDofBloomSamplers[] = {
    { "texSampler0", 0 },
    { "texSampler1", 1 },
    { "texSampler2", 2 },
    { "texSampler3", 3 },
    { "texSampler4", 4 },
    { NULL, -1 },
};

// field order: fragment id, name, fragment source, samplers, env0, env1,
// glow, bump constants, vertex kind mask, vertex source, program, locs...,
// fx const mask, fx locs, failed, activationLogged, declineLogged
#define kFxBit( c ) ( 1u << (c) )
// effects materials pair with the dualtex vertex kind (the 2D setup's
// DRAWMODE_SPRITE program, for the final pass) and with the fixed-function
// "no vertex program" pairing (the pbuffer passes)
#define kPilotFxKindMask ( kPilotKindBit( kPilotVertexKind_DualTex ) | \
                           kPilotKindBit( kPilotVertexKind_FixedFunction ) )
static tPilotMaterial s_materials[kPilotMaterial_Count] = {
    { 0, "BLENDMODE_MODULATE",           s_modulateFragmentSource,           s_dualSamplers,       false, false, false, false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "BLENDMODE_MULTIPLY",           s_multiplyFragmentSource,           s_dualSamplers,       true,  false, false, false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "BLENDMODE_COLORBLEND_DUAL",    s_colorBlendDualFragmentSource,     s_dualTintSamplers,   true,  true,  false, false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "BLENDMODE_ADDGLOW",            s_addGlowFragmentSource,            s_addGlowSamplers,    true,  false, true,  false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "BLENDMODE_ALPHADETAIL",        s_alphaDetailFragmentSource,        s_dualSamplers,       true,  false, false, false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "BLENDMODE_BUMPMAP_COLORBLEND_DUAL", s_bumpColorBlendDualFragmentSource, s_bumpDualTintSamplers, true, true, false, true, false, kPilotBumpKindMask, s_bumpDualVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "BLENDMODE_BUMPMAP_COLORBLEND_DUAL_HQ", s_bumpColorBlendDualHQFragmentSource, s_bumpDualTintSamplers, true, true, false, true, false, kPilotBumpHQKindMask, s_bumpDualHQVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    // the model-space bump material (the water-surface fallback); pairs with
    // the bump.vp (DIFFUSE) and bump_rgb.vp (PRELIT) vertex variants through
    // the model-space bump vertex shader, mode-switched by g_Prelit
    { 0, "BLENDMODE_BUMPMAP_MULTIPLY", s_bumpMultiplyFragmentSource, s_bumpMultiplySamplers, true, false, false, false, false, kPilotBumpModelKindMask, s_bumpModelVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    // the fancy-water material (variant 0: refraction only); pairs with the
    // bump_dual_multi (FAUX_MULTI) static vertex variant, and consumes the
    // bump lighting constant set plus the water/multitex mirrors
    // (usesBumpConstants covers ambient/diffuse/gloss/specular1)
    { 0, "BLENDMODE_WATER", s_waterFragmentSource, s_waterSamplers, true, false, false, true, true, kPilotBumpMultiKindMask, s_bumpMultiVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    // the same material with BIT_SHADOWMAP. It shares the vertex program and
    // water/refraction constants; pilotInit additionally resolves the
    // cascaded-shadow uniforms and activates the shadow branch.
    { 0, "BLENDMODE_WATER_SHADOW", s_waterFragmentSource, s_waterSamplers, true, false, false, true, true, kPilotBumpMultiKindMask, s_bumpMultiVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    // the same material with BIT_PLANAR_REFLECTION. It deliberately has no
    // shadow block: the combined planar+shadow permutation is registered as
    // a separate row below. The static bump_dual_multi vertex pairing is
    // unchanged.
    { 0, "BLENDMODE_WATER_PLANAR", s_waterFragmentSource, s_waterSamplers, true, false, false, true, true, kPilotBumpMultiKindMask, s_bumpMultiVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    // composition of the verified shadow and planar variants; both existing
    // state blocks are enabled by pilotInit for this row
    { 0, "BLENDMODE_WATER_SHADOW_PLANAR", s_waterFragmentSource, s_waterSamplers, true, false, false, true, true, kPilotBumpMultiKindMask, s_bumpMultiVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    // multi9 (BLENDMODE_MULTI): the five static-map variants; the LQ variants
    // pair with the PRELIT_WHITE FAUX_MULTI vertex variant (the RGBS baked-
    // lighting pairing stays on ARB), the HQ variants with its BIT_HIGH_QUALITY
    // sibling. usesBumpConstants covers the shared lighting constants; the
    // multi9 mirrors carry the selector flags, scroll/scale array, glow params
    // and (dual-material variants) the material-2 tints + specular2.
    { 0, "BLENDMODE_MULTI", s_multi9FullFragmentSource, s_multi9Samplers, true, true, false, true, false, kPilotBumpMultiKindMask, s_bumpMultiVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, { -1, -1, -1, -1, -1, -1 }, false, false, false, false, false, true, true, -1, -1 },
    { 0, "BLENDMODE_MULTI HQ", s_multi9FullHQFragmentSource, s_multi9Samplers, true, true, false, true, false, kPilotBumpMultiHQKindMask, s_bumpMultiHQVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, { -1, -1, -1, -1, -1, -1 }, false, false, false, false, false, true, true, -1, -1 },
    { 0, "BLENDMODE_MULTI single", s_multi9SingleFragmentSource, s_multi9Samplers, true, true, false, true, false, kPilotBumpMultiKindMask, s_bumpMultiVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, { -1, -1, -1, -1, -1, -1 }, false, false, false, false, false, true, false, -1, -1 },
    { 0, "BLENDMODE_MULTI single HQ", s_multi9SingleHQFragmentSource, s_multi9Samplers, true, true, false, true, false, kPilotBumpMultiHQKindMask, s_bumpMultiHQVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, { -1, -1, -1, -1, -1, -1 }, false, false, false, false, false, true, false, -1, -1 },
    { 0, "BLENDMODE_MULTI building", s_multi9BuildingFragmentSource, s_multi9Samplers, true, true, false, true, false, kPilotBumpMultiKindMask, s_bumpMultiVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, { -1, -1, -1, -1, -1, -1 }, false, false, false, false, false, true, false, -1, -1 },
    // effects/post-processing materials (see the sources above for the
    // per-pass math); they pair with the dualtex vertex kind because the 2D
    // rendering setup force-binds the DRAWMODE_SPRITE vertex variant
    { 0, "FX_SHRINK_EXTEND",         s_fxShrinkExtendFragmentSource,      s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_HBLUR",                 s_fxHBlurFragmentSource,             s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_VBLUR",                 s_fxVBlurFragmentSource,             s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_TONEMAP",               s_fxTonemapFragmentSource,           s_fxTex01Samplers,    false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_SHRINK2",               s_fxShrinkPlainFragmentSource,       s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_SHRINK2DOF",            s_fxShrinkPlainFragmentSource,       s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_SHRINK4",               s_fxShrink4FragmentSource,           s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_SHRINK4LUM",            s_fxShrink4LumFragmentSource,        s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_SHRINK4EXP",            s_fxShrink4ExpFragmentSource,        s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_LIGHTADAPTATION",       s_fxLightAdaptationFragmentSource,   s_fxTex01Samplers,    false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TimeStep ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_LOG",                   s_fxLogFragmentSource,               s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_TextTransform ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_BRIGHTPASS",            s_fxBrightpassFragmentSource,        s_fxTex01Samplers,    false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_ExpectedLum ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_TONEMAP2",              s_fxTonemap2FragmentSource,          s_fxTex012Samplers,   false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_ExpectedLum ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_TONEMAP2_DESATURATE",   s_fxTonemap2DesatFragmentSource,     s_fxTex012Samplers,   false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_ExpectedLum ) | kFxBit( kPilotFxConst_DesaturateParam ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_DOF_FINAL",             s_fxDofFinalFragmentSource,          s_fxDofSamplers,      false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_DofParam2 ) | kFxBit( kPilotFxConst_DofProject ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_DOF_FINAL_DESATURATE",  s_fxDofFinalDesatFragmentSource,     s_fxDofSamplers,      false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_DofParam2 ) | kFxBit( kPilotFxConst_DofProject ) | kFxBit( kPilotFxConst_DesaturateParam ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_DOF_BLOOM_FINAL",       s_fxDofBloomFinalFragmentSource,     s_fxDofBloomSamplers, false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_ExpectedLum ) | kFxBit( kPilotFxConst_DofParam2 ) | kFxBit( kPilotFxConst_DofProject ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_DOF_BLOOM_FINAL_DESATURATE", s_fxDofBloomFinalDesatFragmentSource, s_fxDofBloomSamplers, false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_ExpectedLum ) | kFxBit( kPilotFxConst_DofParam2 ) | kFxBit( kPilotFxConst_DofProject ) | kFxBit( kPilotFxConst_DesaturateParam ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
    { 0, "FX_SIMPLE_DESATURATE",     s_fxSimpleDesaturateFragmentSource,  s_fxTex0Samplers,     false, false, false, false, false, kPilotFxKindMask, s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, kFxBit( kPilotFxConst_DesaturateParam ), { -1, -1, -1, -1, -1, -1 }, false, false, false },
};

static int                    s_activeMaterial = -1;    // -1 = pilot inactive
// Only the two canonical Bump ColorBlendDual programs resolve this optional
// response uniform; other material families keep the existing table untouched.
static GLint                  s_modernMaterialParamLoc[kPilotMaterial_Count];
static GLuint                s_vertexShader = 0;        // shared: one source serves the dualtex materials
static bool                    s_vertexShaderFailed = false;
static GLuint                s_bumpVertexShader = 0;    // shared by bump-material programs (static + skinned)
static bool                    s_bumpVertexShaderFailed = false;
static GLuint                s_bumpHQVertexShader = 0;    // shared by HQ bump-material programs
static bool                    s_bumpHQVertexShaderFailed = false;
static GLuint                s_bumpModelVertexShader = 0;    // shared by model-space bump programs (bump.vp/bump_rgb.vp)
static bool                    s_bumpModelVertexShaderFailed = false;
static GLfloat                s_reflectionParamMirror[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat                s_envMirrors[2][4] = { { 1.0f, 1.0f, 1.0f, 1.0f },
                                                     { 1.0f, 1.0f, 1.0f, 1.0f } };
static GLfloat                s_glowParamMirror[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
static GLfloat                s_lightDirMirror[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
static GLfloat                s_lightDirFPMirror[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
static GLfloat                s_ambientMirror[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat                s_diffuseMirror[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat                s_glossMirror[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
static GLfloat                s_specular1Mirror[4] = { 1.0f, 1.0f, 1.0f, 8.0f };
static GLfloat                s_texScrollMirrors[2][4] = { { 0.0f, 0.0f, 0.0f, 0.0f },
                                                           { 0.0f, 0.0f, 0.0f, 0.0f } };
static GLfloat                s_ambientVPMirror[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat                s_diffuseVPMirror[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat                s_fxConstMirrors[kPilotFxConst_Count][4]; // ARB local params default to 0
static GLfloat                s_boneMatrixMirror[kPilotMaxBoneVec4s][4];
static GLuint                s_boneMatrixMirrorCount = 0;
// water/multitex fragment constant mirrors (env registers, engine pushes
// them per draw before use — zero defaults match the fx mirror precedent)
#define kPilotMaxScrollScaleVec4s 10    // g_ScrollScaleArrFP[10] (TEXLAYER_MAX_SCROLLABLE)
static GLfloat                s_waterConstMirrors[kPilotWaterConst_Count][4];
static GLfloat                s_scrollScaleMirror[kPilotMaxScrollScaleVec4s][4];
static GLfloat                s_shadowConstMirrors[kPilotShadowConst_Count][4][4];
// multi9 fragment constant mirrors (g_Specular2ColorAndExponentFP /
// g_MiscParamFP; see the header) — the material-2 tint colors and the
// selector flags/scroll array reuse the water mirrors above, since the
// engine pushes them from the same setupBumpMultiPixelShader path
static GLfloat                s_specular2Mirror[4] = { 1.0f, 1.0f, 1.0f, 8.0f };
static GLfloat                s_miscParamMirror[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

static GLuint pilotCompileShader( GLenum type, const char* source, const char* descr )
{
    GLuint shader = __glewCreateShader( type );
    GLint status = 0;
    if ( ! shader )
        return 0;
    __glewShaderSource( shader, 1, &source, NULL );
    __glewCompileShader( shader );
    __glewGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if ( ! status )
    {
        char infoLog[2048];
        infoLog[0] = '\0';
        __glewGetShaderInfoLog( shader, sizeof(infoLog) - 1, NULL, infoLog );
        printf( "GLSL pilot: %s compile failed:\n%s\n", descr, infoLog );
        __glewDeleteShader( shader );
        return 0;
    }
    return shader;
}

static GLuint pilotGetVertexShader( void )
{
    if ( s_vertexShader || s_vertexShaderFailed )
        return s_vertexShader;
    s_vertexShader = pilotCompileShader( GL_VERTEX_SHADER, s_pilotVertexSource, "vertex shader" );
    if ( ! s_vertexShader )
        s_vertexShaderFailed = true;
    return s_vertexShader;
}

static GLuint pilotGetBumpVertexShader( void )
{
    if ( s_bumpVertexShader || s_bumpVertexShaderFailed )
        return s_bumpVertexShader;
    s_bumpVertexShader = pilotCompileShader( GL_VERTEX_SHADER, s_bumpDualVertexSource, "bump_dual vertex shader" );
    if ( ! s_bumpVertexShader )
        s_bumpVertexShaderFailed = true;
    return s_bumpVertexShader;
}

static GLuint pilotGetBumpHQVertexShader( void )
{
    if ( s_bumpHQVertexShader || s_bumpHQVertexShaderFailed )
        return s_bumpHQVertexShader;
    s_bumpHQVertexShader = pilotCompileShader( GL_VERTEX_SHADER, s_bumpDualHQVertexSource, "bump_dual HQ vertex shader" );
    if ( ! s_bumpHQVertexShader )
        s_bumpHQVertexShaderFailed = true;
    return s_bumpHQVertexShader;
}

static GLuint pilotGetBumpModelVertexShader( void )
{
    if ( s_bumpModelVertexShader || s_bumpModelVertexShaderFailed )
        return s_bumpModelVertexShader;
    s_bumpModelVertexShader = pilotCompileShader( GL_VERTEX_SHADER, s_bumpModelVertexSource, "bump (model-space) vertex shader" );
    if ( ! s_bumpModelVertexShader )
        s_bumpModelVertexShaderFailed = true;
    return s_bumpModelVertexShader;
}

static GLuint s_bumpMultiVertexShader = 0;    // shared by the water program (bump_dual_multi)
static bool s_bumpMultiVertexShaderFailed = false;

static GLuint pilotGetBumpMultiVertexShader( void )
{
    if ( s_bumpMultiVertexShader || s_bumpMultiVertexShaderFailed )
        return s_bumpMultiVertexShader;
    s_bumpMultiVertexShader = pilotCompileShader( GL_VERTEX_SHADER, s_bumpMultiVertexSource, "bump_dual_multi vertex shader" );
    if ( ! s_bumpMultiVertexShader )
        s_bumpMultiVertexShaderFailed = true;
    return s_bumpMultiVertexShader;
}

static GLuint s_bumpMultiHQVertexShader = 0;    // shared by the HQ multi9 programs
static bool s_bumpMultiHQVertexShaderFailed = false;

static GLuint pilotGetBumpMultiHQVertexShader( void )
{
    if ( s_bumpMultiHQVertexShader || s_bumpMultiHQVertexShaderFailed )
        return s_bumpMultiHQVertexShader;
    s_bumpMultiHQVertexShader = pilotCompileShader( GL_VERTEX_SHADER, s_bumpMultiHQVertexSource, "bump_dual_multi HQ vertex shader" );
    if ( ! s_bumpMultiHQVertexShader )
        s_bumpMultiHQVertexShaderFailed = true;
    return s_bumpMultiHQVertexShader;
}

static GLuint s_ffVertexShader = 0;
static bool s_ffVertexShaderFailed = false;

static GLuint pilotGetFFVertexShader( void )
{
    if ( s_ffVertexShader || s_ffVertexShaderFailed )
        return s_ffVertexShader;
    s_ffVertexShader = pilotCompileShader( GL_VERTEX_SHADER, s_ffVertexSource, "fixed-function vertex shader" );
    if ( ! s_ffVertexShader )
        s_ffVertexShaderFailed = true;
    return s_ffVertexShader;
}

// Builds the effects material's second program object: same fragment source
// linked with the fixed-function vertex shader for the pbuffer passes that
// run with vertex programs disabled. Returns false on failure (the material
// still works for the dualtex pairing).
static bool pilotInitFF( int material )
{
    tPilotMaterial* m = &s_materials[material];
    GLuint vertexShader = pilotGetFFVertexShader();
    GLuint fragmentShader;
    GLint status = 0;

    if ( m->programFF )
        return true;

    m->programFF = __glewCreateProgram();
    if (( ! m->programFF ) || ( ! vertexShader ))
    {
        printf( "GLSL pilot: %s fixed-function program creation failed\n", m->name );
        m->failedFF = true;
        return false;
    }

    fragmentShader = pilotCompileShader( GL_FRAGMENT_SHADER, m->fragmentSource, m->name );
    if ( ! fragmentShader )
    {
        m->failedFF = true;
        return false;
    }

    __glewAttachShader( m->programFF, vertexShader );
    __glewAttachShader( m->programFF, fragmentShader );
    __glewDeleteShader( fragmentShader );

    __glewLinkProgram( m->programFF );
    __glewGetProgramiv( m->programFF, GL_LINK_STATUS, &status );
    if ( ! status )
    {
        char infoLog[2048];
        infoLog[0] = '\0';
        __glewGetProgramInfoLog( m->programFF, sizeof(infoLog) - 1, NULL, infoLog );
        printf( "GLSL pilot: %s fixed-function program link failed:\n%s\n", m->name, infoLog );
        m->failedFF = true;
        m->programFF = 0;
        return false;
    }

    {
        static const char* fxNames[kPilotFxConst_Count] = {
            "g_Effects_TextTransformFP",
            "g_Effects_ExpectedLumFP",
            "g_TimeStepFP",
            "g_Effects_DofParam2FP",
            "g_Effects_DofProjectFP",
            "g_Effects_DesaturateParamFP",
            "g_Effects_PresentationFP",
        };
        const tPilotSampler* sampler;
        int c;

        for ( c = 0; c < kPilotFxConst_Count; c++ )
            m->locFxFor[c] = -1;

        __glewUseProgram( m->programFF );
        for ( sampler = m->samplers; sampler->name; sampler++ )
            __glewUniform1i( __glewGetUniformLocation( m->programFF, sampler->name ), sampler->unit );
        __glewUseProgram( 0 );

        for ( c = 0; c < kPilotFxConst_Count; c++ )
        {
            if ( m->fxConstMask & kFxBit( c ) )
                m->locFxFor[c] = __glewGetUniformLocation( m->programFF, fxNames[c] );
            if (( m->fxConstMask & kFxBit( c )) && ( m->locFxFor[c] < 0 ))
            {
                printf( "GLSL pilot: %s fixed-function program uniforms missing\n", m->name );
                m->failedFF = true;
                m->programFF = 0;
                return false;
            }
        }
    }

    printf( "GLSL pilot: %s fixed-function program compiled and linked\n", m->name );
    return true;
}

static bool pilotInit( int material )
{
    tPilotMaterial* m = &s_materials[material];
    if (( material == kPilotMaterial_WaterShadow ) ||
        ( material == kPilotMaterial_WaterShadowPlanar ))
        m->usesShadowConstants = true;
    if (( material == kPilotMaterial_WaterPlanar ) ||
        ( material == kPilotMaterial_WaterShadowPlanar ))
        m->usesWaterReflectionConstants = true;
    bool isBumpLQ = ( m->vertexKindMask & kPilotBumpKindMask ) != 0;
    bool isBumpHQ = ( m->vertexKindMask & kPilotBumpHQKindMask ) != 0;
    bool isBumpModel = ( m->vertexKindMask & kPilotBumpModelKindMask ) != 0;
    bool isBumpMulti = ( m->vertexKindMask & kPilotBumpMultiKindMask ) != 0;
    bool isBumpMultiHQ = ( m->vertexKindMask & kPilotBumpMultiHQKindMask ) != 0;
    GLuint vertexShader = isBumpHQ ? pilotGetBumpHQVertexShader()
                      : isBumpModel ? pilotGetBumpModelVertexShader()
                      : isBumpMultiHQ ? pilotGetBumpMultiHQVertexShader()
                      : isBumpMulti ? pilotGetBumpMultiVertexShader()
                      : isBumpLQ ? pilotGetBumpVertexShader()
                                 : pilotGetVertexShader();
    GLuint fragmentShader;
    GLint status = 0;

    m->program = __glewCreateProgram();
    if (( ! m->program ) || ( ! vertexShader ))
    {
        printf( "GLSL pilot: %s program creation failed (GL 2.0 entry points missing?)\n", m->name );
        m->failed = true;
        return false;
    }

    // the engine feeds tangents on generic vertex attribute 7 and the skin
    // weights/indices on attributes 1 and 5 (glVertexAttribPointerARB in
    // rt_model.c / rt_bonedmodel.c); the model-space bump variant gets its
    // baked instance lighting on attribute 11; bind the GLSL attributes to
    // the same indices before linking
    if ( isBumpLQ || isBumpHQ )
    {
        __glewBindAttribLocation( m->program, 7, "attr_tangent" );
        __glewBindAttribLocation( m->program, 1, "attr_boneweights" );
        __glewBindAttribLocation( m->program, 5, "attr_boneindices" );
    }
    if ( isBumpModel )
    {
        __glewBindAttribLocation( m->program, 7, "attr_tangent" );
        __glewBindAttribLocation( m->program, 11, "attr_prelit_color" );
    }
    if ( isBumpMulti || isBumpMultiHQ )
    {
        // static geometry only: no bone attributes, no prelit color
        __glewBindAttribLocation( m->program, 7, "attr_tangent" );
    }

    fragmentShader = pilotCompileShader( GL_FRAGMENT_SHADER, m->fragmentSource, m->name );
    if ( ! fragmentShader )
    {
        m->failed = true;
        return false;
    }

    __glewAttachShader( m->program, vertexShader );
    __glewAttachShader( m->program, fragmentShader );
    __glewDeleteShader( fragmentShader );
    // the shared vertex shader objects are intentionally kept alive for the
    // other materials' programs

    __glewLinkProgram( m->program );
    __glewGetProgramiv( m->program, GL_LINK_STATUS, &status );
    if ( ! status )
    {
        char infoLog[2048];
        infoLog[0] = '\0';
        __glewGetProgramInfoLog( m->program, sizeof(infoLog) - 1, NULL, infoLog );
        printf( "GLSL pilot: %s program link failed:\n%s\n", m->name, infoLog );
        m->failed = true;
        return false;
    }

    m->locReflectionParam = __glewGetUniformLocation( m->program, "g_ReflectionParamVP" );
    m->locVertexLitMode = __glewGetUniformLocation( m->program, "g_VertexLitMode" );
    if ( m->usesEnv0 )
        m->locEnv0 = __glewGetUniformLocation( m->program, "g_Env0FP" );
    if ( m->usesEnv1 )
        m->locEnv1 = __glewGetUniformLocation( m->program, "g_Env1FP" );
    if ( m->usesGlowParam )
        m->locGlowParam = __glewGetUniformLocation( m->program, "g_GlowParamFP" );
    if ( isBumpLQ )
    {
        m->locLightDir = __glewGetUniformLocation( m->program, "g_LightDirVP" );
        m->locSkinned = __glewGetUniformLocation( m->program, "g_Skinned" );
        m->locBoneMatrices = __glewGetUniformLocation( m->program, "g_BoneMatrixArrVP[0]" );
    }
    if ( isBumpHQ )
    {
        m->locLightDirFP = __glewGetUniformLocation( m->program, "g_LightDirFP" );
        m->locSkinned = __glewGetUniformLocation( m->program, "g_Skinned" );
        m->locBoneMatrices = __glewGetUniformLocation( m->program, "g_BoneMatrixArrVP[0]" );
    }
    if ( isBumpModel )
    {
        // g_LightDirVP carries a model-space light POSITION for these draws;
        // the fragment consumes the specular1 constant even though the other
        // bump fragment constants are dead in this variant
        m->locLightDir = __glewGetUniformLocation( m->program, "g_LightDirVP" );
        m->locSpecular1 = __glewGetUniformLocation( m->program, "g_Specular1ColorAndExponentFP" );
        m->locTexScroll0 = __glewGetUniformLocation( m->program, "g_TexScroll0VP" );
        m->locTexScroll1 = __glewGetUniformLocation( m->program, "g_TexScroll1VP" );
        m->locAmbientVP = __glewGetUniformLocation( m->program, "g_AmbientParameterVP" );
        m->locDiffuseVP = __glewGetUniformLocation( m->program, "g_DiffuseParameterVP" );
        m->locPrelit = __glewGetUniformLocation( m->program, "g_Prelit" );
    }
    if ( isBumpMulti )
    {
        m->locLightDir = __glewGetUniformLocation( m->program, "g_LightDirVP" );
    }
    if ( isBumpMultiHQ )
    {
        // the HQ multi9 fragment reads g_LightDirFP instead of the
        // vertex-interpolated light vector
        m->locLightDirFP = __glewGetUniformLocation( m->program, "g_LightDirFP" );
    }
    if ( m->usesMultiConstants )
    {
        // the multi9 mirrors: selector flags, scroll/scale array and the
        // 'lights on' glow params for all five variants, plus the material-2
        // tint colors and specular2 for the dual-material variants
        m->locBumpMultiFlags = __glewGetUniformLocation( m->program, "g_BumpMultiFlagsFP" );
        m->locScrollScaleArr = __glewGetUniformLocation( m->program, "g_ScrollScaleArrFP[0]" );
        m->locMiscParam = __glewGetUniformLocation( m->program, "g_MiscParamFP" );
        if ( m->usesMultiMat2 )
        {
            m->locConstColor0FP = __glewGetUniformLocation( m->program, "g_ConstColor0FP" );
            m->locConstColor1FP = __glewGetUniformLocation( m->program, "g_ConstColor1FP" );
            m->locSpecular2 = __glewGetUniformLocation( m->program, "g_Specular2ColorAndExponentFP" );
        }
    }
    if ( m->usesWaterConstants )
    {
        // the water fragment consumes the bump lighting constants through
        // usesBumpConstants plus its own mirror set; the scroll/scale array
        // is one uniform location covering all ten layers
        m->locConstColor0FP = __glewGetUniformLocation( m->program, "g_ConstColor0FP" );
        m->locConstColor1FP = __glewGetUniformLocation( m->program, "g_ConstColor1FP" );
        m->locWaterRefractTransform = __glewGetUniformLocation( m->program, "g_WaterRefractionTransformFP" );
        m->locWaterRefractParams = __glewGetUniformLocation( m->program, "g_WaterRefractionParamsFP" );
        m->locBumpMultiFlags = __glewGetUniformLocation( m->program, "g_BumpMultiFlagsFP" );
        m->locScrollScaleArr = __glewGetUniformLocation( m->program, "g_ScrollScaleArrFP[0]" );
        if ( m->usesWaterReflectionConstants )
        {
            m->locWaterReflectionTransform = __glewGetUniformLocation( m->program, "g_WaterReflectionTransformFP" );
            m->locWaterReflectionParams = __glewGetUniformLocation( m->program, "g_WaterReflectionParamsFP" );
            m->locWaterFresnelParams = __glewGetUniformLocation( m->program, "g_WaterFresnelParamsFP" );
            m->locUsePlanarReflection = __glewGetUniformLocation( m->program, "g_UsePlanarReflection" );
        }
    }
    if ( m->usesShadowConstants )
    {
        m->locUseShadowMap = __glewGetUniformLocation( m->program, "g_UseShadowMap" );
        m->locShadowFilterMode = __glewGetUniformLocation( m->program, "g_ShadowFilterMode" );
        m->locShadowMap[0] = __glewGetUniformLocation( m->program, "g_ShadowMap1MatrixFP[0]" );
        m->locShadowMap[1] = __glewGetUniformLocation( m->program, "g_ShadowMap2MatrixFP[0]" );
        m->locShadowMap[2] = __glewGetUniformLocation( m->program, "g_ShadowMap3MatrixFP[0]" );
        m->locShadowMap[3] = __glewGetUniformLocation( m->program, "g_ShadowMap4MatrixFP[0]" );
        m->locShadowParams = __glewGetUniformLocation( m->program, "g_ShadowParamsFP" );
        m->locShadowSplits = __glewGetUniformLocation( m->program, "g_ShadowSplitsFP" );
        m->locShadowParams2 = __glewGetUniformLocation( m->program, "g_ShadowParams2FP" );
        m->locShadowParams3 = __glewGetUniformLocation( m->program, "g_ShadowParams3FP" );
    }
    if ( m->usesBumpConstants )
    {
        m->locAmbient = __glewGetUniformLocation( m->program, "g_AmbientColorFP" );
        m->locDiffuse = __glewGetUniformLocation( m->program, "g_DiffuseColorFP" );
        m->locGloss = __glewGetUniformLocation( m->program, "g_GlossParamFP" );
        m->locSpecular1 = __glewGetUniformLocation( m->program, "g_Specular1ColorAndExponentFP" );
    }
    if (( material == kPilotMaterial_BumpColorBlendDual ) ||
        ( material == kPilotMaterial_BumpColorBlendDualHQ ))
        s_modernMaterialParamLoc[material] = __glewGetUniformLocation( m->program, "g_ModernMaterialParamsFP" );
    if ( m->fxConstMask )
    {
        // effects constants are per-program locals in the Cg sources;
        // several share local slot numbers across different programs, so
        // they are keyed by constant identity here
        static const char* fxNames[kPilotFxConst_Count] = {
            "g_Effects_TextTransformFP",
            "g_Effects_ExpectedLumFP",
            "g_TimeStepFP",
            "g_Effects_DofParam2FP",
            "g_Effects_DofProjectFP",
            "g_Effects_DesaturateParamFP",
            "g_Effects_PresentationFP",
        };
        int c;
        for ( c = 0; c < kPilotFxConst_Count; c++ )
        {
            if ( m->fxConstMask & kFxBit( c ) )
                m->locFx[c] = __glewGetUniformLocation( m->program, fxNames[c] );
            else
                m->locFx[c] = -1;
        }
    }
    if (( ! isBumpLQ && ! isBumpHQ && ! isBumpModel && ! isBumpMulti && (( m->locReflectionParam < 0 ) || ( m->locVertexLitMode < 0 ))) ||
        ( isBumpLQ && (( m->locLightDir < 0 ) || ( m->locSkinned < 0 ) || ( m->locBoneMatrices < 0 ))) ||
        ( isBumpHQ && (( m->locLightDirFP < 0 ) || ( m->locSkinned < 0 ) || ( m->locBoneMatrices < 0 ))) ||
        ( isBumpModel && (( m->locLightDir < 0 ) || ( m->locSpecular1 < 0 ) || ( m->locTexScroll0 < 0 ) ||
                          ( m->locTexScroll1 < 0 ) || ( m->locAmbientVP < 0 ) || ( m->locDiffuseVP < 0 ) ||
                          ( m->locPrelit < 0 ))) ||
        ( isBumpMulti && ( m->locLightDir < 0 )) ||
        ( isBumpMultiHQ && ( m->locLightDirFP < 0 )) ||
        ( m->usesMultiConstants && (( m->locBumpMultiFlags < 0 ) || ( m->locScrollScaleArr < 0 ) ||
                                    ( m->locMiscParam < 0 ))) ||
        ( m->usesMultiMat2 && (( m->locConstColor0FP < 0 ) || ( m->locConstColor1FP < 0 ) ||
                              ( m->locSpecular2 < 0 ))) ||
        ( m->usesWaterConstants && (( m->locConstColor0FP < 0 ) || ( m->locConstColor1FP < 0 ) ||
                                    ( m->locWaterRefractTransform < 0 ) || ( m->locWaterRefractParams < 0 ) ||
                                    ( m->locBumpMultiFlags < 0 ) || ( m->locScrollScaleArr < 0 ))) ||
        ( m->usesWaterReflectionConstants && (( m->locWaterReflectionTransform < 0 ) ||
                                              ( m->locWaterReflectionParams < 0 ) ||
                                              ( m->locWaterFresnelParams < 0 ) ||
                                              ( m->locUsePlanarReflection < 0 ))) ||
        ( m->usesShadowConstants && (( m->locUseShadowMap < 0 ) || ( m->locShadowFilterMode < 0 ) ||
                                     ( m->locShadowMap[0] < 0 ) || ( m->locShadowMap[1] < 0 ) ||
                                     ( m->locShadowMap[2] < 0 ) || ( m->locShadowMap[3] < 0 ) ||
                                     ( m->locShadowParams < 0 ) || ( m->locShadowSplits < 0 ) ||
                                     ( m->locShadowParams2 < 0 ) || ( m->locShadowParams3 < 0 ))) ||
        ( m->usesEnv0 && ( m->locEnv0 < 0 )) ||
        ( m->usesEnv1 && ( m->locEnv1 < 0 )) ||
        ( m->usesGlowParam && ( m->locGlowParam < 0 )) ||
        ( m->usesBumpConstants && (( m->locAmbient < 0 ) || ( m->locDiffuse < 0 ) ||
                                    ( m->locGloss < 0 ) || ( m->locSpecular1 < 0 ))) ||
        ((( material == kPilotMaterial_BumpColorBlendDual ) ||
          ( material == kPilotMaterial_BumpColorBlendDualHQ )) &&
         ( s_modernMaterialParamLoc[material] < 0 )))
    {
        printf( "GLSL pilot: %s required uniforms optimized away or missing\n", m->name );
        m->failed = true;
        return false;
    }
    {
        // every masked fx constant must have resolved
        int c;
        for ( c = 0; c < kPilotFxConst_Count; c++ )
        {
            if (( m->fxConstMask & kFxBit( c )) && ( m->locFx[c] < 0 ))
            {
                printf( "GLSL pilot: %s required uniforms optimized away or missing\n", m->name );
                m->failed = true;
                return false;
            }
        }
    }

    // bind each sampler uniform to its fixed TEXUNITn; the engine binds the
    // textures themselves through the normal state machine either way
    __glewUseProgram( m->program );
    {
        const tPilotSampler* sampler;
        for ( sampler = m->samplers; sampler->name; sampler++ )
        {
            __glewUniform1i( __glewGetUniformLocation( m->program, sampler->name ), sampler->unit );
        }
    }
    __glewUseProgram( 0 );

    printf( "GLSL pilot: %s program compiled and linked\n", m->name );
    return true;
}

static void pilotDeactivate( void )
{
    if ( s_activeMaterial >= 0 )
    {
        __glewUseProgram( 0 );
        s_activeMaterial = -1;
    }
}

static int pilotFindVertexEntry( GLuint vertexPgmId )
{
    int i;
    // id 0 is a legitimate entry: the fixed-function pairing for the
    // pbuffer effects passes (only the post-reset sentinel is excluded)
    for ( i = 0; i < s_vertexEntryCount; i++ )
    {
        if ( s_vertexEntries[i].pgmId == vertexPgmId )
            return i;
    }
    return -1;
}

static int pilotFindMaterial( GLuint fragmentPgmId )
{
    int i;
    if ( ! fragmentPgmId )
        return -1;
    for ( i = 0; i < kPilotMaterial_Count; i++ )
    {
        if ( s_materials[i].arbFragmentId == fragmentPgmId )
            return i;
    }
    return -1;
}

// A registered vertex entry can drive the material only when it was built
// from one of the vp_master variants the material pairs with; the dualtex
// variants additionally need their nonzero VERTEX_LIT mode.
static bool pilotVertexEntryDrives( const tPilotVertexEntry* entry, const tPilotMaterial* m )
{
    if ( !( m->vertexKindMask & kPilotKindBit( entry->kind ) ))
        return false;
    if (( entry->kind == kPilotVertexKind_DualTex ) && ( ! entry->vertexLitMode ))
        return false;
    return true;
}

static const char* pilotVertexKindName( tPilotVertexKind kind )
{
    switch ( kind )
    {
        case kPilotVertexKind_SkinBump: return "skin_bump";
        case kPilotVertexKind_BumpDual: return "bump_dual";
        case kPilotVertexKind_SkinBumpHQ: return "skin_bump HQ";
        case kPilotVertexKind_BumpDualHQ: return "bump_dual HQ";
        case kPilotVertexKind_BumpNormals: return "bump (model-space)";
        case kPilotVertexKind_BumpRGBS: return "bump_rgb (model-space)";
        case kPilotVertexKind_BumpMulti: return "bump_dual_multi";
        case kPilotVertexKind_BumpMultiHQ: return "bump_dual_multi HQ";
        case kPilotVertexKind_FixedFunction: return "fixed-function";
        default: return "dualtex";
    }
}

static const char* pilotDrawModeName( DrawModeType drawMode )
{
    switch ( drawMode )
    {
        case DRAWMODE_SPRITE: return "DRAWMODE_SPRITE";
        case DRAWMODE_DUALTEX: return "DRAWMODE_DUALTEX";
        case DRAWMODE_COLORONLY: return "DRAWMODE_COLORONLY";
        case DRAWMODE_DUALTEX_NORMALS: return "DRAWMODE_DUALTEX_NORMALS";
        case DRAWMODE_DUALTEX_LIT_PP: return "DRAWMODE_DUALTEX_LIT_PP";
        case DRAWMODE_FILL: return "DRAWMODE_FILL";
        case DRAWMODE_BUMPMAP_SKINNED: return "DRAWMODE_BUMPMAP_SKINNED";
        case DRAWMODE_HW_SKINNED: return "DRAWMODE_HW_SKINNED";
        case DRAWMODE_BUMPMAP_NORMALS: return "DRAWMODE_BUMPMAP_NORMALS";
        case DRAWMODE_BUMPMAP_NORMALS_PP: return "DRAWMODE_BUMPMAP_NORMALS_PP";
        case DRAWMODE_BUMPMAP_DUALTEX: return "DRAWMODE_BUMPMAP_DUALTEX";
        case DRAWMODE_BUMPMAP_RGBS: return "DRAWMODE_BUMPMAP_RGBS";
        case DRAWMODE_BUMPMAP_MULTITEX: return "DRAWMODE_BUMPMAP_MULTITEX";
        case DRAWMODE_BUMPMAP_MULTITEX_RGBS: return "DRAWMODE_BUMPMAP_MULTITEX_RGBS";
        case DRAWMODE_BUMPMAP_SKINNED_MULTITEX: return "DRAWMODE_BUMPMAP_SKINNED_MULTITEX";
        case DRAWMODE_DEPTH_ONLY: return "DRAWMODE_DEPTH_ONLY";
        case DRAWMODE_DEPTHALPHA_ONLY: return "DRAWMODE_DEPTHALPHA_ONLY";
        case DRAWMODE_DEPTH_SKINNED: return "DRAWMODE_DEPTH_SKINNED";
        default: return "DRAWMODE_UNKNOWN";
    }
}

static void pilotAppendBmbName( char* buffer, size_t bufferSize, const char* name, bool* first )
{
    if ( !*first )
        strcat_s( buffer, bufferSize, "|" );
    strcat_s( buffer, bufferSize, name );
    *first = false;
}

static void pilotFormatBmb( int bmb, char* buffer, size_t bufferSize )
{
    bool first = true;
    buffer[0] = '\0';
    if ( !bmb )
    {
        strcpy_s( buffer, bufferSize, "BMB_DEFAULT" );
        return;
    }
    if ( bmb & BMB_HIGH_QUALITY )
        pilotAppendBmbName( buffer, bufferSize, "BMB_HIGH_QUALITY", &first );
    if ( bmb & BMB_SHADOWMAP )
        pilotAppendBmbName( buffer, bufferSize, "BMB_SHADOWMAP", &first );
    if ( bmb & BMB_CUBEMAP )
        pilotAppendBmbName( buffer, bufferSize, "BMB_CUBEMAP", &first );
    if ( bmb & BMB_PLANAR_REFLECTION )
        pilotAppendBmbName( buffer, bufferSize, "BMB_PLANAR_REFLECTION", &first );
    if ( bmb & BMB_SINGLE_MATERIAL )
        pilotAppendBmbName( buffer, bufferSize, "BMB_SINGLE_MATERIAL", &first );
    if ( bmb & BMB_BUILDING )
        pilotAppendBmbName( buffer, bufferSize, "BMB_BUILDING", &first );
#ifndef FINAL
    if ( bmb & BMB_DEBUG )
        pilotAppendBmbName( buffer, bufferSize, "BMB_DEBUG", &first );
#endif
    if ( first )
        sprintf_s( buffer, bufferSize, "BMB_UNKNOWN_0x%X", bmb );
}

static bool pilotDescribeFragmentProgram( GLuint fragmentPgmId, char* buffer, size_t bufferSize )
{
    int shader;
    int bmb;
    char bmbName[160];
    char effectPath[256];

    for ( shader = 0; shader < BLENDMODE_NUMENTRIES; ++shader )
    {
        for ( bmb = 0; bmb < BMB_VARIANT_COUNT; ++bmb )
        {
            if ( g_shaderMgrFragmentProgramVariants[shader][bmb] == (int)fragmentPgmId )
            {
                pilotFormatBmb( bmb, bmbName, sizeof( bmbName ) );
                sprintf_s( buffer, bufferSize, "BlendModeShader=%s variant=%s",
                    blend_mode_names[shader], bmbName );
                return true;
            }
        }
    }

    for ( shader = 0; getSpecialShaderName( shader, effectPath, sizeof( effectPath ), NULL, NULL ); ++shader )
    {
        if ( shaderEffectsPrograms[shader] == (int)fragmentPgmId )
        {
            sprintf_s( buffer, bufferSize, "EffectShader[%d]=%s", shader, effectPath );
            return true;
        }
    }

    strcpy_s( buffer, bufferSize, "unknown-fragment" );
    return false;
}

static void pilotDescribeVertexProgram( GLuint vertexPgmId, char* buffer, size_t bufferSize )
{
    int entryIndex = pilotFindVertexEntry( vertexPgmId );
    int drawMode;

    if ( vertexPgmId == 0 )
    {
        strcpy_s( buffer, bufferSize, "vertex=0 kind=fixed-function" );
        return;
    }
    if ( entryIndex >= 0 )
    {
        sprintf_s( buffer, bufferSize, "vertex=%d kind=%s litMode=%d",
            (int)vertexPgmId, pilotVertexKindName( s_vertexEntries[entryIndex].kind ),
            s_vertexEntries[entryIndex].vertexLitMode );
        return;
    }
    for ( drawMode = 0; drawMode < DRAWMODE_NUMENTRIES; ++drawMode )
    {
        if ( shaderMgrVertexPrograms[drawMode] == (int)vertexPgmId )
        {
            sprintf_s( buffer, bufferSize, "vertex=%d drawMode=%s quality=LQ pilot-unregistered",
                (int)vertexPgmId, pilotDrawModeName( (DrawModeType)drawMode ) );
            return;
        }
        if ( shaderMgrVertexProgramsHQ[drawMode] == (int)vertexPgmId )
        {
            sprintf_s( buffer, bufferSize, "vertex=%d drawMode=%s quality=HQ pilot-unregistered",
                (int)vertexPgmId, pilotDrawModeName( (DrawModeType)drawMode ) );
            return;
        }
    }
    sprintf_s( buffer, bufferSize, "vertex=%d kind=unregistered", (int)vertexPgmId );
}

static bool pilotActivate( int material, const tPilotVertexEntry* entry )
{
    tPilotMaterial* m = &s_materials[material];
    // effects materials keep two program objects: one linked with the
    // dualtex vertex shader (final pass, DRAWMODE_SPRITE bound) and one
    // with the fixed-function vertex shader (pbuffer passes, no vertex
    // program bound)
    bool useFF = ( entry->kind == kPilotVertexKind_FixedFunction );

    if ( useFF )
    {
        if (( ! m->programFF ) && ( ! pilotInitFF( material ) ))
            return false;
    }
    else if ( ! m->program && ! pilotInit( material ) )
        return false;

    // the mirrors track the engine constants continuously (see
    // rt_glslpilot_onReflectionParam / rt_glslpilot_onEnvParam /
    // rt_glslpilot_onGlowParam), so they are valid at any activation time
    __glewUseProgram( useFF ? m->programFF : m->program );
    if ( ! useFF )
    {
        if ( m->locReflectionParam >= 0 )
            __glewUniform4fv( m->locReflectionParam, 1, s_reflectionParamMirror );
        if ( m->locVertexLitMode >= 0 )
            __glewUniform1i( m->locVertexLitMode, entry->vertexLitMode );
        if ( m->locEnv0 >= 0 )
            __glewUniform4fv( m->locEnv0, 1, s_envMirrors[0] );
        if ( m->locEnv1 >= 0 )
            __glewUniform4fv( m->locEnv1, 1, s_envMirrors[1] );
        if ( m->locGlowParam >= 0 )
            __glewUniform4fv( m->locGlowParam, 1, s_glowParamMirror );
        if ( m->locLightDir >= 0 )
            __glewUniform4fv( m->locLightDir, 1, s_lightDirMirror );
        if ( m->locLightDirFP >= 0 )
            __glewUniform4fv( m->locLightDirFP, 1, s_lightDirFPMirror );
        if ( m->locSkinned >= 0 )
            __glewUniform1i( m->locSkinned,
                (( entry->kind == kPilotVertexKind_SkinBump ) ||
                 ( entry->kind == kPilotVertexKind_SkinBumpHQ )) ? 1 : 0 );
        if ( m->locBoneMatrices >= 0 && s_boneMatrixMirrorCount > 0 )
            __glewUniform4fv( m->locBoneMatrices, s_boneMatrixMirrorCount, &s_boneMatrixMirror[0][0] );
        if ( m->locTexScroll0 >= 0 )
            __glewUniform4fv( m->locTexScroll0, 1, s_texScrollMirrors[0] );
        if ( m->locTexScroll1 >= 0 )
            __glewUniform4fv( m->locTexScroll1, 1, s_texScrollMirrors[1] );
        if ( m->locAmbientVP >= 0 )
            __glewUniform4fv( m->locAmbientVP, 1, s_ambientVPMirror );
        if ( m->locDiffuseVP >= 0 )
            __glewUniform4fv( m->locDiffuseVP, 1, s_diffuseVPMirror );
        if ( m->locPrelit >= 0 )
            __glewUniform1i( m->locPrelit,
                ( entry->kind == kPilotVertexKind_BumpRGBS ) ? 1 : 0 );
        if ( m->usesWaterConstants )
        {
            __glewUniform4fv( m->locConstColor0FP, 1, s_waterConstMirrors[kPilotWaterConst_ConstColor0] );
            __glewUniform4fv( m->locConstColor1FP, 1, s_waterConstMirrors[kPilotWaterConst_ConstColor1] );
            __glewUniform4fv( m->locWaterRefractTransform, 1, s_waterConstMirrors[kPilotWaterConst_RefractionTransform] );
            __glewUniform4fv( m->locWaterRefractParams, 1, s_waterConstMirrors[kPilotWaterConst_RefractionParams] );
            __glewUniform4fv( m->locBumpMultiFlags, 1, s_waterConstMirrors[kPilotWaterConst_BumpMultiFlags] );
            __glewUniform4fv( m->locScrollScaleArr, kPilotMaxScrollScaleVec4s, &s_scrollScaleMirror[0][0] );
            if ( m->usesWaterReflectionConstants )
            {
                __glewUniform4fv( m->locWaterReflectionTransform, 1, s_waterConstMirrors[kPilotWaterConst_ReflectionTransform] );
                __glewUniform4fv( m->locWaterReflectionParams, 1, s_waterConstMirrors[kPilotWaterConst_ReflectionParams] );
                __glewUniform4fv( m->locWaterFresnelParams, 1, s_waterConstMirrors[kPilotWaterConst_FresnelParams] );
                __glewUniform1i( m->locUsePlanarReflection, 1 );
            }
        }
        if ( m->usesShadowConstants )
        {
            int shadowMap;
            __glewUniform1i( m->locUseShadowMap, 1 );
            __glewUniform1i( m->locShadowFilterMode, (int)game_state.shadowShaderSelection );
            for ( shadowMap = 0; shadowMap < 4; shadowMap++ )
                __glewUniform4fv( m->locShadowMap[shadowMap], 4, &s_shadowConstMirrors[shadowMap][0][0] );
            __glewUniform4fv( m->locShadowParams, 1, &s_shadowConstMirrors[kPilotShadowConst_Params][0][0] );
            __glewUniform4fv( m->locShadowSplits, 1, &s_shadowConstMirrors[kPilotShadowConst_Splits][0][0] );
            __glewUniform4fv( m->locShadowParams2, 1, &s_shadowConstMirrors[kPilotShadowConst_Params2][0][0] );
            __glewUniform4fv( m->locShadowParams3, 1, &s_shadowConstMirrors[kPilotShadowConst_Params3][0][0] );
        }
        if ( m->usesMultiConstants )
        {
            if ( m->locConstColor0FP >= 0 )
                __glewUniform4fv( m->locConstColor0FP, 1, s_waterConstMirrors[kPilotWaterConst_ConstColor0] );
            if ( m->locConstColor1FP >= 0 )
                __glewUniform4fv( m->locConstColor1FP, 1, s_waterConstMirrors[kPilotWaterConst_ConstColor1] );
            __glewUniform4fv( m->locBumpMultiFlags, 1, s_waterConstMirrors[kPilotWaterConst_BumpMultiFlags] );
            __glewUniform4fv( m->locScrollScaleArr, kPilotMaxScrollScaleVec4s, &s_scrollScaleMirror[0][0] );
            if ( m->locSpecular2 >= 0 )
                __glewUniform4fv( m->locSpecular2, 1, s_specular2Mirror );
            if ( m->locMiscParam >= 0 )
                __glewUniform4fv( m->locMiscParam, 1, s_miscParamMirror );
        }
        if (( ! m->usesBumpConstants ) && ( m->locSpecular1 >= 0 ))
            __glewUniform4fv( m->locSpecular1, 1, s_specular1Mirror );
        if ( m->usesBumpConstants )
        {
            __glewUniform4fv( m->locAmbient, 1, s_ambientMirror );
            __glewUniform4fv( m->locDiffuse, 1, s_diffuseMirror );
            __glewUniform4fv( m->locGloss, 1, s_glossMirror );
            __glewUniform4fv( m->locSpecular1, 1, s_specular1Mirror );
        }
        if (( material == kPilotMaterial_BumpColorBlendDual ) ||
            ( material == kPilotMaterial_BumpColorBlendDualHQ ))
        {
            GLfloat modernMaterialParams[4] = {
                game_state.modernMaterials ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f
            };
            __glewUniform4fv( s_modernMaterialParamLoc[material], 1, modernMaterialParams );
        }
    }
    if ( m->fxConstMask )
    {
        const GLint* fxLocs = useFF ? m->locFxFor : m->locFx;
        int c;
        for ( c = 0; c < kPilotFxConst_Count; c++ )
        {
            if ( fxLocs[c] >= 0 )
                __glewUniform4fv( fxLocs[c], 1, s_fxConstMirrors[c] );
        }
    }
    s_activeMaterial = material;
    m->activeFF = useFF;
    {
        if (( ( material == kPilotMaterial_WaterPlanar ) ||
              ( material == kPilotMaterial_WaterShadowPlanar )) &&
            ( ! m->activationLogged ))
            printf( "GLSL pilot: %s runtime fragment=%d vertex=%d kind=%s\n",
                m->name, (int)m->arbFragmentId, (int)entry->pgmId,
                pilotVertexKindName( entry->kind ) );
        m->activationLogged = true;
        if ( m->vertexKindMask & ( kPilotBumpKindMask | kPilotBumpHQKindMask | kPilotBumpModelKindMask |
                                   kPilotBumpMultiKindMask | kPilotBumpMultiHQKindMask ))
            printf( "GLSL pilot: %s active (%s vertex variant)\n", m->name,
                pilotVertexKindName( entry->kind ) );
        else if ( entry->kind == kPilotVertexKind_FixedFunction )
            printf( "GLSL pilot: %s active (fixed-function quad)\n", m->name );
        else
            printf( "GLSL pilot: %s active (vertex lit mode %d)\n", m->name, entry->vertexLitMode );
    }
    return true;
}

bool rt_glslpilot_tryBindFragment( GLuint fragmentPgmId, GLuint vertexPgmId )
{
    int material = pilotFindMaterial( fragmentPgmId );
    int entryIndex;
    const tPilotVertexEntry* entry;

    if (( s_activeMaterial >= 0 ) && ( s_activeMaterial != material ))
        pilotDeactivate();

    if ( ! game_state.glslPilot || ( material < 0 ) || s_materials[material].failed )
        return false;

    entryIndex = pilotFindVertexEntry( vertexPgmId );
    entry = ( entryIndex >= 0 ) ? &s_vertexEntries[entryIndex] : NULL;
    if (( ! entry ) || ( ! pilotVertexEntryDrives( entry, &s_materials[material] ) ))
    {
        if ( ! s_materials[material].declineLogged )
        {
            s_materials[material].declineLogged = true;
            printf( "GLSL pilot: %s bind declined, vertex program %d %s\n",
                s_materials[material].name, (int)vertexPgmId,
                ( ! entry ) ? "not registered"
                            : "registered for a different vertex variant" );
        }
        pilotDeactivate();
        return false;
    }

    if ( ! pilotActivate( material, entry ) )
    {
        pilotDeactivate();
        return false;
    }
    return true;
}

void rt_glslpilot_tryBindVertex( GLuint vertexPgmId, GLuint fragmentPgmId )
{
    int entryIndex = pilotFindVertexEntry( vertexPgmId );
    int material = pilotFindMaterial( fragmentPgmId );
    const tPilotVertexEntry* entry = ( entryIndex >= 0 ) ? &s_vertexEntries[entryIndex] : NULL;

    if ( s_activeMaterial < 0 )
    {
        if (( ! game_state.glslPilot ) || ( material < 0 ) || s_materials[material].failed ||
            ( ! entry ) || ( ! pilotVertexEntryDrives( entry, &s_materials[material] ) ))
            return;
        pilotActivate( material, entry );
        return;
    }

    if ( s_activeMaterial != material )
    {
        // the tracked fragment program moved off the pilot targets without
        // a bind call to tell us; stop overriding the pipeline
        pilotDeactivate();
        return;
    }

    if (( ! entry ) || ( ! pilotVertexEntryDrives( entry, &s_materials[s_activeMaterial] ) ))
    {
        pilotDeactivate();
        return;
    }

    // still a pilot fragment target; a registered vertex variant took over,
    // so re-activate — bump variants re-mode the skinning switch, effects
    // materials switch between their dualtex and fixed-function program
    // objects (bone matrices refresh via the mirror push that follows the
    // vertex bind)
    if ( ! pilotActivate( s_activeMaterial, entry ) )
        pilotDeactivate();
}

void rt_glslpilot_onVertexProgramChange( GLuint vertexPgmId )
{
    (void)vertexPgmId;
    // The engine is disabling or resetting vertex program state.
    pilotDeactivate();
}

void rt_glslpilot_onFragmentProgramDisable( void )
{
    pilotDeactivate();
}

bool rt_glslpilot_isActive( void )
{
    return s_activeMaterial >= 0;
}

void rt_glslpilot_noteUnportedFragmentBind( GLuint fragmentPgmId, GLuint vertexPgmId )
{
    typedef struct tPilotFragmentFallback {
        GLuint fragmentPgmId;
        GLuint vertexPgmId;
    } tPilotFragmentFallback;
    static tPilotFragmentFallback seen[128];
    static int seenCount = 0;
    char fragmentDescription[256];
    char vertexDescription[192];
    int material;
    int i;

    if ( ! game_state.glslPilot || ( ! fragmentPgmId ) || ( fragmentPgmId == 0xFFFFFFFF ) )
        return;
    for ( i = 0; i < seenCount; i++ )
    {
        if (( seen[i].fragmentPgmId == fragmentPgmId ) &&
            ( seen[i].vertexPgmId == vertexPgmId ))
            return;
    }
    if ( seenCount >= (int)( sizeof( seen ) / sizeof( seen[0] ) ) )
        return;
    seen[seenCount].fragmentPgmId = fragmentPgmId;
    seen[seenCount].vertexPgmId = vertexPgmId;
    seenCount++;
    material = pilotFindMaterial( fragmentPgmId );
    pilotDescribeFragmentProgram( fragmentPgmId, fragmentDescription, sizeof( fragmentDescription ) );
    pilotDescribeVertexProgram( vertexPgmId, vertexDescription, sizeof( vertexDescription ) );
    printf( "GLSL pilot: coverage: fallback %s (arbFragment=%d) %s; classification=%s\n",
        fragmentDescription, (int)fragmentPgmId, vertexDescription,
        ( material >= 0 ) ? "pilot-target-declined-on-vertex-pairing"
                          : "unported-fragment-variant" );
}

void rt_glslpilot_noteVertexFallback( GLuint vertexPgmId, GLuint logicalFragmentPgmId )
{
    typedef struct tPilotFallbackDiagnostic {
        GLuint vertexPgmId;
        GLuint logicalFragmentPgmId;
    } tPilotFallbackDiagnostic;
    static tPilotFallbackDiagnostic seen[128];
    char fragmentDescription[256];
    char vertexDescription[192];
    int material;
    static int seenCount = 0;
    int i;

    if ( ! game_state.glslPilot || ( logicalFragmentPgmId == 0xFFFFFFFF ) )
        return;
    for ( i = 0; i < seenCount; i++ )
    {
        if (( seen[i].vertexPgmId == vertexPgmId ) &&
            ( seen[i].logicalFragmentPgmId == logicalFragmentPgmId ))
            return;
    }
    if ( seenCount >= (int)( sizeof( seen ) / sizeof( seen[0] ) ) )
        return;
    seen[seenCount].vertexPgmId = vertexPgmId;
    seen[seenCount].logicalFragmentPgmId = logicalFragmentPgmId;
    seenCount++;
    material = pilotFindMaterial( logicalFragmentPgmId );
    pilotDescribeFragmentProgram( logicalFragmentPgmId, fragmentDescription, sizeof( fragmentDescription ) );
    pilotDescribeVertexProgram( vertexPgmId, vertexDescription, sizeof( vertexDescription ) );
    printf( "GLSL pilot: fallback pair: %s (logicalFragment=%d) %s; classification=%s; "
            "legacy fragment request is synchronized; final prep will bind before draw\n",
            fragmentDescription, (int)logicalFragmentPgmId, vertexDescription,
            ( material >= 0 ) ? "pilot-target-declined-on-vertex-pairing"
                              : "unported-fragment-with-legacy-vertex-pairing" );
}

void rt_glslpilot_onReflectionParam( const GLfloat* vec4 )
{
    // always mirror, active or not: the value must be correct whenever the
    // pilot activates next, and engine bind order is not guaranteed relative
    // to activation
    memcpy( s_reflectionParamMirror, vec4, sizeof( s_reflectionParamMirror ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locReflectionParam >= 0 )
            __glewUniform4fv( m->locReflectionParam, 1, s_reflectionParamMirror );
    }
}

void rt_glslpilot_onEnvParam( int index, const GLfloat* vec4 )
{
    // always mirror, active or not (same rationale as the reflection param;
    // constColor0/constColor1 change with draw alpha/tinting while other
    // materials draw); index 0 = g_Env0FP, 1 = g_Env1FP
    if (( index < 0 ) || ( index > 1 ))
        return;
    memcpy( s_envMirrors[index], vec4, sizeof( s_envMirrors[0] ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        GLint loc = ( index == 0 ) ? m->locEnv0 : m->locEnv1;
        if ( loc >= 0 )
            __glewUniform4fv( loc, 1, s_envMirrors[index] );
    }
}

void rt_glslpilot_onGlowParam( const GLfloat* vec4 )
{
    // always mirror, active or not; rt_tricks.c pushes this between the
    // addglow program bind and its draws, so the value is fresh whenever
    // the pilot activates for the material
    memcpy( s_glowParamMirror, vec4, sizeof( s_glowParamMirror ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locGlowParam >= 0 )
            __glewUniform4fv( m->locGlowParam, 1, s_glowParamMirror );
    }
}

// Bump lighting constant mirrors (same always-mirror contract): the light
// direction is pushed per draw by the bump draw paths, the fragment
// lighting constants by setupBumpPixelShader/setupSpecularColor right
// before the draws that use them
void rt_glslpilot_onLightDirParam( const GLfloat* vec4 )
{
    memcpy( s_lightDirMirror, vec4, sizeof( s_lightDirMirror ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locLightDir >= 0 )
            __glewUniform4fv( m->locLightDir, 1, s_lightDirMirror );
    }
}

// g_LightDirFP (TIE(ENV11)): the HQ bump fragment's light direction, pushed
// by setupBumpPixelShader only for BMB_HIGH_QUALITY draws; the env register
// persists between pushes, so the mirror deliberately keeps the last value
// the same way the ARB path would
void rt_glslpilot_onLightDirFPParam( const GLfloat* vec4 )
{
    memcpy( s_lightDirFPMirror, vec4, sizeof( s_lightDirFPMirror ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locLightDirFP >= 0 )
            __glewUniform4fv( m->locLightDirFP, 1, s_lightDirFPMirror );
    }
}

// Effects/post-processing fragment constants: program-local params in the
// Cg sources (several share local slot numbers across different programs,
// hence the identity-keyed slots). rt_effects.c pushes them between the
// program bind and the draw of each pass.
void rt_glslpilot_onEffectsParam( int fxConstSlot, const GLfloat* vec4 )
{
    if (( fxConstSlot < 0 ) || ( fxConstSlot >= kPilotFxConst_Count ))
        return;
    memcpy( s_fxConstMirrors[fxConstSlot], vec4, sizeof( s_fxConstMirrors[0] ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->fxConstMask & kFxBit( fxConstSlot ) )
        {
            // effects materials keep two program objects (dualtex vertex for
            // the final composite pass, fixed-function vertex for the pbuffer
            // passes); push to whichever one is currently bound
            GLint loc = m->activeFF ? m->locFxFor[fxConstSlot] : m->locFx[fxConstSlot];
            if ( loc >= 0 )
                __glewUniform4fv( loc, 1, s_fxConstMirrors[fxConstSlot] );
        }
    }
}

void rt_glslpilot_onAmbientColorParam( const GLfloat* vec4 )
{
    memcpy( s_ambientMirror, vec4, sizeof( s_ambientMirror ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locAmbient >= 0 )
            __glewUniform4fv( m->locAmbient, 1, s_ambientMirror );
    }
}

void rt_glslpilot_onDiffuseColorParam( const GLfloat* vec4 )
{
    memcpy( s_diffuseMirror, vec4, sizeof( s_diffuseMirror ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locDiffuse >= 0 )
            __glewUniform4fv( m->locDiffuse, 1, s_diffuseMirror );
    }
}

void rt_glslpilot_onGlossParam( const GLfloat* vec4 )
{
    memcpy( s_glossMirror, vec4, sizeof( s_glossMirror ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locGloss >= 0 )
            __glewUniform4fv( m->locGloss, 1, s_glossMirror );
    }
}

void rt_glslpilot_onSpecular1Param( const GLfloat* vec4 )
{
    memcpy( s_specular1Mirror, vec4, sizeof( s_specular1Mirror ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locSpecular1 >= 0 )
            __glewUniform4fv( m->locSpecular1, 1, s_specular1Mirror );
    }
}

void rt_glslpilot_onBoneMatrixParam( const GLfloat* vec4Arr, GLuint nNumVec4s )
{
    // loadBoneMatrices pushes bone_count*3 vec4s per skinned model, between
    // the vertex program bind and the draws; keep the mirror (and the
    // active program's uniform) current so any activation sees fresh bones.
    // The tail beyond the pushed count deliberately keeps stale rows: the
    // shaders only index bones < 3*bone_count, matching the ARB env
    // register semantics.
    if ( nNumVec4s > kPilotMaxBoneVec4s )
        nNumVec4s = kPilotMaxBoneVec4s;
    memcpy( s_boneMatrixMirror, vec4Arr, nNumVec4s * sizeof( GLfloat ) * 4 );
    s_boneMatrixMirrorCount = nNumVec4s;
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locBoneMatrices >= 0 )
            __glewUniform4fv( m->locBoneMatrices, nNumVec4s, &s_boneMatrixMirror[0][0] );
    }
}

// Model-space bump (bumpmapMultiply) vertex constants: the texcoord scrolls
// (texScrollIndex 0 = g_TexScroll0VP, 1 = g_TexScroll1VP) and the per-vertex
// diffuse lighting terms (which 0 = g_AmbientParameterVP, 1 =
// g_DiffuseParameterVP). drawLoopBump and the bumpmapMultiply draw paths in
// rt_model.c push these between the program binds and the draws; same
// always-mirror contract as the other constants.
void rt_glslpilot_onTexScrollParam( int texScrollIndex, const GLfloat* vec4 )
{
    if (( texScrollIndex < 0 ) || ( texScrollIndex > 1 ))
        return;
    memcpy( s_texScrollMirrors[texScrollIndex], vec4, sizeof( s_texScrollMirrors[0] ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        GLint loc = ( texScrollIndex == 0 ) ? m->locTexScroll0 : m->locTexScroll1;
        if ( loc >= 0 )
            __glewUniform4fv( loc, 1, s_texScrollMirrors[texScrollIndex] );
    }
}

void rt_glslpilot_onAmbientDiffuseParam( int which, const GLfloat* vec4 )
{
    GLfloat* mirror = NULL;
    GLint loc = -1;
    if ( which == 0 )
    {
        mirror = s_ambientVPMirror;
        if ( s_activeMaterial >= 0 )
            loc = s_materials[s_activeMaterial].locAmbientVP;
    }
    else if ( which == 1 )
    {
        mirror = s_diffuseVPMirror;
        if ( s_activeMaterial >= 0 )
            loc = s_materials[s_activeMaterial].locDiffuseVP;
    }
    else
        return;
    memcpy( mirror, vec4, sizeof( s_ambientVPMirror ) );
    if (( s_activeMaterial >= 0 ) && ( loc >= 0 ))
        __glewUniform4fv( loc, 1, mirror );
}

// Water/multitex fragment constants (waterConstSlot is a tPilotWaterConstId):
// the material tint colors and multi-selector flags pushed by
// setupBumpMultiPixelShader (rt_model.c) and the refraction transform/skew
// parameters pushed per draw by rt_water.c. Same always-mirror contract.
void rt_glslpilot_onWaterParam( int waterConstSlot, const GLfloat* vec4 )
{
    GLint loc = -1;
    if (( waterConstSlot < 0 ) || ( waterConstSlot >= kPilotWaterConst_Count ))
        return;
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        switch ( waterConstSlot )
        {
            case kPilotWaterConst_ConstColor0:        loc = m->locConstColor0FP; break;
            case kPilotWaterConst_ConstColor1:        loc = m->locConstColor1FP; break;
            case kPilotWaterConst_RefractionTransform: loc = m->locWaterRefractTransform; break;
            case kPilotWaterConst_RefractionParams:   loc = m->locWaterRefractParams; break;
            case kPilotWaterConst_BumpMultiFlags:     loc = m->locBumpMultiFlags; break;
            case kPilotWaterConst_ReflectionTransform: loc = m->locWaterReflectionTransform; break;
            case kPilotWaterConst_ReflectionParams:   loc = m->locWaterReflectionParams; break;
            case kPilotWaterConst_FresnelParams:      loc = m->locWaterFresnelParams; break;
            default: break;
        }
    }
    memcpy( s_waterConstMirrors[waterConstSlot], vec4, sizeof( s_waterConstMirrors[0] ) );
    if (( s_activeMaterial >= 0 ) && ( loc >= 0 ))
        __glewUniform4fv( loc, 1, s_waterConstMirrors[waterConstSlot] );
}

// g_ScrollScaleArrFP: the whole per-layer scroll/scale array is pushed in
// one call by setupBumpMultiPixelShader (rt_model.c); mirror all ten vec4s
// and forward them to the active program in one upload.
void rt_glslpilot_onScrollScaleParam( const GLfloat* vec4Arr, GLuint nNumVec4s )
{
    if ( nNumVec4s > kPilotMaxScrollScaleVec4s )
        nNumVec4s = kPilotMaxScrollScaleVec4s;
    memcpy( s_scrollScaleMirror, vec4Arr, nNumVec4s * sizeof( GLfloat ) * 4 );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locScrollScaleArr >= 0 )
            __glewUniform4fv( m->locScrollScaleArr, nNumVec4s, &s_scrollScaleMirror[0][0] );
    }
}

// Shadow-map fragment constants: rt_shadowmap.c pushes the four cascade
// matrices and the four scalar parameter blocks through the same Cg constant
// setter used by the legacy program. Keep a full mirror so activation order
// cannot lose a value, then forward the matching array to the active shadow
// water program.
void rt_glslpilot_onShadowParam( int shadowConstSlot, const GLfloat* vec4Arr, GLuint nNumVec4s )
{
    GLint loc = -1;
    if (( shadowConstSlot < 0 ) || ( shadowConstSlot >= kPilotShadowConst_Count ))
        return;
    if ( nNumVec4s > 4 )
        nNumVec4s = 4;
    memcpy( s_shadowConstMirrors[shadowConstSlot], vec4Arr,
        nNumVec4s * sizeof( s_shadowConstMirrors[shadowConstSlot][0] ) );
    if (( s_activeMaterial >= 0 ) && s_materials[s_activeMaterial].usesShadowConstants )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( shadowConstSlot <= kPilotShadowConst_Map4 )
            loc = m->locShadowMap[shadowConstSlot];
        else if ( shadowConstSlot == kPilotShadowConst_Params )
            loc = m->locShadowParams;
        else if ( shadowConstSlot == kPilotShadowConst_Splits )
            loc = m->locShadowSplits;
        else if ( shadowConstSlot == kPilotShadowConst_Params2 )
            loc = m->locShadowParams2;
        else if ( shadowConstSlot == kPilotShadowConst_Params3 )
            loc = m->locShadowParams3;
        if ( loc >= 0 )
            __glewUniform4fv( loc, nNumVec4s, &s_shadowConstMirrors[shadowConstSlot][0][0] );
    }
}

// multi9 fragment constants (same always-mirror contract): the material-2
// specular (setupSpecularColor, dual-material MULTI draws only) and the
// 'lights on' addglow threshold/seed (setupBumpMultiPixelShader)
void rt_glslpilot_onSpecular2Param( const GLfloat* vec4 )
{
    memcpy( s_specular2Mirror, vec4, sizeof( s_specular2Mirror ) );
    if (( s_activeMaterial >= 0 ) && ( s_materials[s_activeMaterial].locSpecular2 >= 0 ))
        __glewUniform4fv( s_materials[s_activeMaterial].locSpecular2, 1, s_specular2Mirror );
}

void rt_glslpilot_onMiscParam( const GLfloat* vec4 )
{
    memcpy( s_miscParamMirror, vec4, sizeof( s_miscParamMirror ) );
    if (( s_activeMaterial >= 0 ) && ( s_materials[s_activeMaterial].locMiscParam >= 0 ))
        __glewUniform4fv( s_materials[s_activeMaterial].locMiscParam, 1, s_miscParamMirror );
}

void rt_glslpilot_resetPrograms( void )
{
    // only the vertex table; fragment targets are refreshed independently
    // by rt_glslpilot_setFragmentTarget because shaderMgr_InitVPs and
    // shaderMgr_InitFPs run in no guaranteed order
    s_vertexEntryCount = 0;
}

void rt_glslpilot_addVertexProgram( GLuint vertexPgmId, tPilotVertexKind kind, int vertexLitMode )
{
    // vertexPgmId 0 is the fixed-function entry (pbuffer effects passes)
    if (( vertexPgmId != 0xFFFFFFFF ) && ( s_vertexEntryCount < kPilotMaxVertexEntries ))
    {
        s_vertexEntries[s_vertexEntryCount].pgmId = vertexPgmId;
        s_vertexEntries[s_vertexEntryCount].kind = kind;
        s_vertexEntries[s_vertexEntryCount].vertexLitMode = vertexLitMode;
        s_vertexEntryCount++;
        if ( game_state.glslPilot )
        {
            printf( "GLSL pilot: registered vertex program %d kind %s lit mode %d\n",
                (int)vertexPgmId, pilotVertexKindName( kind ), vertexLitMode );
        }
    }
}

void rt_glslpilot_setFragmentTarget( tPilotMaterialId material, GLuint fragmentPgmId )
{
    if (( material < 0 ) || ( material >= kPilotMaterial_Count ))
        return;
    s_materials[material].arbFragmentId = fragmentPgmId;
    // The four bloom/tone-map final programs share the opt-in presentation
    // uniform. Keep it out of the large positional initializer table while
    // still resolving and mirroring it before those programs are compiled.
    if (( material == kPilotMaterial_FxTonemap2 ) ||
        ( material == kPilotMaterial_FxTonemap2Desat ) ||
        ( material == kPilotMaterial_FxDofBloomFinal ) ||
        ( material == kPilotMaterial_FxDofBloomFinalDesat ))
    {
        s_materials[material].fxConstMask |= kFxBit( kPilotFxConst_Presentation );
    }
    if ( game_state.glslPilot )
    {
        printf( "GLSL pilot: fragment targets modulate=%d multiply=%d colorBlendDual=%d addGlow=%d alphaDetail=%d bumpColorBlendDual=%d bumpColorBlendDualHQ=%d bumpMultiply=%d water=%d waterShadow=%d waterPlanar=%d waterShadowPlanar=%d, %d registered vertex programs\n",
            (int)s_materials[kPilotMaterial_Modulate].arbFragmentId,
            (int)s_materials[kPilotMaterial_Multiply].arbFragmentId,
            (int)s_materials[kPilotMaterial_ColorBlendDual].arbFragmentId,
            (int)s_materials[kPilotMaterial_AddGlow].arbFragmentId,
            (int)s_materials[kPilotMaterial_AlphaDetail].arbFragmentId,
            (int)s_materials[kPilotMaterial_BumpColorBlendDual].arbFragmentId,
            (int)s_materials[kPilotMaterial_BumpColorBlendDualHQ].arbFragmentId,
            (int)s_materials[kPilotMaterial_BumpMultiply].arbFragmentId,
            (int)s_materials[kPilotMaterial_Water].arbFragmentId,
            (int)s_materials[kPilotMaterial_WaterShadow].arbFragmentId,
            (int)s_materials[kPilotMaterial_WaterPlanar].arbFragmentId,
            (int)s_materials[kPilotMaterial_WaterShadowPlanar].arbFragmentId,
            s_vertexEntryCount );
        if ( material == kPilotMaterial_Multi9Building )
        {
            printf( "GLSL pilot: Multi9 targets full=%d fullHQ=%d single=%d singleHQ=%d building=%d\n",
                (int)s_materials[kPilotMaterial_Multi9Full].arbFragmentId,
                (int)s_materials[kPilotMaterial_Multi9FullHQ].arbFragmentId,
                (int)s_materials[kPilotMaterial_Multi9Single].arbFragmentId,
                (int)s_materials[kPilotMaterial_Multi9SingleHQ].arbFragmentId,
                (int)s_materials[kPilotMaterial_Multi9Building].arbFragmentId );
        }
    }
}
