// Native GLSL pilot path for BLENDMODE_MODULATE. See rt_glslpilot.h.
//
// The shader math replicates the Cg sources shipped in the piggs:
//   shaders/cgfx/modulatefp.cg          (fragment)
//   shaders/cgfx/vp_master_vp.cg        (vertex, DUALTEX variants)
//   shaders/cgfx/functions.cgh          (calc_fogged_color, faux reflection)
// The GLSL builds on compatibility-profile built-ins (gl_ModelViewMatrix,
// gl_TextureMatrix, gl_LightSource[0], gl_Fog, gl_Color, gl_FogFragCoord)
// which read the same GL server state the Cg `state.*` semantics read, so
// the engine needs no new parameter plumbing beyond g_ReflectionParamVP.

#include "render/thread/ogl.h"
#include "render/thread/rt_glslpilot.h"
#include "cmdparse/cmdgame.h"
#include <string.h>

// ogl.h #undef's the GLEW macro names for GL 2.0 shader entry points as a
// "do not use" policy for the fixed pipelines; the underlying GLEW function
// pointers (GLEW_GET_FUN targets, gl-prefix stripped) remain the supported
// way to reach them.

static const char* s_pilotVertexSource =
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

static const char* s_pilotFragmentSource =
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

// variants.cgh values for g_VertexLitMode
enum {
    kPilotVertLit_VertColor = 1,
    kPilotVertLit_FF_Lit    = 4,
    kPilotVertLit_FF_Unlit  = 5,
};

typedef struct tPilotVertexEntry
{
    GLuint        pgmId;
    int            vertexLitMode;
} tPilotVertexEntry;

#define kPilotMaxVertexEntries 8

static tPilotVertexEntry    s_vertexEntries[kPilotMaxVertexEntries];
static int                    s_vertexEntryCount = 0;
static GLuint                s_fragmentTarget = 0;

static GLuint                s_program = 0;
static GLint                s_locReflectionParam = -1;
static GLint                s_locVertexLitMode = -1;
static bool                s_active = false;
static bool                s_failed = false;
static GLfloat                s_reflectionParamMirror[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

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

static bool pilotInit( void )
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint status = 0;

    s_program = __glewCreateProgram();
    if ( ! s_program )
    {
        printf( "GLSL pilot: glCreateProgram failed (GL 2.0 entry points missing?)\n" );
        s_failed = true;
        return false;
    }

    vertexShader = pilotCompileShader( GL_VERTEX_SHADER, s_pilotVertexSource, "vertex shader" );
    fragmentShader = pilotCompileShader( GL_FRAGMENT_SHADER, s_pilotFragmentSource, "fragment shader" );
    if ( ! vertexShader || ! fragmentShader )
    {
        s_failed = true;
        return false;
    }

    __glewAttachShader( s_program, vertexShader );
    __glewAttachShader( s_program, fragmentShader );
    __glewDeleteShader( vertexShader );
    __glewDeleteShader( fragmentShader );

    __glewLinkProgram( s_program );
    __glewGetProgramiv( s_program, GL_LINK_STATUS, &status );
    if ( ! status )
    {
        char infoLog[2048];
        infoLog[0] = '\0';
        __glewGetProgramInfoLog( s_program, sizeof(infoLog) - 1, NULL, infoLog );
        printf( "GLSL pilot: program link failed:\n%s\n", infoLog );
        s_failed = true;
        return false;
    }

    s_locReflectionParam = __glewGetUniformLocation( s_program, "g_ReflectionParamVP" );
    s_locVertexLitMode = __glewGetUniformLocation( s_program, "g_VertexLitMode" );
    if (( s_locReflectionParam < 0 ) || ( s_locVertexLitMode < 0 ))
    {
        printf( "GLSL pilot: required uniforms optimized away or missing\n" );
        s_failed = true;
        return false;
    }

    // sampler units are fixed for this material: base on TEXUNIT0, blend on TEXUNIT1
    __glewUseProgram( s_program );
    __glewUniform1i( __glewGetUniformLocation( s_program, "sampler_base" ), 0 );
    __glewUniform1i( __glewGetUniformLocation( s_program, "sampler_blend" ), 1 );
    __glewUseProgram( 0 );

    printf( "GLSL pilot: BLENDMODE_MODULATE program compiled and linked\n" );
    return true;
}

static void pilotDeactivate( void )
{
    if ( s_active )
    {
        __glewUseProgram( 0 );
        s_active = false;
    }
}

static int pilotFindVertexMode( GLuint vertexPgmId )
{
    int i;
    for ( i = 0; i < s_vertexEntryCount; i++ )
    {
        if ( s_vertexEntries[i].pgmId == vertexPgmId )
            return s_vertexEntries[i].vertexLitMode;
    }
    return 0;
}

static bool pilotActivate( int vertexLitMode )
{
    if ( ! s_program && ! pilotInit() )
        return false;

    // s_reflectionParamMirror tracks the engine constant continuously (see
    // rt_glslpilot_onReflectionParam), so it is valid at any activation time
    __glewUseProgram( s_program );
    __glewUniform4fv( s_locReflectionParam, 1, s_reflectionParamMirror );
    __glewUniform1i( s_locVertexLitMode, vertexLitMode );
    s_active = true;
    return true;
}

bool rt_glslpilot_tryBindFragment( GLuint fragmentPgmId, GLuint vertexPgmId )
{
    int vertexLitMode;

    if ( s_active && ( fragmentPgmId != s_fragmentTarget ) )
        pilotDeactivate();

    if ( ! game_state.glslPilot || s_failed || ( fragmentPgmId != s_fragmentTarget ) )
        return false;

    vertexLitMode = pilotFindVertexMode( vertexPgmId );
    if ( ! vertexLitMode )
    {
        pilotDeactivate();
        return false;
    }

    return pilotActivate( vertexLitMode );
}

void rt_glslpilot_tryBindVertex( GLuint vertexPgmId, GLuint fragmentPgmId )
{
    int vertexLitMode = pilotFindVertexMode( vertexPgmId );

    if ( ! s_active )
    {
        if (( ! game_state.glslPilot ) || s_failed || ( fragmentPgmId != s_fragmentTarget ) || ( ! vertexLitMode ))
            return;
        pilotActivate( vertexLitMode );
        return;
    }

    if ( fragmentPgmId != s_fragmentTarget )
    {
        // the tracked fragment program moved off the pilot target without a
        // bind call to tell us; stop overriding the pipeline
        pilotDeactivate();
        return;
    }

    if ( ! vertexLitMode )
    {
        pilotDeactivate();
        return;
    }

    // still the modulate fragment target; a registered vertex variant took
    // over, so just re-mode the vertex shader
    __glewUniform1i( s_locVertexLitMode, vertexLitMode );
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
    return s_active;
}

void rt_glslpilot_onReflectionParam( const GLfloat* vec4 )
{
    // always mirror, active or not: the value must be correct whenever the
    // pilot activates next, and engine bind order is not guaranteed relative
    // to activation
    memcpy( s_reflectionParamMirror, vec4, sizeof( s_reflectionParamMirror ) );
    if ( s_active && ( s_locReflectionParam >= 0 ) )
    {
        __glewUniform4fv( s_locReflectionParam, 1, s_reflectionParamMirror );
    }
}

void rt_glslpilot_resetPrograms( void )
{
    // only the vertex table; the fragment target is refreshed independently
    // by rt_glslpilot_setFragmentTarget because shaderMgr_InitVPs and
    // shaderMgr_InitFPs run in no guaranteed order
    s_vertexEntryCount = 0;
}

void rt_glslpilot_addVertexProgram( GLuint vertexPgmId, int vertexLitMode )
{
    if (( vertexPgmId ) && ( s_vertexEntryCount < kPilotMaxVertexEntries ))
    {
        s_vertexEntries[s_vertexEntryCount].pgmId = vertexPgmId;
        s_vertexEntries[s_vertexEntryCount].vertexLitMode = vertexLitMode;
        s_vertexEntryCount++;
    }
}

void rt_glslpilot_setFragmentTarget( GLuint fragmentPgmId )
{
    s_fragmentTarget = fragmentPgmId;
    if ( game_state.glslPilot )
    {
        printf( "GLSL pilot: fragment target id %d, %d registered vertex programs\n", (int)fragmentPgmId, s_vertexEntryCount );
    }
}
