#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "shader.h"

std::experimental::optional<GLuint> load_shaders(
  std::string const& vertex_file_path,
  std::string const& fragment_file_path,
  std::experimental::optional<std::string> const& geom_file_path
) {
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint GeomShaderID;
  if(geom_file_path) {
    GeomShaderID = glCreateShader(GL_GEOMETRY_SHADER);
  }

  // Read the Vertex Shader code from the file
  std::string vert_shader_str;
  std::ifstream vert_shader_file(vertex_file_path, std::ios::in);
  if(vert_shader_file.is_open()){
    std::string line = "";
    while(getline(vert_shader_file, line)) {
      vert_shader_str += "\n" + line;
    }
    vert_shader_file.close();
    std::cerr << "loaded vertex shader " << vertex_file_path << std::endl;
  }
  else {
    std::cerr << "unable to open " << vertex_file_path <<
      ". Check to make sure the file exists and you passed in the right filepath!"
      << std::endl;
    return {};
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string line = "";
    while(getline(FragmentShaderStream, line)) {
      FragmentShaderCode += "\n" + line;
    }
    FragmentShaderStream.close();
  }
  else {
    std::cerr << "unable to open " << fragment_file_path << std::endl;
    return {};
  }

  // Read the Fragment Shader code from the file
  std::string GeomShaderCode;
  if(geom_file_path) {
    std::ifstream GeomShaderStream(*geom_file_path, std::ios::in);
    if(GeomShaderStream.is_open()){
      std::string line = "";
      while(getline(GeomShaderStream, line)) {
        GeomShaderCode += "\n" + line;
      }
      GeomShaderStream.close();
    }
    else {
      std::cerr << "unable to open geom_file_path: " << *geom_file_path << std::endl;
      return {};
    }
  }


  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  std::cout << "Compiling vertex shader :" << vertex_file_path << std::endl;
  char const * VertexSourcePointer = vert_shader_str.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    printf("%s\n", &VertexShaderErrorMessage[0]);
  }
  else {
    printf("Successfully compiled vertex shader!\n");
  }


  // Compile Fragment Shader
  std::cout << "Compiling fragment shader :" << fragment_file_path << std::endl;
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    printf("%s\n", &FragmentShaderErrorMessage[0]);
  }
  else {
    printf("Successfully compiled fragment shader!\n");
  }


  // Compile Fragment Shader
  if(geom_file_path) {
    std::cout << "Compiling geometry shader :" << *geom_file_path << std::endl;
    char const * GeomSourcePointer = GeomShaderCode.c_str();
    glShaderSource(GeomShaderID, 1, &GeomSourcePointer , NULL);
    glCompileShader(GeomShaderID);

    // Check Geom Shader
    glGetShaderiv(GeomShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(GeomShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> GeomShaderErrorMessage(InfoLogLength+1);
      glGetShaderInfoLog(GeomShaderID, InfoLogLength, NULL, &GeomShaderErrorMessage[0]);
      printf("%s\n", &GeomShaderErrorMessage[0]);
    }
    else {
      printf("Successfully compiled fragment shader!\n");
    }
  }



  // Link the program
  printf("Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  if(geom_file_path) {
    glAttachShader(ProgramID, GeomShaderID);
  }
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    std::vector<char> ProgramErrorMessage(InfoLogLength+1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    printf("%s\n", &ProgramErrorMessage[0]);
  }

  glDetachShader(ProgramID, VertexShaderID);
  glDetachShader(ProgramID, FragmentShaderID);

  // glDeleteShader(VertexShaderID);
  // glDeleteShader(FragmentShaderID);

  if(geom_file_path) {
    glDetachShader(ProgramID, GeomShaderID);
    // glDeleteShader(GeomShaderID);
  }


  return {ProgramID};
}
