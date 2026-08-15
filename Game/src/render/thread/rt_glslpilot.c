// Native GLSL pilot path for BLENDMODE_MODULATE and BLENDMODE_MULTIPLY.
// See rt_glslpilot.h.
//
// The shader math replicates the Cg sources shipped in the piggs:
//   shaders/cgfx/modulatefp.cg          (modulate fragment)
//   shaders/cgfx/multiplyRegfp.cg       (multiply fragment)
//   shaders/cgfx/vp_master_vp.cg        (vertex, DUALTEX variants)
//   shaders/cgfx/functions.cgh          (calc_fogged_color, faux reflection)
// The GLSL builds on compatibility-profile built-ins (gl_ModelViewMatrix,
// gl_TextureMatrix, gl_LightSource[0], gl_Fog, gl_Color, gl_FogFragCoord)
// which read the same GL server state the Cg `state.*` semantics read, so
// the engine needs no new parameter plumbing beyond the two mirrored
// program-local constants (g_ReflectionParamVP, g_Env0FP).

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

typedef struct tPilotSampler
{
    const char*    name;                // GLSL sampler uniform name
    int                unit;                // fixed texture unit (TEXUNITn)
} tPilotSampler;

typedef struct tPilotMaterial
{
    GLuint        arbFragmentId;        // target ARB/Cg program id (0 = none)
    const char*    name;                // for logging
    const char*    fragmentSource;      // GLSL fragment source
    const tPilotSampler* samplers;      // NULL-terminated sampler/unit map
    bool            usesEnv0;            // fragment consumes the g_Env0FP mirror
    bool            usesEnv1;            // fragment consumes the g_Env1FP mirror
    bool            usesGlowParam;        // fragment consumes the g_GlowParamFP mirror
    GLuint        program;                // compiled GLSL program (0 = not yet)
    GLint            locReflectionParam;
    GLint            locVertexLitMode;
    GLint            locEnv0;
    GLint            locEnv1;
    GLint            locGlowParam;
    bool            failed;
    bool            activationLogged;    // one-time activation evidence for logs
    bool            declineLogged;        // one-time unregistered-vertex diagnostic
} tPilotMaterial;

#define kPilotMaxVertexEntries 8

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

static tPilotMaterial s_materials[kPilotMaterial_Count] = {
    { 0, "BLENDMODE_MODULATE",       s_modulateFragmentSource,       s_dualSamplers,     false, false, false, 0, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_MULTIPLY",       s_multiplyFragmentSource,       s_dualSamplers,     true,  false, false, 0, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_COLORBLEND_DUAL", s_colorBlendDualFragmentSource, s_dualTintSamplers, true,  true,  false, 0, -1, -1, -1, -1, -1, false, false, false },
    { 0, "BLENDMODE_ADDGLOW",        s_addGlowFragmentSource,        s_addGlowSamplers,  true,  false, true,  0, -1, -1, -1, -1, -1, false, false, false },
};

static int                    s_activeMaterial = -1;    // -1 = pilot inactive
static GLuint                s_vertexShader = 0;        // shared: one source serves all materials
static bool                    s_vertexShaderFailed = false;
static GLfloat                s_reflectionParamMirror[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat                s_envMirrors[2][4] = { { 1.0f, 1.0f, 1.0f, 1.0f },
                                                     { 1.0f, 1.0f, 1.0f, 1.0f } };
static GLfloat                s_glowParamMirror[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

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

static bool pilotInit( int material )
{
    tPilotMaterial* m = &s_materials[material];
    GLuint vertexShader = pilotGetVertexShader();
    GLuint fragmentShader;
    GLint status = 0;

    m->program = __glewCreateProgram();
    if (( ! m->program ) || ( ! vertexShader ))
    {
        printf( "GLSL pilot: %s program creation failed (GL 2.0 entry points missing?)\n", m->name );
        m->failed = true;
        return false;
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
    // the shared vertex shader object is intentionally kept alive for the
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
    if (( m->locReflectionParam < 0 ) || ( m->locVertexLitMode < 0 ) ||
        ( m->usesEnv0 && ( m->locEnv0 < 0 )) ||
        ( m->usesEnv1 && ( m->locEnv1 < 0 )) ||
        ( m->usesGlowParam && ( m->locGlowParam < 0 )))
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

static bool pilotActivate( int material, int vertexLitMode )
{
    tPilotMaterial* m = &s_materials[material];

    if ( ! m->program && ! pilotInit( material ) )
        return false;

    // the mirrors track the engine constants continuously (see
    // rt_glslpilot_onReflectionParam / rt_glslpilot_onEnvParam /
    // rt_glslpilot_onGlowParam), so they are valid at any activation time
    __glewUseProgram( m->program );
    __glewUniform4fv( m->locReflectionParam, 1, s_reflectionParamMirror );
    __glewUniform1i( m->locVertexLitMode, vertexLitMode );
    if ( m->locEnv0 >= 0 )
        __glewUniform4fv( m->locEnv0, 1, s_envMirrors[0] );
    if ( m->locEnv1 >= 0 )
        __glewUniform4fv( m->locEnv1, 1, s_envMirrors[1] );
    if ( m->locGlowParam >= 0 )
        __glewUniform4fv( m->locGlowParam, 1, s_glowParamMirror );
    s_activeMaterial = material;

    if ( ! m->activationLogged )
    {
        m->activationLogged = true;
        printf( "GLSL pilot: %s active (vertex lit mode %d)\n", m->name, vertexLitMode );
    }
    return true;
}

bool rt_glslpilot_tryBindFragment( GLuint fragmentPgmId, GLuint vertexPgmId )
{
    int material = pilotFindMaterial( fragmentPgmId );
    int vertexLitMode;

    if (( s_activeMaterial >= 0 ) && ( s_activeMaterial != material ))
        pilotDeactivate();

    if ( ! game_state.glslPilot || ( material < 0 ) || s_materials[material].failed )
        return false;

    vertexLitMode = pilotFindVertexMode( vertexPgmId );
    if ( ! vertexLitMode )
    {
        if ( ! s_materials[material].declineLogged )
        {
            s_materials[material].declineLogged = true;
            printf( "GLSL pilot: %s bind declined, vertex program %d not registered\n",
                s_materials[material].name, (int)vertexPgmId );
        }
        pilotDeactivate();
        return false;
    }

    return pilotActivate( material, vertexLitMode );
}

void rt_glslpilot_tryBindVertex( GLuint vertexPgmId, GLuint fragmentPgmId )
{
    int vertexLitMode = pilotFindVertexMode( vertexPgmId );
    int material = pilotFindMaterial( fragmentPgmId );

    if ( s_activeMaterial < 0 )
    {
        if (( ! game_state.glslPilot ) || ( material < 0 ) || s_materials[material].failed || ( ! vertexLitMode ))
            return;
        pilotActivate( material, vertexLitMode );
        return;
    }

    if ( s_activeMaterial != material )
    {
        // the tracked fragment program moved off the pilot targets without
        // a bind call to tell us; stop overriding the pipeline
        pilotDeactivate();
        return;
    }

    if ( ! vertexLitMode )
    {
        pilotDeactivate();
        return;
    }

    // still a pilot fragment target; a registered vertex variant took over,
    // so just re-mode the vertex shader
    __glewUniform1i( s_materials[s_activeMaterial].locVertexLitMode, vertexLitMode );
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

void rt_glslpilot_resetPrograms( void )
{
    // only the vertex table; fragment targets are refreshed independently
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

void rt_glslpilot_setFragmentTarget( tPilotMaterialId material, GLuint fragmentPgmId )
{
    if (( material < 0 ) || ( material >= kPilotMaterial_Count ))
        return;
    s_materials[material].arbFragmentId = fragmentPgmId;
    if ( game_state.glslPilot )
    {
        printf( "GLSL pilot: fragment targets modulate=%d multiply=%d colorBlendDual=%d addGlow=%d, %d registered vertex programs\n",
            (int)s_materials[kPilotMaterial_Modulate].arbFragmentId,
            (int)s_materials[kPilotMaterial_Multiply].arbFragmentId,
            (int)s_materials[kPilotMaterial_ColorBlendDual].arbFragmentId,
            (int)s_materials[kPilotMaterial_AddGlow].arbFragmentId,
            s_vertexEntryCount );
    }
}
