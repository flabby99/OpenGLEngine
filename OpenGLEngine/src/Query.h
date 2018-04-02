#pragma once
#include "ErrorHandling.h"
#include "glew\glew.h"
namespace render
{
  class Query
  {
  private:
    GLuint id_;
    GLenum type_;
  public:
    inline Query(const GLenum& type) : type_(type)
    {
      GLCall(glGenQueries(1, &id_));
    }
    inline ~Query()
    {
      GLCall(glDeleteQueries(1, &id_));
    }
    inline void Begin() {
      GLCall(glBeginQuery(type_, id_));
    }
    inline void End() {
      GLCall(glEndQuery(type_));
    }
    inline void Result(GLuint *result) {
      GLCall(glGetQueryObjectuiv(id_, GL_QUERY_RESULT, result));
    }
    inline void isResultReady(GLint *result) {
      GLCall(glGetQueryObjectiv(id_, GL_QUERY_RESULT_AVAILABLE, result));
    }
    //The following does not modify result if the query is not available
    inline void ResultNoWait(GLuint *result) {
      GLCall(glGetQueryObjectuiv(id_, GL_QUERY_RESULT_NO_WAIT, result));
    }
  };
}
