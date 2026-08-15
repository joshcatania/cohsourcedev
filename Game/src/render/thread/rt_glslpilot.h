#ifndef _RT_GLSLPILOT_H
#define _RT_GLSLPILOT_H

// Native GLSL pilot path (development experiment).
//
// Renders BLENDMODE_MODULATE, BLENDMODE_MULTIPLY, and
// BLENDMODE_COLORBLEND_DUAL draws through hand-written GLSL 1.20
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
// consumes g_Env0FP and g_GlowParamFP (the window-glow threshold/seed).
typedef enum ePilotMaterial {
    kPilotMaterial_Modulate = 0,    // modulatefp.cg
    kPilotMaterial_Multiply,        // multiplyRegfp.cg
    kPilotMaterial_ColorBlendDual,  // colorBlendDualfp.cg
    kPilotMaterial_AddGlow,         // addglowfp.cg
    kPilotMaterial_AlphaDetail,     // alphaDetailfp.cg
    kPilotMaterial_Count
} tPilotMaterialId;

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

// Coverage diagnostic: called by WCW_BindFragmentProgram for binds the pilot
// did not handle. Logs each distinct fragment program id once (per process)
// so a capture's client log enumerates which materials still render through
// the ARB/Cg path. No-ops unless the pilot switch is on.
void rt_glslpilot_noteUnportedFragmentBind( GLuint fragmentPgmId );

// Program registration from rt_shaderMgr.c. vertexLitMode uses the
// variants.cgh values (VERT_COLOR=1, FF_LIT_GL=4, FF_UNLIT_GL=5); only
// registered vertex programs can activate the pilot. Re-runs after every
// shader reload because program ids are regenerated.
void rt_glslpilot_resetPrograms( void );
void rt_glslpilot_addVertexProgram( GLuint vertexPgmId, int vertexLitMode );
void rt_glslpilot_setFragmentTarget( tPilotMaterialId material, GLuint fragmentPgmId );

#endif
