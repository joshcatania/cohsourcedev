#ifndef _RT_GLSLPILOT_H
#define _RT_GLSLPILOT_H

// Native GLSL pilot path (development experiment).
//
// Renders the simple materials (MODULATE, MULTIPLY, COLORBLEND_DUAL, ADDGLOW,
// ALPHADETAIL) and the bumped dual-tint material (BUMPMAP_COLORBLEND_DUAL,
// default and BIT_HIGH_QUALITY variants) through hand-written GLSL 1.20
// compatibility-profile programs instead of the Cg->ARB pipeline, to prove
// that materials can be moved to native GLSL without a visual regression
// (verified with agent/compare-captures.ps1). The GLSL programs ride on top
// of the existing ARB binding state: while a pilot program is bound with
// glUseProgram it simply overrides the bound ARB vertex/fragment programs,
// and unbinding it restores them untouched. See docs/agent-status.md for
// the pilot report.

// Which material a fragment program id belongs to. Multiply consumes the
// g_Env0FP engine constant (constColor0); colorBlendDual consumes g_Env0FP
// and g_Env1FP (constColor0/constColor1, the dual tint colors); addGlow
// consumes g_Env0FP and g_GlowParamFP (the window-glow threshold/seed);
// bumpColorBlendDual additionally consumes the bump lighting constants
// (ambient/diffuse/gloss/specular1 fragment constants, lightdir vertex
// constant) and reads the tangent vertex attribute.
typedef enum ePilotMaterial {
    kPilotMaterial_Modulate = 0,    // modulatefp.cg
    kPilotMaterial_Multiply,        // multiplyRegfp.cg
    kPilotMaterial_ColorBlendDual,  // colorBlendDualfp.cg
    kPilotMaterial_AddGlow,         // addglowfp.cg
    kPilotMaterial_AlphaDetail,     // alphaDetailfp.cg
    kPilotMaterial_BumpColorBlendDual, // bumpmapColorblendDualfp.cg (variant 0)
    kPilotMaterial_BumpColorBlendDualHQ, // bumpmapColorblendDualfp.cg (BIT_HIGH_QUALITY)
    // effects/ post-processing family (rt_effects.c fullscreen quads; they
    // pair with the DRAWMODE_SPRITE dualtex vertex variant that the 2D
    // rendering setup force-binds). SHADER_SUNFLAREADAPTATION and
    // SHADER_PERFORMANCE_TEST are not ported.
    kPilotMaterial_FxShrinkExtend,      // shrinkfp.cg HIGH_RANGE
    kPilotMaterial_FxHBlur,             // blurfp.cg BLUR_HOR
    kPilotMaterial_FxVBlur,             // blurfp.cg BLUR_VER
    kPilotMaterial_FxTonemap,           // tonemapfp.cg
    kPilotMaterial_FxShrink2,           // shrinkfp.cg
    kPilotMaterial_FxShrink2Dof,        // shrinkfp.cg (DOF branch compiled out)
    kPilotMaterial_FxShrink4,           // shrink4xfp.cg
    kPilotMaterial_FxShrink4Lum,        // shrink4xfp.cg USE_LUMINANCE
    kPilotMaterial_FxShrink4Exp,        // shrink4xfp.cg USE_EXP
    kPilotMaterial_FxLightAdaptation,   // lightAdaptationfp.cg
    kPilotMaterial_FxLog,               // logfp.cg
    kPilotMaterial_FxBrightpass,        // brightpassfp.cg
    kPilotMaterial_FxTonemap2,          // tonemap2fp.cg
    kPilotMaterial_FxTonemap2Desat,     // tonemap2fp.cg DESATURATE
    kPilotMaterial_FxDofFinal,          // dofFinalfp.cg
    kPilotMaterial_FxDofFinalDesat,     // dofFinalfp.cg DESATURATE
    kPilotMaterial_FxDofBloomFinal,     // dofBloomFinalfp.cg
    kPilotMaterial_FxDofBloomFinalDesat,// dofBloomFinalfp.cg DESATURATE
    kPilotMaterial_FxSimpleDesaturate,  // simple_desaturatefp.cg
    kPilotMaterial_Count
} tPilotMaterialId;

// Effects fragment constants (per-program local params in the Cg sources —
// several share local slot numbers across different programs, so the mirror
// is keyed by constant identity, not slot).
typedef enum ePilotFxConst {
    kPilotFxConst_TextTransform = 0,    // g_Effects_TextTransformFP
    kPilotFxConst_ExpectedLum,          // g_Effects_ExpectedLumFP
    kPilotFxConst_TimeStep,             // g_TimeStepFP
    kPilotFxConst_DofParam2,            // g_Effects_DofParam2FP
    kPilotFxConst_DofProject,           // g_Effects_DofProjectFP
    kPilotFxConst_DesaturateParam,      // g_Effects_DesaturateParamFP
    kPilotFxConst_Count
} tPilotFxConstId;

// Which replicated vp_master_vp.cg variant a registered vertex program id
// was built from. The simple materials pair with the DUALTEX-family
// variants (differing only in VERTEX_LIT mode); the bump material pairs
// with the bump_dual variant (static geometry) and the skin_bump variant
// (boned models: the player/NPC costumes), both VERTEX_LIT=NONE
// TC_XFORM=NONE PIXEL_LIT=BUMP_ALL — the pilot's bump vertex shader covers
// both behind a skinning uniform.
typedef enum ePilotVertexKind {
    kPilotVertexKind_DualTex = 0,
    kPilotVertexKind_BumpDual,
    kPilotVertexKind_SkinBump,
    kPilotVertexKind_BumpDualHQ,    // shaderMgrVertexProgramsHQ[DRAWMODE_BUMPMAP_DUALTEX]
    kPilotVertexKind_SkinBumpHQ,    // shaderMgrVertexProgramsHQ[DRAWMODE_BUMPMAP_SKINNED]
    kPilotVertexKind_FixedFunction, // no vertex program (pbuffer effects passes)
} tPilotVertexKind;

// Called by WCW_BindFragmentProgram. Returns true when the pilot handled
// the bind (a GLSL program is in use and the caller must not bind the
// ARB/Cg fragment program). vertexPgmId is the currently bound vertex
// program; the pilot only activates for vertex variants it replicates.
bool rt_glslpilot_tryBindFragment( GLuint fragmentPgmId, GLuint vertexPgmId );

// Called by WCW_BindVertexProgram with the incoming vertex program id and
// the currently bound fragment program id. The engine's binds are
// state-cached, so a (pilot fragment, replicated vertex) pairing can be
// observed on either bind path; this activates, re-modes, or deactivates
// the pilot accordingly. The caller still binds the ARB vertex program for
// state tracking; the GLSL program overrides it while active.
void rt_glslpilot_tryBindVertex( GLuint vertexPgmId, GLuint fragmentPgmId );

// Called whenever the engine disables a program or resets program state.
// Deactivates the pilot so the ARB programs the engine is about to manage
// take effect again.
void rt_glslpilot_onVertexProgramChange( GLuint vertexPgmId );
void rt_glslpilot_onFragmentProgramDisable( void );

bool rt_glslpilot_isActive( void );

// Mirrors of the engine constants the pilot cannot read from GL state.
// g_ReflectionParamVP is written to program.env[1] (vertex target) by
// WCW_SetCgShaderParamArray4fv; g_Env0FP/g_Env1FP are written to fragment
// program.env[8]/env[9] (TIE(ENV8)/TIE(ENV9)) by setFragmentProgramConstColor
// from the engine's constColor0/constColor1; g_GlowParamFP is a program
// local constant pushed by WCW_SetCgShaderParamArray4fv from rt_tricks.c.
// GLSL has no access to program env registers, so the engine pushes these
// into the pilot's uniforms as well; all are mirrored continuously, not
// just while active, so they are correct at any activation time.
// index 0 = g_Env0FP, 1 = g_Env1FP.
void rt_glslpilot_onReflectionParam( const GLfloat* vec4 );
void rt_glslpilot_onEnvParam( int index, const GLfloat* vec4 );
void rt_glslpilot_onGlowParam( const GLfloat* vec4 );

// Mirrors of the bump lighting constants (consumed by the
// bumpColorBlendDual materials): g_LightDirVP is the view-space light
// direction pushed to the vertex program by rt_model.c/rt_bonedmodel.c;
// g_LightDirFP is its fragment-program sibling (TIE(ENV11)) that the HQ
// fragment variant consumes instead, pushed by setupBumpPixelShader only
// when BMB_HIGH_QUALITY is set;
// g_AmbientColorFP/g_DiffuseColorFP/g_GlossParamFP/g_Specular1ColorAndExponentFP
// are the fragment constants pushed by setupBumpPixelShader/setupSpecularColor
// (TIE(ENV0)/ENV1/ENV2/ENV5); the bone matrix array (TIE(ENV16), up to 48
// vec4s = 16 bones x 3 rows) is pushed by loadBoneMatrices for the skinned
// bump draws. Same always-mirror contract as above.
void rt_glslpilot_onLightDirParam( const GLfloat* vec4 );
void rt_glslpilot_onLightDirFPParam( const GLfloat* vec4 );

// Mirror for the effects/post-processing fragment constants (consumed by the
// kPilotMaterial_Fx* materials). These are program-local params in the
// Cg sources, pushed by rt_effects.c between the effects program binds and
// their draws; same always-mirror contract. fxConstSlot is a
// tPilotFxConstId.
void rt_glslpilot_onEffectsParam( int fxConstSlot, const GLfloat* vec4 );
void rt_glslpilot_onAmbientColorParam( const GLfloat* vec4 );
void rt_glslpilot_onDiffuseColorParam( const GLfloat* vec4 );
void rt_glslpilot_onGlossParam( const GLfloat* vec4 );
void rt_glslpilot_onSpecular1Param( const GLfloat* vec4 );
void rt_glslpilot_onBoneMatrixParam( const GLfloat* vec4Arr, GLuint nNumVec4s );

// Coverage diagnostic: called by WCW_BindFragmentProgram for binds the pilot
// did not handle. Logs each distinct fragment program id once (per process)
// so a capture's client log enumerates which materials still render through
// the ARB/Cg path. No-ops unless the pilot switch is on.
void rt_glslpilot_noteUnportedFragmentBind( GLuint fragmentPgmId );

// Program registration from rt_shaderMgr.c. vertexLitMode uses the
// variants.cgh values (VERT_COLOR=1, FF_LIT_GL=4, FF_UNLIT_GL=5; NONE=0 for
// the bump_dual variant, which has no vertex-lit mode); only registered
// vertex programs can activate the pilot, and the kind must match the
// material's replicated vertex variant. Re-runs after every shader reload
// because program ids are regenerated.
void rt_glslpilot_resetPrograms( void );
void rt_glslpilot_addVertexProgram( GLuint vertexPgmId, tPilotVertexKind kind, int vertexLitMode );
void rt_glslpilot_setFragmentTarget( tPilotMaterialId material, GLuint fragmentPgmId );

#endif
