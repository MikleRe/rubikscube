//
// Created by Dan Simonin on 06.02.2024.
//

int shaderFromFile(const std::string& filePath, GLenum shaderType) {
    // open file
    std::ifstream f;
    f.open(filePath.c_str(), std::ios::in);
    if(!f.is_open()){
        throw std::runtime_error(std::string("Failed to open file: ") + filePath);
    }

    // read whole file into stringstream buffer
    std::stringstream buffer;
    buffer << f.rdbuf();
    buffer << "\0";
    f.close();
    // need to copy, as pointer is deleted when call is finished
    std::string shaderCode = buffer.str().c_str();

    //create new shader
    int ShaderID = glCreateShader(shaderType);

    //set the source code
    const GLchar* code = (const GLchar *) shaderCode.c_str();

    glShaderSource(ShaderID, 1, &code, NULL);
    //compile
    glCompileShader(ShaderID);

    //throw exception if compile error occurred
    GLint status;
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &status);
    std::cout << "Status from compile (" << filePath << ") :" << status << "\r\n";
    if (status == GL_FALSE) {
        std::string msg("Compile failure in shader:\n");

        GLint infoLogLength;
        glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        char* strInfoLog = new char[infoLogLength + 1];
        glGetShaderInfoLog(ShaderID, infoLogLength, NULL, strInfoLog);
        msg += strInfoLog;
        delete[] strInfoLog;

        glDeleteShader(ShaderID); ShaderID = 0;
        throw std::runtime_error(msg);
    }

    return ShaderID;
}

GLuint createShaderProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    GLuint vertexShader = shaderFromFile(vertexShaderPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = shaderFromFile(fragmentShaderPath, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check linking status
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return 0;
    }

    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up texture sampler uniform
    GLint textureUniformLocation = glGetUniformLocation(program, "textureSampler");
    glUniform1i(textureUniformLocation, 0);


    return program;
}