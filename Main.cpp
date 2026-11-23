

#include <iostream>   // Para imprimir mensajes en consola (std::cout)
#include <vector>     // Para guardar listas de datos (vértices, índices, mallas)
#include <cmath>      // Para std::fmod, std::atan2 y funciones matemáticas

#include <glad/glad.h>        // Carga las funciones de OpenGL (debe ir antes de GLFW)
#include <GLFW/glfw3.h>       // Crea la ventana y maneja teclado y mouse

// stb_image: librería para leer imágenes (jpg, png, etc.)
// La definición de STB_IMAGE_IMPLEMENTATION hace que se incluya el código de la librería una sola vez.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLM: librería de matemáticas para gráficos (vectores y matrices)
#include <glm/glm.hpp>                        // Vectores y matrices básicas
#include <glm/gtc/matrix_transform.hpp>       // translate, rotate, perspective, lookAt
#include <glm/gtc/type_ptr.hpp>               // value_ptr: convierte matrices a punteros para OpenGL

// Assimp: librería para leer modelos 3D (gltf, obj, fbx, etc.)
#include <assimp/Importer.hpp>        // Clase Importer, que abre el archivo del modelo
#include <assimp/scene.h>             // Estructuras de la escena (aiScene, aiMesh, aiNode...)
#include <assimp/postprocess.h>       // Opciones de procesamiento al cargar (triangular, voltear UVs...)


// CONFIGURACIÓN DE LA VENTANA
const unsigned int SCR_WIDTH = 800;   // Ancho de la ventana en píxeles
const unsigned int SCR_HEIGHT = 800;  // Alto de la ventana en píxeles

// CÁMARA
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);  // Dónde está la cámara en el mundo
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Hacia dónde mira la cámara
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);  // Cuál es "arriba" para la cámara

// TIEMPO
float deltaTime = 0.0f;   // Tiempo que pasó entre un frame y el anterior (para que el movimiento sea parejo)
float lastFrame = 0.0f;   // Momento en que empezó el frame anterior

// MOUSE (para girar la cámara)
bool  firstMouse = true;  // Indica si es la primera vez que se mueve el mouse
float yaw = -90.0f;     // Ángulo horizontal de la cámara (girar a izquierda/derecha)
float pitch = 0.0f;       // Ángulo vertical de la cámara (mirar arriba/abajo)
float lastX = 400.0f;     // Última posición X del mouse
float lastY = 400.0f;     // Última posición Y del mouse

// CURVA DE BÉZIER
// Una curva de Bézier cúbica se define con 4 puntos de control.
// La curva empieza en el primero, termina en el último, y los otros dos
// le dan la forma. El sillón se mueve sobre esta curva.
glm::vec3 bezierP[4] = {
    glm::vec3(-2.0f, -1.0f,  0.0f),   // P0: punto inicial
    glm::vec3(-1.0f,  2.5f, -1.0f),   // P1: controla la curva al inicio
    glm::vec3(1.0f, -2.5f, -1.0f),   // P2: controla la curva al final
    glm::vec3(2.0f,  1.0f,  0.0f)    // P3: punto final
};

// Devuelve el punto de la curva en la posición t (t va de 0 a 1).
// t = 0 es el inicio de la curva, t = 1 es el final.
glm::vec3 bezierPoint(float t) {
    float u = 1.0f - t;   // Complemento de t
    return u * u * u * bezierP[0] + 3.0f * u * u * t * bezierP[1] + 3.0f * u * t * t * bezierP[2] + t * t * t * bezierP[3];
}

// Devuelve la tangente de la curva en t: la dirección hacia donde va la curva en ese punto.
// Se usa para saber hacia dónde debe mirar el sillón.
glm::vec3 bezierTangent(float t) {
    float u = 1.0f - t;
    return 3.0f * u * u * (bezierP[1] - bezierP[0]) + 6.0f * u * t * (bezierP[2] - bezierP[1]) + 3.0f * t * t * (bezierP[3] - bezierP[2]);
}

// Prototipos: avisan al compilador que estas funciones existen (están definidas más abajo).
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);


// MALLA
// Una "malla" es una parte del modelo. Cada una tiene sus propios datos en la GPU:
//   VAO: "receta" que dice cómo leer los datos de los vértices.
//   VBO: buffer con los datos de los vértices (posición, normal, UV).
//   EBO: buffer con los índices, que dicen qué vértices forman cada triángulo.
struct Mesh {
    GLuint VAO, VBO, EBO;
    size_t indexCount;    // Cantidad de índices, necesaria para dibujar
};

// SHADERS
// Los shaders son programas que se ejecutan en la tarjeta gráfica.
//   - Vertex shader: calcula la posición de cada vértice en la pantalla.
//   - Fragment shader: calcula el color de cada píxel.

// Vertex shader del sillón
const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"           // Entrada: posición del vértice
"layout (location = 1) in vec3 aNormal;\n"        // Entrada: normal (hacia dónde mira la superficie)
"layout (location = 2) in vec2 aTexCoord;\n"      // Entrada: coordenadas de textura (UV)
"out vec3 FragPos;\n"                             // Salida: posición en el mundo, para el fragment
"out vec3 Normal;\n"                              // Salida: normal ya transformada
"out vec2 TexCoord;\n"                            // Salida: UV, para el fragment
"uniform mat4 uMVP;\n"                            // Matriz final: modelo * vista * proyección
"uniform mat4 uModel;\n"                          // Matriz de modelo: ubica el sillón en el mundo
"void main()\n"
"{\n"
"   FragPos = vec3(uModel * vec4(aPos, 1.0));\n"                      // Posición en el mundo
"   Normal = mat3(transpose(inverse(uModel))) * aNormal;\n"            // Normal ajustada al giro del modelo
"   TexCoord = aTexCoord;\n"                                           // Pasa las UV al fragment
"   gl_Position = uMVP * vec4(aPos, 1.0);\n"                           // Posición final en la pantalla
"}\0";

// Fragment shader del sillón (aquí está la iluminación Phong)
const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"                           // Salida: color final del píxel
"in vec3 FragPos;\n"                              // Entrada: posición del píxel en el mundo
"in vec3 Normal;\n"                               // Entrada: normal del píxel
"in vec2 TexCoord;\n"                             // Entrada: UV del píxel
"uniform sampler2D ourTexture;\n"                 // La textura del sillón
"uniform mat4 uView;\n"                           // Matriz de vista, para saber dónde está la cámara
"void main()\n"
"{\n"
"   vec3 lightPos = vec3(2.0, 3.0, 2.0);\n"       // Posición de la luz (fija)
"   vec3 lightColor = vec3(1.0, 1.0, 1.0);\n"     // Color de la luz (blanco)
"   vec3 viewPos = vec3(inverse(uView)[3]);\n"    // Posición de la cámara en el mundo
"   vec3 objColor = texture(ourTexture, TexCoord).rgb;\n"   // Color de la textura en este píxel
"   vec3 ambient = 0.2 * lightColor;\n"           // Luz base: evita que la parte en sombra sea negra
"   vec3 norm = normalize(Normal);\n"             // Normal normalizada (largo 1)
"   vec3 lightDir = normalize(lightPos - FragPos);\n"        // Dirección hacia la luz
"   float diff = max(dot(norm, lightDir), 0.0);\n"            // Qué tanto mira la superficie hacia la luz
"   vec3 diffuse = diff * lightColor;\n"          // Luz difusa: brillo mate según el ángulo
"   vec3 viewDir = normalize(viewPos - FragPos);\n"          // Dirección hacia la cámara
"   vec3 reflectDir = reflect(-lightDir, norm);\n"           // Dirección en que rebota la luz
"   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n"  // Brillo especular (reflejo)
"   vec3 specular = 0.5 * spec * lightColor;\n"   // Luz especular: el brillo que cambia con la cámara
"   FragColor = vec4((ambient + diffuse + specular) * objColor, 1.0);\n"  // Suma de luces por el color de la textura
"}\0";

// PROCESAR UNA MALLA DE ASSIMP
// Toma una malla del modelo, extrae sus vértices e índices,
// y los sube a la GPU (VAO, VBO y EBO).
Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<float> vertices;     // Aquí se guardan todos los datos de los vértices
    std::vector<unsigned int> indices; // Aquí se guardan qué vértices forman cada triángulo

    // Recorre cada vértice de la malla
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // Posición (x, y, z)
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        // Normal (x, y, z): si el modelo no tiene normales, usa (0, 0, 1) por defecto
        if (mesh->mNormals) {
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        }
        else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
        }

        // Coordenadas de textura (u, v): si no tiene, usa (0, 0)
        if (mesh->mTextureCoords[0]) {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }
        else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    // Recorre cada cara (triángulo) y guarda sus índices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    Mesh myMesh;
    myMesh.indexCount = indices.size();   // Guarda cuántos índices hay para dibujar después

    // Crea los objetos de OpenGL
    glGenVertexArrays(1, &myMesh.VAO);
    glGenBuffers(1, &myMesh.VBO);
    glGenBuffers(1, &myMesh.EBO);

    // Activa el VAO: todo lo que se configure a continuación queda guardado en él
    glBindVertexArray(myMesh.VAO);

    // Sube los vértices a la GPU
    glBindBuffer(GL_ARRAY_BUFFER, myMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Sube los índices a la GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Le dice a OpenGL cómo leer los datos. Cada vértice ocupa 8 floats: 3 posición + 3 normal + 2 UV.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);                   // Posición
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // UV
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);   // Desactiva el VAO para no modificarlo por error
    return myMesh;
}

// RECORRER EL ÁRBOL DEL MODELO
// Un modelo 3D es un árbol de "nodos". Cada nodo puede tener mallas
// y nodos hijos. Esta función recorre el árbol y procesa todas las mallas.
void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& modelMeshes) {
    // Procesa las mallas de este nodo
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        modelMeshes.push_back(processMesh(mesh, scene));
    }
    // Procesa los nodos hijos (con recursión, la función se llama a sí misma)
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, modelMeshes);
    }
}


// FUNCIÓN PRINCIPAL
int main()
{
    // 1. Crear la ventana
    glfwInit();                                                   // Inicia GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                // Pide OpenGL versión 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                // Pide versión 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Perfil moderno (core)

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sillon 3D - Control Libre y Giro", NULL, NULL);
    if (window == NULL)   // Si no se pudo crear la ventana, cierra el programa
    {
        std::cout << "Fallo al crear la ventana de GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);   // Activa esta ventana para dibujar en ella

    // Carga las funciones de OpenGL con GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Error al inicializar GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);                     // Zona de la ventana donde se dibuja
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Oculta el cursor para mover la cámara con el mouse
    glfwSetCursorPosCallback(window, mouse_callback);            // Función que se llama al mover el mouse
    glEnable(GL_DEPTH_TEST);                                     // Activa la profundidad (lo que está adelante tapa lo de atrás)

    // 2. Compilar y enlazar los shaders del sillón
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);      // Crea el vertex shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);  // Le pasa el código
    glCompileShader(vertexShader);                               // Lo compila

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);  // Crea el fragment shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Se une el vertex y el fragment shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Ya no hacen falta los shaders sueltos, el programa ya los tiene
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 3. Shader sencillo para dibujar la curva (color fijo, sin luz)
    const char* curveVS =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"       // Solo recibe la posición
        "uniform mat4 uMVP;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = uMVP * vec4(aPos, 1.0);\n"
        "}\0";

    const char* curveFS =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0, 0.8, 0.2, 1.0);\n"  // Color amarillo para la curva
        "}\0";

    GLuint cVS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cVS, 1, &curveVS, NULL);
    glCompileShader(cVS);
    GLuint cFS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cFS, 1, &curveFS, NULL);
    glCompileShader(cFS);
    GLuint curveProgram = glCreateProgram();
    glAttachShader(curveProgram, cVS);
    glAttachShader(curveProgram, cFS);
    glLinkProgram(curveProgram);
    glDeleteShader(cVS);
    glDeleteShader(cFS);

    GLint curveMvpLoc = glGetUniformLocation(curveProgram, "uMVP");   // Ubicación de uMVP en el shader de la curva

    // 4. Generar los puntos de la curva
    const int CURVE_SEGMENTS = 100;   // Cuántos segmentos tiene la línea (más = más suave)
    std::vector<float> curvePts;
    for (int i = 0; i <= CURVE_SEGMENTS; i++) {
        glm::vec3 p = bezierPoint(i / (float)CURVE_SEGMENTS);   // Punto de la curva en t = i / 100
        curvePts.push_back(p.x);
        curvePts.push_back(p.y);
        curvePts.push_back(p.z);
    }

    // Sube los puntos de la curva a la GPU
    GLuint curveVAO, curveVBO;
    glGenVertexArrays(1, &curveVAO);
    glGenBuffers(1, &curveVBO);
    glBindVertexArray(curveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
    glBufferData(GL_ARRAY_BUFFER, curvePts.size() * sizeof(float), curvePts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // 5. Configurar la textura
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);         // Repite la textura en horizontal
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);         // Repite la textura en vertical
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Filtro al alejar
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);     // Filtro al acercar

    // Assimp ya voltea las UVs (aiProcess_FlipUVs), así que stb no debe voltear la imagen
    stbi_set_flip_vertically_on_load(false);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("textures/sillon.jpeg", &width, &height, &nrChannels, 0);   // Lee la imagen
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;   // Con transparencia o sin ella
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); // Sube la imagen a la GPU
        glGenerateMipmap(GL_TEXTURE_2D);   // Genera versiones pequeñas de la textura
        stbi_image_free(data);             // Libera la imagen de la memoria del CPU
        std::cout << "Textura cargada con exito." << std::endl;
    }
    else   // Si no encontró la imagen, usa un color marrón de respaldo
    {
        std::cout << "Advertencia: No se encontro textures/sillon.jpeg, usando respaldo." << std::endl;
        unsigned char fallback[] = { 180, 100, 50 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, fallback);
    }

    // 6. Cargar el modelo 3D 
    std::vector<Mesh> modelMeshes;   // Aquí se guardan las mallas del modelo
    Assimp::Importer importer;       // Objeto que lee el archivo
    // Lee scene.gltf y aplica: triangular, voltear UVs, calcular normales y corregir caras
    const aiScene* scene = importer.ReadFile("scene.gltf", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_FixInfacingNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "Error Assimp: " << importer.GetErrorString() << std::endl;
    }
    else {
        processNode(scene->mRootNode, scene, modelMeshes);   // Convierte el modelo en mallas de OpenGL
    }

    // Ubicaciones de las variables uniform en el shader del sillón
    GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
    GLint modelLoc = glGetUniformLocation(shaderProgram, "uModel");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "uView");

    // 7. BUCLE PRINCIPAL: se repite una vez por cada frame 
    while (!glfwWindowShouldClose(window))   // Sigue mientras no se cierre la ventana
    {
        // Calcula cuánto tiempo pasó desde el último frame
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);   // Lee el teclado y mueve la cámara

        // Limpia la pantalla: color de fondo y profundidad
        glClearColor(0.12f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matriz de proyección: da perspectiva (lo lejano se ve más chico)
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        // Matriz de vista: ubica la cámara en el mundo
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Posición del sillón sobre la curva. t va de 0 a 1 y se repite cada 10 segundos.
        float t = std::fmod(currentFrame * 0.1f, 1.0f);
        glm::vec3 pos = bezierPoint(t);          // Punto donde está el sillón
        glm::vec3 tangent = bezierTangent(t);    // Hacia dónde va la curva en ese punto
        float heading = std::atan2(tangent.x, tangent.z);   // Ángulo para orientar el sillón

        // Matriz de modelo: ubica y orienta el sillón.
        // El orden importa: se aplica de adentro hacia afuera.
        glm::mat4 model = glm::mat4(1.0f);                                                // Matriz identidad (sin cambios)
        model = glm::translate(model, pos);                                               // 1. Lo mueve a su punto en la curva
        model = glm::rotate(model, heading, glm::vec3(0.0f, 1.0f, 0.0f));                 // 2. Lo orienta hacia donde avanza
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));    // 3. Corrige la posición inicial del modelo
        model = glm::rotate(model, currentFrame * 0.4f, glm::vec3(0.0f, 0.0f, 1.0f));     // 4. Lo hace girar sobre sí mismo

        glm::mat4 mvp = projection * view * model;   // Combina las tres matrices en una sola

        // Dibuja el sillón
        glUseProgram(shaderProgram);                               // Usa el shader del sillón
        glBindTexture(GL_TEXTURE_2D, texture);                     // Activa la textura
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));     // Envía la matriz final al shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Envía la matriz de modelo
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));   // Envía la matriz de vista (para la luz)

        for (const Mesh& mesh : modelMeshes)   // Dibuja cada parte del modelo
        {
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indexCount), GL_UNSIGNED_INT, nullptr);
        }

        // Dibuja la curva 
        glUseProgram(curveProgram);                                                    // Usa el shader de la curva
        glUniformMatrix4fv(curveMvpLoc, 1, GL_FALSE, glm::value_ptr(projection * view)); // La curva no se mueve, no necesita model
        glBindVertexArray(curveVAO);
        glDrawArrays(GL_LINE_STRIP, 0, CURVE_SEGMENTS + 1);   // Dibuja la curva como una línea continua

        glBindVertexArray(0);          // Desactiva el VAO
        glfwSwapBuffers(window);       // Muestra en pantalla lo que se dibujó (doble buffer)
        glfwPollEvents();              // Revisa eventos como teclado y mouse
    }

    //  8. Liberar memoria al cerrar 
    for (const Mesh& mesh : modelMeshes)
    {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }

    glDeleteVertexArrays(1, &curveVAO);
    glDeleteBuffers(1, &curveVBO);
    glDeleteProgram(curveProgram);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// PROCESAR TECLADO
// Se llama en cada frame. Mueve la cámara según las teclas presionadas.
void processInput(GLFWwindow* window)
{
    // ESC cierra el programa
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Velocidad de la cámara: 2.5 unidades por segundo, sin importar la velocidad de la computadora
    float cameraSpeed = 2.5f * deltaTime;

    // W: avanza hacia donde mira la cámara
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    // S: retrocede
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    // A: se mueve a la izquierda (producto cruz da el vector lateral)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    // D: se mueve a la derecha
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// MOVIMIENTO DEL MOUSE
// Se llama cada vez que el mouse se mueve. Calcula cuánto giró
// y actualiza la dirección hacia donde mira la cámara.
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);   // Posición actual X del mouse
    float ypos = static_cast<float>(yposIn);   // Posición actual Y del mouse

    // La primera vez, guarda la posición para evitar un salto brusco
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Cuánto se movió el mouse desde la última vez
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;   // Invertido: en pantalla, Y crece hacia abajo
    lastX = xpos;
    lastY = ypos;

    // Sensibilidad: reduce el movimiento para que no sea tan rápido
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;     // Gira a izquierda/derecha
    pitch += yoffset;   // Mira arriba/abajo

    // Limita el ángulo vertical para no dar la vuelta completa
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Convierte los ángulos yaw y pitch en un vector de dirección
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);   // Normaliza para que tenga largo 1
}