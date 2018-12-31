#include "shader.h"

#include <stdio.h>
#include "macro.h"
#include "glad.h"
#include "fnv.h"

enum ErrorCheckType
{
    Shader = 0,
    Program,
};

void CheckCompileErrors(uint32_t id, ErrorCheckType type)
{
    int32_t success = 0;
    char infoLog[1024] = {0};
    switch(type)
    {
        case Shader:
        {
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(id, 1024, nullptr, infoLog);
                puts(infoLog);
                Assert(false);
            }
        }
        break;
        case Program:
        {
            glGetProgramiv(id, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(id, 1024, nullptr, infoLog);
                puts(infoLog);
                Assert(false);
            }
        }
        break;
    }
}

void GLShader::Init(const char* vs, const char* fs)
{
    m_id = 0;

    DebugGL();
    uint32_t vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vs, nullptr);
    glCompileShader(vertex);
    CheckCompileErrors(vertex, Shader);

    DebugGL();
    uint32_t fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fs, nullptr);
    glCompileShader(fragment);
    CheckCompileErrors(fragment, Shader);

    DebugGL();
    m_id = glCreateProgram();
    glAttachShader(m_id, vertex);
    glAttachShader(m_id, fragment);
    glLinkProgram(m_id);
    CheckCompileErrors(m_id, Program);
    
    DebugGL();
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    DebugGL();
}
void GLShader::Shutdown()
{
    glDeleteProgram(m_id);
    DebugGL();
    m_id = 0;
}
void GLShader::Use()
{
    glUseProgram(m_id);
    DebugGL();
}
void GLShader::SetInt(const char* name, int32_t value)
{
    glUniform1i(GetUniformLocation(name), value);
    DebugGL();
}
void GLShader::SetFloat(const char* name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
    DebugGL();
}
void GLShader::SetVec2(const char* name, const vec2& value)
{
    glUniform2fv(GetUniformLocation(name), 1, &value[0]);
    DebugGL();
}
void GLShader::SetVec3(const char* name, const vec3& value)
{
    glUniform3fv(GetUniformLocation(name), 1, &value[0]);
    DebugGL();
}
void GLShader::SetVec4(const char* name, const vec4& value)
{
    glUniform4fv(GetUniformLocation(name), 1, &value[0]);
    DebugGL();
}
void GLShader::SetMat3(const char* name, const mat3& value)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    DebugGL();
}
void GLShader::SetMat4(const char* name, const mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    DebugGL();
}
int32_t GLShader::GetUniformLocation(const char* name)
{
    uint64_t hash = Fnv64(name);
    int32_t idx = m_uniforms.find({ -1, hash });
    if(idx == -1)
    {
        idx = m_uniforms.count();
        int32_t loc = glGetUniformLocation(m_id, name);
        DebugGL();
        //Assert(loc != -1); // Sometimes deadstripped by linker
        m_uniforms.grow() = 
        {
            loc,
            hash
        };
    }
    return m_uniforms[idx].location;
}
