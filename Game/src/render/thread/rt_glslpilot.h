#ifndef _RT_GLSLPILOT_H
#define _RT_GLSLPILOT_H

// Native GLSL pilot path (development experiment).
//
// Renders BLENDMODE_MODULATE draws through one hand-written GLSL 1.20
// compatibility-profile program instead of the Cg->ARB pipeline, to prove
// that a material can be moved to native GLSL without a visual regression
// (verified with agent/compare-captures.ps1). The GLSL program rides on top
// of the existing ARB binding state: while it is bound with glUseProgram it
// simply overrides the bound ARB vertex/fragment programs, and unbinding it
// restores them untouched. See docs/agent-status.md for the pilot report.

// Called by WCW_BindFragmentProgram. Returns true when the pilot handled
// the bind (the GLSL program is in use and the caller must not bind the
// ARB/Cg fragment program). vertexPgmId is the currently bound vertex
// program; the pilot only activates for vertex variants it replicates.
bool rt_glslpilot_tryBindFragment( GLuint fragmentPgmId, GLuint vertexPgmId );

// Called by WCW_BindVertexProgram with the incoming vertex program id and
// the currently bound fragment program id. The engine's binds are
// state-cached, so the (modulate fragment, DUALTEX vertex) pairing can be
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

// Mirror of the engine constant the pilot cannot read from GL state.
// g_ReflectionParamVP is written to program.env[1] (vertex target) by
// WCW_SetCgShaderParamArray4fv; GLSL has no access to program env
// registers, so the engine pushes it into the pilot's uniform as well.
void rt_glslpilot_onReflectionParam( const GLfloat* vec4 );

// Program registration from rt_shaderMgr.c. vertexLitMode uses the
// variants.cgh values (VERT_COLOR=1, FF_LIT_GL=4, FF_UNLIT_GL=5); only
// registered vertex programs can activate the pilot. Re-runs after every
// shader reload because program ids are regenerated.
void rt_glslpilot_resetPrograms( void );
void rt_glslpilot_addVertexProgram( GLuint vertexPgmId, int vertexLitMode );
void rt_glslpilot_setFragmentTarget( GLuint fragmentPgmId );

#endif
