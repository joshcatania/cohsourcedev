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
// The GLSL builds on compatibility-profile built-ins (gl_ModelViewMatrix,
// gl_TextureMatrix, gl_LightSource[0], gl_Fog, gl_Color, gl_FogFragCoord)
// which read the same GL server state the Cg `state.*` semantics read, so
// the engine needs no new parameter plumbing beyond the mirrored
// program-local constants (g_ReflectionParamVP, g_Env0/1FP, g_GlowParamFP,
// and the bump lighting constants + tangent attribute for the bump
// materials; the HQ bump fragment reads g_LightDirFP instead of a
// vertex-interpolated light vector).

#include "render/thread/ogl.h"
#include "render/thread/rt_glslpilot.h"
#include "cmdparse/cmdgame.h"
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
"    vec3 gloss = clamp( specular * g_GlossParamFP.w * g_Specular1ColorAndExponentFP.rgb, 0.0, 1.0 );\n"
"    out_color.rgb = out_color.rgb * ( ambient + diffuse ) + gloss;\n"
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
"    vec3 gloss = clamp( specular * g_GlossParamFP.w * g_Specular1ColorAndExponentFP.rgb, 0.0, 1.0 );\n"
"    out_color.rgb = out_color.rgb * ( ambient + diffuse ) + gloss;\n"
"\n"
"    // calc_fogged_color (same GL fog state as the other materials)\n"
"    float fogAmount = clamp( gl_Fog.scale * ( gl_Fog.end - gl_FogFragCoord ), 0.0, 1.0 );\n"
"    out_color.rgb = mix( gl_Fog.color.rgb, out_color.rgb, fogAmount );\n"
"\n"
"    gl_FragColor = out_color;\n"
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
    bool                failed;
    bool                activationLogged;    // one-time activation evidence for logs
    bool                declineLogged;        // one-time unregistered-vertex diagnostic
} tPilotMaterial;

#define kPilotKindBit( kind ) ( 1u << (kind) )
#define kPilotBumpKindMask ( kPilotKindBit( kPilotVertexKind_BumpDual ) | \
                             kPilotKindBit( kPilotVertexKind_SkinBump ) )
#define kPilotBumpHQKindMask ( kPilotKindBit( kPilotVertexKind_BumpDualHQ ) | \
                               kPilotKindBit( kPilotVertexKind_SkinBumpHQ ) )

#define kPilotMaxVertexEntries 12

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

// field order: fragment id, name, fragment source, samplers, env0, env1,
// glow, bump constants, vertex kind mask, vertex source, program, locs...,
// failed, activationLogged, declineLogged
static tPilotMaterial s_materials[kPilotMaterial_Count] = {
    { 0, "BLENDMODE_MODULATE",           s_modulateFragmentSource,           s_dualSamplers,       false, false, false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_MULTIPLY",           s_multiplyFragmentSource,           s_dualSamplers,       true,  false, false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_COLORBLEND_DUAL",    s_colorBlendDualFragmentSource,     s_dualTintSamplers,   true,  true,  false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_ADDGLOW",            s_addGlowFragmentSource,            s_addGlowSamplers,    true,  false, true,  false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_ALPHADETAIL",        s_alphaDetailFragmentSource,        s_dualSamplers,       true,  false, false, false, kPilotKindBit( kPilotVertexKind_DualTex ), s_pilotVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_BUMPMAP_COLORBLEND_DUAL", s_bumpColorBlendDualFragmentSource, s_bumpDualTintSamplers, true, true, false, true, kPilotBumpKindMask, s_bumpDualVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_BUMPMAP_COLORBLEND_DUAL_HQ", s_bumpColorBlendDualHQFragmentSource, s_bumpDualTintSamplers, true, true, false, true, kPilotBumpHQKindMask, s_bumpDualHQVertexSource, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false, false },
};

static int                    s_activeMaterial = -1;    // -1 = pilot inactive
static GLuint                s_vertexShader = 0;        // shared: one source serves the dualtex materials
static bool                    s_vertexShaderFailed = false;
static GLuint                s_bumpVertexShader = 0;    // shared by bump-material programs (static + skinned)
static bool                    s_bumpVertexShaderFailed = false;
static GLuint                s_bumpHQVertexShader = 0;    // shared by HQ bump-material programs
static bool                    s_bumpHQVertexShaderFailed = false;
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
static GLfloat                s_boneMatrixMirror[kPilotMaxBoneVec4s][4];
static GLuint                s_boneMatrixMirrorCount = 0;

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

static bool pilotInit( int material )
{
    tPilotMaterial* m = &s_materials[material];
    bool isBumpLQ = ( m->vertexKindMask & kPilotBumpKindMask ) != 0;
    bool isBumpHQ = ( m->vertexKindMask & kPilotBumpHQKindMask ) != 0;
    GLuint vertexShader = isBumpHQ ? pilotGetBumpHQVertexShader()
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
    // rt_model.c / rt_bonedmodel.c); bind the GLSL attributes to the same
    // indices before linking
    if ( isBumpLQ || isBumpHQ )
    {
        __glewBindAttribLocation( m->program, 7, "attr_tangent" );
        __glewBindAttribLocation( m->program, 1, "attr_boneweights" );
        __glewBindAttribLocation( m->program, 5, "attr_boneindices" );
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
    if ( m->usesBumpConstants )
    {
        m->locAmbient = __glewGetUniformLocation( m->program, "g_AmbientColorFP" );
        m->locDiffuse = __glewGetUniformLocation( m->program, "g_DiffuseColorFP" );
        m->locGloss = __glewGetUniformLocation( m->program, "g_GlossParamFP" );
        m->locSpecular1 = __glewGetUniformLocation( m->program, "g_Specular1ColorAndExponentFP" );
    }
    if (( ! isBumpLQ && ! isBumpHQ && (( m->locReflectionParam < 0 ) || ( m->locVertexLitMode < 0 ))) ||
        ( isBumpLQ && (( m->locLightDir < 0 ) || ( m->locSkinned < 0 ) || ( m->locBoneMatrices < 0 ))) ||
        ( isBumpHQ && (( m->locLightDirFP < 0 ) || ( m->locSkinned < 0 ) || ( m->locBoneMatrices < 0 ))) ||
        ( m->usesEnv0 && ( m->locEnv0 < 0 )) ||
        ( m->usesEnv1 && ( m->locEnv1 < 0 )) ||
        ( m->usesGlowParam && ( m->locGlowParam < 0 )) ||
        ( m->usesBumpConstants && (( m->locAmbient < 0 ) || ( m->locDiffuse < 0 ) ||
                                    ( m->locGloss < 0 ) || ( m->locSpecular1 < 0 ))))
    {
        printf( "GLSL pilot: %s required uniforms optimized away or missing\n", m->name );
        m->failed = true;
        return false;
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
    if ( ! vertexPgmId )
        return -1;
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
        default: return "dualtex";
    }
}

static bool pilotActivate( int material, const tPilotVertexEntry* entry )
{
    tPilotMaterial* m = &s_materials[material];

    if ( ! m->program && ! pilotInit( material ) )
        return false;

    // the mirrors track the engine constants continuously (see
    // rt_glslpilot_onReflectionParam / rt_glslpilot_onEnvParam /
    // rt_glslpilot_onGlowParam), so they are valid at any activation time
    __glewUseProgram( m->program );
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
    if ( m->usesBumpConstants )
    {
        __glewUniform4fv( m->locAmbient, 1, s_ambientMirror );
        __glewUniform4fv( m->locDiffuse, 1, s_diffuseMirror );
        __glewUniform4fv( m->locGloss, 1, s_glossMirror );
        __glewUniform4fv( m->locSpecular1, 1, s_specular1Mirror );
    }
    s_activeMaterial = material;

    if ( ! m->activationLogged )
    {
        m->activationLogged = true;
        if ( m->vertexKindMask & ( kPilotBumpKindMask | kPilotBumpHQKindMask ))
            printf( "GLSL pilot: %s active (%s vertex variant)\n", m->name,
                pilotVertexKindName( entry->kind ) );
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

    return pilotActivate( material, entry );
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
    // so re-mode the vertex shader (bump variants switch static/skinned
    // instead of lit mode; bone matrices refresh via the mirror push that
    // follows the vertex bind)
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if ( m->locVertexLitMode >= 0 )
            __glewUniform1i( m->locVertexLitMode, entry->vertexLitMode );
        if ( m->locSkinned >= 0 )
            __glewUniform1i( m->locSkinned,
                (( entry->kind == kPilotVertexKind_SkinBump ) ||
                 ( entry->kind == kPilotVertexKind_SkinBumpHQ )) ? 1 : 0 );
    }
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

void rt_glslpilot_noteUnportedFragmentBind( GLuint fragmentPgmId )
{
    // one-time-per-id coverage map: with the pilot on, the client log ends
    // up enumerating every material that still renders through ARB/Cg
    static GLuint seenIds[32];
    static int seenCount = 0;
    int i;

    if ( ! game_state.glslPilot || ( ! fragmentPgmId ) || ( fragmentPgmId == 0xFFFFFFFF ) )
        return;
    for ( i = 0; i < seenCount; i++ )
    {
        if ( seenIds[i] == fragmentPgmId )
            return;
    }
    if ( seenCount < (int)( sizeof( seenIds ) / sizeof( seenIds[0] ) ) )
        seenIds[seenCount++] = fragmentPgmId;
    printf( "GLSL pilot: coverage: unported fragment program %d bound\n", (int)fragmentPgmId );
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

void rt_glslpilot_resetPrograms( void )
{
    // only the vertex table; fragment targets are refreshed independently
    // by rt_glslpilot_setFragmentTarget because shaderMgr_InitVPs and
    // shaderMgr_InitFPs run in no guaranteed order
    s_vertexEntryCount = 0;
}

void rt_glslpilot_addVertexProgram( GLuint vertexPgmId, tPilotVertexKind kind, int vertexLitMode )
{
    if (( vertexPgmId ) && ( s_vertexEntryCount < kPilotMaxVertexEntries ))
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
    if ( game_state.glslPilot )
    {
        printf( "GLSL pilot: fragment targets modulate=%d multiply=%d colorBlendDual=%d addGlow=%d alphaDetail=%d bumpColorBlendDual=%d bumpColorBlendDualHQ=%d, %d registered vertex programs\n",
            (int)s_materials[kPilotMaterial_Modulate].arbFragmentId,
            (int)s_materials[kPilotMaterial_Multiply].arbFragmentId,
            (int)s_materials[kPilotMaterial_ColorBlendDual].arbFragmentId,
            (int)s_materials[kPilotMaterial_AddGlow].arbFragmentId,
            (int)s_materials[kPilotMaterial_AlphaDetail].arbFragmentId,
            (int)s_materials[kPilotMaterial_BumpColorBlendDual].arbFragmentId,
            (int)s_materials[kPilotMaterial_BumpColorBlendDualHQ].arbFragmentId,
            s_vertexEntryCount );
    }
}
